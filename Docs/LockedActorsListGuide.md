# Locked Actors List Implementation Guide

> 目标：在 UE 5.6 Editor 插件里实现一个可以过滤、锁定、双击聚焦的 Actor 列表（含 `SHeaderRow`），便于掌握完整的 Slate/模块对接流程。

---

## 1. Slate Widget 骨架

- **数据模型**：在 `LockedActorsListWidget.h` 中声明
  ```cpp
  struct FLockedActorListItem
  {
      TWeakObjectPtr<AActor> Actor;
      bool bIsLocked = false;
  };
  enum class ELockedActorsViewMode : uint8 { AllActors, LockedOnly };
  ```
- **Widget 壳子**：`SLockedActorsListTab : public SCompoundWidget`
  - `SLATE_ARGUMENT(TArray<TSharedPtr<FLockedActorListItem>>, InitialItems)`
  - `SLATE_EVENT(FOnSetActorLockState, OnSetActorLockState)`
  - `SLATE_EVENT(FOnUnlockAllActors, OnUnlockAllActors)`
  - `SLATE_EVENT(FOnRequestLockedActorRows, OnRequestRefreshData)`
  - `SLATE_EVENT(FOnLockedActorRowDoubleClicked, OnRowDoubleClicked)`
  - 对外暴露 `HighlightActorRow()`/`RequestRefresh()` 方便模块操控
  - 内部字段：`SourceActors`、`DisplayedActors`、`TSharedPtr<SListView<...>>`、`FilterOptions` 等

## 2. Header Row + Multi-Column Row

- **头文件引用**（放 cpp 内部即可）：
  ```cpp
  #include "Widgets/Views/SHeaderRow.h"
  #include "Widgets/Views/STableRow.h" // SMultiColumnTableRow 就定义在这里
  ```
- **列标识**：
  ```cpp
  namespace LockedActorsListColumns
  {
      static const FName LockColumn(TEXT("Lock"));
      static const FName ActorColumn(TEXT("Actor"));
      static const FName ClassColumn(TEXT("Class"));
  }
  ```
- **自定义 Row**：
  ```cpp
  class SLockedActorsListRow : public SMultiColumnTableRow<TSharedPtr<FLockedActorListItem>>
  {
  public:
      SLATE_BEGIN_ARGS(SLockedActorsListRow) {}
          SLATE_ARGUMENT(TSharedPtr<FLockedActorListItem>, Item)
          SLATE_ARGUMENT(TWeakPtr<SLockedActorsListTab>, OwnerWidget)
      SLATE_END_ARGS()
  ```
  - `GenerateWidgetForColumn` 里判断 `ColumnName`，分别返回：
    - `Lock` 列：`SBorder` + `SCheckBox`（用 `FSlateRoundedBoxBrush` 画圆角背景，`OnCheckStateChanged` 调回 `SLockedActorsListTab`）
    - `Actor` 列：`STextBlock` + `BoldFont` + 浅灰色，显示 `Actor->GetActorLabel()`
    - `Class` 列：`STextBlock` + `ItalicFont` + 蓝灰色，显示 `Actor->GetClass()->GetName()`

- **构建 ListView + Header**：
  ```cpp
  SAssignNew(ActorsListView, SListView<TSharedPtr<FLockedActorListItem>>)
      .ListItemsSource(&DisplayedActors)
      .SelectionMode(ESelectionMode::Single)
      .OnGenerateRow(this, &SLockedActorsListTab::OnGenerateRow)
      .OnSelectionChanged(this, &SLockedActorsListTab::HandleListSelectionChanged)
      .OnMouseButtonDoubleClick(this, &SLockedActorsListTab::HandleRowDoubleClicked)
      .HeaderRow
      (
          SNew(SHeaderRow)
          .Style(&FAppStyle::Get().GetWidgetStyle<FHeaderRowStyle>("TableView.Header"))
          + SHeaderRow::Column(LockedActorsListColumns::LockColumn)
            .DefaultLabel(LOCTEXT("HeaderLocked", "锁定"))
            .FixedWidth(90.f)
            .HAlignCell(HAlign_Center)
          + SHeaderRow::Column(LockedActorsListColumns::ActorColumn)
            .DefaultLabel(LOCTEXT("HeaderActor", "Actor Label"))
            .FillWidth(0.55f)
          + SHeaderRow::Column(LockedActorsListColumns::ClassColumn)
            .DefaultLabel(LOCTEXT("HeaderClass", "Class Name"))
            .FillWidth(0.45f)
      );
  ```
- **Row 生成**：`return SNew(SLockedActorsListRow, OwnerTable).Item(Item).OwnerWidget(SharedThis(this));`

## 3. 过滤、搜索与焦点联动

