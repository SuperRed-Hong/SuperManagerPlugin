# StageRegistry 持久化架构重设计

> 创建日期: 2025-12-01
> 状态: 讨论中
> 参与者: User, Claude

---

## 1. 问题背景

### 1.1 发现的核心问题

当前 `StageManagerSubsystem` 的注册表设计存在根本性缺陷：

```
当前设计：
StageRegistry = TMap<StageID, TWeakObjectPtr<AStage>>  ← 内存中的运行时数据
NextStageID = int32  ← 发号器，不持久化

问题链：
1. Subsystem 初始化时 NextStageID = 1
2. 通过 ScanWorldForExistingStages() 遍历已加载 Stage 来恢复 NextStageID
3. WP Streaming 卸载 Stage → WeakPtr 失效 → 注册表"丢失"这个 Stage
4. 如果此时新建 Stage → 可能分配到已存在（但未加载）的 StageID
5. 违反了 "StageID 全局唯一且稳定" 的设计目标
```

### 1.2 根因分析

**我们把"Stage 的存在性"和"Stage Actor 的加载状态"混为一谈了。**

- Stage 的存在性：Stage 属于某个 Level，无论是否被 WP Streaming 加载
- Stage Actor 的加载状态：Actor 是否在内存中

正确的概念模型：

```
┌─────────────────────────────────────────────────────────────┐
│                      Level (持久化)                          │
│  ┌─────────────────────────────────────────────────────┐    │
│  │ Stage 定义 (应该持久化)                              │    │
│  │  - StageID = 1, DataLayer = DL_Stage_Forest         │    │
│  │  - StageID = 2, DataLayer = DL_Stage_Castle         │    │
│  └─────────────────────────────────────────────────────┘    │
│                          ↓                                   │
│  ┌─────────────────────────────────────────────────────┐    │
│  │ Stage Actor 实例 (运行时，可能被 WP 卸载)            │    │
│  │  - AStage_Forest (Loaded)    ← 在内存中              │    │
│  │  - AStage_Castle (Unloaded)  ← 不在内存，但仍属于Level│    │
│  └─────────────────────────────────────────────────────┘    │
└─────────────────────────────────────────────────────────────┘
```

---

## 2. 设计目标

1. **StageID 持久化**：NextStageID 和 Stage 列表必须持久化存储
2. **支持 LevelInstance**：同一个 SubLevel 被多个 LevelInstance 加载时，能正确区分
3. **数据与逻辑分离**：DataAsset 存储数据，Subsystem 处理运行时逻辑
4. **向后兼容**：现有项目能平滑迁移

---

## 3. 架构设计

### 3.1 双层架构

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                              持久化层 (DataAsset)                            │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                             │
│  UStageRegistryAsset (每个 Level 一个)                                       │
│  ├── OwningLevel: TSoftObjectPtr<UWorld>   ← 软引用，跟随移动/重命名         │
│  ├── NextStageID: int32                    ← 持久化发号器                    │
│  └── StageEntries: TArray<FStageRegistryEntry>                              │
│       ├── StageID                                                           │
│       ├── StageActorPath (软引用)          ← 即使 Actor 未加载也能查询        │
│       ├── StageDataLayerPath                                                │
│       ├── DisplayName                                                       │
│       └── ActCount                                                          │
│                                                                             │
├─────────────────────────────────────────────────────────────────────────────┤
│                              运行时层 (Subsystem)                            │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                             │
│  UStageManagerSubsystem                                                     │
│  │                                                                          │
│  ├── 【持久化数据访问】                                                      │
│  │    LoadedRegistries: TMap<LevelPath, UStageRegistryAsset*>               │
│  │                                                                          │
│  ├── 【运行时缓存】                                                          │
│  │    RuntimeStageMap: TMap<FStageRuntimeID, TWeakObjectPtr<AStage>>        │
│  │    └── FStageRuntimeID = (FLevelInstanceID, StageID)                     │
│  │                                                                          │
│  └── 【职责】                                                                │
│       - 加载/管理 RegistryAsset                                             │
│       - 运行时 Stage 查找                                                   │
│       - Cross-Stage 通信                                                    │
│       - LevelInstance 适配                                                  │
│                                                                             │
└─────────────────────────────────────────────────────────────────────────────┘
```

### 3.2 数据结构定义

```cpp
/**
 * 单个 Stage 的注册信息（持久化）
 */
USTRUCT(BlueprintType)
struct FStageRegistryEntry
{
    GENERATED_BODY()

    /** Level 内唯一的 StageID */
    UPROPERTY(VisibleAnywhere)
    int32 StageID = 0;

    /** Stage Actor 的软引用（用于离线查询） */
    UPROPERTY(VisibleAnywhere)
    TSoftObjectPtr<AStage> StageActor;

    /** 关联的 Stage DataLayer（便于 DataLayer 同步系统查询） */
    UPROPERTY(VisibleAnywhere)
    TSoftObjectPtr<UDataLayerAsset> StageDataLayer;

    /** Stage 显示名称（冗余存储，便于离线查看） */
    UPROPERTY(VisibleAnywhere)
    FString DisplayName;

    /** Acts 数量（冗余存储，便于统计） */
    UPROPERTY(VisibleAnywhere)
    int32 ActCount = 0;
};

/**
 * 每个 Level 对应的 Stage 注册表（持久化 DataAsset）
 */
UCLASS(BlueprintType)
class UStageRegistryAsset : public UDataAsset
{
    GENERATED_BODY()

public:
    /** 关联的 Level - 软引用，跟随移动/重命名 */
    UPROPERTY(VisibleAnywhere, Category = "Registry")
    TSoftObjectPtr<UWorld> OwningLevel;

    /** 下一个可分配的 StageID（发号器） */
    UPROPERTY(VisibleAnywhere, Category = "Registry")
    int32 NextStageID = 1;

    /** 所有已注册的 Stage 列表 */
    UPROPERTY(VisibleAnywhere, Category = "Registry")
    TArray<FStageRegistryEntry> StageEntries;

    // === API ===

    /** 分配新 StageID 并添加条目 */
    int32 AllocateAndRegister(AStage* Stage);

    /** 根据 StageID 查找条目 */
    FStageRegistryEntry* FindEntry(int32 StageID);

    /** 移除条目（Stage 被删除时） */
    bool RemoveEntry(int32 StageID);

    /** 更新条目（Stage 数据变化时） */
    void UpdateEntry(AStage* Stage);
};
```

### 3.3 LevelInstance 支持

UE5 原生提供了 `FLevelInstanceID`：

```cpp
// Engine/Source/Runtime/Engine/Public/LevelInstance/LevelInstanceTypes.h
struct FLevelInstanceID
{
    uint64 Hash = 0;                    // 运行时计算的唯一 Hash
    FActorContainerID ContainerID;      // 容器 ID
    FName ActorName;                    // Actor 名称
    FString PackageShortName;           // 包短名

