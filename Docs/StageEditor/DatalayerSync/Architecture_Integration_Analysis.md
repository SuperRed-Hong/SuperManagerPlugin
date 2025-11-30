# StageEditor 与 DataLayerOutliner 架构整合分析

> 日期: 2025-11-30
> 状态: ✅ Phase 9 全部完成
> 触发问题: Import 按钮逻辑与 StageEditorPanel 创建逻辑冲突
> 最后更新: 2025-11-30 14:30

---

## 1. 问题发现

### 1.1 直接问题：Import 按钮会重复创建 DataLayer

**现象**：
- `StageDataLayerOutliner` 的 Import 按钮调用 `FDataLayerImporter::ExecuteImport()`
- 该函数创建 `AStage` 后，`StageEditorController::OnLevelActorAdded()` 被触发
- `OnLevelActorAdded` 检测到新 Stage，自动调用 `CreateDataLayerForStage()` 和 `CreateDataLayerForAct(1)`
- **结果**：重复创建 DataLayer，或覆盖 Import 指定的 DataLayer

**代码路径**：
```
StageDataLayerOutliner → Import 按钮
    → FDataLayerImporter::ExecuteImport()
        → CreateStageActor()
        → 设置 Stage->StageDataLayerAsset = 导入的DataLayer

    同时触发:
    → FStageEditorController::OnLevelActorAdded()
        → CreateDataLayerForStage() // 再次创建！
        → CreateDataLayerForAct(1)  // 创建 DefaultAct！
```

### 1.2 根本问题：两套独立的数据管理流

```
┌──────────────────────────────────────────────────────────────────┐
│                     当前架构（割裂）                              │
├──────────────────────────────────────────────────────────────────┤
│                                                                  │
│  ┌─────────────────────┐         ┌─────────────────────┐        │
│  │  StageEditorPanel   │         │ StageDataLayerOutliner│       │
│  │                     │         │                      │        │
│  │  - 创建 Stage       │  独立   │  - Import DataLayer  │        │
│  │  - 创建 Act         │◄──────►│  - Std Rename        │        │
│  │  - 管理 Props       │         │  - Sync              │        │
│  │  - 重命名           │         │  - 查看同步状态      │        │
│  └──────────┬──────────┘         └──────────┬───────────┘        │
│             │                               │                    │
│             ▼                               ▼                    │
│  ┌─────────────────────┐         ┌─────────────────────┐        │
│  │StageEditorController│         │  FDataLayerImporter │        │
│  │                     │         │                      │        │
│  │  - 事务管理         │   无    │  - 独立创建逻辑     │        │
│  │  - DataLayer创建    │◄──────►│  - 不经过Controller │        │
│  │  - OnLevelActorAdded│  协调   │  - 直接操作 Stage   │        │
│  └─────────────────────┘         └─────────────────────┘        │
│                                                                  │
└──────────────────────────────────────────────────────────────────┘
```

### 1.3 潜在隐患：多入口重命名不同步

**DataLayer 重命名入口**：
1. **StageEditorPanel** - 通过 Details Panel 或自定义 UI 修改 `Stage->StageDataLayerName`
2. **原生 UE DataLayerOutliner** - F2 重命名 `UDataLayerAsset`
3. **StageDataLayerOutliner** - Std Rename 按钮

**问题**：
- 入口 2 和 3 修改的是 `UDataLayerAsset` 的名字
- `AStage::StageDataLayerName` 只是一个**显示用的副本**
- 重命名后 `StageDataLayerName` 与实际 Asset 名字不同步
- `FAct::AssociatedDataLayer` 是直接引用，名字变化不影响，但显示会混乱

---

## 2. 当前架构分析

### 2.1 数据模型关系

