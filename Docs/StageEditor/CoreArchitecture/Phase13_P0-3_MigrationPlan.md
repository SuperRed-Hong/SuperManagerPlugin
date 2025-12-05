# Phase 13 - P0-3: 旧数据迁移详细方案

> **创建日期**: 2025-12-05
> **相关文档**: [Phase13_StageRegistry_Discussion.md](Phase13_StageRegistry_Discussion.md)
> **评审报告**: [Phase13_ExpertReview_Report.md](Phase13_ExpertReview_Report.md)

本文档针对专家评审报告中提出的 P0-3 级问题（旧数据迁移方案不够详细）提供完整的迁移方案。

---

## 问题描述

### 核心问题

当前文档 5.5 节提到"旧数据迁移"，但缺少关键细节：
- ❌ 如何保证迁移完整性？
- ❌ 如何验证迁移结果？
- ❌ 如果迁移失败怎么办？
- ❌ 如何处理 StageID 冲突？

### 迁移场景

**场景描述**:

用户在 Phase 13 实施前已经有一个项目，Level 中已存在多个 Stage Actor：

```
ForestLevel.umap
├── Stage_MainArea (StageID = 1, Acts: 3)
├── Stage_BossRoom (StageID = 2, Acts: 2)
├── Stage_SecretArea (StageID = 0)  ← 未初始化
└── Stage_TestArea (StageID = 1)    ← ID 冲突！
```

**问题**:
1. Stage_SecretArea 的 `StageID = 0`（未初始化）
2. Stage_MainArea 和 Stage_TestArea 有相同的 `StageID = 1`（冲突）
3. 用户不知道哪些 Stage 已正确初始化

---

## 迁移策略设计

### 迁移时机：UI 驱动

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
│ 1. 加载 Registry│ 1. 显示警告条：              │
│                 │    "当前 Level 未初始化       │
│ 2. 验证完整性   │     Stage Registry"           │
│    (P2-7)       │                              │
│                 │ 2. 提供两个选项：             │
│                 │    [创建空 Registry]          │
│                 │    [迁移现有 Stages]          │
└─────────────────┴──────────────────────────────┘
```

### 迁移流程设计

```
用户点击 [迁移现有 Stages]
    ↓
步骤 1: 扫描 Level 中所有 Stage Actor
    ↓
步骤 2: 分析并分类
    ├─ ✅ 已初始化且无冲突
    ├─ ⚠️ 未初始化 (StageID = 0)
    └─ ❌ StageID 冲突
    ↓
步骤 3: 显示迁移预览对话框
    └─ 用户确认迁移计划
    ↓
步骤 4: 执行迁移
    ├─ 创建 RegistryAsset
    ├─ 处理未初始化的 Stage（分配新 ID）
    ├─ 处理冲突的 Stage（重新分配 ID）
    └─ 更新所有 Stage 的 StageID
    ↓
步骤 5: 生成迁移报告
    └─ 显示迁移前后对照表
    ↓
步骤 6: 提示用户保存
    └─ Level + RegistryAsset
```

---

## 详细实现方案

### 数据结构定义

#### 迁移分析结果

```cpp
// Plugins/StageEditor/Source/StageEditor/Public/DataLayerSync/StageMigrationAnalyzer.h

/**
 * Stage 迁移状态分类
 */
UENUM()
enum class EStageMigrationStatus : uint8
{
    /** 已正确初始化，无需修改 */
    Valid,

    /** 未初始化 (StageID = 0)，需要分配新 ID */
    Uninitialized,

    /** StageID 冲突，需要重新分配 ID */
    Conflict,

    /** Stage 数据损坏（极少见） */
    Corrupted
};

/**
 * 单个 Stage 的迁移分析结果
 */
USTRUCT()
struct FStageMigrationAnalysis
{
    GENERATED_BODY()

    /** Stage Actor 引用 */
    UPROPERTY()
    TObjectPtr<AStage> Stage = nullptr;

    /** 当前 StageID */
    UPROPERTY()
    int32 CurrentStageID = 0;

    /** 迁移后的 StageID（如果需要重新分配） */
    UPROPERTY()
    int32 NewStageID = 0;

    /** 迁移状态 */
    UPROPERTY()
    EStageMigrationStatus Status = EStageMigrationStatus::Valid;

    /** 问题描述（如果有） */
    UPROPERTY()
    FString IssueDescription;

    /** 建议的操作 */
    UPROPERTY()
    FString SuggestedAction;
};

/**
 * 整体迁移分析结果
 */
USTRUCT()
struct FMigrationAnalysisResult
{
    GENERATED_BODY()

    /** 所有 Stage 的分析结果 */
    UPROPERTY()
    TArray<FStageMigrationAnalysis> StageAnalyses;

    /** 有效的 Stage 数量 */
    UPROPERTY()
    int32 ValidStageCount = 0;

    /** 未初始化的 Stage 数量 */
    UPROPERTY()
    int32 UninitializedStageCount = 0;

    /** 冲突的 Stage 数量 */
    UPROPERTY()
    int32 ConflictStageCount = 0;

    /** 是否有问题需要处理 */
    bool HasIssues() const
    {
        return UninitializedStageCount > 0 || ConflictStageCount > 0;
    }

    /** 下一个可分配的 StageID */
    UPROPERTY()
    int32 NextAvailableStageID = 1;
};
```

---

### 迁移分析器

```cpp
// Plugins/StageEditor/Source/StageEditor/Public/DataLayerSync/StageMigrationAnalyzer.h

/**
 * Stage 迁移分析器
 * 负责扫描 Level 中的 Stage 并分析迁移需求
 */