    // 关键：这是运行时唯一 ID，基于 LevelInstance Actor Guid 和所有祖先的 Guid 计算
    // 能区分同一个 SubLevel 的多个实例
};
```

**运行时全局 Stage ID：**

```cpp
USTRUCT()
struct FStageRuntimeID
{
    /** LevelInstance 的运行时唯一 ID（主 Level 为空/默认） */
    FLevelInstanceID LevelInstanceID;

    /** Level 内的 StageID（来自 RegistryAsset） */
    int32 StageID = 0;
};
```

---

## 4. CRUD 操作设计

### 4.1 Create (注册 Stage)

```cpp
// 新流程
RegisterStage(Stage)
  → GetOrLoadRegistryAsset(Stage->GetLevel())  // 加载对应 Level 的 RegistryAsset
  → RegistryAsset->AllocateAndRegister(Stage)  // 从 DataAsset 分配 ID
  → RegistryAsset->MarkPackageDirty()          // 标记需要保存
  → RuntimeStageMap.Add(RuntimeID, Stage)      // 添加到运行时缓存
```

### 4.2 Read (查询 Stage)

```cpp
// 运行时查询（需要 Actor 在内存中）
GetStage(StageID)
  → RuntimeStageMap.Find(RuntimeID)

// 离线查询（不需要 Actor 在内存中）
GetStageInfo(StageID)
  → RegistryAsset->FindEntry(StageID)
```

### 4.3 Update (更新 Stage 信息)

```cpp
UpdateStageEntry(Stage)
  → RegistryAsset->UpdateEntry(Stage)  // 同步 DisplayName, ActCount 等
  → RegistryAsset->MarkPackageDirty()
```

### 4.4 Delete (删除 Stage)

```cpp
UnregisterStage(Stage)
  → RegistryAsset->RemoveEntry(Stage->GetStageID())
  → RegistryAsset->MarkPackageDirty()
  → RuntimeStageMap.Remove(RuntimeID)
```

---

## 5. RegistryAsset 管理

### 5.1 存储位置

**默认位置：**
```
/Game/StageEditor/Registries/<LevelName>_StageRegistry.uasset
```

**用户自定义：**
在 `FAssetCreationSettings` 中添加：
```cpp
/** If true, use custom path for Registry Assets. Otherwise use default plugin path. */
UPROPERTY(EditAnywhere, Category = "Asset Creation")
bool bIsCustomRegistryAssetPath = false;

/** Custom folder path for Registry Asset creation */
UPROPERTY(EditAnywhere, Category = "Asset Creation",
    meta = (EditCondition = "bIsCustomRegistryAssetPath", ContentDir, RelativeToGameContentDir))
FDirectoryPath RegistryAssetFolderPath;
```

### 5.2 Level ↔ Registry 关联

**方案：TSoftObjectPtr 软引用**

```cpp
UPROPERTY(VisibleAnywhere, Category = "Registry")
TSoftObjectPtr<UWorld> OwningLevel;
```

优点：
- 内部存储资产 GUID，不是路径
- Level 移动/重命名时，引用自动更新（UE 资产重定向机制）

**查找策略：**
1. 按命名约定在默认目录查找（O(1)）
2. 找不到则扫描目录，检查 `OwningLevel` 匹配（兜底）

### 5.3 创建时机 - UI 驱动

```
用户打开 Level
    ↓
StageEditorPanel 初始化
    ↓
检查当前 Level 是否有对应的 RegistryAsset
    ↓
┌─────────────────┬──────────────────────────────┐
│     有          │            无                 │
├─────────────────┼──────────────────────────────┤
│ 加载并使用      │ 顶部显示警告条：              │
│                 │ "当前 Level 未初始化 Stage    │
│                 │  Registry，请先创建"          │
│                 │ [Create Registry] 按钮        │
│                 │                              │
│                 │ 其他 UI 操作禁用              │
└─────────────────┴──────────────────────────────┘
```

### 5.4 保存时机 - UI 操作驱动

| UI 操作 | Registry 变化 | 保存行为 |
|---------|--------------|----------|
| 创建 Stage | 添加 Entry, NextStageID++ | MarkDirty，等用户保存 |
| 删除 Stage | 移除 Entry | MarkDirty |
| 重命名 Stage | 更新 DisplayName | MarkDirty |
| 添加/删除 Act | 更新 ActCount | MarkDirty |
| 用户保存 Level | - | Registry 一起保存 |

### 5.5 旧数据迁移

```
场景：用户有一个旧 Level，里面已有 Stage Actor，但没有 RegistryAsset

用户打开 Level → StageEditorPanel
    ↓
检测到 Level 无 RegistryAsset
    ↓
显示警告条 + [Create Registry] 按钮
    ↓
用户点击 Create
    ↓
创建 RegistryAsset，同时：
  1. 扫描 Level 中已有的 Stage Actor
  2. 将它们的信息填入 StageEntries
  3. 根据最大 StageID 设置 NextStageID = MaxID + 1
    ↓
完成迁移，UI 恢复正常
```

---

## 6. 已确定设计

### 6.1 RegistryAsset 查找策略 ✅

#### 查找流程

```cpp
UStageRegistryAsset* GetOrLoadRegistryAsset(UWorld* Level)
{
    // 1. 检查内存缓存 (O(1))
    if (Cached) return Cached;

    // 2. 按命名约定在默认目录查找 (快速路径)
    //    路径: /Game/StageEditor/Registries/<LevelName>_StageRegistry
    if (Found) return Registry;

    // 3. 扫描整个项目中所有 UStageRegistryAsset (兜底)
    //    - 使用 Asset Registry 递归查找所有类型资产
    //    - 逐个加载并检查 OwningLevel 字段匹配
    return ScanAllRegistriesForLevel(Level);
}
```

#### 性能评估

| 场景 | 耗时 | 触发频率 |
|------|------|----------|
| 缓存命中 | <1ms | 99% (WP Streaming 反复加载) |
| 默认路径命中 | <10ms | 90% (首次打开 Level) |
| 全扫描（50 个资产） | ~250ms | <5% (Level 重命名/移动后首次打开) |

**设计决策：**
- ✅ 项目规模假设：<50 个 RegistryAsset
- ✅ 全扫描支持用户自定义目录存放（通过 `bRecursivePaths = true` 扫描整个项目）
- ✅ 内存缓存避免重复查找
- ⚠️ 后续优化：如果项目规模超出预期，可添加 Asset Registry Tag 优化

#### 实现要点

```cpp
// StageManagerSubsystem.cpp

