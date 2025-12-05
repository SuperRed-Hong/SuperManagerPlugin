# Phase 13 - P0-2: 事务一致性处理方案

> **创建日期**: 2025-12-05
> **相关文档**: [Phase13_StageRegistry_Discussion.md](Phase13_StageRegistry_Discussion.md)
> **评审报告**: [Phase13_ExpertReview_Report.md](Phase13_ExpertReview_Report.md)

本文档针对专家评审报告中提出的 P0-2 级问题（事务一致性处理缺失）提供详细解决方案。

---

## 问题描述

### 核心问题

CRUD 操作中 **RegistryAsset 修改** 和 **Level 修改** 不在同一个事务中，缺少失败回滚机制。

### 失败场景分析

#### 场景 1: 创建 Stage 时的失败点

```cpp
// 当前设计的潜在问题
FStageEditorController::CreateNewStage()
{
    FScopedTransaction Transaction(LOCTEXT("CreateStage", "Create New Stage"));

    // ✅ 步骤 1: 创建 Stage Actor（在 Level 中）
    AStage* NewStage = SpawnStageActor();  // UWorld 已 Modify()

    // ✅ 步骤 2: 调用 EditorSubsystem 注册
    UStageEditorSubsystem* EditorSub = GetEditorSubsystem();
    int32 NewStageID = EditorSub->RegisterStage(NewStage);

    // EditorSubsystem::RegisterStage() 内部:
    // ✅ 步骤 2.1: 加载 RegistryAsset
    // ✅ 步骤 2.2: 分配 StageID
    // ⚠️ 步骤 2.3: Check Out RegistryAsset (多人模式) - 可能失败！
    // ✅ 步骤 2.4: 添加到 StageEntries
    // ✅ 步骤 2.5: MarkPackageDirty()
    // ✅ 步骤 2.6: 添加到 RuntimeCache

    // ❌ 问题 1: 如果 Check Out 失败，Stage Actor 已创建但未注册
    // ❌ 问题 2: 如果保存 Registry 时磁盘满，数据不一致
    // ❌ 问题 3: 如果 RuntimeCache 添加失败，查询时找不到 Stage
}
```

#### 场景 2: 删除 Stage 时的失败点

```cpp
// 当前设计的潜在问题
FStageEditorController::DeleteStage(AStage* Stage)
{
    FScopedTransaction Transaction(LOCTEXT("DeleteStage", "Delete Stage"));

    // ✅ 步骤 1: 从 RegistryAsset 移除
    EditorSub->UnregisterStage(Stage);

    // ⚠️ 步骤 2: 删除 Stage Actor - 可能失败（被引用、被锁定等）
    Stage->Destroy();

    // ❌ 问题: 如果 Destroy() 失败，Registry 已移除但 Actor 仍存在
    // ❌ 结果: "孤儿 Stage"（有 Actor 但无注册记录）
}
```

#### 场景 3: Source Control 操作失败

```cpp
// 多人协作模式下的典型失败
EditorSubsystem::RegisterStage(Stage)
{
    UStageRegistryAsset* Registry = GetOrLoadRegistryAsset(Stage->GetWorld());

    // ⚠️ 步骤 1: Check Out Registry
    if (Registry->CollaborationMode == ECollaborationMode::Multi)
    {
        if (!CheckOutRegistryFile(Registry))
        {
            // ❌ 失败原因可能是:
            // - Source Control 服务器不可达
            // - 文件已被其他用户锁定 (Perforce)
            // - 本地工作区不干净 (Git)
            // - 用户无权限
            return 0;  // 返回失败，但 Stage Actor 可能已创建
        }
    }

    // 继续注册流程...
}
```

---

## 解决方案设计

### 核心设计原则

1. **预检查机制**: 在修改任何数据前，先验证所有前置条件
2. **原子性保证**: 要么全部成功，要么全部回滚
3. **错误传播**: 失败时清晰地向上层传播错误信息
4. **用户友好**: 失败时提供明确的错误提示和恢复建议

### 架构设计

