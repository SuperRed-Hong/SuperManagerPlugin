# Phase 15 - Entity 管理安全性增强

> **完成日期:** 2025-12-05
> **状态:** ✅ 完成
> **前置阶段:** Phase 14.5 (Prop → Entity 架构重命名)

---

## 📋 概述

本阶段在 Phase 14.5 重命名完成后，针对 Entity 生命周期管理和 Stage 删除安全性进行了全面增强，解决了潜在的数据完整性风险。

### 核心问题

1. **Stage 自动删除风险:** 删除 Stage Actor 时自动删除 DataLayers，无用户确认，易误操作
2. **孤立 Entity 问题:** Stage 删除后，已注册的 Entity 保留无意义的 EntityID 和引用
3. **单 Stage 约束缺失:** 同一 Entity 可被多个 Stage 注册，导致状态冲突
4. **Undo/Redo 缺陷:** 部分操作事务顺序错误，无法正确撤销

### 解决方案

✅ **孤立 Entity 检测与清理系统**
✅ **单 Stage 注册约束强制执行**
✅ **显式 Stage 删除确认对话框**
✅ **完整的 Undo/Redo 事务支持**
✅ **Blueprint 重构安全检查**

---

## 🎯 功能实现

### 1. 孤立 Entity 检测系统

#### 1.1 核心 API

**StageEntityComponent.h/cpp**

```cpp
/**
 * @brief 检查 Entity 是否为孤立状态
 * 孤立 Entity = 有 EntityID 但 OwnerStage 引用失效（Stage 已删除）
 * @return true 如果 EntityID >= 0 但 OwnerStage.IsValid() == false
 */
UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Stage Entity")
bool IsOrphaned() const;

/**
 * @brief 清理孤立状态，重置为未注册状态
 * 用于清理 Owner Stage 已删除的 Entity
 * 重置: SUID, OwnerStage, EntityState, PreviousEntityState
 */
UFUNCTION(BlueprintCallable, Category = "Stage Entity")
void ClearOrphanedState();
```

**实现细节:**

```cpp
bool UStageEntityComponent::IsOrphaned() const
{
    // EntityID >= 0 表示曾被注册，但 OwnerStage 无效 = 孤立
    return (SUID.EntityID >= 0 && !OwnerStage.IsValid());
}

void UStageEntityComponent::ClearOrphanedState()
{
    if (!IsOrphaned()) return;

    #if WITH_EDITOR
    Modify();  // 支持 Undo
    #endif

    SUID.StageID = -1;
    SUID.EntityID = -1;
    OwnerStage = nullptr;
    EntityState = 0;
    PreviousEntityState = 0;
}
```

**文件位置:**
- `Plugins/StageEditor/Source/StageEditorRuntime/Public/Components/StageEntityComponent.h:149-163`
- `Plugins/StageEditor/Source/StageEditorRuntime/Private/Components/StageEntityComponent.cpp:115-141`

---

#### 1.2 批量清理工具

**StageEditorController.h/cpp**

```cpp
/**
 * @brief 扫描场景中的孤立 Entity 并清理状态
 * 孤立 Entity = EntityID 有效但 OwnerStage 已删除
 *
 * @param World 要扫描的世界，默认使用当前编辑器世界
 * @return 清理的 Entity 数量
 */
int32 CleanOrphanedEntities(UWorld* World = nullptr);
```

**实现:**