```
┌─────────────────────────────────────────────────────────────┐
│                        AStage                                │
├─────────────────────────────────────────────────────────────┤
│  SUID: { StageID: 1 }                                       │
│  StageName: "MainStage"                                     │
│  StageDataLayerAsset: → DL_Stage_MainStage (Asset*)         │
│  StageDataLayerName: "DL_Stage_MainStage" (FString 副本)    │  ← 潜在不同步
│                                                              │
│  Acts: [                                                     │
│    FAct {                                                    │
│      SUID: { StageID: 1, ActID: 1 }                         │
│      DisplayName: "Default"                                  │
│      AssociatedDataLayer: → DL_Act_MainStage_Default        │
│    },                                                        │
│    FAct {                                                    │
│      SUID: { StageID: 1, ActID: 2 }                         │
│      DisplayName: "Combat"                                   │
│      AssociatedDataLayer: → DL_Act_MainStage_Combat         │
│    }                                                         │
│  ]                                                           │
│                                                              │
│  PropRegistry: { PropID → AActor* }                         │
└─────────────────────────────────────────────────────────────┘
```

### 2.2 操作来源与控制器关系

| 操作 | 来源 UI | 执行者 | 是否经过 Controller |
|------|---------|--------|---------------------|
| 创建 Stage | StageEditorPanel | OnLevelActorAdded() | ✅ 是 |
| 创建 Stage (Import) | StageDataLayerOutliner | FDataLayerImporter | ❌ 否 |
| 创建 Act | StageEditorPanel | CreateNewAct() | ✅ 是 |
| 创建 Act (Import) | StageDataLayerOutliner | FDataLayerImporter | ❌ 否 |
| 重命名 DataLayer | 多处 | 各自实现 | ❌ 否 |
| 删除 Act | StageEditorPanel | DeleteAct() | ✅ 是 |
| 注册 Prop | StageEditorPanel | RegisterProps() | ✅ 是 |

---

## 3. 架构改进方案

### 3.1 核心原则：单一数据操作入口

**所有对 Stage-Act-Prop 数据的修改，必须通过 `FStageEditorController`。**

```
┌──────────────────────────────────────────────────────────────────┐
│                     改进后架构（统一）                            │
├──────────────────────────────────────────────────────────────────┤
│                                                                  │
│  ┌─────────────────────┐         ┌─────────────────────┐        │
│  │  StageEditorPanel   │         │ StageDataLayerOutliner│       │
│  │                     │         │                      │        │
│  │  - UI 展示          │         │  - UI 展示          │        │
│  │  - 用户交互         │         │  - 用户交互         │        │
│  └──────────┬──────────┘         └──────────┬───────────┘        │
│             │                               │                    │
│             │                               │                    │
│             ▼                               ▼                    │
│         ┌───────────────────────────────────────┐               │
│         │       FStageEditorController          │               │
│         │            (单一入口)                  │               │
│         │                                        │               │
│         │  // Stage 操作                         │               │
│         │  + CreateStage(...)                   │               │
│         │  + ImportStageFromDataLayer(...)   // 新增             │
│         │  + RenameStageDataLayer(...)       // 新增             │
│         │                                        │               │
│         │  // Act 操作                           │               │
│         │  + CreateNewAct()                     │               │
│         │  + ImportActFromDataLayer(...)     // 新增             │
│         │  + RenameActDataLayer(...)         // 新增             │
│         │                                        │               │
│         │  // 事件广播                           │               │
│         │  + OnModelChanged                     │               │
│         │  + OnDataLayerRenamed              // 新增             │
│         └───────────────────────────────────────┘               │
│                          │                                       │
│                          ▼                                       │
│         ┌───────────────────────────────────────┐               │
│         │    AStage / StageManagerSubsystem     │               │
│         │             (数据存储)                 │               │
│         └───────────────────────────────────────┘               │
│                                                                  │
└──────────────────────────────────────────────────────────────────┘
```

### 3.2 具体改进点

#### 3.2.1 新增 Controller API