UStageRegistryAsset* UStageManagerSubsystem::ScanAllRegistriesForLevel(UWorld* Level)
{
    FSoftObjectPath TargetLevelPath(Level);

    IAssetRegistry& AssetRegistry =
        FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry").Get();

    FARFilter Filter;
    Filter.ClassPaths.Add(UStageRegistryAsset::StaticClass()->GetClassPathName());
    Filter.bRecursivePaths = true;  // ← 递归扫描整个项目，不限定目录

    TArray<FAssetData> AssetDataList;
    AssetRegistry.GetAssets(Filter, AssetDataList);

    // 遍历所有 RegistryAsset，检查 OwningLevel 匹配
    for (const FAssetData& AssetData : AssetDataList)
    {
        UStageRegistryAsset* Registry = Cast<UStageRegistryAsset>(AssetData.GetAsset());
        if (Registry && Registry->OwningLevel.ToSoftObjectPath() == TargetLevelPath)
        {
            return Registry;
        }
    }

    return nullptr;
}
```

---

### 6.2 LevelInstance 支持与 Cross-Stage 通信 ✅

#### 问题背景

引入 LevelInstance 后，同一个 StageID 可能在多个 LevelInstance 中重复：

```
PersistentLevel (主关卡)
├── Stage_MainCity (StageID = 1)
└── LevelInstance_Forest (加载 SubLevel_Forest)
    └── SubLevel_Forest
        └── Stage_ForestArea (StageID = 1)  ← StageID 冲突
```

调用 `ActivateStage(1)` 会产生歧义。

#### 解决方案：LevelInstanceID + StageID

**核心思路**：使用 `(FLevelInstanceID, StageID)` 组合唯一标识跨 LevelInstance 的 Stage。

**数据结构**：
```cpp
// StageRegistryTypes.h

/**
 * 运行时全局 Stage ID
 */
USTRUCT(BlueprintType)
struct FStageRuntimeID
{
    GENERATED_BODY()

    /** LevelInstance 的运行时唯一 ID（主 Level 为空/默认） */
    UPROPERTY(BlueprintReadWrite)
    FLevelInstanceID LevelInstanceID;

    /** Level 内的 StageID（来自 RegistryAsset） */
    UPROPERTY(BlueprintReadWrite)
    int32 StageID = 0;

    bool operator==(const FStageRuntimeID& Other) const
    {
        return LevelInstanceID == Other.LevelInstanceID && StageID == Other.StageID;
    }

    friend uint32 GetTypeHash(const FStageRuntimeID& ID)
    {
        return HashCombine(GetTypeHash(ID.LevelInstanceID), GetTypeHash(ID.StageID));
    }
};
```

**运行时缓存**：
```cpp
// StageManagerSubsystem.h
TMap<FStageRuntimeID, TWeakObjectPtr<AStage>> RuntimeStageMap;
```

#### 蓝图 API 设计

提供三层 API，从简单到复杂：

```cpp
// StageManagerSubsystem.h

/**
 * 【简化版】激活当前主 Level 的 Stage
 * 适用场景：90% 的简单项目（无 LevelInstance）
 */
UFUNCTION(BlueprintCallable, Category = "Stage",
    meta = (DisplayName = "Activate Stage"))
void ActivateStage(int32 StageID);

/**
 * 【辅助函数】从 LevelInstance Actor 获取其运行时 ID
 * 适用场景：用户需要指定 LevelInstance 中的 Stage
 */
UFUNCTION(BlueprintPure, Category = "Stage|LevelInstance",
    meta = (DisplayName = "Get LevelInstance ID"))
static FLevelInstanceID GetLevelInstanceID(AActor* LevelInstanceActor);

/**
 * 【完整版】激活指定 LevelInstance 中的 Stage
 * 适用场景：复杂项目（有多个 LevelInstance）
 */
UFUNCTION(BlueprintCallable, Category = "Stage|LevelInstance",
    meta = (DisplayName = "Activate Stage In LevelInstance"))
void ActivateStageInLevelInstance(
    const FLevelInstanceID& LevelInstanceID,
    int32 StageID);
```

#### 蓝图使用示例

**场景 1：激活主 Level 的 Stage（简单）**

```
蓝图节点流程：

[Event Begin Play]
       ↓
[Activate Stage]
  └─ Stage ID: 1

→ 激活主关卡的 Stage (StageID=1)
```

**场景 2：激活 LevelInstance 中的 Stage（复杂）**

```
蓝图节点流程：

[Event Begin Play]
       ↓
[Get Actor Of Class]  ← 步骤 1：找到 LevelInstance Actor
  └─ Actor Class: LevelInstance
       ↓
    [Return Value] (Actor 引用)
       ↓
[Get LevelInstance ID]  ← 步骤 2：获取 LevelInstanceID
  └─ LevelInstance Actor: [Actor 引用]
       ↓
    [Return Value] (FLevelInstanceID)
       ↓
[Activate Stage In LevelInstance]  ← 步骤 3：激活
  ├─ LevelInstance ID: [LevelInstanceID]
  └─ Stage ID: 1

→ 激活 LevelInstance 中的 Stage (StageID=1)
```

#### 实现逻辑

```cpp
// StageManagerSubsystem.cpp

FLevelInstanceID UStageManagerSubsystem::GetLevelInstanceID(AActor* LevelInstanceActor)
{
    if (!LevelInstanceActor)
    {
        return FLevelInstanceID();  // 空 ID = 主 Level
    }

    UWorld* World = LevelInstanceActor->GetWorld();
    if (!World) return FLevelInstanceID();

    ULevelInstanceSubsystem* LISubsystem = World->GetSubsystem<ULevelInstanceSubsystem>();
    if (!LISubsystem)
    {
        UE_LOG(LogStageEditor, Warning,
            TEXT("LevelInstanceSubsystem not found"));
        return FLevelInstanceID();
    }

    // 使用 UE5 原生 API 获取 LevelInstanceID（基于 Actor GUID 计算）
    return LISubsystem->GetLevelInstanceID(LevelInstanceActor);
}

void UStageManagerSubsystem::ActivateStageInLevelInstance(
    const FLevelInstanceID& LevelInstanceID,
    int32 StageID)
{
    // 构建 RuntimeID
    FStageRuntimeID RuntimeID;
    RuntimeID.LevelInstanceID = LevelInstanceID;
    RuntimeID.StageID = StageID;

    // 查找并激活
    TWeakObjectPtr<AStage>* FoundStage = RuntimeStageMap.Find(RuntimeID);
    if (FoundStage && FoundStage->IsValid())
    {
        (*FoundStage)->ActivateStage();
    }
    else
    {
        UE_LOG(LogStageEditor, Warning,
            TEXT("Stage not found: LevelInstanceID=%s, StageID=%d"),
            *LevelInstanceID.ToString(), StageID);
    }
}