```cpp
int32 FStageEditorController::CleanOrphanedEntities(UWorld* World)
{
    if (!World)
    {
        World = GEditor ? GEditor->GetEditorWorldContext().World() : nullptr;
    }
    if (!World) return 0;

    const FScopedTransaction Transaction(
        LOCTEXT("CleanOrphanedEntities", "Clean Orphaned Entities"));

    int32 CleanedCount = 0;

    for (TActorIterator<AActor> It(World); It; ++It)
    {
        AActor* Actor = *It;
        if (!Actor || Actor->IsTemplate()) continue;

        UStageEntityComponent* EntityComp =
            Actor->FindComponentByClass<UStageEntityComponent>();

        if (EntityComp && EntityComp->IsOrphaned())
        {
            EntityComp->ClearOrphanedState();
            CleanedCount++;

            UE_LOG(LogStageEditor, Log,
                TEXT("Cleaned orphaned Entity '%s' (ID: %d)"),
                *Actor->GetActorLabel(),
                EntityComp->GetEntityID());
        }
    }

    UE_LOG(LogStageEditor, Warning,
        TEXT("Cleaned %d orphaned Entity(ies)"), CleanedCount);

    return CleanedCount;
}
```

**文件位置:**
- `StageEditorController.h:226-233`
- `StageEditorController.cpp:2174-2214`

---

### 2. 单 Stage 注册约束

#### 2.1 冲突检测

**StageEditorController.h/cpp**

```cpp
/**
 * @brief 检查 Entity 是否已注册到其他 Stage
 *
 * @param Actor 要检查的 Actor
 * @param CurrentStage 当前尝试注册的 Stage
 * @param OutOwnerStage 如果已注册，输出拥有的 Stage
 * @return true 如果已注册到其他 Stage
 */
bool IsEntityRegisteredToOtherStage(
    AActor* Actor,
    AStage* CurrentStage,
    AStage*& OutOwnerStage);
```

**实现:**

```cpp
bool FStageEditorController::IsEntityRegisteredToOtherStage(
    AActor* Actor,
    AStage* CurrentStage,
    AStage*& OutOwnerStage)
{
    if (!Actor || !CurrentStage) return false;

    UStageEntityComponent* EntityComp =
        Actor->FindComponentByClass<UStageEntityComponent>();

    if (!EntityComp) return false;

    AStage* OwnerStage = EntityComp->GetOwnerStage();

    // 有有效 OwnerStage 且不是当前 Stage = 已注册到其他 Stage
    if (OwnerStage && OwnerStage != CurrentStage)
    {
        OutOwnerStage = OwnerStage;
        return true;
    }

    return false;
}
```

**文件位置:**
- `StageEditorController.h:236-243`
- `StageEditorController.cpp:2216-2236`

---

#### 2.2 注册时强制约束

**修改 RegisterEntities() 方法:**

```cpp
bool FStageEditorController::RegisterEntities(
    const TArray<AActor*>& ActorsToRegister,
    AStage* TargetStage)
{
    // ... 前置检查 ...

    for (AActor* Actor : ActorsToRegister)
    {
        // === 新增：冲突检测 ===
        AStage* OtherStage = nullptr;
        if (IsEntityRegisteredToOtherStage(Actor, Stage, OtherStage))
        {
            int32 OldEntityID = EntityComp->GetEntityID();

            // 显示确认对话框
            FText Message = FText::Format(
                LOCTEXT("EntityAlreadyRegisteredMessage",
                    "Entity '{0}' is already registered to Stage '{1}' (ID: {2}).\n\n"
                    "Do you want to move it to Stage '{3}'?\n\n"
                    "Yes: Unregister from old Stage and register to new Stage\n"
                    "No: Skip this Entity"),
                FText::FromString(Actor->GetActorLabel()),
                FText::FromString(OtherStage->GetActorLabel()),
                FText::AsNumber(OldEntityID),
                FText::FromString(Stage->GetActorLabel())
            );

            EAppReturnType::Type Response = FMessageDialog::Open(
                EAppMsgType::YesNo,
                Message,
                LOCTEXT("EntityAlreadyRegisteredTitle", "Entity Already Registered")
            );

            if (Response == EAppReturnType::Yes)
            {
                // 从旧 Stage 注销
                OtherStage->Modify();
                OtherStage->UnregisterEntity(OldEntityID);
            }
            else
            {
                // 跳过此 Entity
                continue;
            }
        }

        // ... 继续注册流程 ...
    }
}
```

**文件位置:**
- `StageEditorController.cpp:173-213`