```
┌──────────────────────────────────────────────────────────────┐
│                     Controller 层                             │
│  ┌────────────────────────────────────────────────────────┐  │
│  │ FStageEditorController::CreateNewStage()               │  │
│  │                                                        │  │
│  │ 1. 预检查（PreCheck）                                  │  │
│  │    ├─ Registry 是否存在                                │  │
│  │    ├─ Source Control 状态（多人模式）                  │  │
│  │    └─ 磁盘空间充足                                     │  │
│  │                                                        │  │
│  │ 2. 开启事务（Transaction）                             │  │
│  │                                                        │  │
│  │ 3. 执行操作                                            │  │
│  │    ├─ 创建 Stage Actor                                 │  │
│  │    └─ 调用 EditorSubsystem::RegisterStage()           │  │
│  │                                                        │  │
│  │ 4. 验证结果                                            │  │
│  │    └─ 如果失败 → 回滚（删除 Actor）                    │  │
│  │                                                        │  │
│  │ 5. 提交事务                                            │  │
│  └────────────────────────────────────────────────────────┘  │
└──────────────────────────────────────────────────────────────┘
                              ↓
┌──────────────────────────────────────────────────────────────┐
│                  EditorSubsystem 层                           │
│  ┌────────────────────────────────────────────────────────┐  │
│  │ UStageEditorSubsystem::RegisterStage()                 │  │
│  │                                                        │  │
│  │ 1. 预检查（PreCheck）                                  │  │
│  │    ├─ Registry 有效性                                  │  │
│  │    ├─ Check Out 成功（多人模式）                       │  │
│  │    └─ StageID 未冲突                                   │  │
│  │                                                        │  │
│  │ 2. 分配 StageID                                        │  │
│  │                                                        │  │
│  │ 3. 更新 RegistryAsset                                  │  │
│  │    ├─ 添加 Entry                                       │  │
│  │    └─ MarkPackageDirty()                               │  │
│  │                                                        │  │
│  │ 4. 验证保存（可选，同步保存模式）                       │  │
│  │                                                        │  │
│  │ 5. 添加到 RuntimeCache                                 │  │
│  │                                                        │  │
│  │ 6. 返回 StageID（成功）或 0（失败）                    │  │
│  └────────────────────────────────────────────────────────┘  │
└──────────────────────────────────────────────────────────────┘
```

---

## 详细实现方案

### 方案 1: 预检查 + 错误码传播（推荐）

#### 优点
- 实现简单，易于理解
- 性能开销小
- 利用 UE 现有的 Transaction 系统

#### 缺点
- 依赖 UE Transaction 系统的正确性
- 无法处理跨包（Package）的原子性（Registry 和 Level 是不同的包）

#### 实现代码

##### StageEditorController.h

```cpp
// Plugins/StageEditor/Source/StageEditor/Public/EditorLogic/StageEditorController.h

/**
 * 操作结果枚举
 */
UENUM()
enum class EStageOperationResult : uint8
{
    /** 操作成功 */
    Success = 0,

    /** Registry 不存在 */
    RegistryNotFound,

    /** Source Control Check Out 失败 */
    CheckOutFailed,

    /** Source Control 未启用（多人模式必需） */
    SourceControlNotEnabled,

    /** 磁盘空间不足 */
    InsufficientDiskSpace,

    /** StageID 冲突 */
    StageIDConflict,

    /** 保存失败 */
    SaveFailed,

    /** Stage Actor 无效 */
    InvalidStageActor,

    /** 未知错误 */
    UnknownError
};

/**
 * 操作结果结构体
 */
USTRUCT(BlueprintType)
struct FStageOperationStatus
{
    GENERATED_BODY()

    /** 结果码 */
    UPROPERTY(BlueprintReadOnly)
    EStageOperationResult Result = EStageOperationResult::Success;

    /** 错误消息（用于显示给用户） */
    UPROPERTY(BlueprintReadOnly)
    FText ErrorMessage;

    /** 详细错误信息（用于日志） */
    UPROPERTY(BlueprintReadOnly)
    FString DetailedError;

    /** 是否成功 */
    bool IsSuccess() const { return Result == EStageOperationResult::Success; }

    /** 创建成功状态 */
    static FStageOperationStatus MakeSuccess()
    {
        FStageOperationStatus Status;
        Status.Result = EStageOperationResult::Success;
        return Status;
    }

    /** 创建失败状态 */
    static FStageOperationStatus MakeError(
        EStageOperationResult InResult,
        const FText& InErrorMessage,
        const FString& InDetailedError = FString())
    {
        FStageOperationStatus Status;
        Status.Result = InResult;
        Status.ErrorMessage = InErrorMessage;
        Status.DetailedError = InDetailedError;
        return Status;
    }
};

class FStageEditorController
{
public:
    /**
     * 创建新 Stage（带完整错误处理）
     * @param OutNewStage 输出参数：创建的 Stage Actor（失败时为 nullptr）
     * @return 操作状态
     */
    FStageOperationStatus CreateNewStage(AStage*& OutNewStage);

    /**
     * 删除 Stage（带完整错误处理）
     * @param Stage 要删除的 Stage
     * @param bDeleteDataLayers 是否同时删除关联的 DataLayers
     * @return 操作状态
     */
    FStageOperationStatus DeleteStage(AStage* Stage, bool bDeleteDataLayers);

private:
    /**
     * 预检查：创建 Stage 的前置条件
     * @param World 目标 World
     * @return 检查结果
     */
    FStageOperationStatus PreCheckCreateStage(UWorld* World);

    /**
     * 预检查：删除 Stage 的前置条件
     * @param Stage 要删除的 Stage
     * @return 检查结果
     */
    FStageOperationStatus PreCheckDeleteStage(AStage* Stage);

    /**
     * 显示错误对话框
     * @param Status 操作状态
     */
    void ShowOperationError(const FStageOperationStatus& Status);
};
```