class STAGEEDITOR_API FStageMigrationAnalyzer
{
public:
    /**
     * 分析 Level 中的所有 Stage
     *
     * @param World 目标 World
     * @return 分析结果
     */
    static FMigrationAnalysisResult AnalyzeStages(UWorld* World);

private:
    /**
     * 查找 Level 中的所有 Stage Actor
     */
    static TArray<AStage*> FindAllStages(UWorld* World);

    /**
     * 检测 StageID 冲突
     */
    static TMap<int32, TArray<AStage*>> DetectStageIDConflicts(const TArray<AStage*>& Stages);

    /**
     * 分配新 StageID（避免冲突）
     */
    static int32 AllocateNewStageID(
        const TArray<AStage*>& Stages,
        int32& NextID);
};
```

```cpp
// Plugins/StageEditor/Source/StageEditor/Private/DataLayerSync/StageMigrationAnalyzer.cpp

FMigrationAnalysisResult FStageMigrationAnalyzer::AnalyzeStages(UWorld* World)
{
    FMigrationAnalysisResult Result;

    if (!World)
    {
        UE_LOG(LogStageEditor, Error, TEXT("AnalyzeStages: World is nullptr"));
        return Result;
    }

    // 1. 查找所有 Stage Actor
    TArray<AStage*> AllStages = FindAllStages(World);

    if (AllStages.Num() == 0)
    {
        UE_LOG(LogStageEditor, Log, TEXT("No Stages found in World: %s"), *World->GetName());
        return Result;
    }

    UE_LOG(LogStageEditor, Log,
        TEXT("Found %d Stages in World: %s"), AllStages.Num(), *World->GetName());

    // 2. 检测 StageID 冲突
    TMap<int32, TArray<AStage*>> ConflictMap = DetectStageIDConflicts(AllStages);

    // 3. 计算下一个可用的 StageID
    int32 MaxStageID = 0;
    for (AStage* Stage : AllStages)
    {
        if (Stage->StageID > MaxStageID)
        {
            MaxStageID = Stage->StageID;
        }
    }
    Result.NextAvailableStageID = MaxStageID + 1;

    // 4. 分析每个 Stage
    for (AStage* Stage : AllStages)
    {
        FStageMigrationAnalysis Analysis;
        Analysis.Stage = Stage;
        Analysis.CurrentStageID = Stage->StageID;
        Analysis.NewStageID = Stage->StageID;  // 默认保持不变

        // 4.1 检查是否未初始化
        if (Stage->StageID == 0)
        {
            Analysis.Status = EStageMigrationStatus::Uninitialized;
            Analysis.IssueDescription = TEXT("StageID 未初始化");
            Analysis.SuggestedAction = FString::Printf(
                TEXT("将分配新 ID: %d"), Result.NextAvailableStageID);
            Analysis.NewStageID = Result.NextAvailableStageID;
            Result.NextAvailableStageID++;
            Result.UninitializedStageCount++;
        }
        // 4.2 检查是否有冲突
        else if (ConflictMap.Contains(Stage->StageID) &&
                 ConflictMap[Stage->StageID].Num() > 1)
        {
            // 冲突：同一个 ID 被多个 Stage 使用
            TArray<AStage*>& ConflictingStages = ConflictMap[Stage->StageID];

            // 保留第一个，其余重新分配 ID
            if (ConflictingStages[0] == Stage)
            {
                // 这是冲突组中的第一个，保留 ID
                Analysis.Status = EStageMigrationStatus::Valid;
                Analysis.IssueDescription = FString::Printf(
                    TEXT("StageID %d 有冲突（保留此 ID）"), Stage->StageID);
                Analysis.SuggestedAction = TEXT("保持 ID 不变");
            }
            else
            {
                // 这是冲突组中的后续 Stage，重新分配 ID
                Analysis.Status = EStageMigrationStatus::Conflict;
                Analysis.IssueDescription = FString::Printf(
                    TEXT("StageID %d 冲突（与 %s）"),
                    Stage->StageID,
                    *ConflictingStages[0]->GetName());
                Analysis.SuggestedAction = FString::Printf(
                    TEXT("重新分配 ID: %d"), Result.NextAvailableStageID);
                Analysis.NewStageID = Result.NextAvailableStageID;
                Result.NextAvailableStageID++;
                Result.ConflictStageCount++;
            }
        }
        else
        {
            // 4.3 有效的 Stage
            Analysis.Status = EStageMigrationStatus::Valid;
            Analysis.SuggestedAction = TEXT("无需修改");
            Result.ValidStageCount++;
        }

        Result.StageAnalyses.Add(Analysis);
    }

    UE_LOG(LogStageEditor, Log,
        TEXT("Analysis complete: Valid=%d, Uninitialized=%d, Conflict=%d"),
        Result.ValidStageCount,
        Result.UninitializedStageCount,
        Result.ConflictStageCount);

    return Result;
}

TArray<AStage*> FStageMigrationAnalyzer::FindAllStages(UWorld* World)
{
    TArray<AStage*> Result;

    if (!World)
    {
        return Result;
    }

    // 遍历所有 Level（包括 SubLevel）
    for (ULevel* Level : World->GetLevels())
    {
        if (!Level)
            continue;

        // 遍历 Level 中的所有 Actor
        for (AActor* Actor : Level->Actors)
        {
            if (AStage* Stage = Cast<AStage>(Actor))
            {
                Result.Add(Stage);
            }
        }
    }

    return Result;
}