```cpp
class FStageEditorController
{
public:
    //----------------------------------------------------------------
    // Import API (from StageDataLayerOutliner)
    //----------------------------------------------------------------

    /**
     * 从现有 DataLayer 导入创建 Stage
     * - 不会创建新的 DataLayer
     * - 使用传入的 DataLayerAsset 作为 StageDataLayerAsset
     * - 自动处理子 DataLayer 作为 Acts
     */
    AStage* ImportStageFromDataLayer(UDataLayerAsset* StageDataLayerAsset);

    /**
     * 检查是否可以导入（用于 UI 显示）
     */
    bool CanImportDataLayer(UDataLayerAsset* DataLayerAsset, FText& OutReason);

    //----------------------------------------------------------------
    // Rename API (统一入口)
    //----------------------------------------------------------------

    /**
     * 重命名 Stage 的 DataLayer
     * - 同时更新 UDataLayerAsset 名称
     * - 同时更新 AStage::StageDataLayerName
     * - 广播 OnDataLayerRenamed
     */
    bool RenameStageDataLayer(AStage* Stage, const FString& NewName);

    /**
     * 重命名 Act 的 DataLayer
     * - 同时更新 UDataLayerAsset 名称
     * - 广播 OnDataLayerRenamed
     */
    bool RenameActDataLayer(AStage* Stage, int32 ActID, const FString& NewName);

    //----------------------------------------------------------------
    // 新增委托
    //----------------------------------------------------------------

    /** 当 DataLayer 名称变化时广播（用于 UI 刷新） */
    DECLARE_MULTICAST_DELEGATE_TwoParams(FOnDataLayerRenamed, UDataLayerAsset*, const FString& NewName);
    FOnDataLayerRenamed OnDataLayerRenamed;
};
```

#### 3.2.2 修改 OnLevelActorAdded 逻辑

```cpp
void FStageEditorController::OnLevelActorAdded(AActor* InActor)
{
    AStage* NewStage = Cast<AStage>(InActor);
    if (!NewStage) return;

    // 检查是否已有 DataLayer（Import 场景）
    if (NewStage->StageDataLayerAsset)
    {
        // 已有 DataLayer，说明是 Import 创建的，不要再创建
        UE_LOG(LogTemp, Log, TEXT("Stage '%s' already has DataLayer, skipping auto-creation"),
            *NewStage->GetActorLabel());
    }
    else
    {
        // 新建场景，自动创建 DataLayer
        CreateDataLayerForStage(NewStage);
        // ... 创建 DefaultAct DataLayer
    }

    FindStageInWorld();
}
```

#### 3.2.3 FDataLayerImporter 改为调用 Controller

```cpp
// 当前：FDataLayerImporter 直接操作 AStage
AStage* FDataLayerImporter::CreateStageActor(...)
{
    AStage* NewStage = World->SpawnActor<AStage>(...);
    NewStage->StageDataLayerAsset = StageDataLayerAsset;  // 直接设置
    return NewStage;
}

// 改进：FDataLayerImporter 调用 Controller
FDataLayerImportResult FDataLayerImporter::ExecuteImport(...)
{
    // 获取 Controller（需要传入或全局访问）
    FStageEditorController* Controller = GetStageEditorController();
    if (Controller)
    {
        AStage* NewStage = Controller->ImportStageFromDataLayer(StageDataLayerAsset);
        // ... Controller 内部处理所有逻辑
    }
}
```

#### 3.2.4 移除 StageDataLayerName 冗余字段（可选）

```cpp
// 当前：AStage 存储了两个关于 DataLayer 名字的字段
UPROPERTY()
TObjectPtr<UDataLayerAsset> StageDataLayerAsset;  // 实际引用

UPROPERTY()
FString StageDataLayerName;  // 名字副本（容易不同步）

// 改进选项 A：删除 StageDataLayerName，需要时从 Asset 获取
FString AStage::GetStageDataLayerName() const
{
    return StageDataLayerAsset ? StageDataLayerAsset->GetName() : TEXT("");
}

// 改进选项 B：保留但自动同步
// 在 Controller::RenameStageDataLayer() 中同时更新两者
```

### 3.3 Std Rename 整合

