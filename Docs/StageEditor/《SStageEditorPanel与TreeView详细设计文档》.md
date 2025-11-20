# 《SStageEditorPanel与TreeView详细设计文档》

---

## **《SStageEditorPanel与TreeView详细设计文档》**

**文档版本:** 1.0

**作者:** Gemini (UE Slate UI Programmer)

**关联文档:**

- 《“关卡编辑器”系统需求说明书 (PRD) V2.1》
- 《“关卡编辑器”系统概要设计文档 V4.1》
- 《ID核心数据结构与本地索引机制详细设计文档 V1.1》
- 《“关卡编辑器”系统：AStage Actor数据结构与核心逻辑详细设计 V4.2》
- 《FStageEditorController详细设计文档 V1.0》

---

### 1. 引言

本文档旨在为“动态舞台”系统的核心编辑器界面 `SStageEditorPanel` 提供详细的技术设计方案。作为MVC设计模式中的**视图 (View)**，`SStageEditorPanel` 的核心职责是：

1. **数据呈现**：清晰、高效地可视化当前编辑的 `AStage` Actor（模型）中的数据，主要是 `Act` 列表及其关联的 `Prop` 状态。
2. **用户交互**：提供一套完整的UI控件（按钮、右键菜单、数值输入框等），捕捉策划（用户）的所有操作意图。
3. **操作转发**：将所有用户操作严格地转发给**控制器** (`FStageEditorController`) 进行处理，自身不包含任何业务逻辑。
4. **响应更新**：订阅控制器的模型变更通知，并以最高效的方式响应式地更新UI，确保视图与模型数据的实时同步。

本设计的核心是围绕一个 `STreeView` 控件构建，它将以层级结构直观地展示 `Act` 与 `Prop` 之间的关系。

### 2. 整体UI布局设计

`SStageEditorPanel` 将采用一个标准的编辑器面板布局，由一个垂直框 `SVerticalBox` 构成，从上至下依次包含工具栏、搜索栏和核心的树状视图。

**Slate伪代码布局:**

```
// SStageEditorPanel::Construct
SNew(SVerticalBox)

// 1. 工具栏 (Toolbar)
+ SVerticalBox::Slot()
.AutoHeight()
[
    SNew(SHorizontalBox)
    + SHorizontalBox::Slot().AutoWidth() [
        // 调用Controller的CreateNewAct
        SNew(SButton)
        .Text(LOCTEXT("CreateActButton", "新建 Act"))
        .OnClicked(this, &SStageEditorPanel::OnCreateActClicked)
    ]
    + SHorizontalBox::Slot().AutoWidth() [
        // 调用Controller的DeleteAct
        SNew(SButton)
        .Text(LOCTEXT("DeleteActButton", "删除选中 Act"))
        .OnClicked(this, &SStageEditorPanel::OnDeleteActClicked)
        .IsEnabled(this, &SStageEditorPanel::IsActSelected)
    ]
    // ... 其他全局操作按钮
]

// 2. 搜索/过滤栏 (Filter Bar)
+ SVerticalBox::Slot()
.AutoHeight()
[
    SAssignNew(SearchBox, SSearchBox)
    .OnTextChanged(this, &SStageEditorPanel::OnFilterTextChanged)
]

// 3. 核心TreeView
+ SVerticalBox::Slot()
.FillHeight(1.0f)
[
    SAssignNew(this->TreeView, STreeView<TSharedPtr<FStageTreeItem>>)
        .TreeItemsSource(&this->TreeRootItems) // ViewModel数据源
        .OnGenerateRow(this, &SStageEditorPanel::OnGenerateRowForTreeView)
        .OnGetChildren(this, &SStageEditorPanel::OnGetChildrenForTreeView)
        .HeaderRow
        (
            SNew(SHeaderRow)
            + SHeaderRow::Column("Name")
                .DefaultLabel(LOCTEXT("NameColumn", "名称"))
                .FillWidth(0.7f)
            + SHeaderRow::Column("PropState")
                .DefaultLabel(LOCTEXT("PropStateColumn", "状态值"))
                .FillWidth(0.3f)
        )
]

```

### 3. TreeView的ViewModel (FStageTreeItem)详细设计

为了将UI展示与底层数据模型 (`FAct`, `AProp`) 解耦，我们设计一个专门的ViewModel `FStageTreeItem`。TreeView绑定的数据源是 `TArray<TSharedPtr<FStageTreeItem>>`，而不是直接引用模型数据。