TMap<int32, TArray<AStage*>> FStageMigrationAnalyzer::DetectStageIDConflicts(
    const TArray<AStage*>& Stages)
{
    TMap<int32, TArray<AStage*>> IDToStages;

    for (AStage* Stage : Stages)
    {
        // 跳过未初始化的 Stage（StageID = 0）
        if (Stage->StageID == 0)
            continue;

        IDToStages.FindOrAdd(Stage->StageID).Add(Stage);
    }

    // 只返回有冲突的（同一个 ID 有多个 Stage）
    TMap<int32, TArray<AStage*>> Conflicts;
    for (const auto& Pair : IDToStages)
    {
        if (Pair.Value.Num() > 1)
        {
            Conflicts.Add(Pair.Key, Pair.Value);
            UE_LOG(LogStageEditor, Warning,
                TEXT("StageID %d conflict: %d Stages using the same ID"),
                Pair.Key, Pair.Value.Num());
        }
    }

    return Conflicts;
}
```

---

### 迁移执行器

```cpp
// Plugins/StageEditor/Source/StageEditor/Public/DataLayerSync/StageMigrationExecutor.h

/**
 * Stage 迁移执行器
 * 负责根据分析结果执行迁移操作
 */
class STAGEEDITOR_API FStageMigrationExecutor
{
public:
    /**
     * 执行迁移
     *
     * @param World 目标 World
     * @param AnalysisResult 分析结果
     * @param CollaborationMode 协作模式
     * @return 是否成功
     */
    static bool ExecuteMigration(
        UWorld* World,
        const FMigrationAnalysisResult& AnalysisResult,
        ECollaborationMode CollaborationMode);

private:
    /**
     * 创建 RegistryAsset
     */
    static UStageRegistryAsset* CreateRegistryAsset(
        UWorld* World,
        ECollaborationMode CollaborationMode);

    /**
     * 更新 Stage 的 StageID
     */
    static bool UpdateStageID(AStage* Stage, int32 NewStageID);

    /**
     * 生成迁移报告
     */
    static void GenerateMigrationReport(
        const FMigrationAnalysisResult& AnalysisResult,
        const FString& ReportPath);
};
```

```cpp
// Plugins/StageEditor/Source/StageEditor/Private/DataLayerSync/StageMigrationExecutor.cpp

bool FStageMigrationExecutor::ExecuteMigration(
    UWorld* World,
    const FMigrationAnalysisResult& AnalysisResult,
    ECollaborationMode CollaborationMode)
{
    if (!World)
    {
        UE_LOG(LogStageEditor, Error, TEXT("ExecuteMigration: World is nullptr"));
        return false;
    }

    // 1. 创建 RegistryAsset
    UStageRegistryAsset* Registry = CreateRegistryAsset(World, CollaborationMode);
    if (!Registry)
    {
        UE_LOG(LogStageEditor, Error, TEXT("Failed to create RegistryAsset"));
        return false;
    }

    UE_LOG(LogStageEditor, Log, TEXT("Created RegistryAsset for World: %s"), *World->GetName());

    // 2. 开启事务
    FScopedTransaction Transaction(LOCTEXT("MigrateStages", "Migrate Stages to Registry"));

    Registry->Modify();

    // 3. 设置 NextStageID
    Registry->NextStageID = AnalysisResult.NextAvailableStageID;

    // 4. 处理每个 Stage
    for (const FStageMigrationAnalysis& Analysis : AnalysisResult.StageAnalyses)
    {
        AStage* Stage = Analysis.Stage;
        if (!Stage)
            continue;

        Stage->Modify();

        // 4.1 如果需要更新 StageID
        if (Analysis.NewStageID != Analysis.CurrentStageID)
        {
            Stage->StageID = Analysis.NewStageID;
            UE_LOG(LogStageEditor, Log,
                TEXT("Updated Stage %s: ID %d -> %d"),
                *Stage->GetName(),
                Analysis.CurrentStageID,
                Analysis.NewStageID);
        }

        // 4.2 添加到 Registry
        FStageRegistryEntry Entry;
        Entry.StageID = Stage->StageID;
        Entry.StageActor = Stage;
        Entry.DisplayName = Stage->StageName.IsEmpty() ?
            Stage->GetName() : Stage->StageName;
        Entry.ActCount = Stage->Acts.Num();

        // 查找 Stage DataLayer
        for (const FAct& Act : Stage->Acts)
        {
            if (Act.AssociatedDataLayer.IsValid())
            {
                Entry.StageDataLayer = Act.AssociatedDataLayer;
                break;
            }
        }

        Registry->StageEntries.Add(Entry);

        UE_LOG(LogStageEditor, Log,
            TEXT("Registered Stage: %s (StageID=%d, Acts=%d)"),
            *Entry.DisplayName,
            Entry.StageID,
            Entry.ActCount);
    }

    // 5. 标记为 Dirty
    Registry->MarkPackageDirty();

    // 6. 生成迁移报告
    FString ReportPath = FPaths::ProjectSavedDir() /
        TEXT("StageEditor") /
        TEXT("MigrationReports") /
        FString::Printf(TEXT("%s_Migration_%s.txt"),
            *World->GetName(),
            *FDateTime::Now().ToString());

    GenerateMigrationReport(AnalysisResult, ReportPath);

    UE_LOG(LogStageEditor, Log,
        TEXT("Migration completed successfully. Report saved to: %s"), *ReportPath);

    return true;
}

UStageRegistryAsset* FStageMigrationExecutor::CreateRegistryAsset(
    UWorld* World,
    ECollaborationMode CollaborationMode)
{
    UStageEditorSubsystem* EditorSub = GEditor->GetEditorSubsystem<UStageEditorSubsystem>();
    if (!EditorSub)
    {
        return nullptr;
    }

    // 调用 EditorSubsystem 创建 Registry
    return EditorSub->CreateRegistryAsset(World, CollaborationMode);
}