void UStageManagerSubsystem::ActivateStage(int32 StageID)
{
    // 简化版本：LevelInstanceID 为空 = 主 Level
    FLevelInstanceID EmptyID;
    ActivateStageInLevelInstance(EmptyID, StageID);
}
```

#### 设计优势

| 特性 | 说明 |
|------|------|
| **渐进式复杂度** | 简单项目用 `ActivateStage()`，复杂项目用 `ActivateStageInLevelInstance()` |
| **向后兼容** | 现有蓝图无需修改，继续使用简化 API |
| **蓝图友好** | 用户只需两步：获取 Actor → 获取 ID → 激活 |
| **利用引擎能力** | 直接使用 UE5 的 `FLevelInstanceID`，无需自己维护 |

---

### 6.3 多人协作场景 ✅

#### 核心设计理念

通过**用户主动声明协作模式**来简化设计，避免复杂的离线注册表和自动检测机制。

#### 设计方案

**在创建 RegistryAsset 时，用户选择协作模式：**

```
┌──────────────────────────────────────────────────────────┐
│  创建 Stage Registry                                      │
├──────────────────────────────────────────────────────────┤
│  关卡: ForestLevel                                       │
│                                                          │
│  协作模式:                                                │
│  ◉ 单人开发 - 适合独立开发，无需 Source Control          │
│  ○ 多人协作 - 需要启用 Source Control 保护               │
│                                                          │
│  ╔════════════════════════════════════════════════╗     │
│  ║ [根据选择动态显示的提示信息]                    ║     │
│  ╚════════════════════════════════════════════════╝     │
│                                                          │
│  [ 创建 ]  [ 取消 ]                                      │
└──────────────────────────────────────────────────────────┘
```

#### 两种模式的行为

| 项目 | 单人开发模式 | 多人协作模式 |
|------|-------------|-------------|
| **Source Control 检查** | ❌ 完全忽略 SC 状态 | ✅ 强制检查，未启用则阻止创建 |
| **创建条件** | 无条件创建 | 必须启用 SC |
| **文件命名** | `<LevelName>_StageRegistry.uasset` | `<LevelName>_StageRegistry.uasset` (相同) |
| **UI 标识** | 👤 灰色图标 | 👥 绿色图标 |
| **模式切换** | ❌ 不支持（需删除重建） | ❌ 不支持（需删除重建） |
| **风险提示** | 提示无法切换为多人模式 | 检查 SC + 提示无法切换为单人模式 |

#### 数据结构

```cpp
// StageRegistryTypes.h

/**
 * 协作模式枚举
 */
UENUM(BlueprintType)
enum class ECollaborationMode : uint8
{
    /** 单人开发模式 - 不依赖 Source Control */
    Solo UMETA(DisplayName = "单人开发"),

    /** 多人协作模式 - 强制使用 Source Control */
    Multi UMETA(DisplayName = "多人协作")
};

/**
 * Stage Registry Asset
 */
UCLASS()
class UStageRegistryAsset : public UDataAsset
{
    GENERATED_BODY()

public:
    /** 关联的 Level */
    UPROPERTY(VisibleAnywhere, Category = "Registry")
    TSoftObjectPtr<UWorld> OwningLevel;

    /** 下一个可分配的 StageID */
    UPROPERTY(VisibleAnywhere, Category = "Registry")
    int32 NextStageID = 1;

    /** Stage 条目列表 */
    UPROPERTY(VisibleAnywhere, Category = "Registry")
    TArray<FStageRegistryEntry> StageEntries;

    /** 协作模式（创建后不可修改） */
    UPROPERTY(VisibleAnywhere, Category = "Registry")
    ECollaborationMode CollaborationMode = ECollaborationMode::Solo;
};
```

#### 创建时的风险提示内容

**单人开发模式提示：**
```
ℹ️ 单人开发模式

适用场景:
• 您是唯一的关卡开发者
• 快速原型开发

特点:
• 无需启用 Source Control
• 修改不受版本控制保护

⚠️ 重要提示:
• 创建后无法切换为多人协作模式
• 如需切换，必须删除 Registry 并重新创建
```

**多人协作模式提示（SC 已启用）：**
```
✅ 多人协作模式

Source Control 状态: ✅ 已启用 (Git/Perforce)

适用场景:
• 多人共同开发此关卡
• 需要版本控制保护

特点:
• 所有修改通过 Source Control
• 自动 Check Out / Check In
• 防止数据冲突

⚠️ 重要提示:
• 创建后无法切换为单人开发模式
• 所有协作者都需要启用 Source Control
• 修改前请确保同步最新版本
```

**多人协作模式提示（SC 未启用）：**
```
❌ 无法创建多人协作模式

Source Control 状态: ❌ 未启用

多人协作模式需要 Source Control 保护。

请先完成以下步骤:
1. 打开 Editor Preferences → Source Control
2. 配置 Git 或 Perforce
3. 连接到代码仓库

配置完成后，重新创建 Registry。

[ 打开 Source Control 设置 ]  ← 快捷按钮
```

**此时 [创建] 按钮禁用，无法创建。**

#### UI 标识方案

**不使用文件名区分**，通过 UI 图标和颜色标识：

| 模式 | 图标 | 颜色 | Tooltip |
|------|------|------|---------|
| 单人开发 | 👤 (User) | 灰色/蓝色 | "单人开发模式 - 无 Source Control 保护" |
| 多人协作 | 👥 (Groups) | 绿色 | "多人协作模式 - Source Control 已启用" |

**显示位置：**

1. **StageEditorPanel 顶部信息栏**
```
┌─────────────────────────────────────────────────┐
│ Stage Editor - ForestLevel                       │
├─────────────────────────────────────────────────┤
│ 📋 Registry: 👥 多人协作模式 (Source Control)   │
│ ─────────────────────────────────────────────── │
│ ├─ 📁 Acts (2)                                   │
└─────────────────────────────────────────────────┘
```

2. **DataLayerOutliner 中的前缀图标**
```
├─ 👥 DL_Stage_ForestLevel      ← 多人模式
│   └─ DL_Act_ForestLevel_Combat
├─ 👤 DL_Stage_TestLevel        ← 单人模式
    └─ DL_Act_TestLevel_Proto
```

3. **Content Browser 资产角标**
```
┌─────────────┐
│             │ 👥  ← 右上角角标
│  Registry   │
│ ForestLevel │
└─────────────┘
```

#### 实现逻辑

```cpp
// StageEditorController.cpp

UStageRegistryAsset* FStageEditorController::CreateRegistryAsset(
    UWorld* Level,
    ECollaborationMode Mode)
{
    // 多人协作模式：强制检查 SC
    if (Mode == ECollaborationMode::Multi)
    {
        if (!IsSourceControlEnabled())
        {
            ShowError(
                TEXT("无法创建多人协作模式的 Registry"),
                TEXT("多人协作模式需要启用 Source Control。\n"
                     "请先配置 Git 或 Perforce，然后重新创建。"));
            return nullptr;
        }
    }

    // 创建 Registry
    UStageRegistryAsset* Registry = CreateRegistryAssetInternal(Level);
    Registry->CollaborationMode = Mode;

    // 多人模式：Check Out 文件
    if (Mode == ECollaborationMode::Multi)
    {
        CheckOutRegistryFile(Registry);
    }

    Registry->MarkPackageDirty();
    return Registry;
}