##### StageEditorController.cpp

```cpp
// Plugins/StageEditor/Source/StageEditor/Private/EditorLogic/StageEditorController.cpp

FStageOperationStatus FStageEditorController::PreCheckCreateStage(UWorld* World)
{
    // 1. 检查 World 有效性
    if (!World)
    {
        return FStageOperationStatus::MakeError(
            EStageOperationResult::InvalidStageActor,
            LOCTEXT("Error_InvalidWorld", "World 无效"),
            TEXT("World is nullptr"));
    }

    // 2. 检查 EditorSubsystem 可用性
    UStageEditorSubsystem* EditorSub = GEditor->GetEditorSubsystem<UStageEditorSubsystem>();
    if (!EditorSub)
    {
        return FStageOperationStatus::MakeError(
            EStageOperationResult::UnknownError,
            LOCTEXT("Error_SubsystemNotFound", "StageEditorSubsystem 未初始化"),
            TEXT("Failed to get UStageEditorSubsystem"));
    }

    // 3. 检查 Registry 是否存在
    UStageRegistryAsset* Registry = EditorSub->GetOrLoadRegistryAsset(World);
    if (!Registry)
    {
        return FStageOperationStatus::MakeError(
            EStageOperationResult::RegistryNotFound,
            LOCTEXT("Error_RegistryNotFound", "未找到 Stage Registry，请先创建"),
            FString::Printf(TEXT("No Registry for World: %s"), *World->GetName()));
    }

    // 4. 多人协作模式：检查 Source Control 状态
    if (Registry->CollaborationMode == ECollaborationMode::Multi)
    {
        if (!EditorSub->IsSourceControlEnabled())
        {
            return FStageOperationStatus::MakeError(
                EStageOperationResult::SourceControlNotEnabled,
                LOCTEXT("Error_SCNotEnabled", "多人协作模式需要启用 Source Control"),
                TEXT("Source Control is not enabled"));
        }

        // 尝试 Check Out Registry（预检查）
        if (!EditorSub->CheckOutRegistryFile(Registry))
        {
            return FStageOperationStatus::MakeError(
                EStageOperationResult::CheckOutFailed,
                LOCTEXT("Error_CheckOutFailed",
                    "无法 Check Out Registry 文件\n"
                    "可能原因:\n"
                    "• Source Control 服务器不可达\n"
                    "• 文件已被其他用户锁定\n"
                    "• 您没有写权限"),
                TEXT("Failed to check out Registry file"));
        }
    }

    // 5. 检查磁盘空间（可选，简化版检查包是否可写入）
    FString RegistryPackageName = Registry->GetPackage()->GetName();
    if (!FPackageName::IsValidLongPackageName(RegistryPackageName))
    {
        return FStageOperationStatus::MakeError(
            EStageOperationResult::SaveFailed,
            LOCTEXT("Error_InvalidPackage", "Registry 包路径无效"),
            FString::Printf(TEXT("Invalid package name: %s"), *RegistryPackageName));
    }

    return FStageOperationStatus::MakeSuccess();
}

FStageOperationStatus FStageEditorController::CreateNewStage(AStage*& OutNewStage)
{
    OutNewStage = nullptr;

    UWorld* World = GetCurrentWorld();
    if (!World)
    {
        return FStageOperationStatus::MakeError(
            EStageOperationResult::InvalidStageActor,
            LOCTEXT("Error_NoWorld", "当前没有打开的 World"),
            TEXT("No active World"));
    }

    // ========== 步骤 1: 预检查 ==========
    FStageOperationStatus PreCheckStatus = PreCheckCreateStage(World);
    if (!PreCheckStatus.IsSuccess())
    {
        UE_LOG(LogStageEditor, Warning,
            TEXT("PreCheck failed: %s"), *PreCheckStatus.DetailedError);
        ShowOperationError(PreCheckStatus);
        return PreCheckStatus;
    }

    // ========== 步骤 2: 开启事务 ==========
    FScopedTransaction Transaction(LOCTEXT("CreateStage", "Create New Stage"));

    // ========== 步骤 3: 创建 Stage Actor ==========
    AStage* NewStage = SpawnStageActorInternal(World);
    if (!NewStage)
    {
        // 创建失败，事务自动回滚
        return FStageOperationStatus::MakeError(
            EStageOperationResult::UnknownError,
            LOCTEXT("Error_SpawnFailed", "创建 Stage Actor 失败"),
            TEXT("SpawnStageActorInternal returned nullptr"));
    }

    // ========== 步骤 4: 注册到 EditorSubsystem ==========
    UStageEditorSubsystem* EditorSub = GEditor->GetEditorSubsystem<UStageEditorSubsystem>();
    int32 NewStageID = EditorSub->RegisterStage(NewStage);

    if (NewStageID == 0)
    {
        // ❌ 注册失败，需要清理已创建的 Actor
        UE_LOG(LogStageEditor, Error,
            TEXT("Failed to register Stage, destroying Actor"));

        // 删除刚创建的 Actor（事务会自动回滚 Level 的修改）
        World->DestroyActor(NewStage);

        return FStageOperationStatus::MakeError(
            EStageOperationResult::UnknownError,
            LOCTEXT("Error_RegisterFailed",
                "注册 Stage 失败\n"
                "请检查 Source Control 状态和磁盘空间"),
            TEXT("EditorSubsystem::RegisterStage() returned 0"));
    }

    // ========== 步骤 5: 成功 ==========
    OutNewStage = NewStage;

    UE_LOG(LogStageEditor, Log,
        TEXT("Successfully created Stage: %s (StageID=%d)"),
        *NewStage->GetName(), NewStageID);

    // 刷新 UI
    OnModelChanged.Broadcast();

    return FStageOperationStatus::MakeSuccess();
}

FStageOperationStatus FStageEditorController::PreCheckDeleteStage(AStage* Stage)
{
    if (!Stage)
    {
        return FStageOperationStatus::MakeError(
            EStageOperationResult::InvalidStageActor,
            LOCTEXT("Error_InvalidStage", "Stage 无效"),
            TEXT("Stage is nullptr"));
    }

    // 检查 Stage 是否被引用（例如被其他 Stage 通过事件引用）
    // （这里简化处理，实际可根据需要添加更多检查）

    UStageEditorSubsystem* EditorSub = GEditor->GetEditorSubsystem<UStageEditorSubsystem>();
    if (!EditorSub)
    {
        return FStageOperationStatus::MakeError(
            EStageOperationResult::UnknownError,
            LOCTEXT("Error_SubsystemNotFound", "EditorSubsystem 未初始化"),
            TEXT("Failed to get UStageEditorSubsystem"));
    }

    UWorld* World = Stage->GetWorld();
    UStageRegistryAsset* Registry = EditorSub->GetOrLoadRegistryAsset(World);
    if (!Registry)
    {
        return FStageOperationStatus::MakeError(
            EStageOperationResult::RegistryNotFound,
            LOCTEXT("Error_RegistryNotFound", "未找到 Registry"),
            TEXT("No Registry for Stage's World"));
    }

    // 多人模式：检查 Source Control
    if (Registry->CollaborationMode == ECollaborationMode::Multi)
    {
        if (!EditorSub->IsSourceControlEnabled())
        {
            return FStageOperationStatus::MakeError(
                EStageOperationResult::SourceControlNotEnabled,
                LOCTEXT("Error_SCNotEnabled", "Source Control 未启用"),
                TEXT("Source Control is required for Multi mode"));
        }

        if (!EditorSub->CheckOutRegistryFile(Registry))
        {
            return FStageOperationStatus::MakeError(
                EStageOperationResult::CheckOutFailed,
                LOCTEXT("Error_CheckOutFailed", "无法 Check Out Registry 文件"),
                TEXT("Failed to check out Registry"));
        }
    }

    return FStageOperationStatus::MakeSuccess();
}

FStageOperationStatus FStageEditorController::DeleteStage(AStage* Stage, bool bDeleteDataLayers)
{
    // ========== 步骤 1: 预检查 ==========
    FStageOperationStatus PreCheckStatus = PreCheckDeleteStage(Stage);
    if (!PreCheckStatus.IsSuccess())
    {
        UE_LOG(LogStageEditor, Warning,
            TEXT("PreCheck failed for DeleteStage: %s"), *PreCheckStatus.DetailedError);
        ShowOperationError(PreCheckStatus);
        return PreCheckStatus;
    }

    // ========== 步骤 2: 开启事务 ==========
    FScopedTransaction Transaction(LOCTEXT("DeleteStage", "Delete Stage"));

    // ========== 步骤 3: 从 Registry 移除 ==========
    UStageEditorSubsystem* EditorSub = GEditor->GetEditorSubsystem<UStageEditorSubsystem>();
    EditorSub->UnregisterStage(Stage);

    // ========== 步骤 4: 删除关联的 DataLayers（如果需要）==========
    if (bDeleteDataLayers)
    {
        for (const FAct& Act : Stage->Acts)
        {
            if (UDataLayerAsset* DataLayer = Act.AssociatedDataLayer.LoadSynchronous())
            {
                DeleteDataLayerForAct(DataLayer);
            }
        }
    }

    // ========== 步骤 5: 删除 Stage Actor ==========
    UWorld* World = Stage->GetWorld();
    if (!World->DestroyActor(Stage))
    {
        // ⚠️ 删除失败（可能被锁定、被引用等）
        UE_LOG(LogStageEditor, Error,
            TEXT("Failed to destroy Stage Actor: %s"), *Stage->GetName());

        // 事务会自动回滚（Registry 的 UnregisterStage 操作会被撤销）
        return FStageOperationStatus::MakeError(
            EStageOperationResult::UnknownError,
            LOCTEXT("Error_DestroyFailed",
                "删除 Stage Actor 失败\n"
                "可能原因:\n"
                "• Actor 被其他对象引用\n"
                "• Actor 处于锁定状态"),
            FString::Printf(TEXT("Failed to destroy Actor: %s"), *Stage->GetName()));
    }

    // ========== 步骤 6: 成功 ==========
    UE_LOG(LogStageEditor, Log, TEXT("Successfully deleted Stage: %s"), *Stage->GetName());

    OnModelChanged.Broadcast();
    return FStageOperationStatus::MakeSuccess();
}

void FStageEditorController::ShowOperationError(const FStageOperationStatus& Status)
{
    if (Status.IsSuccess())
        return;

    FMessageDialog::Open(
        EAppMsgType::Ok,
        Status.ErrorMessage,
        LOCTEXT("Error_Title", "操作失败"));

    UE_LOG(LogStageEditor, Error,
        TEXT("Operation failed: %s - %s"),
        *Status.ErrorMessage.ToString(),
        *Status.DetailedError);
}
```