void FStageMigrationExecutor::GenerateMigrationReport(
    const FMigrationAnalysisResult& AnalysisResult,
    const FString& ReportPath)
{
    FString Report;

    // 标题
    Report += TEXT("========================================\n");
    Report += TEXT("Stage Migration Report\n");
    Report += FString::Printf(TEXT("Date: %s\n"), *FDateTime::Now().ToString());
    Report += TEXT("========================================\n\n");

    // 摘要
    Report += TEXT("Summary:\n");
    Report += FString::Printf(TEXT("  Total Stages: %d\n"),
        AnalysisResult.StageAnalyses.Num());
    Report += FString::Printf(TEXT("  Valid: %d\n"),
        AnalysisResult.ValidStageCount);
    Report += FString::Printf(TEXT("  Uninitialized: %d\n"),
        AnalysisResult.UninitializedStageCount);
    Report += FString::Printf(TEXT("  Conflicts: %d\n"),
        AnalysisResult.ConflictStageCount);
    Report += FString::Printf(TEXT("  Next Available StageID: %d\n\n"),
        AnalysisResult.NextAvailableStageID);

    // 详细信息
    Report += TEXT("Details:\n");
    Report += TEXT("--------\n");

    for (const FStageMigrationAnalysis& Analysis : AnalysisResult.StageAnalyses)
    {
        if (Analysis.Status == EStageMigrationStatus::Valid &&
            Analysis.CurrentStageID == Analysis.NewStageID)
        {
            // 跳过无变化的有效 Stage
            continue;
        }

        Report += FString::Printf(TEXT("\nStage: %s\n"),
            *Analysis.Stage->GetName());
        Report += FString::Printf(TEXT("  Status: %s\n"),
            *UEnum::GetValueAsString(Analysis.Status));
        Report += FString::Printf(TEXT("  Old StageID: %d\n"),
            Analysis.CurrentStageID);
        Report += FString::Printf(TEXT("  New StageID: %d\n"),
            Analysis.NewStageID);

        if (!Analysis.IssueDescription.IsEmpty())
        {
            Report += FString::Printf(TEXT("  Issue: %s\n"),
                *Analysis.IssueDescription);
        }

        Report += FString::Printf(TEXT("  Action: %s\n"),
            *Analysis.SuggestedAction);
    }

    // 保存报告
    FFileHelper::SaveStringToFile(Report, *ReportPath);
}
```

---

### 迁移 UI 对话框

```cpp
// Plugins/StageEditor/Source/StageEditor/Public/DataLayerSync/SStageMigrationDialog.h

/**
 * Stage 迁移预览对话框
 */
class SStageMigrationDialog : public SCompoundWidget
{
public:
    SLATE_BEGIN_ARGS(SStageMigrationDialog) {}
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs, const FMigrationAnalysisResult& InAnalysis);

    /** 用户是否确认迁移 */
    bool ShouldProceedWithMigration() const { return bUserConfirmed; }

    /** 选择的协作模式 */
    ECollaborationMode GetSelectedCollaborationMode() const { return SelectedCollaborationMode; }

private:
    /** 生成内容 */
    TSharedRef<SWidget> GenerateContent();

    /** 生成摘要部分 */
    TSharedRef<SWidget> GenerateSummary();

    /** 生成详细列表 */
    TSharedRef<SWidget> GenerateDetailList();

    /** 确认按钮点击 */
    FReply OnConfirmClicked();

    /** 取消按钮点击 */
    FReply OnCancelClicked();

    /** 导出报告按钮点击 */
    FReply OnExportReportClicked();

private:
    FMigrationAnalysisResult AnalysisResult;
    bool bUserConfirmed = false;
    ECollaborationMode SelectedCollaborationMode = ECollaborationMode::Solo;
};
```

```cpp
// Plugins/StageEditor/Source/StageEditor/Private/DataLayerSync/SStageMigrationDialog.cpp

void SStageMigrationDialog::Construct(
    const FArguments& InArgs,
    const FMigrationAnalysisResult& InAnalysis)
{
    AnalysisResult = InAnalysis;

    ChildSlot
    [
        SNew(SBorder)
        .BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
        .Padding(16.0f)
        [
            SNew(SVerticalBox)

            // 标题
            + SVerticalBox::Slot()
            .AutoHeight()
            .Padding(0, 0, 0, 16)
            [
                SNew(STextBlock)
                .Text(LOCTEXT("MigrationDialog_Title", "迁移现有 Stages 到 Registry"))
                .Font(FCoreStyle::GetDefaultFontStyle("Bold", 14))
            ]

            // 摘要
            + SVerticalBox::Slot()
            .AutoHeight()
            .Padding(0, 0, 0, 12)
            [
                GenerateSummary()
            ]

            // 详细列表
            + SVerticalBox::Slot()
            .FillHeight(1.0f)
            .Padding(0, 0, 0, 12)
            [
                SNew(SBorder)
                .BorderImage(FAppStyle::GetBrush("DetailsView.CategoryTop"))
                .Padding(8.0f)
                [
                    GenerateDetailList()
                ]
            ]

            // 协作模式选择
            + SVerticalBox::Slot()
            .AutoHeight()
            .Padding(0, 8, 0, 12)
            [
                SNew(SHorizontalBox)

                + SHorizontalBox::Slot()
                .AutoWidth()
                .VAlign(VAlign_Center)
                .Padding(0, 0, 8, 0)
                [
                    SNew(STextBlock)
                    .Text(LOCTEXT("CollaborationMode", "协作模式:"))
                ]

                + SHorizontalBox::Slot()
                .AutoWidth()
                [
                    SNew(SComboButton)
                    .OnGetMenuContent(this, &SStageMigrationDialog::GenerateCollaborationModeMenu)
                    .ButtonContent()
                    [
                        SNew(STextBlock)
                        .Text(this, &SStageMigrationDialog::GetCollaborationModeText)
                    ]
                ]
            ]

            // 按钮
            + SVerticalBox::Slot()
            .AutoHeight()
            .HAlign(HAlign_Right)
            [
                SNew(SHorizontalBox)

                // 导出报告按钮
                + SHorizontalBox::Slot()
                .AutoWidth()
                .Padding(0, 0, 8, 0)
                [
                    SNew(SButton)
                    .Text(LOCTEXT("ExportReport", "导出报告"))
                    .OnClicked(this, &SStageMigrationDialog::OnExportReportClicked)
                ]

                // 取消按钮
                + SHorizontalBox::Slot()
                .AutoWidth()
                .Padding(0, 0, 8, 0)
                [
                    SNew(SButton)
                    .Text(LOCTEXT("Cancel", "取消"))
                    .OnClicked(this, &SStageMigrationDialog::OnCancelClicked)
                ]

                // 确认迁移按钮
                + SHorizontalBox::Slot()
                .AutoWidth()
                [
                    SNew(SButton)
                    .Text(LOCTEXT("Migrate", "确认迁移"))
                    .ButtonStyle(FAppStyle::Get(), "PrimaryButton")
                    .OnClicked(this, &SStageMigrationDialog::OnConfirmClicked)
                ]
            ]
        ]
    ];
}