bool FStageEditorController::IsSourceControlEnabled() const
{
    ISourceControlModule& SCModule =
        FModuleManager::LoadModuleChecked<ISourceControlModule>("SourceControl");

    return SCModule.IsEnabled() && SCModule.GetProvider().IsAvailable();
}
```

#### CRUD 操作中的 SC 检查

```cpp
// StageEditorController.cpp

int32 FStageEditorController::RegisterStage(AStage* Stage)
{
    UStageManagerSubsystem* Subsystem = GetSubsystem();
    UStageRegistryAsset* Registry = Subsystem->GetOrLoadRegistryAsset(Stage->GetWorld());

    if (!Registry)
    {
        ShowError(TEXT("未找到 Registry，请先创建"));
        return 0;
    }

    // 多人协作模式：检查 SC 并 Check Out
    if (Registry->CollaborationMode == ECollaborationMode::Multi)
    {
        if (!CheckOutRegistryFile(Registry))
        {
            ShowError(TEXT("无法 Check Out Registry 文件，请检查 Source Control 状态"));
            return 0;
        }
    }

    // 执行注册
    int32 NewStageID = Registry->AllocateAndRegister(Stage);
    Registry->MarkPackageDirty();

    return NewStageID;
}
```

#### 设计决策总结

| 决策点 | 方案 | 理由 |
|--------|------|------|
| **模式切换** | ❌ 不支持 | • 避免复杂的离线注册表设计<br>• 降低实施和维护成本<br>• 通过删除重建解决需求 |
| **风险防护** | ✅ 创建时充分提示 | • 降低误操作概率<br>• 用户知情选择 |
| **模式标识** | ✅ UI 图标/颜色 | • 避免资产重命名复杂度<br>• 视觉效果更好 |
| **团队规模** | ❌ 不区分大小团队 | • 技术实现一致<br>• 简化选择逻辑 |
| **强制 SC** | ✅ 多人模式强制 | • 符合最佳实践<br>• 保证数据一致性 |

#### 实施难度评估

| 模块 | 代码量 | 难度 |
|------|--------|------|
| 数据结构（`ECollaborationMode`） | ~50 行 | ⭐ 低 |
| 创建对话框 UI + 动态提示 | ~150 行 | ⭐⭐ 中 |
| SC 检查逻辑 | ~50 行 | ⭐ 低 |
| UI 图标标识 | ~50 行 | ⭐ 低 |
| CRUD 操作集成 | ~50 行 | ⭐ 低 |
| **总计** | **~350 行** | **⭐⭐ 中低** |

#### 优势

✅ **简化设计** - 无需离线注册表、合并工具等复杂机制
✅ **降低成本** - 代码量约 350 行，难度中低
✅ **清晰明确** - 用户主动声明意图，减少歧义
✅ **符合最佳实践** - 多人协作强制使用版本控制
✅ **容易理解** - 两个选项，概念简单
✅ **风险可控** - 通过充分提示降低误操作概率

---

### 6.4 Subsystem 架构拆分 ✅

#### 问题背景

当前 `UStageManagerSubsystem` 存在于 Runtime 模块中，但同时承担了两类职责：

1. **运行时职责**：Stage 查询、Cross-Stage 通信、运行时缓存
2. **编辑器职责**：RegistryAsset 管理、StageID 分配、Source Control 操作

这导致了以下问题：

| 问题 | 描述 |
|------|------|
| **❌ 运行时污染** | Runtime 模块不应该有任何 RegistryAsset 操作代码 |
| **❌ 性能浪费** | 编辑器相关代码在打包后依然编译进 Runtime，浪费性能 |
| **❌ 误用风险** | 编辑器方法可能在运行时被错误调用（虽然有 `#if WITH_EDITOR` 保护） |
| **❌ 职责不清** | 单个 Subsystem 承担过多职责，违反单一职责原则 |

#### 解决方案：双 Subsystem 架构

**核心理念：Runtime 和 Editor 完全分离，通过 Delegate 通信。**

```
┌─────────────────────────────────────────────────────────────────────┐
│                        Runtime 模块                                  │
│  ┌───────────────────────────────────────────────────────────────┐  │
│  │ UStageManagerSubsystem : UWorldSubsystem                      │  │
│  ├───────────────────────────────────────────────────────────────┤  │
│  │ 【职责】纯运行时查询与通信                                     │  │
│  │                                                                │  │
│  │ RuntimeStageMap: TMap<FStageRuntimeID, TWeakObjectPtr<AStage>>│  │
│  │                                                                │  │
│  │ API:                                                           │  │
│  │  - GetStage(StageID)                ← 运行时查询               │  │
│  │  - ActivateStage(StageID)           ← 运行时激活               │  │
│  │  - AddStageToRuntimeCache()         ← 缓存管理                 │  │
│  │  - RemoveStageFromRuntimeCache()    ← 缓存管理                 │  │
│  │                                                                │  │
│  │ Delegate:                                                      │  │
│  │  - OnStageLoadedDelegate            ← 通知 EditorSubsystem    │  │
│  └───────────────────────────────────────────────────────────────┘  │
└─────────────────────────────────────────────────────────────────────┘
                                   ↑ OnStageLoaded(Stage) 触发
                                   │
                                   │ (Delegate 通信)
                                   │
┌─────────────────────────────────────────────────────────────────────┐
│                        Editor 模块                                   │
│  ┌───────────────────────────────────────────────────────────────┐  │
│  │ UStageEditorSubsystem : UEditorSubsystem                      │  │
│  ├───────────────────────────────────────────────────────────────┤  │
│  │ 【职责】纯编辑器功能：RegistryAsset 管理                       │  │
│  │                                                                │  │
│  │ LoadedRegistries: TMap<FSoftObjectPath, UStageRegistryAsset*>│  │
│  │                                                                │  │
│  │ API:                                                           │  │
│  │  - RegisterStage(Stage)             ← StageID 分配             │  │
│  │  - UnregisterStage(Stage)           ← Registry 更新            │  │
│  │  - GetOrLoadRegistryAsset(Level)    ← Registry 加载            │  │
│  │  - CreateRegistryAsset(Level, Mode) ← Registry 创建            │  │
│  │  - CheckOutRegistryFile()           ← Source Control          │  │
│  │                                                                │  │
│  │ Event Handlers:                                                │  │
│  │  - HandleStageLoaded(Stage)         ← 监听 OnStageLoaded      │  │
│  └───────────────────────────────────────────────────────────────┘  │
└─────────────────────────────────────────────────────────────────────┘
```

#### 数据流示例

**场景 1：编辑器创建 Stage**