- **组合过滤**：`FilterOptions` + `ApplyFiltersAndSorting()`，结合 `CurrentFilterOption` 控制是否仅显示锁定 Actor。
- **搜索框**：`SSearchBox` 绑定 `OnSearchTextChanged`，`DoesItemMatchSearch()` 同时匹配 Actor Label 与 Class Name。
- **排序**：在 `SHeaderRow::Column` 中配置 `.SortMode/.OnSort`，用 `ActiveSortColumn`、`bSortAscending` 驱动 `DisplayedActors.Sort(...)`。
- **批量锁定**：在锁定列的 Head 中放 `SCheckBox`，`OnHeaderLockCheckStateChanged` 调用 `SetActorLockDelegate` 批量锁/解锁，并在 `UpdateHeaderLockCheckState()` 中反映三种状态。
- **危险操作确认**：`HandleUnlockAllButtonClicked` 在执行前调用 `FMessageDialog::Open(EAppMsgType::YesNo, ...)` 进行提示，防止误操作。
- **高亮 Actor**：
  - `HighlightedActorPtr` 保存当前高亮
  - `HandleListSelectionChanged` / `HighlightActorRow` 更新它
  - `SyncSelectionToHighlightedActor()` 遍历 `DisplayedActors` 找到对应行 → `ActorsListView->SetSelection()` + `RequestScrollIntoView`
- **双击行为**：`HandleRowDoubleClicked` 调用对外委托 → 模块完成场景选择与镜头移动；Widget 自己也调用 `HighlightActorRow()` 保持 UI 状态一致。

## 4. 模块对接

文件：`Plugins/SuperManager/Source/SuperManager/Public/SuperManager.h/.cpp`

1. **缓存 Widget 指针**：`TWeakPtr<SLockedActorsListTab> LockedActorsListWidget;`
   ```cpp
   LockedActorsListWidget = LockedActorsWidget;
   ```
2. **新增辅助函数**：
   ```cpp
   void HighlightLockedActorRow(TWeakObjectPtr<AActor> ActorPtr);
   void RefreshLockedActorsWidget();
   ```
3. **委托绑定**（`OnSpawnLockedActorsTab`）：
   ```cpp
   .OnSetActorLockState(...)
   .OnUnlockAllActors(...)
   .OnRequestRefreshData(...)
   .OnRowDoubleClicked(FOnLockedActorRowDoubleClicked::CreateRaw(this, &FSuperManagerModule::HandleLockedActorRowDoubleClicked));
   ```
4. **双击回调**：`HandleLockedActorRowDoubleClicked`
   - `GEditor->SelectNone(false, true);`
   - `WeakEditorActorSubsystem->SetActorSelectionState(Actor, true);`
   - `GEditor->SelectActor(...)`, `GEditor->MoveViewportCamerasToActor(*Actor, false);`
   - `HighlightLockedActorRow(Actor);`
5. **状态同步**：
   - `LockActorSelection`/`UnlockActorSelection` 内调用 `RefreshLockedActorsWidget()`，任何蓝图/脚本修改都能立即刷新 UI。
   - 批量锁/解锁后同样刷新。
   - `OnActorSelected` 如果发现选中的是锁定 Actor，调用 `HighlightLockedActorRow(SelectedActor)`，即使 Editor Selection 被清空，列表也能高亮该行。

## 5. 额外细节 / UI 风格

- 圆角卡片：将配色集中到 `LockedActorsListStyle` 命名空间（玻璃卡、Toolbar、控件、Accent 色），然后用 `SBorder + FSlateRoundedBoxBrush` 组合即可画出 Fluent 风“玻璃卡片”。Toolbar 左侧放置 `SSearchBox` + 过滤下拉，右侧用 `SSpacer` 分隔批量操作按钮，并附带 `SThrobber` 显示刷新状态。
- 列内容前景色采用略淡的灰/蓝，字体用 `FAppStyle::GetFontStyle("BoldFont")`、`"ItalicFont"`。
- `SButton`/`SComboBox` 直接沿用 `AppStyle` 默认风格即可，与 Editor 主 UI 保持一致。
- 交互回路：`bCanSupportFocus = true;` 保证 ComboBox/按钮能接收键盘输入；`SelectionMode` 用 `Single` 才能触发高亮和 `OnSelectionChanged`。

---

照这个套路，任何 Slate 列表（包括你后面要写的 Todo、资产管理面板等）都能快速落地：**Widget 负责 UI/交互，模块负责 Editor API + 数据**。卡在 `SHeaderRow` 时记得三步：

1. 定义列 `FName`
2. `SListView` 上挂 `.HeaderRow(SNew(SHeaderRow) ... )`
3. Row 继承 `SMultiColumnTableRow` 并在 `GenerateWidgetForColumn` 返回对应 Widget

希望这个文档能帮你少踩坑，Slate 的上手门槛一下子就降下来了。写完有新想法继续折腾吧 😄