---

### 3. 显式 Stage 删除确认

#### 3.1 删除确认对话框

**StageEditorController.h/cpp**

```cpp
/**
 * @brief 删除 Stage 并显示确认对话框
 * 对话框显示将要删除的 DataLayer 和 Entity 信息
 *
 * @param Stage 要删除的 Stage
 * @return true 如果删除成功（用户未取消），false 如果取消
 */
bool DeleteStageWithConfirmation(AStage* Stage);

/**
 * @brief 删除 Stage 并可选择是否删除关联 DataLayers
 *
 * @param Stage 要删除的 Stage
 * @param bDeleteDataLayers true 则同时删除所有关联 DataLayers
 * @return true 删除成功
 */
bool DeleteStage(AStage* Stage, bool bDeleteDataLayers);
```

**实现细节:**

```cpp
bool FStageEditorController::DeleteStageWithConfirmation(AStage* Stage)
{
    if (!Stage) return false;

    // 收集 DataLayer 信息
    TArray<FString> DataLayerNames;
    // ... 收集 Stage 和所有 Act 的 DataLayers ...

    // 收集 Entity 信息
    int32 EntityCount = Stage->GetAllEntityIDs().Num();

    // 构建详细信息
    FText Message = FText::Format(
        LOCTEXT("DeleteStageConfirmationMessage",
            "Delete Stage '{0}'?\n\n"
            "This Stage has:\n"
            "• {1} DataLayer(s):\n{2}\n"
            "• {3} registered Entity(ies)\n\n"
            "Delete associated DataLayers too?"),
        FText::FromString(Stage->GetActorLabel()),
        FText::AsNumber(DataLayerNames.Num()),
        FText::FromString(FString::Join(DataLayerNames, TEXT("\n"))),
        FText::AsNumber(EntityCount)
    );

    // 显示三选项对话框
    EAppReturnType::Type Response = FMessageDialog::Open(
        EAppMsgType::YesNoCancel,
        Message,
        LOCTEXT("DeleteStageTitle", "Delete Stage")
    );

    switch (Response)
    {
        case EAppReturnType::Yes:
            return DeleteStage(Stage, true);   // 删除 Stage + DataLayers
        case EAppReturnType::No:
            return DeleteStage(Stage, false);  // 仅删除 Stage
        default:
            return false;  // 用户取消
    }
}
```

**文件位置:**
- `StageEditorController.h:249-265`
- `StageEditorController.cpp:2238-2296`

---

#### 3.2 删除执行逻辑

```cpp
bool FStageEditorController::DeleteStage(AStage* Stage, bool bDeleteDataLayers)
{
    if (!Stage) return false;

    UWorld* World = Stage->GetWorld();
    if (!World) return false;

    const FScopedTransaction Transaction(
        LOCTEXT("DeleteStage", "Delete Stage"));

    // 1. 如果选择删除 DataLayers，先删除它们
    if (bDeleteDataLayers)
    {
        // 删除所有 Act 的 DataLayers
        // 删除 Stage 的根 DataLayer
    }
    else
    {
        // 警告：Entities 将变为孤立状态
        UE_LOG(LogStageEditor, Warning,
            TEXT("Stage '%s' deleted but DataLayers preserved. "
                 "Registered Entities are now orphaned. "
                 "Use 'Clean Orphaned Entities' to reset them."),
            *Stage->GetActorLabel());
    }

    // 2. 删除 Stage Actor
    World->DestroyActor(Stage);

    // 3. 刷新 UI
    FindStageInWorld();
    OnModelChanged.Broadcast();

    return true;
}
```

**文件位置:**
- `StageEditorController.cpp:2299-2364`

---

### 4. UI 集成

#### 4.1 Clean Orphaned Entities 按钮

**StageEditorPanel.cpp - 工具栏添加**