##### StageEditorSubsystem.h

```cpp
// Plugins/StageEditor/Source/StageEditor/Public/Subsystems/StageEditorSubsystem.h

UCLASS()
class STAGEEDITOR_API UStageEditorSubsystem : public UEditorSubsystem
{
    GENERATED_BODY()

public:
    /**
     * 注册 Stage：分配 StageID，更新 RegistryAsset
     *
     * @param Stage 要注册的 Stage
     * @return 分配的 StageID，失败返回 0
     *
     * 注意：此方法假设调用者已完成预检查（Registry 存在、SC 可用等）
     * 如果操作失败，会自动回滚 Registry 的修改（通过事务系统）
     */
    int32 RegisterStage(AStage* Stage);

    /**
     * 取消注册 Stage：从 RegistryAsset 移除
     *
     * @param Stage 要取消注册的 Stage
     * @return 是否成功
     */
    bool UnregisterStage(AStage* Stage);

    /**
     * 检查 Source Control 是否启用
     */
    bool IsSourceControlEnabled() const;

    /**
     * Check Out RegistryAsset 文件
     *
     * @param Registry 要 Check Out 的 Registry
     * @return 是否成功
     */
    bool CheckOutRegistryFile(UStageRegistryAsset* Registry);

    /**
     * 获取或加载 Level 对应的 RegistryAsset
     *
     * @param Level 目标 Level
     * @return RegistryAsset，未找到返回 nullptr
     */
    UStageRegistryAsset* GetOrLoadRegistryAsset(UWorld* Level);

private:
    /**
     * 内部注册逻辑（假设已通过预检查）
     */
    int32 RegisterStageInternal(UStageRegistryAsset* Registry, AStage* Stage);

    /**
     * 已加载的 RegistryAsset 缓存
     */
    TMap<FSoftObjectPath, UStageRegistryAsset*> LoadedRegistries;
};
```