```cpp
// SStdRenamePopup::ExecuteRename() 改为调用 Controller
bool SStdRenamePopup::ExecuteRename()
{
    FStageEditorController* Controller = GetController();
    if (!Controller) return false;

    FString NewName = NamePrefix + UserInput;

    // 通过 Controller 统一入口
    if (bIsStage)
    {
        AStage* Stage = FindStageForDataLayer(DataLayerAsset);
        if (Stage)
        {
            return Controller->RenameStageDataLayer(Stage, NewName);
        }
    }
    else
    {
        // Act 场景
        AStage* Stage = FindStageForDataLayer(DataLayerAsset);
        int32 ActID = FindActIDForDataLayer(Stage, DataLayerAsset);
        if (Stage && ActID >= 0)
        {
            return Controller->RenameActDataLayer(Stage, ActID, NewName);
        }
    }

    // 未导入的 DataLayer：直接重命名 Asset（不经过 Controller）
    return RenameAssetDirect(NewName);
}
```

---

## 4. 数据同步机制

### 4.1 事件订阅关系（改进后）

```
┌─────────────────────────────────────────────────────────────┐
│                    事件流（改进后）                          │
├─────────────────────────────────────────────────────────────┤
│                                                              │
│  UDataLayerEditorSubsystem::OnDataLayerChanged               │
│       ↓                                                      │
│  ┌─────────────────────────────────────────┐                │
│  │     FStageEditorController              │                │
│  │  (监听原生 DataLayer 变化)               │                │
│  │                                          │                │
│  │  - OnDataLayerChanged → 检测外部重命名   │                │
│  │  - 如果是已导入的 DL → 同步 Stage 数据   │                │
│  └─────────────────────────────────────────┘                │
│       ↓ 广播 OnModelChanged / OnDataLayerRenamed            │
│                                                              │
│  ┌───────────────────┐   ┌─────────────────────┐            │
│  │ StageEditorPanel  │   │StageDataLayerOutliner│           │
│  │ (订阅 Controller) │   │ (订阅 Controller)    │           │
│  │                   │   │                      │           │
│  │  刷新 Stage 列表  │   │  刷新 DataLayer 列表 │           │
│  └───────────────────┘   └─────────────────────┘            │
│                                                              │
└─────────────────────────────────────────────────────────────┘
```

### 4.2 外部重命名处理

当用户在原生 UE DataLayerOutliner 中重命名 DataLayer 时：

```cpp
void FStageEditorController::OnExternalDataLayerChanged(
    EDataLayerAction Action,
    const UDataLayerInstance* Instance,
    FName OldName)
{
    if (Action != EDataLayerAction::Rename) return;

    UDataLayerAsset* Asset = Instance->GetAsset();
    AStage* Stage = GetSubsystem()->FindStageByDataLayer(Asset);

    if (Stage)
    {
        // 已导入的 DataLayer 被外部重命名
        // 同步更新 Stage 的 StageDataLayerName
        if (Stage->StageDataLayerAsset == Asset)
        {
            Stage->StageDataLayerName = Asset->GetName();
        }

        // 广播变化
        OnModelChanged.Broadcast();
        OnDataLayerRenamed.Broadcast(Asset, Asset->GetName());
    }
}
```

---

## 5. 实施计划

### Phase 9.1: 紧急修复 Import 冲突 ✅ 已完成

**实现方案**：使用 RAII 模式的 `FScopedImportBypass` 临时禁止自动创建 DataLayer

**核心修改**：

1. **StageEditorController.h** - 新增静态 bypass 机制：
```cpp
// RAII helper to suppress automatic DataLayer creation during import
struct STAGEEDITOR_API FScopedImportBypass
{
    FScopedImportBypass() { ++FStageEditorController::ImportBypassCounter; }
    ~FScopedImportBypass() { --FStageEditorController::ImportBypassCounter; }
};

static bool IsImportBypassActive() { return ImportBypassCounter > 0; }

private:
    static int32 ImportBypassCounter;
```