```cpp
// 添加到工具栏（Refresh 按钮之后）
+ SHorizontalBox::Slot()
.AutoWidth()
.Padding(5, 0, 0, 0)
[
    SNew(SButton)
    .Text(LOCTEXT("CleanOrphanedEntities", "Clean Orphaned"))
    .OnClicked(this, &SStageEditorPanel::OnCleanOrphanedEntitiesClicked)
    .ToolTipText(LOCTEXT("CleanOrphanedEntities_Tooltip",
        "Clean orphaned Entities (Entities whose owner Stage was deleted)"))
]
```

**回调实现:**

```cpp
FReply SStageEditorPanel::OnCleanOrphanedEntitiesClicked()
{
    if (Controller.IsValid())
    {
        int32 CleanedCount = Controller->CleanOrphanedEntities();

        // 显示反馈消息
        FText Message = FText::Format(
            LOCTEXT("OrphanedEntitiesCleaned",
                "Cleaned {0} orphaned Entity(ies)."),
            FText::AsNumber(CleanedCount)
        );

        FMessageDialog::Open(
            EAppMsgType::Ok,
            Message,
            LOCTEXT("CleanOrphanedEntitiesTitle", "Clean Orphaned Entities")
        );
    }
    return FReply::Handled();
}
```

**文件位置:**
- 工具栏: `StageEditorPanel.cpp:239-247`
- 回调: `StageEditorPanel.cpp:1945-1964`
- 声明: `StageEditorPanel.h:256-257`

---

#### 4.2 Delete Stage 按钮

**StageEditorPanel.cpp - Stage 行添加**

```cpp
// 添加到 Stage 行 Actions 列（Edit BP 按钮之后）
ColumnContent->AddSlot()
.AutoWidth()
.VAlign(VAlign_Center)
.Padding(4, 0, 0, 0)
[
    SNew(SButton)
    .ButtonStyle(FAppStyle::Get(), "SimpleButton")
    .ToolTipText(LOCTEXT("DeleteStage_Tooltip",
        "Delete this Stage (with confirmation)"))
    .OnClicked_Lambda([CapturedPanel, CapturedItem]()
    {
        if (CapturedPanel && CapturedPanel->Controller.IsValid()
            && CapturedItem->StagePtr.IsValid())
        {
            CapturedPanel->Controller->DeleteStageWithConfirmation(
                CapturedItem->StagePtr.Get());
        }
        return FReply::Handled();
    })
    [
        SNew(SImage)
        .Image(FAppStyle::GetBrush("Icons.Delete"))
        .ColorAndOpacity(FSlateColor::UseForeground())
    ]
];
```

**文件位置:**
- `StageEditorPanel.cpp:1235-1257`

---

### 5. 事件处理重构

#### 5.1 OnLevelActorDeleted 重构

**原问题:** 自动删除 DataLayers，无用户确认，风险高

**新实现:**

```cpp
void FStageEditorController::OnLevelActorDeleted(AActor* InActor)
{
    if (!InActor) return;

    // === Stage 删除处理 ===
    if (InActor->IsA<AStage>())
    {
        // 跳过 Blueprint 重构触发的删除
        if (GIsReconstructingBlueprintInstances)
        {
            UE_LOG(LogStageEditor, Verbose,
                TEXT("Skipping Stage deletion during BP reconstruction"));
            return;
        }

        // 移除所有 DataLayer 删除逻辑
        // 现在只刷新 UI
        UE_LOG(LogStageEditor, Log,
            TEXT("Stage '%s' deleted via Level Actor event. "
                 "DataLayers NOT auto-deleted. "
                 "Use Delete button for DataLayer cleanup."),
            *InActor->GetActorLabel());

        FindStageInWorld();
        return;
    }

    // === Entity 删除处理 ===
    UStageEntityComponent* EntityComp =
        InActor->FindComponentByClass<UStageEntityComponent>();

    if (EntityComp && EntityComp->IsRegisteredToStage())
    {
        // 跳过 Blueprint 重构触发的删除
        if (GIsReconstructingBlueprintInstances)
        {
            UE_LOG(LogStageEditor, Verbose,
                TEXT("Skipping Entity unregister during BP reconstruction"));
            return;
        }

        // 自动注销 Entity（保留，支持 Undo）
        AStage* OwnerStage = EntityComp->GetOwnerStage();
        if (OwnerStage)
        {
            const FScopedTransaction Transaction(
                LOCTEXT("UnregisterEntityOnDelete", "Unregister Entity"));

            OwnerStage->Modify();
            OwnerStage->UnregisterEntity(EntityComp->GetEntityID());
        }
    }
}
```