**C++结构体定义:**

```
// 定义树状视图条目的类型
enum class EStageTreeItemType
{
    Act,           // 代表一个Act
    PropInAct      // 代表某个Act下的一个Prop状态定义
};

// TreeView中每个条目的数据模型
struct FStageTreeItem : public TSharedFromThis<FStageTreeItem>
{
public:
    // 条目类型
    EStageTreeItemType Type;

    // 条目的唯一ID
    // 对于Act, ID.ActID有效
    // 对于PropInAct, ID.PropID和ParentActID.ActID均有效
    FStageHierarchicalId ID;

    // 当类型为PropInAct时, 记录其所属的Act的ID
    FStageHierarchicalId ParentActID;

    // 用于UI显示的名称
    FString DisplayName;

    // 子节点列表
    TArray<TSharedPtr<FStageTreeItem>> Children;

    // 对控制器的弱引用, 用于从行Widget直接调用API
    TWeakPtr<FStageEditorController> Controller;

public:
    // 工厂方法, 用于创建不同类型的条目
    static TSharedRef<FStageTreeItem> CreateAct(const FAct& InAct, TWeakPtr<FStageEditorController> InController);
    static TSharedRef<FStageTreeItem> CreatePropInAct(const FStageHierarchicalId& PropID, const FString& PropName, const FStageHierarchicalId& InParentActID, TWeakPtr<FStageEditorController> InController);
};

```

**设计解析:**

- **`Type`**: 明确区分了条目的种类，这是 `OnGenerateRow` 逻辑的核心依据。
- **`ID`** 和 `ParentActID**`: 提供了调用Controller API时所需的全部身份信息，如 `SetPropStateForAct(ParentActID, ID, NewValue)`。
- **`DisplayName`**: 缓存了用于UI显示的文本，避免在UI刷新时频繁查询。
- **`Controller`**: 允许行控件 (Row Widget) 自身封装交互逻辑，直接、安全地调用控制器接口。

### 4. 核心视图 (STreeView)实现方案

### 4.1 数据源填充

`SStageEditorPanel` 将持有一个 `TArray<TSharedPtr<FStageTreeItem>> TreeRootItems` 作为TreeView的数据源。当接收到 `FullRefresh` 或 `ActList` 类型的模型变更通知时，将执行以下逻辑来重新构建这个数组：

1. 清空 `TreeRootItems`。
2. 从 `FStageEditorController` 获取当前的 `AStage` Actor 引用。
3. 遍历 `AStage->Acts` 数组，为每一个 `FAct` 创建一个 `FStageTreeItem::CreateAct` 实例，并添加到 `TreeRootItems` 中。

### 4.2 OnGetChildren 委托

此委托的逻辑非常简单：

```
void SStageEditorPanel::OnGetChildrenForTreeView(TSharedPtr<FStageTreeItem> InItem, TArray<TSharedPtr<FStageTreeItem>>& OutChildren)
{
    OutChildren = InItem->Children;
}

```

当一个 `Act` 类型的条目被展开时，我们会动态为其填充 `Children` 数组，包含其下的所有 `PropInAct` 条目。

### 4.3 OnGenerateRow 委托 (核心逻辑)

这是UI构建的核心，它根据 `FStageTreeItem` 的类型动态生成不同的行控件。

```
TSharedRef<ITableRow> SStageEditorPanel::OnGenerateRowForTreeView(TSharedPtr<FStageTreeItem> InItem, const TSharedRef<STableViewBase>& OwnerTable)
{
    switch (InItem->Type)
    {
        case EStageTreeItemType::Act:
            // Act条目: 只在"Name"列显示Act名称
            return SNew(STableRow<TSharedPtr<FStageTreeItem>>, OwnerTable)
            [
                SNew(STextBlock).Text(FText::FromString(InItem->DisplayName))
            ];

        case EStageTreeItemType::PropInAct:
            // PropInAct条目: 显示两列, 名称和可编辑的状态值
            return SNew(STableRow<TSharedPtr<FStageTreeItem>>, OwnerTable)
            .Content()
            [
                SNew(SHorizontalBox)
                + SHorizontalBox::Slot().FillWidth(0.7f)
                [
                    SNew(STextBlock).Text(FText::FromString(InItem->DisplayName))
                ]
                + SHorizontalBox::Slot().FillWidth(0.3f)
                [
                    // 内联一个数字输入框
                    SNew(SNumericEntryBox<int32>)
                    .Value(this, &SStageEditorPanel::GetPropStateValue, InItem) // 获取当前值
                    .OnValueCommitted(this, &SStageEditorPanel::OnPropStateValueCommitted, InItem) // 提交修改
                ]
            ];
    }
    return SNew(STableRow<TSharedPtr<FStageTreeItem>>, OwnerTable);
}

```