```cpp
// StageEditorController.cpp
FStageEditorController::CreateNewStage()
{
    // 1. 创建 Stage Actor
    AStage* NewStage = SpawnStageActor();

    // 2. 调用 EditorSubsystem 分配 StageID
    UStageEditorSubsystem* EditorSub = GEditor->GetEditorSubsystem<UStageEditorSubsystem>();
    int32 NewStageID = EditorSub->RegisterStage(NewStage);

    // EditorSubsystem 内部:
    //   - 加载 RegistryAsset
    //   - 分配 StageID
    //   - 调用 RuntimeSubsystem->AddStageToRuntimeCache()
    //   - 保存 Registry

    // 3. Stage 已注册，同时被添加到运行时缓存
}
```

**场景 2：World Partition Streaming 加载 Stage**

```cpp
// Stage.cpp
void AStage::PostLoad()
{
    Super::PostLoad();

#if WITH_EDITOR
    UWorld* World = GetWorld();
    if (!World || World->IsPlayInEditor() || World->IsGameWorld())
        return;

    // 触发 RuntimeSubsystem 的 Delegate
    UStageManagerSubsystem* RuntimeSub = World->GetSubsystem<UStageManagerSubsystem>();
    RuntimeSub->OnStageLoaded(this);

    // RuntimeSubsystem 内部:
    //   - 触发 OnStageLoadedDelegate
    //   - 广播给所有订阅者 (包括 EditorSubsystem)
#endif
}

// StageEditorSubsystem.cpp
void UStageEditorSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    // 订阅所有 World 的 StageManagerSubsystem 的 OnStageLoaded 事件
    // (实现略，需要监听 World 创建事件)
}

void UStageEditorSubsystem::HandleStageLoaded(AStage* Stage)
{
    // Stage 被 WP Streaming 加载

    // 1. 检查是否已有 StageID
    if (Stage->StageID != 0)
    {
        // 已注册，直接添加到运行时缓存
        UStageManagerSubsystem* RuntimeSub = Stage->GetWorld()->GetSubsystem<UStageManagerSubsystem>();
        RuntimeSub->AddStageToRuntimeCache(Stage);
    }
    else
    {
        // 未注册（旧数据迁移场景）
        RegisterStage(Stage);
    }
}
```

**场景 3：运行时激活 Stage（打包后）**

```cpp
// 蓝图调用
ActivateStage(1)
    ↓
// StageManagerSubsystem.cpp (Runtime 模块)
void UStageManagerSubsystem::ActivateStage(int32 StageID)
{
    FStageRuntimeID RuntimeID;
    RuntimeID.StageID = StageID;

    // 查询运行时缓存
    TWeakObjectPtr<AStage>* FoundStage = RuntimeStageMap.Find(RuntimeID);
    if (FoundStage && FoundStage->IsValid())
    {
        (*FoundStage)->ActivateStage();
    }
}

// 注意：此时 UStageEditorSubsystem 完全不存在（Editor 模块未加载）
```

#### 核心代码示例

**StageManagerSubsystem.h (Runtime 模块)**

```cpp
// Plugins/StageEditor/Source/StageEditorRuntime/Public/Subsystems/StageManagerSubsystem.h

/**
 * Runtime Subsystem - 纯查询与通信
 *
 * 职责:
 * - 运行时 Stage 查询
 * - Cross-Stage 通信
 * - 运行时缓存管理
 *
 * 不负责:
 * - ❌ RegistryAsset 管理
 * - ❌ StageID 分配
 * - ❌ Source Control 操作
 */
UCLASS()
class STAGEEDITORRUNTIME_API UStageManagerSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    // === Runtime Query API ===

    /**
     * 查询运行时已加载的 Stage
     * @param StageID Level 内的 StageID
     * @return Stage Actor 指针（如果已加载）
     */
    UFUNCTION(BlueprintCallable, Category = "Stage")
    AStage* GetStage(int32 StageID) const;

    /**
     * 强制激活 Stage（仅在运行时调用）
     * @param StageID Level 内的 StageID
     */
    UFUNCTION(BlueprintCallable, Category = "Stage")
    void ForceActivateStage(int32 StageID);

    // === Runtime Cache Management ===

    /**
     * 添加 Stage 到运行时缓存（由 EditorSubsystem 或 PostLoad 调用）
     */
    void AddStageToRuntimeCache(AStage* Stage);

    /**
     * 从运行时缓存移除 Stage
     */
    void RemoveStageFromRuntimeCache(AStage* Stage);

    // === Lifecycle Events ===

    /**
     * Stage 加载事件（WP Streaming 或手动加载触发）
     * EditorSubsystem 订阅此事件以处理 StageID 分配
     */
    DECLARE_MULTICAST_DELEGATE_OneParam(FOnStageLoaded, AStage* /*LoadedStage*/);
    FOnStageLoaded OnStageLoadedDelegate;

    /**
     * 触发 Stage 加载事件（由 Stage::PostLoad 调用）
     */
    void OnStageLoaded(AStage* Stage);

private:
    /**
     * 运行时缓存：RuntimeID → Stage Actor
     * 仅存储当前加载的 Stage
     */
    TMap<FStageRuntimeID, TWeakObjectPtr<AStage>> RuntimeStageMap;
};
```

**StageEditorSubsystem.h (Editor 模块)**

```cpp
// Plugins/StageEditor/Source/StageEditor/Public/Subsystems/StageEditorSubsystem.h

/**
 * Editor Subsystem - 纯编辑器功能
 *
 * 职责:
 * - RegistryAsset 管理
 * - StageID 分配
 * - Source Control 操作
 *
 * 不参与运行时:
 * - ❌ 打包后完全移除
 * - ❌ PIE/Runtime 不存在
 */
UCLASS()
class STAGEEDITOR_API UStageEditorSubsystem : public UEditorSubsystem
{
    GENERATED_BODY()

public:
    // === UEditorSubsystem Interface ===

    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // === Registry Management API ===

    /**
     * 注册 Stage：分配 StageID，更新 RegistryAsset
     * @return 分配的 StageID
     */
    int32 RegisterStage(AStage* Stage);

    /**
     * 取消注册 Stage：从 RegistryAsset 移除
     */
    void UnregisterStage(AStage* Stage);

    /**
     * 获取或加载 Level 对应的 RegistryAsset
     */
    UStageRegistryAsset* GetOrLoadRegistryAsset(UWorld* Level);

    /**
     * 创建新的 RegistryAsset
     * @param Level 关联的 Level
     * @param Mode 协作模式（Solo/Multi）
     */
    UStageRegistryAsset* CreateRegistryAsset(UWorld* Level, ECollaborationMode Mode);

    /**
     * 检查 Source Control 是否启用
     */
    bool IsSourceControlEnabled() const;

private:
    /**
     * 已加载的 RegistryAsset 缓存
     */
    TMap<FSoftObjectPath, UStageRegistryAsset*> LoadedRegistries;

    /**
     * 处理 Stage 加载事件（监听 RuntimeSubsystem 的 OnStageLoaded）
     */
    void HandleStageLoaded(AStage* Stage);

    /**
     * Check Out RegistryAsset 文件（多人协作模式）
     */
    bool CheckOutRegistryFile(UStageRegistryAsset* Registry);

    /**
     * 订阅 World 的 StageManagerSubsystem 事件
     */
    void BindToWorld(UWorld* World);
};
```