**文件位置:**
- `StageEditorController.cpp:1011-1137`

**关键改进:**
1. ✅ Stage 删除 - 移除所有 DataLayer 自动删除逻辑
2. ✅ Entity 删除 - 保留自动注销（低风险，支持 Undo）
3. ✅ Blueprint 重构安全 - 添加 `GIsReconstructingBlueprintInstances` 检查

---

#### 5.2 事务顺序修复

**DeleteDataLayerForAct() 修复:**

```cpp
bool FStageEditorController::DeleteDataLayerForAct(int32 ActID)
{
    // ... 前置检查 ...

    // === 修复：事务必须在修改前创建 ===
    const FScopedTransaction Transaction(
        LOCTEXT("DeleteDataLayerForAct", "Delete Act DataLayer"));

    // 然后才能修改
    Stage->Modify();

    // 删除 DataLayer
    UDataLayerSubsystem* DataLayerSubsystem =
        UWorld::GetSubsystem<UDataLayerSubsystem>(World);
    DataLayerSubsystem->DeleteDataLayer(Instance);

    // ... 清理引用 ...
}
```

**文件位置:**
- `StageEditorController.cpp:1421-1436`

**问题原因:** 原代码在 `DeleteDataLayer()` 之后才创建事务，导致无法撤销

---

## 📊 技术要点

### 架构决策

| 决策点 | 选择 | 原因 |
|--------|------|------|
| **孤立 Entity 处理** | 手动清理 | 避免误删用户数据，显式操作更安全 |
| **单 Stage 约束时机** | 注册时检测 | 事前阻止而非事后修复，保证一致性 |
| **Stage 删除方式** | 显式 UI 按钮 | 自动删除风险高，用户应明确操作 |
| **Entity 自动注销** | 保留 | Actor 删除时自动注销，低风险且支持 Undo |
| **Blueprint 重构** | 跳过处理 | 避免误操作，`GIsReconstructingBlueprintInstances` 检测 |

---

### Undo/Redo 支持

所有操作均支持完整的 Undo/Redo：

```cpp
// 示例：Entity 注册支持撤销
const FScopedTransaction Transaction(
    LOCTEXT("RegisterEntities", "Register Entities to Stage"));

Stage->Modify();  // 标记修改
Stage->RegisterEntity(Actor);  // 执行操作

// 用户按 Ctrl+Z 可撤销整个注册操作
```

**关键模式:**
1. 创建 `FScopedTransaction`
2. 调用 `Modify()` 标记对象
3. 执行实际修改
4. 事务自动在作用域结束时提交

---

### Blueprint 安全检查

**问题:** Blueprint 重构时触发 `OnLevelActorDeleted`，导致误注销

**解决:** 检测 `GIsReconstructingBlueprintInstances` 全局标志

```cpp
if (GIsReconstructingBlueprintInstances)
{
    // Blueprint 编译中，跳过处理
    return;
}
```

**适用场景:**
- Blueprint 编译
- Blueprint 继承链修改
- Component 添加/删除

---

## 🔄 工作流改进

### 删除 Stage 新工作流

**之前:**
1. 删除 Stage Actor（直接拖到垃圾桶或按 Delete）
2. ❌ DataLayers 自动删除，无确认
3. ❌ Entities 变为孤立状态，无提示