- **`GetPropStateValue`**: 这个函数会通过 `Controller` 查询模型，获取指定 `ActID` 和 `PropID` 对应的当前 `PropState` 值。
- **`OnPropStateValueCommitted`**: 当用户修改完 `SNumericEntryBox` 的值并提交后，此函数被调用。它会立即调用 `Controller->SetPropStateForAct(InItem->ParentActID, InItem->ID, NewValue)`。

### 5. View与Controller的交互机制

### 5.1 输入 (View -> Controller): 委托绑定

UI控件的用户交互事件将通过C++委托直接绑定到 `SStageEditorPanel` 的成员函数上，这些函数再调用 `FStageEditorController` 提供的公共API。

| UI 控件 / 事件 | 绑定的处理函数 | 调用的Controller API |
| --- | --- | --- |
| "新建 Act" 按钮 `OnClicked` | `OnCreateActClicked` | `Controller->CreateNewAct()` |
| "删除选中 Act" 按钮 `OnClicked` | `OnDeleteActClicked` | `Controller->DeleteAct(SelectedActID)` |
| `SNumericEntryBox` `OnValueCommitted` | `OnPropStateValueCommitted` | `Controller->SetPropStateForAct(ActID, PropID, NewValue)` |
| TreeView `OnDrop` (从World Outliner拖入) | `OnActorDroppedOnTreeView` | `Controller->RegisterProps(DroppedActors)` |
| TreeView右键菜单 "注销Prop" | `OnUnregisterPropClicked` | `Controller->UnregisterProps(SelectedPropIDs)` |

### 5.2 输出 (Controller -> View): 模型变更通知

这是实现UI响应式更新的关键。

1. **订阅**: 在 `SStageEditorPanel::Construct` 方法中，我们将一个处理函数绑定到控制器的 `FOnStageModelChanged` 委托上。
    
    ```
    // ControllerPtr 是持有的FStageEditorController的TSharedPtr
    ControllerPtr->OnModelChanged.AddSP(this, &SStageEditorPanel::HandleModelChanged);
    
    ```
    
2. **处理**: `HandleModelChanged` 函数接收 `EStageModelUpdateType` 参数，并据此决定最高效的刷新策略。
    
    ```
    void SStageEditorPanel::HandleModelChanged(EStageModelUpdateType UpdateType)
    {
        switch (UpdateType)
        {
            case EStageModelUpdateType::FullRefresh:
                // 通常在切换编辑的AStage时发生
                // 1. 从Controller获取新Model的数据
                // 2. 完全重建TreeRootItems数组
                // 3. 调用TreeView->RequestTreeRefresh()
                RebuildTree(true);
                break;
    
            case EStageModelUpdateType::ActList:
                // Act被创建或删除
                // 1. 仅重新构建顶层的TreeRootItems (Act列表)
                // 2. 调用TreeView->RequestTreeRefresh()
                RebuildTree(false); // false表示仅刷新顶层
                break;
    
            case EStageModelUpdateType::PropList:
                // Prop被注册或注销
                // 1. 找到所有受影响的Act条目
                // 2. 清空并重建其Children数组
                // 3. 调用TreeView->RequestTreeRefresh()
                RefreshAffectedActChildren();
                break;
    
            case EStageModelUpdateType::PropState:
                // 这是一个高频操作, 避免完全刷新
                // 理想情况下, Controller的委托会附带更改的ID
                // 但根据当前设计, View需要自行处理
                // 注意: STreeView没有原生API刷新单个单元格,
                // RequestTreeRefresh()是官方推荐的最直接方式, Slate会优化掉未变化的部分。
                // 因此, 此处行为与PropList一致, 但逻辑上是增量更新。
                TreeView->RequestTreeRefresh();
                break;
        }
    }
    
    ```
    

通过这种机制，`SStageEditorPanel` 实现了与 `FStageEditorController` 的清晰解耦。UI只负责展示和转发，而控制器负责处理逻辑和数据变更，最终通过委托通知UI进行刷新，形成了一个完整的单向数据流闭环。