TSharedRef<SWidget> SStageMigrationDialog::GenerateSummary()
{
    return SNew(SVerticalBox)

        // 检测到的 Stages 数量
        + SVerticalBox::Slot()
        .AutoHeight()
        .Padding(0, 4)
        [
            SNew(STextBlock)
            .Text(FText::Format(
                LOCTEXT("TotalStages", "检测到 {0} 个 Stage Actor:"),
                FText::AsNumber(AnalysisResult.StageAnalyses.Num())))
        ]

        // 有效的 Stages
        + SVerticalBox::Slot()
        .AutoHeight()
        .Padding(16, 4, 0, 0)
        [
            SNew(SHorizontalBox)

            + SHorizontalBox::Slot()
            .AutoWidth()
            [
                SNew(STextBlock)
                .Text(LOCTEXT("ValidIcon", "✅"))
                .ColorAndOpacity(FLinearColor::Green)
            ]

            + SHorizontalBox::Slot()
            .AutoWidth()
            .Padding(8, 0, 0, 0)
            [
                SNew(STextBlock)
                .Text(FText::Format(
                    LOCTEXT("ValidStages", "{0} 个已正确初始化"),
                    FText::AsNumber(AnalysisResult.ValidStageCount)))
            ]
        ]

        // 未初始化的 Stages
        + SVerticalBox::Slot()
        .AutoHeight()
        .Padding(16, 4, 0, 0)
        .Visibility(AnalysisResult.UninitializedStageCount > 0 ?
            EVisibility::Visible : EVisibility::Collapsed)
        [
            SNew(SHorizontalBox)

            + SHorizontalBox::Slot()
            .AutoWidth()
            [
                SNew(STextBlock)
                .Text(LOCTEXT("WarningIcon", "⚠️"))
                .ColorAndOpacity(FLinearColor::Yellow)
            ]

            + SHorizontalBox::Slot()
            .AutoWidth()
            .Padding(8, 0, 0, 0)
            [
                SNew(STextBlock)
                .Text(FText::Format(
                    LOCTEXT("UninitializedStages", "{0} 个未初始化 (StageID = 0)"),
                    FText::AsNumber(AnalysisResult.UninitializedStageCount)))
            ]
        ]

        // 冲突的 Stages
        + SVerticalBox::Slot()
        .AutoHeight()
        .Padding(16, 4, 0, 0)
        .Visibility(AnalysisResult.ConflictStageCount > 0 ?
            EVisibility::Visible : EVisibility::Collapsed)
        [
            SNew(SHorizontalBox)

            + SHorizontalBox::Slot()
            .AutoWidth()
            [
                SNew(STextBlock)
                .Text(LOCTEXT("ErrorIcon", "❌"))
                .ColorAndOpacity(FLinearColor::Red)
            ]

            + SHorizontalBox::Slot()
            .AutoWidth()
            .Padding(8, 0, 0, 0)
            [
                SNew(STextBlock)
                .Text(FText::Format(
                    LOCTEXT("ConflictStages", "{0} 个 StageID 冲突"),
                    FText::AsNumber(AnalysisResult.ConflictStageCount)))
            ]
        ];
}

TSharedRef<SWidget> SStageMigrationDialog::GenerateDetailList()
{
    // 使用 ListView 显示所有 Stages
    // （实现略，类似于 Phase 5 的导入预览对话框）

    return SNew(SListView<TSharedPtr<FStageMigrationAnalysis>>)
        .ListItemsSource(&AnalysisListItems)
        .OnGenerateRow(this, &SStageMigrationDialog::OnGenerateRow)
        .HeaderRow(
            SNew(SHeaderRow)
            + SHeaderRow::Column("StageName")
              .DefaultLabel(LOCTEXT("ColumnStageName", "Stage 名称"))
              .FillWidth(0.3f)
            + SHeaderRow::Column("CurrentID")
              .DefaultLabel(LOCTEXT("ColumnCurrentID", "当前 ID"))
              .FillWidth(0.15f)
            + SHeaderRow::Column("NewID")
              .DefaultLabel(LOCTEXT("ColumnNewID", "新 ID"))
              .FillWidth(0.15f)
            + SHeaderRow::Column("Status")
              .DefaultLabel(LOCTEXT("ColumnStatus", "状态"))
              .FillWidth(0.2f)
            + SHeaderRow::Column("Action")
              .DefaultLabel(LOCTEXT("ColumnAction", "操作"))
              .FillWidth(0.2f)
        );
}

FReply SStageMigrationDialog::OnConfirmClicked()
{
    bUserConfirmed = true;

    // 关闭对话框
    TSharedPtr<SWindow> Window = FSlateApplication::Get().FindWidgetWindow(AsShared());
    if (Window.IsValid())
    {
        Window->RequestDestroyWindow();
    }

    return FReply::Handled();
}