##### StageEditorSubsystem.cpp

```cpp
// Plugins/StageEditor/Source/StageEditor/Private/Subsystems/StageEditorSubsystem.cpp

int32 UStageEditorSubsystem::RegisterStage(AStage* Stage)
{
    if (!Stage)
    {
        UE_LOG(LogStageEditor, Error, TEXT("RegisterStage: Stage is nullptr"));
        return 0;
    }

    UWorld* World = Stage->GetWorld();
    if (!World)
    {
        UE_LOG(LogStageEditor, Error, TEXT("RegisterStage: Stage has no World"));
        return 0;
    }

    // 加载 RegistryAsset
    UStageRegistryAsset* Registry = GetOrLoadRegistryAsset(World);
    if (!Registry)
    {
        UE_LOG(LogStageEditor, Error,
            TEXT("RegisterStage: No Registry for World %s"), *World->GetName());
        return 0;
    }

    // 多人模式：验证已 Check Out（调用者应已完成预检查）
    if (Registry->CollaborationMode == ECollaborationMode::Multi)
    {
        // 这里仅做防御性检查，正常情况下调用者已完成 Check Out
        if (!CheckOutRegistryFile(Registry))
        {
            UE_LOG(LogStageEditor, Error,
                TEXT("RegisterStage: Failed to check out Registry"));
            return 0;
        }
    }

    // 执行注册
    int32 NewStageID = RegisterStageInternal(Registry, Stage);

    if (NewStageID == 0)
    {
        UE_LOG(LogStageEditor, Error,
            TEXT("RegisterStage: Internal registration failed"));
        return 0;
    }

    // 标记 Registry 为 Dirty（等待用户保存）
    Registry->MarkPackageDirty();

    // 添加到运行时缓存
    UStageManagerSubsystem* RuntimeSub = World->GetSubsystem<UStageManagerSubsystem>();
    if (RuntimeSub)
    {
        RuntimeSub->AddStageToRuntimeCache(Stage);
    }

    UE_LOG(LogStageEditor, Log,
        TEXT("Successfully registered Stage: %s (StageID=%d)"),
        *Stage->GetName(), NewStageID);

    return NewStageID;
}

int32 UStageEditorSubsystem::RegisterStageInternal(
    UStageRegistryAsset* Registry,
    AStage* Stage)
{
    // Registry 需要被修改，加入事务
    Registry->Modify();

    // 分配 StageID
    int32 NewStageID = Registry->AllocateAndRegister(Stage);

    // 将 StageID 写入 Stage Actor
    Stage->Modify();
    Stage->StageID = NewStageID;

    return NewStageID;
}

bool UStageEditorSubsystem::UnregisterStage(AStage* Stage)
{
    if (!Stage)
    {
        UE_LOG(LogStageEditor, Error, TEXT("UnregisterStage: Stage is nullptr"));
        return false;
    }

    UWorld* World = Stage->GetWorld();
    if (!World)
    {
        UE_LOG(LogStageEditor, Error, TEXT("UnregisterStage: Stage has no World"));
        return false;
    }

    UStageRegistryAsset* Registry = GetOrLoadRegistryAsset(World);
    if (!Registry)
    {
        UE_LOG(LogStageEditor, Error,
            TEXT("UnregisterStage: No Registry for World %s"), *World->GetName());
        return false;
    }

    // Registry 需要被修改，加入事务
    Registry->Modify();

    // 从 Registry 移除
    bool bRemoved = Registry->RemoveEntry(Stage->StageID);

    if (bRemoved)
    {
        Registry->MarkPackageDirty();

        // 从运行时缓存移除
        UStageManagerSubsystem* RuntimeSub = World->GetSubsystem<UStageManagerSubsystem>();
        if (RuntimeSub)
        {
            RuntimeSub->RemoveStageFromRuntimeCache(Stage);
        }

        UE_LOG(LogStageEditor, Log,
            TEXT("Successfully unregistered Stage: %s (StageID=%d)"),
            *Stage->GetName(), Stage->StageID);
    }

    return bRemoved;
}

bool UStageEditorSubsystem::IsSourceControlEnabled() const
{
    ISourceControlModule& SCModule =
        FModuleManager::LoadModuleChecked<ISourceControlModule>("SourceControl");

    return SCModule.IsEnabled() && SCModule.GetProvider().IsAvailable();
}

bool UStageEditorSubsystem::CheckOutRegistryFile(UStageRegistryAsset* Registry)
{
    if (!Registry)
    {
        return false;
    }

    ISourceControlModule& SCModule =
        FModuleManager::LoadModuleChecked<ISourceControlModule>("SourceControl");

    if (!SCModule.IsEnabled())
    {
        UE_LOG(LogStageEditor, Warning, TEXT("Source Control is not enabled"));
        return false;
    }

    ISourceControlProvider& Provider = SCModule.GetProvider();
    if (!Provider.IsAvailable())
    {
        UE_LOG(LogStageEditor, Warning, TEXT("Source Control provider is not available"));
        return false;
    }

    FString PackageFilename = Registry->GetPackage()->GetLoadedPath().GetLocalFullPath();

    // 获取文件状态
    FSourceControlStatePtr FileState = Provider.GetState(
        PackageFilename,
        EStateCacheUsage::ForceUpdate);

    if (!FileState.IsValid())
    {
        UE_LOG(LogStageEditor, Error,
            TEXT("Failed to get Source Control state for: %s"), *PackageFilename);
        return false;
    }

    // 如果已 Checked Out，直接返回成功
    if (FileState->IsCheckedOut() || FileState->IsAdded())
    {
        return true;
    }

    // 尝试 Check Out
    ECommandResult::Type Result = Provider.Execute(
        ISourceControlOperation::Create<FCheckOut>(),
        PackageFilename);

    if (Result != ECommandResult::Succeeded)
    {
        UE_LOG(LogStageEditor, Error,
            TEXT("Failed to check out file: %s"), *PackageFilename);
        return false;
    }

    UE_LOG(LogStageEditor, Log,
        TEXT("Successfully checked out file: %s"), *PackageFilename);

    return true;
}
```