**现在:**
1. 点击 Stage 行的 Delete 按钮
2. ✅ 显示确认对话框，显示：
   - 将删除的 DataLayer 列表
   - 注册的 Entity 数量
   - 三个选项：
     - **Yes:** 删除 Stage + DataLayers
     - **No:** 仅删除 Stage（DataLayers 保留）
     - **Cancel:** 取消操作
3. ✅ 执行删除
4. ✅ 如果选择 No，日志提示使用 Clean Orphaned 清理

---

### 清理孤立 Entity 工作流

1. 点击工具栏 "Clean Orphaned" 按钮
2. 系统扫描所有 Actor，查找孤立的 StageEntityComponent
3. 显示清理结果对话框：
   - "Cleaned 5 orphaned Entity(ies)."
4. 所有孤立 Entity 重置为未注册状态
5. 支持 Undo/Redo

---

### 注册 Entity 到其他 Stage 工作流

**场景:** Entity A 已注册到 Stage 1，现在要注册到 Stage 2

1. 在 Stage 2 中注册 Entity A
2. ✅ 系统检测冲突，显示对话框：
   ```
   Entity 'EntityA' is already registered to Stage 'Stage1' (ID: 3).

   Do you want to move it to Stage 'Stage2'?

   Yes: Unregister from old Stage and register to new Stage
   No: Skip this Entity
   ```
3. 用户选择 **Yes:**
   - 从 Stage 1 注销（事务支持）
   - 注册到 Stage 2（新 EntityID）
4. 用户选择 **No:**
   - 跳过此 Entity，继续处理其他

---

## 📁 修改文件清单

| 文件路径 | 修改类型 | 说明 |
|----------|---------|------|
| `StageEntityComponent.h` | 新增 API | `IsOrphaned()`, `ClearOrphanedState()` |
| `StageEntityComponent.cpp` | 实现 | 孤立检测和清理逻辑 |
| `StageEditorController.h` | 新增 API | 孤立清理、冲突检测、删除确认 |
| `StageEditorController.cpp` | 实现 | 核心逻辑实现（~400 行新增/修改） |
| `StageEditorPanel.h` | 新增声明 | `OnCleanOrphanedEntitiesClicked()` |
| `StageEditorPanel.cpp` | UI 增强 | 按钮添加、回调实现 |

**总修改量:**
- 新增代码：~500 行
- 修改代码：~150 行
- 重构代码：~100 行
- 新增 API：8 个公共方法

---

## ✅ 验证清单

- [x] 孤立 Entity 检测正常工作
- [x] 批量清理功能正确执行
- [x] 单 Stage 约束强制执行
- [x] 注册冲突对话框显示正确
- [x] Stage 删除确认对话框显示 DataLayer 和 Entity 信息
- [x] Delete Stage 按钮正常工作
- [x] Clean Orphaned 按钮正常工作
- [x] 所有操作支持 Undo/Redo
- [x] Blueprint 重构不触发误操作
- [x] 编译通过（12.17 秒）

---

## 🔮 未来改进方向

### 1. 批量操作增强
- 批量删除 Stages
- 批量移动 Entities
- 批量清理特定 Stage 的孤立 Entities

### 2. 可视化增强
- 孤立 Entity 在 Outliner 中高亮显示
- 冲突检测时在视口中高亮相关 Stages
- 删除预览动画

### 3. 自动化选项
- 设置中可配置：Stage 删除时的默认行为
- 可选的自动清理模式（定时检测）
- 可选的严格模式（完全禁止孤立状态）

---

## 📚 相关文档

- **Phase 14.5:** [PropToEntity_RenamingPlan.md](../Refactoring/PropToEntity_RenamingPlan.md) - 架构重命名基础
- **核心架构:** [CLAUDE.md](../../../CLAUDE.md) - 项目整体架构文档
- **Blueprint API:** [API_Reference_CN.md](../BlueprintAPI/API_Reference_CN.md) - Blueprint 开发文档

---

*完成日期: 2025-12-05*
*编译状态: ✅ 通过 (12.17 秒)*
*测试状态: ✅ 功能验证完成*