FReply SStageMigrationDialog::OnCancelClicked()
{
    bUserConfirmed = false;

    // 关闭对话框
    TSharedPtr<SWindow> Window = FSlateApplication::Get().FindWidgetWindow(AsShared());
    if (Window.IsValid())
    {
        Window->RequestDestroyWindow();
    }

    return FReply::Handled();
}

FReply SStageMigrationDialog::OnExportReportClicked()
{
    // 导出分析报告到文件
    FString ReportPath = FPaths::ProjectSavedDir() /
        TEXT("StageEditor") /
        TEXT("MigrationReports") /
        FString::Printf(TEXT("Analysis_%s.txt"),
            *FDateTime::Now().ToString());

    FStageMigrationExecutor::GenerateMigrationReport(AnalysisResult, ReportPath);

    // 提示用户
    FMessageDialog::Open(
        EAppMsgType::Ok,
        FText::Format(
            LOCTEXT("ReportExported", "报告已导出到:\n{0}"),
            FText::FromString(ReportPath)));

    return FReply::Handled();
}
```

---

## 集成到 StageEditorPanel

### StageEditorPanel 检测逻辑

```cpp
// Plugins/StageEditor/Source/StageEditor/Private/EditorUI/StageEditorPanel.cpp

void SStageEditorPanel::Construct(const FArguments& InArgs)
{
    // ... 现有代码 ...

    // 检查 Registry 是否存在
    CheckAndPromptRegistryCreation();
}

void SStageEditorPanel::CheckAndPromptRegistryCreation()
{
    UWorld* World = GetCurrentWorld();
    if (!World)
        return;

    UStageEditorSubsystem* EditorSub = GEditor->GetEditorSubsystem<UStageEditorSubsystem>();
    if (!EditorSub)
        return;

    // 检查是否已有 Registry
    UStageRegistryAsset* Registry = EditorSub->GetOrLoadRegistryAsset(World);
    if (Registry)
    {
        // 已有 Registry，验证完整性（P2-7）
        // ValidateRegistryIntegrity(Registry);
        return;
    }

    // ========== 没有 Registry：显示警告条 ==========

    // 检查是否有现有的 Stage Actor
    TArray<AStage*> ExistingStages = FindAllStagesInWorld(World);

    if (ExistingStages.Num() > 0)
    {
        // 有现有 Stages → 提示迁移
        ShowMigrationPrompt(ExistingStages.Num());
    }
    else
    {
        // 没有现有 Stages → 提示创建空 Registry
        ShowCreateEmptyRegistryPrompt();
    }
}

void SStageEditorPanel::ShowMigrationPrompt(int32 StageCount)
{
    // 在 UI 顶部显示警告条
    WarningBarContent = SNew(SBorder)
        .BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
        .BorderBackgroundColor(FLinearColor(1.0f, 0.8f, 0.0f, 1.0f))  // 黄色警告
        .Padding(12.0f)
        [
            SNew(SHorizontalBox)

            // 警告图标
            + SHorizontalBox::Slot()
            .AutoWidth()
            .VAlign(VAlign_Center)
            .Padding(0, 0, 12, 0)
            [
                SNew(STextBlock)
                .Text(LOCTEXT("WarningIcon", "⚠️"))
                .Font(FCoreStyle::GetDefaultFontStyle("Regular", 24))
            ]

            // 警告消息
            + SHorizontalBox::Slot()
            .FillWidth(1.0f)
            .VAlign(VAlign_Center)
            [
                SNew(SVerticalBox)

                + SVerticalBox::Slot()
                .AutoHeight()
                [
                    SNew(STextBlock)
                    .Text(LOCTEXT("MigrationWarning_Title",
                        "当前 Level 未初始化 Stage Registry"))
                    .Font(FCoreStyle::GetDefaultFontStyle("Bold", 12))
                ]

                + SVerticalBox::Slot()
                .AutoHeight()
                .Padding(0, 4, 0, 0)
                [
                    SNew(STextBlock)
                    .Text(FText::Format(
                        LOCTEXT("MigrationWarning_Message",
                            "检测到 {0} 个现有 Stage Actor。您可以将它们迁移到新的 Registry 系统。"),
                        FText::AsNumber(StageCount)))
                ]
            ]

            // 按钮区
            + SHorizontalBox::Slot()
            .AutoWidth()
            .VAlign(VAlign_Center)
            .Padding(12, 0, 0, 0)
            [
                SNew(SHorizontalBox)

                // 迁移按钮
                + SHorizontalBox::Slot()
                .AutoWidth()
                .Padding(0, 0, 8, 0)
                [
                    SNew(SButton)
                    .Text(LOCTEXT("MigrateStages", "迁移现有 Stages"))
                    .ButtonStyle(FAppStyle::Get(), "PrimaryButton")
                    .OnClicked(this, &SStageEditorPanel::OnMigrateStagesClicked)
                ]

                // 创建空 Registry 按钮
                + SHorizontalBox::Slot()
                .AutoWidth()
                [
                    SNew(SButton)
                    .Text(LOCTEXT("CreateEmptyRegistry", "创建空 Registry"))
                    .OnClicked(this, &SStageEditorPanel::OnCreateEmptyRegistryClicked)
                ]
            ]
        ];

    // 刷新 UI
    RefreshUI();
}