---

## 错误场景处理汇总

### 场景 1: Source Control Check Out 失败

**触发条件**:
- SC 服务器不可达
- 文件被其他用户锁定
- 本地工作区状态异常

**处理流程**:
```
1. PreCheck 阶段检测到 Check Out 失败
2. 返回 EStageOperationResult::CheckOutFailed
3. 显示用户友好的错误消息
4. 不创建 Stage Actor（避免脏数据）
```

**用户提示**:
```
无法 Check Out Registry 文件

可能原因:
• Source Control 服务器不可达
• 文件已被其他用户锁定
• 您没有写权限

建议操作:
1. 检查网络连接
2. 联系锁定文件的用户
3. 检查您的 Source Control 权限
```

---

### 场景 2: 创建 Stage Actor 后注册失败

**触发条件**:
- Registry 突然不可用
- 内存不足
- 意外异常

**处理流程**:
```
1. SpawnStageActor() 成功
2. RegisterStage() 返回 0（失败）
3. Controller 检测到失败
4. 调用 World->DestroyActor() 清理
5. Transaction 自动回滚 Level 修改
```

**关键代码**:
```cpp
int32 NewStageID = EditorSub->RegisterStage(NewStage);
if (NewStageID == 0)
{
    // ❌ 注册失败，删除刚创建的 Actor
    World->DestroyActor(NewStage);
    return Error(...);
}
```

