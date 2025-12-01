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

## 6. 待讨论问题

### 6.1 RegistryAsset 查找效率

当按命名约定找不到时，需要扫描目录。如果项目有很多 RegistryAsset，扫描可能较慢。

**可能的优化：**
- 使用 Asset Registry 异步查询
- 缓存已建立的 Level → Registry 映射

### 6.2 多人协作场景

如果多人同时编辑同一个 Level：
- RegistryAsset 是否会冲突？
- 如何处理 StageID 分配冲突？

### 6.3 Cross-Stage 通信 API 变更

当前 API：
```cpp
void ForceActivateStage(int32 StageID);
```

需要考虑是否改为：
```cpp
void ForceActivateStage(const FStageRuntimeID& RuntimeID);
// 或
void ForceActivateStage(int32 StageID);  // 隐式使用当前 Level
```

---

## 7. 涉及文件汇总

### 7.1 新建文件

| 文件 | 模块 | 说明 |
|------|------|------|
| `Public/Data/StageRegistryAsset.h` | StageEditorRuntime | UStageRegistryAsset 类定义 |
| `Private/Data/StageRegistryAsset.cpp` | StageEditorRuntime | UStageRegistryAsset 实现 |
| `Public/Core/StageRegistryTypes.h` | StageEditorRuntime | FStageRegistryEntry, FStageRuntimeID 结构体 |

### 7.2 需要修改的文件

#### Runtime 模块 (`StageEditorRuntime`)

| 文件 | 改动内容 |
|------|----------|
| `Public/Subsystems/StageManagerSubsystem.h` | - 添加 `LoadedRegistries` 成员<br>- 添加 `RuntimeStageMap` 成员<br>- 添加 RegistryAsset 管理 API<br>- 修改注册/查询 API 签名 |
| `Private/Subsystems/StageManagerSubsystem.cpp` | - 重写 `RegisterStage()` 使用 RegistryAsset<br>- 重写 `UnregisterStage()` 使用 RegistryAsset<br>- 重写 `GetStage()` 等查询方法<br>- 添加 `GetOrLoadRegistryAsset()` 方法<br>- 修改 `ScanWorldForExistingStages()` 逻辑 |
| `Public/Core/StageCoreTypes.h` | - 添加 `FStageRegistryEntry` 结构体<br>- 添加 `FStageRuntimeID` 结构体（如果不单独建文件） |
| `Public/Actors/Stage.h` | - 可能需要调整 SUID 相关逻辑 |
| `Private/Actors/Stage.cpp` | - 调整 `PostLoad()` 中的自注册逻辑<br>- 调整 `EnsureRegisteredWithSubsystem()` |
| `StageEditorRuntime.Build.cs` | - 如有新依赖需添加 |

#### Editor 模块 (`StageEditor`)

| 文件 | 改动内容 |
|------|----------|
| `Public/EditorUI/StageEditorPanel.h` | - 添加 RegistryAsset 路径设置到 `FAssetCreationSettings`<br>- 添加警告条 UI 相关成员 |
| `Private/EditorUI/StageEditorPanel.cpp` | - 添加 Registry 检测逻辑<br>- 添加警告条 UI 构建<br>- 添加 [Create Registry] 按钮处理 |
| `Public/EditorLogic/StageEditorController.h` | - 添加 RegistryAsset 管理 API<br>- 添加 `CreateRegistryAsset()` 方法<br>- 添加 `FindRegistryAssetForLevel()` 方法 |
| `Private/EditorLogic/StageEditorController.cpp` | - 实现 RegistryAsset 创建逻辑<br>- 实现 RegistryAsset 查找逻辑<br>- 修改 Stage CRUD 操作调用 RegistryAsset |
| `Private/DataLayerSync/DataLayerSyncStatusCache.cpp` | - 可能需要适配新的查询 API |
| `Private/DataLayerSync/DataLayerImporter.cpp` | - 可能需要适配新的注册 API |

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
| 新建文件 | 2-3 | ~300 |
| Runtime 模块修改 | 4-5 | ~400 |
| Editor 模块修改 | 4-6 | ~500 |
| **总计** | **10-14** | **~1200** |

---

## 8. 实现计划

> 待确定

---

## 9. 参考资料

- UE5 LevelInstance 文档：https://dev.epicgames.com/documentation/en-us/unreal-engine/level-instancing-in-unreal-engine
- `FLevelInstanceID` 定义：`Engine/Source/Runtime/Engine/Public/LevelInstance/LevelInstanceTypes.h`
- 当前 `StageManagerSubsystem` 实现：`Plugins/StageEditor/Source/StageEditorRuntime/`

---

*最后更新: 2025-12-01*