**Stage.cpp 的 PostLoad 修改**

```cpp
// Plugins/StageEditor/Source/StageEditorRuntime/Private/Actors/Stage.cpp

void AStage::PostLoad()
{
    Super::PostLoad();

#if WITH_EDITOR
    UWorld* World = GetWorld();
    if (!World || World->IsPlayInEditor() || World->IsGameWorld())
        return;

    // 通知 RuntimeSubsystem: Stage 已加载
    UStageManagerSubsystem* RuntimeSub = World->GetSubsystem<UStageManagerSubsystem>();
    if (RuntimeSub)
    {
        RuntimeSub->OnStageLoaded(this);

        // Delegate 触发 → EditorSubsystem::HandleStageLoaded()
        //   → 检查 StageID
        //   → 如果未注册，调用 RegisterStage()
        //   → 如果已注册，调用 AddStageToRuntimeCache()
    }
#endif
}
```

#### 方案对比

| 项目 | 方案 A：双 Subsystem 架构 | 方案 B：单 Subsystem + `#if WITH_EDITOR` |
|------|-------------------------|----------------------------------------|
| **Runtime 隔离** | ✅ 完全隔离，EditorSubsystem 在 Runtime 不存在 | ⚠️ 代码依然编译，但用 `#if` 屏蔽 |
| **模块职责** | ✅ 明确分离（Runtime 查询 + Editor 管理） | ❌ 单个类承担双重职责 |
| **性能** | ✅ 打包后 Editor 代码完全移除 | ⚠️ Editor 代码编译但不执行 |
| **代码量** | ~250 行（新建 EditorSubsystem） | ~150 行（添加 `#if` 宏） |
| **维护性** | ✅ 职责清晰，易于扩展 | ⚠️ 宏分支复杂，易出错 |
| **调试** | ✅ 两个独立类，调试简单 | ⚠️ 宏分支调试困难 |

**推荐：方案 A（双 Subsystem 架构）**

#### 设计优势

| 优势 | 说明 |
|------|------|
| ✅ **完全运行时隔离** | EditorSubsystem 在打包后完全不存在，零运行时开销 |
| ✅ **职责单一** | RuntimeSubsystem 只管查询，EditorSubsystem 只管管理 |
| ✅ **符合 UE 设计模式** | UE 官方推荐 Editor 功能用 UEditorSubsystem |
| ✅ **易于扩展** | 后续添加功能时职责明确（运行时 vs 编辑器） |
| ✅ **防止误用** | 编译期阻止运行时调用编辑器 API（类型不存在） |
| ✅ **性能优化** | 打包后完全移除 Editor 逻辑和依赖 |

#### 实施要点

**1. 模块依赖关系**

```
StageEditorRuntime (Runtime 模块)
  ├── UStageManagerSubsystem
  └── 不依赖任何 Editor 模块

StageEditor (Editor 模块)
  ├── UStageEditorSubsystem
  └── 依赖 StageEditorRuntime (访问 UStageManagerSubsystem)
```

**2. 通信方式**

- **Runtime → Editor**：通过 `OnStageLoadedDelegate` 触发
- **Editor → Runtime**：直接调用 Runtime API（`AddStageToRuntimeCache()` 等）

**3. 生命周期**

| Subsystem | 生命周期 | 何时存在 |
|-----------|---------|---------|
| `UStageManagerSubsystem` | UWorldSubsystem | 编辑器 + Runtime + PIE |
| `UStageEditorSubsystem` | UEditorSubsystem | 仅编辑器（打包后不存在） |

**4. 迁移策略**

```cpp
// 当前代码 (StageManagerSubsystem)
int32 RegisterStage(AStage* Stage)
{
#if WITH_EDITOR
    // 分配 StageID
    // 更新 RegistryAsset
    // ...
#endif
    return NewStageID;
}

// 迁移后
// StageEditorSubsystem.cpp (新建)
int32 UStageEditorSubsystem::RegisterStage(AStage* Stage)
{
    // 分配 StageID
    // 更新 RegistryAsset

    // 通知 RuntimeSubsystem 添加缓存
    UStageManagerSubsystem* RuntimeSub = Stage->GetWorld()->GetSubsystem<UStageManagerSubsystem>();
    RuntimeSub->AddStageToRuntimeCache(Stage);

    return NewStageID;
}

// StageManagerSubsystem.cpp (修改)
void UStageManagerSubsystem::AddStageToRuntimeCache(AStage* Stage)
{
    FStageRuntimeID RuntimeID;
    RuntimeID.StageID = Stage->StageID;
    RuntimeID.LevelInstanceID = GetLevelInstanceID(Stage);

    RuntimeStageMap.Add(RuntimeID, Stage);
}
```

#### 实施难度评估

| 模块 | 代码量 | 难度 |
|------|--------|------|
| 新建 `UStageEditorSubsystem` | ~250 行 | ⭐⭐ 中 |
| 重构 `UStageManagerSubsystem` | ~150 行 | ⭐⭐ 中 |
| 修改 `Stage::PostLoad` | ~20 行 | ⭐ 低 |
| 修改 `StageEditorController` | ~100 行 | ⭐ 低 |
| **总计** | **~520 行** | **⭐⭐ 中** |

#### 设计决策总结

| 决策点 | 方案 | 理由 |
|--------|------|------|
| **架构模式** | ✅ 双 Subsystem | • 完全运行时隔离<br>• 符合 UE 设计模式<br>• 职责清晰 |
| **通信方式** | ✅ Delegate + 直接调用 | • Runtime → Editor: Delegate<br>• Editor → Runtime: 直接调用 |
| **生命周期** | ✅ EditorSubsystem | • 打包后自动移除<br>• 无需手动宏控制 |

---

## 7. 涉及文件汇总

### 7.1 新建文件

| 文件 | 模块 | 说明 |
|------|------|------|
| `Public/Data/StageRegistryAsset.h` | StageEditorRuntime | UStageRegistryAsset 类定义 |
| `Private/Data/StageRegistryAsset.cpp` | StageEditorRuntime | UStageRegistryAsset 实现 |
| `Public/Core/StageRegistryTypes.h` | StageEditorRuntime | FStageRegistryEntry, FStageRuntimeID, ECollaborationMode 结构体 |
| `Public/Subsystems/StageEditorSubsystem.h` | StageEditor | UStageEditorSubsystem 类定义（新架构） |
| `Private/Subsystems/StageEditorSubsystem.cpp` | StageEditor | UStageEditorSubsystem 实现（新架构） |

### 7.2 需要修改的文件