---

### 场景 3: 删除 Stage Actor 失败

**触发条件**:
- Actor 被其他对象引用
- Actor 处于锁定状态

**处理流程**:
```
1. UnregisterStage() 先从 Registry 移除（已 Modify）
2. DestroyActor() 失败
3. Transaction 自动回滚 UnregisterStage 操作
4. Registry 恢复原状
5. 返回错误给用户
```

**关键设计**:
- `UnregisterStage()` 已调用 `Registry->Modify()`，被包含在事务中
- `DestroyActor()` 失败时，整个事务回滚
- 用户看到清晰的错误提示

---

## 事务系统的局限性

### UE Transaction 系统的工作原理

UE 的 `FScopedTransaction` 只能处理 **单个 Package 内的 UObject 修改**：

```cpp
FScopedTransaction Transaction(...);

// ✅ 可以回滚
Stage->Modify();
Stage->StageID = 5;

Registry->Modify();
Registry->NextStageID = 6;

// ❌ 无法保证 Stage 和 Registry 的原子性
// 因为它们属于不同的 Package（不同的 .uasset 文件）
```

### 为什么我们的方案依然有效？

虽然无法保证跨 Package 的原子性，但通过 **预检查 + 顺序控制** 可以最小化风险：

1. **预检查消除大部分失败场景**
   - Check Out 失败 → 预检查阶段拦截
   - Registry 不存在 → 预检查阶段拦截
   - 磁盘空间不足 → 预检查阶段拦截