FReply SStageEditorPanel::OnMigrateStagesClicked()
{
    UWorld* World = GetCurrentWorld();
    if (!World)
        return FReply::Handled();

    // 1. 执行分析
    FMigrationAnalysisResult AnalysisResult = FStageMigrationAnalyzer::AnalyzeStages(World);

    // 2. 显示迁移预览对话框
    TSharedRef<SStageMigrationDialog> MigrationDialog =
        SNew(SStageMigrationDialog, AnalysisResult);

    TSharedRef<SWindow> Window = SNew(SWindow)
        .Title(LOCTEXT("MigrationDialog_WindowTitle", "迁移 Stages 到 Registry"))
        .SizingRule(ESizingRule::UserSized)
        .ClientSize(FVector2D(800, 600))
        .Content()
        [
            MigrationDialog
        ];

    FSlateApplication::Get().AddModalWindow(Window, nullptr);

    // 3. 用户确认后执行迁移
    if (MigrationDialog->ShouldProceedWithMigration())
    {
        ECollaborationMode Mode = MigrationDialog->GetSelectedCollaborationMode();

        bool bSuccess = FStageMigrationExecutor::ExecuteMigration(
            World,
            AnalysisResult,
            Mode);

        if (bSuccess)
        {
            // 迁移成功，隐藏警告条，刷新 UI
            WarningBarContent.Reset();
            RefreshUI();

            FMessageDialog::Open(
                EAppMsgType::Ok,
                LOCTEXT("MigrationSuccess",
                    "迁移成功！\n\n"
                    "请保存以下文件:\n"
                    "• Level 文件（包含更新的 Stage Actors）\n"
                    "• Registry 资产\n\n"
                    "迁移报告已保存到 Saved/StageEditor/MigrationReports/"));
        }
        else
        {
            FMessageDialog::Open(
                EAppMsgType::Ok,
                LOCTEXT("MigrationFailed", "迁移失败，请检查日志"));
        }
    }

    return FReply::Handled();
}
```

---

## 迁移验证工具

### 迁移后完整性检查

```cpp
// Plugins/StageEditor/Source/StageEditor/Public/DataLayerSync/RegistryIntegrityValidator.h

/**
 * Registry 完整性验证器
 */
class STAGEEDITOR_API FRegistryIntegrityValidator
{
public:
    /**
     * 验证 Registry 与 Level 中 Stage 的一致性
     *
     * @param Registry Registry 资产
     * @param World 目标 World
     * @return 发现的问题列表
     */
    static TArray<FString> ValidateRegistryIntegrity(
        UStageRegistryAsset* Registry,
        UWorld* World);
};
```

```cpp
// Plugins/StageEditor/Source/StageEditor/Private/DataLayerSync/RegistryIntegrityValidator.cpp

TArray<FString> FRegistryIntegrityValidator::ValidateRegistryIntegrity(
    UStageRegistryAsset* Registry,
    UWorld* World)
{
    TArray<FString> Issues;

    if (!Registry || !World)
    {
        Issues.Add(TEXT("Registry 或 World 无效"));
        return Issues;
    }

    // 1. 查找 World 中的所有 Stage
    TArray<AStage*> AllStages = FStageMigrationAnalyzer::FindAllStages(World);

    // 2. 验证：Registry 中的每个条目都能找到对应的 Actor
    for (const FStageRegistryEntry& Entry : Registry->StageEntries)
    {
        AStage* Stage = Entry.StageActor.LoadSynchronous();
        if (!Stage)
        {
            Issues.Add(FString::Printf(
                TEXT("Registry 记录了 StageID=%d (%s)，但 Actor 不存在"),
                Entry.StageID,
                *Entry.DisplayName));
        }
        else if (Stage->StageID != Entry.StageID)
        {
            Issues.Add(FString::Printf(
                TEXT("StageID 不匹配: Registry 记录=%d, Actor 实际=%d (%s)"),
                Entry.StageID,
                Stage->StageID,
                *Stage->GetName()));
        }
    }

    // 3. 验证：World 中的每个 Stage 都在 Registry 中
    for (AStage* Stage : AllStages)
    {
        if (Stage->StageID == 0)
        {
            Issues.Add(FString::Printf(
                TEXT("发现未注册的 Stage Actor: %s (StageID=0)"),
                *Stage->GetName()));
            continue;
        }

        const FStageRegistryEntry* Entry = Registry->FindEntry(Stage->StageID);
        if (!Entry)
        {
            Issues.Add(FString::Printf(
                TEXT("Stage Actor %s (StageID=%d) 未在 Registry 中注册"),
                *Stage->GetName(),
                Stage->StageID));
        }
    }

    // 4. 验证：没有重复的 StageID
    TSet<int32> SeenIDs;
    for (const FStageRegistryEntry& Entry : Registry->StageEntries)
    {
        if (SeenIDs.Contains(Entry.StageID))
        {
            Issues.Add(FString::Printf(
                TEXT("Registry 中有重复的 StageID: %d"),
                Entry.StageID));
        }
        SeenIDs.Add(Entry.StageID);
    }

    return Issues;
}
```

---

## 迁移测试计划

### 测试场景

#### 场景 1: 全新 Level（无现有 Stages）

**操作**:
1. 创建新 Level
2. 打开 StageEditorPanel
3. 点击 [创建空 Registry]

**预期结果**:
- ✅ Registry 创建成功
- ✅ NextStageID = 1
- ✅ StageEntries 为空

---

#### 场景 2: 已有 5 个有效 Stages

**初始状态**:
```
Stage_A (StageID = 1)
Stage_B (StageID = 2)
Stage_C (StageID = 3)
Stage_D (StageID = 4)
Stage_E (StageID = 5)
```

**操作**:
1. 打开 Level
2. StageEditorPanel 检测到无 Registry
3. 点击 [迁移现有 Stages]
4. 查看迁移预览对话框
5. 确认迁移

**预期结果**:
- ✅ 分析结果：5 个有效 Stage，0 个未初始化，0 个冲突
- ✅ NextStageID = 6
- ✅ Registry 包含 5 个条目
- ✅ 所有 Stage 的 StageID 保持不变

---

#### 场景 3: 有未初始化的 Stages

**初始状态**:
```
Stage_A (StageID = 1)
Stage_B (StageID = 0)  ← 未初始化
Stage_C (StageID = 0)  ← 未初始化
```

**操作**:
1. 打开 Level
2. 点击 [迁移现有 Stages]
3. 查看预览对话框

**预期结果**:
- ✅ 分析结果：1 个有效，2 个未初始化，0 个冲突
- ✅ Stage_B 将分配 ID = 2
- ✅ Stage_C 将分配 ID = 3
- ✅ NextStageID = 4

---

#### 场景 4: 有 StageID 冲突

**初始状态**:
```
Stage_A (StageID = 1)
Stage_B (StageID = 2)
Stage_C (StageID = 1)  ← 冲突
```

**操作**:
1. 打开 Level
2. 点击 [迁移现有 Stages]
3. 查看预览对话框

**预期结果**:
- ✅ 分析结果：1 个有效，0 个未初始化，1 个冲突
- ✅ Stage_A 保持 ID = 1（第一个）
- ✅ Stage_C 将重新分配 ID = 3（冲突，重新分配）
- ✅ Stage_B 保持 ID = 2
- ✅ NextStageID = 4

---

#### 场景 5: 用户取消迁移

**操作**:
1. 打开 Level
2. 点击 [迁移现有 Stages]
3. 查看预览
4. 点击 [取消]

**预期结果**:
- ✅ 不创建 Registry
- ✅ 警告条继续显示
- ✅ 所有 Stage 保持不变

---

## 迁移报告示例

```
========================================
Stage Migration Report
Date: 2025-12-05 14:30:45
========================================