2. **StageEditorController.cpp** - 修改 `OnLevelActorAdded()`：
```cpp
// Auto-create DataLayers if World Partition is active
// IMPORTANT: Skip if in import bypass mode
if (IsWorldPartitionActive() && !IsImportBypassActive())
{
    // Create Stage's root DataLayer
    if (!NewStage->StageDataLayerAsset && NewStage->GetStageID() > 0)
    {
        CreateDataLayerForStage(NewStage);
    }
    // Create DataLayer for Default Act...
}
else if (IsImportBypassActive())
{
    UE_LOG(LogTemp, Log, TEXT("Skipping auto DataLayer creation (import bypass active)..."));
}
```

3. **DataLayerImporter.cpp** - 在 `ExecuteImport()` 中激活 bypass：
```cpp
FDataLayerImportResult FDataLayerImporter::ExecuteImport(...)
{
    // CRITICAL: Suppress automatic DataLayer creation in OnLevelActorAdded
    FStageEditorController::FScopedImportBypass ImportBypass;

    // 1. Create Stage Actor (OnLevelActorAdded will skip DataLayer creation)
    AStage* NewStage = CreateStageActor(...);

    // 2-4. Set existing DataLayers, process Acts...
}
```

**验证结果**：编译通过 ✅

### Phase 9.2: Controller API 扩展 ✅ 已完成

**新增 Controller API**：

1. **StageEditorController.h** - 新增方法：
```cpp
// Import API
AStage* ImportStageFromDataLayer(UDataLayerAsset* StageDataLayerAsset, UWorld* World = nullptr);
bool CanImportDataLayer(UDataLayerAsset* DataLayerAsset, FText& OutReason);

// Rename API (统一入口)
bool RenameStageDataLayer(AStage* Stage, const FString& NewAssetName);
bool RenameActDataLayer(AStage* Stage, int32 ActID, const FString& NewAssetName);

// Lookup API
AStage* FindStageByDataLayer(UDataLayerAsset* DataLayerAsset);
int32 FindActIDByDataLayer(AStage* Stage, UDataLayerAsset* DataLayerAsset);

// 新委托
FOnDataLayerRenamed OnDataLayerRenamed;
```

2. **StageEditorModule.h** - 新增全局访问：
```cpp
TSharedPtr<FStageEditorController> GetController();
static FStageEditorModule& Get();
static bool IsAvailable();
```

3. **FDataLayerImporter::ExecuteImport** - 重构为调用 Controller：
```cpp
// 旧: 直接操作 AStage
// 新: 委托给 Controller->ImportStageFromDataLayer()
```

4. **SStdRenamePopup::ExecuteRename** - 重构为判断后调用 Controller：
```cpp
// 已导入 DataLayer -> Controller->RenameStageDataLayer() 或 RenameActDataLayer()
// 未导入 DataLayer -> 直接重命名 Asset
```

**验证结果**：编译通过 ✅

### Phase 9.3: 事件同步完善 ✅ 已完成

**核心修改**：

1. **StageEditorController.h** - 新增成员：
```cpp
// Handler for external DataLayer changes
void OnExternalDataLayerChanged(
    const EDataLayerAction Action,
    const TWeakObjectPtr<const UDataLayerInstance>& ChangedDataLayer,
    const FName& ChangedProperty);

// Delegate handle for subscription
FDelegateHandle DataLayerChangedHandle;
```

2. **StageEditorController.cpp** - `BindEditorDelegates()` 订阅：
```cpp
// Subscribe to DataLayer changes (for external rename detection)
if (UDataLayerEditorSubsystem* DataLayerSubsystem = UDataLayerEditorSubsystem::Get())
{
    DataLayerChangedHandle = DataLayerSubsystem->OnDataLayerChanged().AddRaw(
        this, &FStageEditorController::OnExternalDataLayerChanged);
}
```

3. **StageEditorController.cpp** - `UnbindEditorDelegates()` 取消订阅：
```cpp
if (DataLayerChangedHandle.IsValid())
{
    if (UDataLayerEditorSubsystem* DataLayerSubsystem = UDataLayerEditorSubsystem::Get())
    {
        DataLayerSubsystem->OnDataLayerChanged().Remove(DataLayerChangedHandle);
    }
    DataLayerChangedHandle.Reset();
}
```