#### Runtime 模块 (`StageEditorRuntime`)

| 文件 | 改动内容 |
|------|----------|
| `Public/Subsystems/StageManagerSubsystem.h` | - ❌ **移除** `LoadedRegistries` 成员（迁移到 EditorSubsystem）<br>- ✅ **保留** `RuntimeStageMap` 成员<br>- ❌ **移除** RegistryAsset 管理 API（迁移到 EditorSubsystem）<br>- ✅ **保留** 查询 API（`GetStage()`, `ActivateStage()`）<br>- ✅ **添加** `AddStageToRuntimeCache()` / `RemoveStageFromRuntimeCache()`<br>- ✅ **添加** `OnStageLoadedDelegate` 和 `OnStageLoaded()` 方法 |
| `Private/Subsystems/StageManagerSubsystem.cpp` | - ❌ **移除** `RegisterStage()` / `UnregisterStage()`（迁移到 EditorSubsystem）<br>- ✅ **保留** `GetStage()` 等查询方法<br>- ❌ **移除** `GetOrLoadRegistryAsset()`（迁移到 EditorSubsystem）<br>- ✅ **添加** `AddStageToRuntimeCache()` 实现<br>- ✅ **添加** `OnStageLoaded()` 实现（触发 Delegate） |
| `Public/Core/StageCoreTypes.h` | - 添加 `FStageRegistryEntry` 结构体<br>- 添加 `FStageRuntimeID` 结构体（如果不单独建文件）<br>- 添加 `ECollaborationMode` 枚举 |
| `Public/Actors/Stage.h` | - 可能需要调整 SUID 相关逻辑 |
| `Private/Actors/Stage.cpp` | - ✅ 修改 `PostLoad()` 调用 `RuntimeSubsystem->OnStageLoaded(this)`<br>- ❌ 移除直接调用 `RegisterStage()` 的逻辑 |
| `StageEditorRuntime.Build.cs` | - 如有新依赖需添加 |

#### Editor 模块 (`StageEditor`)

| 文件 | 改动内容 |
|------|----------|
| `Public/EditorUI/StageEditorPanel.h` | - 添加 RegistryAsset 路径设置到 `FAssetCreationSettings`<br>- 添加警告条 UI 相关成员<br>- 添加协作模式选择 UI |
| `Private/EditorUI/StageEditorPanel.cpp` | - 添加 Registry 检测逻辑<br>- 添加警告条 UI 构建<br>- 添加 [Create Registry] 对话框（包含协作模式选择）<br>- 添加协作模式图标/颜色标识 |
| `Public/EditorLogic/StageEditorController.h` | - ❌ **移除** RegistryAsset 管理 API（迁移到 EditorSubsystem）<br>- ✅ **修改** Stage CRUD 操作调用 `EditorSubsystem` |
| `Private/EditorLogic/StageEditorController.cpp` | - ❌ **移除** RegistryAsset 创建/查找逻辑<br>- ✅ **修改** `CreateNewStage()` 调用 `EditorSubsystem->RegisterStage()`<br>- ✅ **修改** `DeleteStage()` 调用 `EditorSubsystem->UnregisterStage()` |
| `Private/DataLayerSync/DataLayerSyncStatusCache.cpp` | - 可能需要适配新的查询 API（通过 RuntimeSubsystem） |
| `Private/DataLayerSync/DataLayerImporter.cpp` | - ✅ **修改** 调用 `EditorSubsystem->RegisterStage()` |

### 7.3 文件路径参考

```
Plugins/StageEditor/Source/
├── StageEditorRuntime/
│   ├── Public/
│   │   ├── Core/
│   │   │   ├── StageCoreTypes.h          ← 修改：添加新结构体
│   │   │   └── StageRegistryTypes.h      ← 新建（可选）
│   │   ├── Data/
│   │   │   └── StageRegistryAsset.h      ← 新建
│   │   ├── Subsystems/
│   │   │   └── StageManagerSubsystem.h   ← 修改
│   │   └── Actors/
│   │       └── Stage.h                   ← 修改
│   │
│   └── Private/
│       ├── Data/
│       │   └── StageRegistryAsset.cpp    ← 新建
│       ├── Subsystems/
│       │   └── StageManagerSubsystem.cpp ← 修改（主要改动）
│       └── Actors/
│           └── Stage.cpp                 ← 修改
│
└── StageEditor/
    ├── Public/
    │   ├── EditorUI/
    │   │   └── StageEditorPanel.h        ← 修改
    │   └── EditorLogic/
    │       └── StageEditorController.h   ← 修改
    │
    └── Private/
        ├── EditorUI/
        │   └── StageEditorPanel.cpp      ← 修改
        ├── EditorLogic/
        │   └── StageEditorController.cpp ← 修改（主要改动）
        └── DataLayerSync/
            ├── DataLayerSyncStatusCache.cpp ← 可能修改
            └── DataLayerImporter.cpp        ← 可能修改
```

### 7.4 改动量估计

| 类别 | 文件数 | 预计代码行数 |
|------|--------|-------------|
| 新建文件（Runtime 模块） | 2-3 | ~300（RegistryAsset + Types） |
| 新建文件（Editor 模块） | 2 | ~250（EditorSubsystem） |
| Runtime 模块修改 | 3-4 | ~200（重构 StageManagerSubsystem，精简为查询功能） |
| Editor 模块修改 | 4-6 | ~600（StageEditorController + UI + DataLayerSync 适配） |
| **总计** | **11-15** | **~1350** |

**新增内容（相比单 Subsystem 方案）：**
- ✅ 新建 `UStageEditorSubsystem`：~250 行
- ✅ 重构 `UStageManagerSubsystem` 以移除编辑器逻辑：~100 行
- **双 Subsystem 架构额外成本：~350 行**

---

## 8. 实现计划

> 待确定

---

## 9. 参考资料

- UE5 LevelInstance 文档：https://dev.epicgames.com/documentation/en-us/unreal-engine/level-instancing-in-unreal-engine
- `FLevelInstanceID` 定义：`Engine/Source/Runtime/Engine/Public/LevelInstance/LevelInstanceTypes.h`
- 当前 `StageManagerSubsystem` 实现：`Plugins/StageEditor/Source/StageEditorRuntime/`

---

## 更新记录

| 日期 | 更新内容 |
|------|---------|
| 2025-12-01 | 初始创建，定义核心问题和架构设计 |
| 2025-12-04 | ✅ 确定 RegistryAsset 查找策略（缓存+默认路径+全扫描）<br>✅ 确定 LevelInstance 支持方案（蓝图友好 API）<br>✅ 确定多人协作方案（用户主动声明模式 + 强制 SC 检查）<br>✅ 确定 Subsystem 架构拆分（双 Subsystem 架构：Runtime + Editor）<br><br>**Phase 13 设计完成，所有待确定事项已明确** |

---

*最后更新: 2025-12-04*