Summary:
  Total Stages: 5
  Valid: 2
  Uninitialized: 2
  Conflicts: 1
  Next Available StageID: 6

Details:
--------

Stage: Stage_A
  Status: Valid
  Old StageID: 1
  New StageID: 1
  Action: 无需修改

Stage: Stage_B
  Status: Uninitialized
  Old StageID: 0
  New StageID: 3
  Issue: StageID 未初始化
  Action: 将分配新 ID: 3

Stage: Stage_C
  Status: Uninitialized
  Old StageID: 0
  New StageID: 4
  Issue: StageID 未初始化
  Action: 将分配新 ID: 4

Stage: Stage_D
  Status: Valid
  Old StageID: 2
  New StageID: 2
  Action: 无需修改

Stage: Stage_E
  Status: Conflict
  Old StageID: 1
  New StageID: 5
  Issue: StageID 1 冲突（与 Stage_A）
  Action: 重新分配 ID: 5
```

---

## 错误处理

### 迁移失败场景

#### 场景 1: 创建 Registry 失败

**原因**:
- Source Control 未启用（多人模式）
- 磁盘空间不足
- 权限不足

**处理**:
```cpp
UStageRegistryAsset* Registry = CreateRegistryAsset(World, Mode);
if (!Registry)
{
    FMessageDialog::Open(
        EAppMsgType::Ok,
        LOCTEXT("Error_RegistryCreationFailed",
            "创建 Registry 失败\n\n"
            "可能原因:\n"
            "• Source Control 未启用（多人模式需要）\n"
            "• 磁盘空间不足\n"
            "• 权限不足\n\n"
            "请检查后重试"));
    return false;
}
```

---

#### 场景 2: Stage Actor 无效

**原因**:
- Stage Actor 已被删除
- 引用损坏

**处理**:
```cpp
for (const FStageMigrationAnalysis& Analysis : AnalysisResult.StageAnalyses)
{
    AStage* Stage = Analysis.Stage;
    if (!Stage)
    {
        UE_LOG(LogStageEditor, Warning,
            TEXT("Skipping invalid Stage reference during migration"));
        continue;  // 跳过，继续处理其他 Stage
    }

    // 处理...
}
```

---

## 总结

### 设计要点

1. ✅ **用户驱动的迁移** - 通过 UI 警告条提示用户
2. ✅ **完整的分析预览** - 用户在迁移前可以看到详细的分析结果
3. ✅ **冲突自动解决** - 自动检测并解决 StageID 冲突
4. ✅ **可验证性** - 生成迁移报告，提供验证工具
5. ✅ **可回滚性** - 使用 UE Transaction 系统，支持 Undo

### 迁移流程总览

```
检测无 Registry
    ↓
显示警告条（两个选项：迁移 / 创建空）
    ↓
用户点击 [迁移现有 Stages]
    ↓
扫描并分析所有 Stage
    ↓
显示迁移预览对话框
    ├─ 摘要（有效/未初始化/冲突）
    ├─ 详细列表（每个 Stage 的状态和操作）
    └─ 协作模式选择
    ↓
用户确认
    ↓
执行迁移（事务保护）
    ├─ 创建 RegistryAsset
    ├─ 更新 Stage 的 StageID
    └─ 添加到 Registry
    ↓
生成迁移报告
    ↓
提示用户保存
```

### 代码量估计

| 模块 | 代码量 | 说明 |
|------|--------|------|
| `FStageMigrationAnalyzer` | ~300 行 | 分析器 |
| `FStageMigrationExecutor` | ~250 行 | 执行器 |
| `SStageMigrationDialog` | ~400 行 | UI 对话框 |
| `SStageEditorPanel` 集成 | ~200 行 | 警告条和触发逻辑 |
| `FRegistryIntegrityValidator` | ~150 行 | 验证工具 |
| **总计** | **~1300 行** | |

---

## 下一步

完成 P0-3 后，更新 **Phase13_StageRegistry_Discussion.md 第 8 节实施计划**。

---

*最后更新: 2025-12-05*