4. **StageEditorController.cpp** - 实现 `OnExternalDataLayerChanged()`：
```cpp
void FStageEditorController::OnExternalDataLayerChanged(...)
{
    // Only handle rename actions
    if (Action != EDataLayerAction::Rename) return;

    // Find owning Stage
    AStage* OwnerStage = FindStageByDataLayer(Asset);
    if (!OwnerStage) return; // Not imported

    // Sync StageDataLayerName if it's the root DataLayer
    if (OwnerStage->StageDataLayerAsset == Asset)
    {
        OwnerStage->StageDataLayerName = Asset->GetName();
    }

    // Invalidate cache and broadcast
    FDataLayerSyncStatusCache::Get().InvalidateCache(Asset);
    OnDataLayerRenamed.Broadcast(Asset, NewName);
    OnModelChanged.Broadcast();
}
```

**验证结果**：编译通过 ✅

### Phase 9.4: 冗余字段清理 ✅ 已完成

**评估结论**：
- 删除 `StageDataLayerName` 会破坏 Blueprint 兼容性（有 `BlueprintReadOnly`）
- 更安全的做法：添加 getter 方法，优先读取 Asset 名称

**实施方案**：

1. **Stage.h** - 新增 getter 方法：
```cpp
/**
 * @brief Gets the display name of the Stage's DataLayer.
 * Prioritizes the actual Asset name if available, falls back to cached StageDataLayerName.
 */
UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Stage|DataLayer")
FString GetStageDataLayerDisplayName() const;
```

2. **Stage.cpp** - 实现 getter：
```cpp
FString AStage::GetStageDataLayerDisplayName() const
{
    // Prioritize actual Asset name (always up-to-date)
    if (StageDataLayerAsset)
    {
        return StageDataLayerAsset->GetName();
    }

    // Fallback to cached name (for compatibility when Asset is null)
    return StageDataLayerName;
}
```

3. **Stage.h** - 更新 `StageDataLayerName` 注释：
```cpp
/**
 * Cached display name for the Stage's root Data Layer.
 * Use GetStageDataLayerDisplayName() for reliable access.
 * @note This is kept for Blueprint compatibility. Prefer using the getter.
 */
UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stage|DataLayer",
          meta = (DisplayName = "DataLayer Name (Cached)"))
FString StageDataLayerName;
```

**验证结果**：编译通过 ✅

---

## 6. 风险评估

| 风险 | 影响 | 缓解措施 |
|------|------|----------|
| 现有项目兼容性 | 中 | 保留旧字段，新增 getter |
| 改动范围大 | 高 | 分阶段实施，每阶段独立可测 |
| UI 刷新不及时 | 低 | 统一通过 Controller 委托 |
| 外部重命名同步 | 中 | 明确监听 `OnDataLayerChanged` |

---

## 7. 附录：当前文件依赖关系

```
StageEditorController.h/cpp
├── 被依赖: SStageEditorPanel (直接持有实例)
├── 被依赖: FDataLayerImporter (无，需要改)
├── 依赖: AStage
├── 依赖: StageManagerSubsystem
└── 依赖: UDataLayerEditorSubsystem (间接)

FDataLayerImporter.h/cpp
├── 被依赖: SDataLayerImportPreviewDialog
├── 依赖: AStage (直接操作)
├── 依赖: StageManagerSubsystem
└── 不依赖: StageEditorController (问题根源)

SStdRenamePopup.h/cpp
├── 依赖: UDataLayerAsset
├── 依赖: FAssetToolsModule (重命名)
└── 不依赖: StageEditorController (应该依赖)

SStageDataLayerOutliner.h/cpp
├── 依赖: UDataLayerEditorSubsystem
├── 依赖: FDataLayerSyncStatusCache
└── 间接依赖: FStageEditorController (通过 Import)
```

---

*文档创建: 2025-11-30 12:30*
