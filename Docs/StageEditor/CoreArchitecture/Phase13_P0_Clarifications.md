# Phase 13 - P0 问题澄清文档

> **创建日期**: 2025-12-04
> **相关文档**: [Phase13_StageRegistry_Discussion.md](Phase13_StageRegistry_Discussion.md)
> **评审报告**: [Phase13_ExpertReview_Report.md](Phase13_ExpertReview_Report.md)

本文档针对专家评审报告中提出的 P0 级问题进行详细澄清和补充。

---

## P0-1: FLevelInstanceID 稳定性验证

### 问题描述

文档中提到 `FLevelInstanceID` 是"运行时计算的唯一 Hash"，但没有验证其在多次加载之间的稳定性。如果每次加载生成不同的 Hash，`RuntimeStageMap` 的 Key 就不稳定。

### 调查结果

#### UE5 GUID 机制

根据 UE 官方文档和源码分析：

1. **Actor GUID 持久化**：
   - 每个 Actor 在序列化时都有一个持久化的 GUID
   - GUID 存储在 Level 文件中（`.umap` 或 `.uasset`）
   - GUID 在 Actor 的整个生命周期中保持不变

2. **LevelInstance GUID 问题**：
   - UE Bug Tracker 中存在相关 Issue（[UE-242229](https://issues.unrealengine.com/issue/UE-242229)）
   - 问题：`FLevelInstanceActorGuid::GetGuid()` 断言失败
   - 根因：序列化代码只运行于 PersistentLevel，嵌套的 LevelInstance 可能出现问题

3. **FLevelInstanceID 的计算**：
   - 基于 LevelInstance Actor 的 GUID 和所有祖先的 GUID 计算
   - Hash 计算是确定性的（相同的 GUID 组合产生相同的 Hash）
   - **结论：只要 LevelInstance Actor 不被删除重建，Hash 在多次加载之间是稳定的**

#### 稳定性验证

**场景 1: 正常加载/卸载**
```cpp
时刻 1: Level 加载 LevelInstance Actor (GUID = 12345678-1234-1234-1234-123456789ABC)
        → FLevelInstanceID.Hash = HashCombine(GUID, ...)
        → Hash = 0x8A9B7C6D5E4F3210

时刻 2: WP Streaming 卸载 LevelInstance

时刻 3: WP Streaming 重新加载 LevelInstance (相同的 Actor)
        → FLevelInstanceID.Hash = HashCombine(相同的 GUID, ...)
        → Hash = 0x8A9B7C6D5E4F3210 ✅ 相同
```

**场景 2: LevelInstance Actor 被删除重建**
```cpp
时刻 1: Level 有 LevelInstance Actor (GUID = 旧GUID)
        → Hash = 0x8A9B7C6D5E4F3210

时刻 2: 用户删除 LevelInstance Actor

时刻 3: 用户重新创建 LevelInstance Actor (GUID = 新GUID)
        → Hash = 0x1234567890ABCDEF ❌ 不同

问题：RuntimeStageMap 中旧 Hash 对应的 Stage 记录失效
```

### 解决方案

#### 方案 A: 警告用户不要删除重建 LevelInstance（推荐）

**理由**：
- LevelInstance Actor 删除重建是罕见操作
- 删除 LevelInstance 通常意味着整个 SubLevel 的重建
- 这种情况下，StageID 重新分配是合理的

**实现**：
```cpp
// 在文档中明确说明
## FLevelInstanceID 稳定性保证

**稳定条件**：
- ✅ LevelInstance Actor 不被删除
- ✅ 正常的 World Partition Streaming 加载/卸载
- ✅ 编辑器重启

**不稳定条件**：
- ❌ LevelInstance Actor 被删除并重新创建
- ❌ SubLevel 被重命名或移动后，LevelInstance Actor 被重新引用

**用户指导**：
如果需要重建 LevelInstance，请使用"迁移 Stage"功能重新注册所有 Stage。
```

#### 方案 B: 使用 SubLevel 路径作为额外 Key（备选）

**理由**：
- 提供额外的容错机制
- 即使 Hash 变化，仍可通过 SubLevel 路径查找

**实现**：
```cpp
// StageRegistryTypes.h

/**
 * 运行时全局 Stage ID（增强版）
 */
USTRUCT(BlueprintType)
struct FStageRuntimeID
{
    GENERATED_BODY()

    /** LevelInstance 的运行时唯一 ID（主 Level 为空/默认） */
    UPROPERTY(BlueprintReadWrite)
    FLevelInstanceID LevelInstanceID;

    /** SubLevel 路径（辅助标识，用于容错） */
    UPROPERTY(BlueprintReadWrite)
    FSoftObjectPath SubLevelPath;

    /** Level 内的 StageID（来自 RegistryAsset） */
    UPROPERTY(BlueprintReadWrite)
    int32 StageID = 0;

    bool operator==(const FStageRuntimeID& Other) const
    {
        // 优先使用 LevelInstanceID 匹配
        if (LevelInstanceID.Hash != 0 && Other.LevelInstanceID.Hash != 0)
        {
            return LevelInstanceID == Other.LevelInstanceID && StageID == Other.StageID;
        }

        // 如果 Hash 不可用，使用 SubLevelPath 匹配
        return SubLevelPath == Other.SubLevelPath && StageID == Other.StageID;
    }

    friend uint32 GetTypeHash(const FStageRuntimeID& ID)
    {
        if (ID.LevelInstanceID.Hash != 0)
        {
            return HashCombine(GetTypeHash(ID.LevelInstanceID), GetTypeHash(ID.StageID));
        }
        else
        {
            return HashCombine(GetTypeHash(ID.SubLevelPath), GetTypeHash(ID.StageID));
        }
    }
};
```

**权衡**：
- 增加复杂度
- 增加内存占用（每个 RuntimeID 多存一个 FSoftObjectPath）
- 容错性更好

### 推荐方案

**采用方案 A + 文档警告**

**理由**：
1. 简单直接，符合 UE 设计模式
2. LevelInstance 删除重建是罕见场景
3. 通过文档和 UI 提示引导用户正确操作
4. 如果未来发现问题频繁，可升级到方案 B

### 判空逻辑澄清

```cpp
// StageManagerSubsystem.h

/**
 * 检查 LevelInstanceID 是否为主 Level（即非 LevelInstance）
 *
 * @param ID LevelInstanceID 结构体
 * @return true 如果是主 Level，false 如果是 LevelInstance
 */
bool IsMainLevel(const FLevelInstanceID& ID) const
{
    // FLevelInstanceID 的默认构造函数会将 Hash 初始化为 0
    // Hash == 0 表示没有计算过 LevelInstanceID，即主 Level
    return ID.Hash == 0;
}

/**
 * 获取 Stage 的 LevelInstanceID
 *
 * @param Stage Stage Actor 指针
 * @return LevelInstanceID，如果 Stage 在主 Level 则 Hash 为 0
 */
FLevelInstanceID GetStageLevelInstanceID(AStage* Stage) const
{
    if (!Stage)
    {
        return FLevelInstanceID();  // Hash = 0
    }

    UWorld* World = Stage->GetWorld();
    if (!World)
    {
        return FLevelInstanceID();
    }

    // 检查 Stage 是否在 LevelInstance 中
    ULevelInstanceSubsystem* LISubsystem = World->GetSubsystem<ULevelInstanceSubsystem>();
    if (!LISubsystem)
    {
        return FLevelInstanceID();  // 没有 LevelInstance 支持，视为主 Level
    }

    // 获取 Stage 所在 LevelInstance 的 Actor
    AActor* LevelInstanceActor = LISubsystem->GetOwningLevelInstance(Stage->GetLevel());
    if (!LevelInstanceActor)
    {
        return FLevelInstanceID();  // 不在 LevelInstance 中，在主 Level
    }

    // 获取 LevelInstanceID
    return LISubsystem->GetLevelInstanceID(LevelInstanceActor);
}
```

### 补充文档建议

在 `Phase13_StageRegistry_Discussion.md` 的 6.2 节添加：

```markdown
#### FLevelInstanceID 稳定性保证

**✅ 稳定条件**：
- LevelInstance Actor 不被删除（正常的移动、重命名不影响 GUID）
- 正常的 World Partition Streaming 加载/卸载
- 编辑器重启和 Level 重新加载

**❌ 不稳定条件（需要用户迁移 Stage）**：
- LevelInstance Actor 被删除并重新创建（GUID 变化）
- SubLevel 文件本身被删除重建

**判空逻辑**：
```cpp
bool IsMainLevel(const FLevelInstanceID& ID)
{
    return ID.Hash == 0;  // 默认构造的 FLevelInstanceID
}
```

**用户指导**：
如果用户需要重建 LevelInstance：
1. 先使用"导出 Stage 数据"功能保存 StageID 映射
2. 删除并重建 LevelInstance Actor
3. 使用"迁移 Stage"功能重新注册 Stage（保留原 StageID）

**参考资料**：
- [UE5 LevelInstance Bug Tracker](https://issues.unrealengine.com/issue/UE-242229)
- [GUID - Unreal Wiki](https://wiki.beyondunreal.com/GUID)
```

### 测试计划

创建单元测试验证 FLevelInstanceID 稳定性：

```cpp
// Tests/StageEditorTests.cpp

TEST_CASE("FLevelInstanceID Stability Test", "[StageEditor][LevelInstance]")
{
    SECTION("Same LevelInstance Actor produces same Hash")
    {
        // 1. 创建 LevelInstance Actor
        AActor* LevelInstanceActor = CreateTestLevelInstance();
        FGuid OriginalGUID = LevelInstanceActor->GetGuid();

        // 2. 获取 LevelInstanceID
        ULevelInstanceSubsystem* Subsystem = GetLevelInstanceSubsystem();
        FLevelInstanceID ID1 = Subsystem->GetLevelInstanceID(LevelInstanceActor);

        // 3. 卸载并重新加载
        UnloadLevelInstance(LevelInstanceActor);
        AActor* ReloadedActor = ReloadLevelInstance(OriginalGUID);

        // 4. 重新获取 LevelInstanceID
        FLevelInstanceID ID2 = Subsystem->GetLevelInstanceID(ReloadedActor);

        // 5. 验证 Hash 一致
        REQUIRE(ID1.Hash == ID2.Hash);
    }

    SECTION("Deleted and recreated LevelInstance produces different Hash")
    {
        // 1. 创建 LevelInstance Actor
        AActor* LevelInstanceActor = CreateTestLevelInstance();
        FLevelInstanceID ID1 = GetLevelInstanceID(LevelInstanceActor);

        // 2. 删除 Actor
        LevelInstanceActor->Destroy();

        // 3. 重新创建（新的 GUID）
        AActor* NewLevelInstanceActor = CreateTestLevelInstance();
        FLevelInstanceID ID2 = GetLevelInstanceID(NewLevelInstanceActor);

        // 4. 验证 Hash 不同
        REQUIRE(ID1.Hash != ID2.Hash);
    }

    SECTION("Main Level has Hash == 0")
    {
        // 1. 在主 Level 创建 Stage Actor（非 LevelInstance）
        AStage* MainStage = CreateTestStage(MainLevel);

        // 2. 获取 LevelInstanceID
        FLevelInstanceID ID = GetStageLevelInstanceID(MainStage);

        // 3. 验证 Hash 为 0
        REQUIRE(ID.Hash == 0);
        REQUIRE(IsMainLevel(ID) == true);
    }
}
```

### 结论

✅ **FLevelInstanceID 在正常使用场景下是稳定的**

- 基于持久化的 Actor GUID 计算
- 正常加载/卸载不影响稳定性
- 仅在 LevelInstance Actor 删除重建时会变化（罕见场景）

✅ **判空逻辑明确**

- `Hash == 0` 表示主 Level（非 LevelInstance）
- 使用 UE 的默认构造函数机制

⚠️ **需要文档和 UI 警告**

- 告知用户不要删除重建 LevelInstance Actor
- 提供"迁移 Stage"工具应对特殊情况

---

## P0-2: 事务一致性处理（待补充）

## P0-3: 旧数据迁移方案（待补充）

---

## 参考资料

- [UE5 LevelInstance Bug Tracker - UE-242229](https://issues.unrealengine.com/issue/UE-242229)
- [GUID - Unreal Wiki](https://wiki.beyondunreal.com/GUID)
- [LevelInstance API Documentation](https://docs.unrealengine.com/5.0/en-US/API/Runtime/Engine/LevelInstance/)

---

*最后更新: 2025-12-04*