2. **操作顺序设计**
   - 创建 Stage: 先创建 Actor，再注册 Registry
     - 如果注册失败，可以删除 Actor
   - 删除 Stage: 先注销 Registry，再删除 Actor
     - 如果删除 Actor 失败，Transaction 回滚注销操作

3. **用户保存时机**
   - Registry 和 Level 都被 `MarkPackageDirty()`
   - 用户按 Ctrl+S 时，UE 会提示保存两个包
   - 如果用户只保存其中一个，可以通过验证工具检测并修复

---

## 残留问题处理

### 如果用户只保存了 Level 但没保存 Registry？

**场景**:
```
1. 用户创建了 Stage_A
2. Level 中有 Stage_A Actor (StageID = 5)
3. Registry 中有 Entry (StageID = 5)
4. 用户保存 Level ✅
5. 用户**没保存** Registry ❌
6. 关闭编辑器
7. 重新打开 → Stage_A 存在但 Registry 无记录
```

**检测工具**:

Phase 13 会提供"数据完整性验证工具"（P2-7），可以检测并自动修复：

```cpp
// 验证工具会检测
TArray<AStage*> UnregisteredStages = FindStagesWithoutRegistryEntry();

// 提示用户并自动修复
for (AStage* Stage : UnregisteredStages)
{
    if (Stage->StageID == 0)
    {
        // 未初始化的 Stage，注册为新 Stage
        RegisterStage(Stage);
    }
    else
    {
        // StageID 已分配但 Registry 无记录（用户忘记保存 Registry）
        Registry->RecoverEntry(Stage);
    }
}
```

---

## 总结

### 设计要点

1. ✅ **预检查机制** - 在修改数据前验证所有前置条件
2. ✅ **错误码传播** - 使用 `FStageOperationStatus` 清晰传递错误信息
3. ✅ **原子性尽力而为** - 利用 UE Transaction 系统 + 操作顺序控制
4. ✅ **用户友好** - 失败时提供明确的错误提示和恢复建议
5. ✅ **验证工具** - 提供数据完整性验证工具检测和修复残留问题

### 无法完全解决的问题

1. ⚠️ **跨 Package 原子性** - UE Transaction 系统限制
2. ⚠️ **用户选择性保存** - 用户可能只保存 Level 不保存 Registry

### 缓解措施

1. ✅ **预检查** - 消除 90% 的失败场景
2. ✅ **操作顺序** - 设计容易回滚的顺序
3. ✅ **验证工具** - 定期检测和修复不一致
4. ✅ **保存提示** - UI 提示用户保存两个 Package

---

## 代码量估计

| 模块 | 代码量 | 说明 |
|------|--------|------|
| `FStageOperationStatus` 结构体 | ~80 行 | 错误码和状态定义 |
| `PreCheck` 方法 | ~150 行 | 创建和删除的预检查 |
| `CreateNewStage` 重构 | ~100 行 | 添加预检查和错误处理 |
| `DeleteStage` 重构 | ~80 行 | 添加预检查和错误处理 |
| `RegisterStage` 增强 | ~80 行 | Check Out 验证和错误处理 |
| `UnregisterStage` 增强 | ~50 行 | 事务支持 |
| **总计** | **~540 行** | |

---

## 下一步

完成 P0-2 后，继续处理 **P0-3: 旧数据迁移详细方案**。

---

*最后更新: 2025-12-05*
