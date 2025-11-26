# StageEditor Panel ↔ SceneOutliner 选择同步机制分析

## 当前实现分析

### 📊 同步架构图

```
┌─────────────────────────┐          ┌──────────────────────┐
│  StageEditor Panel      │          │  SceneOutliner       │
│  (TreeView)             │          │  (Viewport)          │
│                         │          │                      │
│  User clicks item       │          │  User clicks Actor   │
│         │               │          │         │            │
│         ▼               │          │         ▼            │
│  OnSelectionChanged()   │          │  SelectObjectEvent   │
│         │               │          │         │            │
│         │               │          │         │            │
│  [Guard: bUpdating      │          │  [Guard: bUpdating   │
│   ViewportSelection     │          │   TreeSelection      │
│   FromPanel = true]     │          │   FromViewport=true] │
│         │               │          │         │            │
│         ▼               │          │         ▼            │
│  SelectActorInViewport()│─────────▶│  GEditor->SelectActor│
│         │               │  同步→   │                      │
│                         │          │                      │
│                         │          │                      │
│  HandleViewportSelection│◀─────────│  SelectObjectEvent   │
│  Changed()              │  ←同步   │  fires               │
│         │               │          │                      │
│         ▼               │          │                      │
│  StageTreeView->        │          │                      │
│  SetSelection()         │          │                      │
└─────────────────────────┘          └──────────────────────┘

        ⚠️ 当前问题：双向同步！
```

---

## 🔍 当前代码流程详解

### 流程 1: Panel → Viewport 同步 ✅

**触发位置:** `StageEditorPanel.cpp:666`

```cpp
void SStageEditorPanel::OnSelectionChanged(TSharedPtr<FStageTreeItem> Item, ESelectInfo::Type SelectInfo)
{
    // 防止循环: 如果是从 Viewport 触发的更新，直接返回
    if (bUpdatingTreeSelectionFromViewport || !Item.IsValid() || !Controller.IsValid())
    {
        return; // 🔴 关键：防止循环
    }

    if (Item->Type == EStageTreeItemType::Stage)
    {
        // Set active stage in controller
        Controller->SetActiveStage(StageItem->StagePtr.Get());
    }
    else if (Item->Type == EStageTreeItemType::Prop && Item->ActorPtr.IsValid())
    {
        // Sync to viewport
        SelectActorInViewport(Item->ActorPtr.Get()); // 🟢 同步到 Viewport
    }
}
```

**`SelectActorInViewport` 实现:** `StageEditorPanel.cpp:1607-1617`

```cpp
void SStageEditorPanel::SelectActorInViewport(AActor* ActorToSelect)
{
    if (!GEditor || !ActorToSelect)
    {
        return;
    }

    // 设置 Guard，防止触发反向同步
    TGuardValue<bool> Guard(bUpdatingViewportSelectionFromPanel, true); // 🔴 关键
    GEditor->SelectNone(false, true);
    GEditor->SelectActor(ActorToSelect, true, true);
}
```

**结果:**
- ✅ 用户点击 Panel 中的 Prop → Viewport 中的 Actor 被选中
- ✅ Guard 阻止了反向同步（Viewport → Panel）

---

### 流程 2: Viewport → Panel 同步 ⚠️ (当前存在)

**触发位置:** `StageEditorPanel.cpp:1502`

```cpp
void SStageEditorPanel::HandleViewportSelectionChanged(UObject* SelectedObject)
{
    // 防止循环: 如果是从 Panel 触发的更新，直接返回
    if (bUpdatingViewportSelectionFromPanel || !StageTreeView.IsValid())
    {
        return; // 🔴 关键：防止循环
    }

    AActor* SelectedActor = Cast<AActor>(SelectedObject);

    // 如果没有选中，清空 Panel 选择
    if (!SelectedActor)
    {
        TGuardValue<bool> Guard(bUpdatingTreeSelectionFromViewport, true);
        StageTreeView->ClearSelection(); // 🟡 清空 Panel 选择
        return;
    }

    // 查找对应的 TreeItem
    const FString ActorPath = SelectedActor->GetPathName();
    if (TWeakPtr<FStageTreeItem>* ItemPtr = ActorPathToTreeItem.Find(ActorPath))
    {
        if (TSharedPtr<FStageTreeItem> TreeItem = ItemPtr->Pin())
        {
            ExpandAncestors(TreeItem);
            TGuardValue<bool> Guard(bUpdatingTreeSelectionFromViewport, true);
            StageTreeView->SetSelection(TreeItem); // 🟢 同步到 Panel
            StageTreeView->RequestScrollIntoView(TreeItem);
        }
    }
}
```

**注册监听:** `StageEditorPanel.cpp:1471-1489`

```cpp
void SStageEditorPanel::RegisterViewportSelectionListener()
{
    if (USelection* ActorSelection = GEditor->GetSelectedActors())
    {
        ActorSelectionPtr = ActorSelection;

        if (!ViewportSelectionDelegateHandle.IsValid())
        {
            // 🔴 监听 Viewport 选择变化
            ViewportSelectionDelegateHandle = ActorSelection->SelectObjectEvent.AddSP(
                this, &SStageEditorPanel::HandleViewportSelectionChanged
            );
        }

        HandleViewportSelectionChanged(nullptr);
    }
}
```

**结果:**
- ⚠️ 用户在 SceneOutliner 中点击 Actor → Panel 中的 Prop 被选中
- ⚠️ 这违反了用户的期望！

---

## 🎯 用户期望 vs 当前行为

### 期望行为（单向同步）

```
用户操作                  StageEditor Panel      SceneOutliner
────────────────────────────────────────────────────────────
1. 点击 Panel 中的 Prop    ✅ Prop 选中          ✅ Actor 选中
2. 点击 Outliner 中的 Actor ❌ 保持原样          ✅ Actor 选中
3. 点击 Panel 中的 Act     ✅ Act 选中           ❌ 无操作
4. 点击 Panel 空白处       ✅ 取消选择           ✅ 取消选择
```

**设计原则:**
- **Panel 是主导者** - Panel 的选择优先级最高
- **Outliner 是跟随者** - Outliner 只反映 Panel 的选择
- **单向流动** - Panel → Outliner (不反向)

---

### 当前行为（双向同步）⚠️

```
用户操作                  StageEditor Panel      SceneOutliner
────────────────────────────────────────────────────────────
1. 点击 Panel 中的 Prop    ✅ Prop 选中          ✅ Actor 选中
2. 点击 Outliner 中的 Actor ⚠️ Prop 也被选中    ✅ Actor 选中  ← 问题！
3. 点击 Panel 中的 Act     ✅ Act 选中           ❌ 无操作
4. 在 Outliner 点击空白    ⚠️ Panel 也清空      ✅ 取消选择  ← 问题！
```

**问题:**
- ❌ Outliner 的操作会影响 Panel（违反设计原则）
- ❌ 用户在 Outliner 中选择 Actor 时，Panel 被迫跟随
- ❌ 用户无法独立操作 Outliner

---

## 🛠️ 解决方案

### 方案 1: 完全禁用 Viewport → Panel 同步 ✅ (推荐)

**修改:** 注释掉或删除反向同步逻辑

```cpp
// StageEditorPanel.cpp:226
void SStageEditorPanel::Construct(const FArguments& InArgs, TSharedPtr<FStageEditorController> InController)
{
    // ... UI 构建 ...

    RefreshUI();

    // ❌ 注释掉这行，不再监听 Viewport 选择
    // RegisterViewportSelectionListener();

    if (Controller.IsValid())
    {
        Controller->Initialize();
    }
}
```

**优点:**
- ✅ 简单直接
- ✅ 完全符合用户期望
- ✅ Panel 完全主导，Outliner 完全跟随

**缺点:**
- 无（这正是期望的行为）

---

### 方案 2: 保留单向跟踪（只读模式）⚠️

**修改:** 保留监听，但只用于高亮显示，不改变选择

```cpp
void SStageEditorPanel::HandleViewportSelectionChanged(UObject* SelectedObject)
{
    // 仅用于高亮/追踪，不修改 Panel 选择
    if (bUpdatingViewportSelectionFromPanel || !StageTreeView.IsValid())
    {
        return;
    }

    AActor* SelectedActor = Cast<AActor>(SelectedObject);

    // ❌ 不再修改 Panel 选择
    // TGuardValue<bool> Guard(bUpdatingTreeSelectionFromViewport, true);
    // StageTreeView->SetSelection(TreeItem);

    // ✅ 只滚动到对应项（不选中）
    if (TreeItem.IsValid())
    {
        ExpandAncestors(TreeItem);
        StageTreeView->RequestScrollIntoView(TreeItem); // 仅滚动
    }
}
```

**优点:**
- ✅ 在 Outliner 中选择 Actor 时，Panel 会滚动到对应项
- ✅ 但不会改变 Panel 的选择状态

**缺点:**
- ⚠️ 可能产生混淆（Outliner 选中 ≠ Panel 选中）

---

### 方案 3: 可配置的同步模式 🔧 (复杂)

**添加配置选项:**

```cpp
// StageEditorPanel.h
enum class ESelectionSyncMode
{
    OneWay_PanelToViewport,      // Panel → Viewport (推荐)
    OneWay_ViewportToPanel,      // Viewport → Panel
    TwoWay_Bidirectional         // 双向同步（当前）
};

ESelectionSyncMode SelectionSyncMode = ESelectionSyncMode::OneWay_PanelToViewport;
```

**优点:**
- ✅ 灵活性最高
- ✅ 可以满足不同用户需求

**缺点:**
- ❌ 增加复杂度
- ❌ 对于明确的设计目标来说过度工程化

---

## ✅ 推荐方案：方案 1

### 修改步骤

#### Step 1: 禁用 Viewport → Panel 同步

**文件:** `StageEditorPanel.cpp`

**修改 1:** 注释掉注册逻辑
```cpp
// Line 226
RefreshUI();

// ❌ 禁用反向同步
// RegisterViewportSelectionListener();

if (Controller.IsValid())
{
    Controller->Initialize();
}
```

**修改 2:** 注释掉析构函数中的注销逻辑（保持一致）
```cpp
SStageEditorPanel::~SStageEditorPanel()
{
    // UnregisterViewportSelectionListener(); // 已禁用
}
```

#### Step 2: 清理 RefreshUI 中的调用

**修改 3:** 移除 RefreshUI 中的同步调用
```cpp
void SStageEditorPanel::RefreshUI()
{
    // ... 构建 tree items ...

    // 恢复展开状态
    RestoreExpansionState(SavedExpansionState);

    // ❌ 不再主动同步
    // HandleViewportSelectionChanged(nullptr);
}
```

#### Step 3: (可选) 删除未使用的代码

如果完全不需要反向同步，可以删除：
- `RegisterViewportSelectionListener()`
- `UnregisterViewportSelectionListener()`
- `HandleViewportSelectionChanged()`
- `bUpdatingTreeSelectionFromViewport` 成员变量
- `ActorPathToTreeItem` 映射表（如果仅用于反向同步）
- `ViewportSelectionDelegateHandle`
- `ActorSelectionPtr`

---

## 📊 修改后的行为

### 最终的单向同步流程

```
┌─────────────────────────┐          ┌──────────────────────┐
│  StageEditor Panel      │          │  SceneOutliner       │
│  (TreeView)             │          │  (Viewport)          │
│                         │          │                      │
│  User clicks item       │          │                      │
│         │               │          │                      │
│         ▼               │          │                      │
│  OnSelectionChanged()   │          │                      │
│         │               │          │                      │
│         ▼               │          │                      │
│  SelectActorInViewport()│─────────▶│  GEditor->SelectActor│
│                         │  同步→   │         │            │
│                         │          │         ▼            │
│                         │          │  Actor 被选中        │
│                         │          │                      │
│                         │  ✅ 单向  │                      │
│                         │          │  User clicks Actor   │
│  (不再响应)             │  ✅ 不同步│         │            │
│                         │          │         ▼            │
│                         │          │  Actor 被选中        │
│                         │          │  (Panel 保持原样)    │
└─────────────────────────┘          └──────────────────────┘

        ✅ 期望行为：Panel 主导，Outliner 跟随
```

### 用户体验变化

| 操作 | 修改前 | 修改后 |
|------|--------|--------|
| 点击 Panel 中的 Prop | Outliner 选中对应 Actor | Outliner 选中对应 Actor ✅ |
| 点击 Outliner 中的 Actor | Panel 选中对应 Prop ⚠️ | Panel 保持原样 ✅ |
| 点击 Panel 空白处 | 两边都取消选择 | Panel 取消，Outliner 取消 ✅ |
| 在 Outliner 取消选择 | Panel 也取消 ⚠️ | Panel 保持原样 ✅ |

**总结:**
- ✅ Panel 是唯一的"指挥官"
- ✅ Outliner 永远跟随 Panel
- ✅ Outliner 的操作不影响 Panel

---

## 🧪 测试验证

### 测试用例

**TC1: Panel → Outliner 同步**
```
1. 在 Panel 中点击一个 Prop
   期望: Outliner 中对应的 Actor 被选中 ✅
2. 在 Panel 中点击另一个 Prop
   期望: Outliner 切换到新的 Actor ✅
```

**TC2: Outliner 独立操作**
```
1. 在 Panel 中选中 Prop A
   期望: Outliner 选中 Actor A ✅
2. 在 Outliner 中点击 Actor B
   期望:
   - Outliner 选中 Actor B ✅
   - Panel 仍然显示 Prop A 被选中 ✅
```

**TC3: 清空选择**
```
1. 在 Panel 中选中 Prop A
2. 在 Panel 中点击空白处
   期望:
   - Panel 取消选择 ✅
   - Outliner 取消选择 ✅
3. 在 Outliner 中点击 Actor B
   期望:
   - Outliner 选中 Actor B ✅
   - Panel 保持空白（不选择任何东西）✅
```

**TC4: 多选（Outliner）**
```
1. 在 Outliner 中 Ctrl+点击多个 Actor
   期望: Panel 不受影响 ✅
```

---

## 📝 总结

### 当前问题
- ❌ 双向同步导致优先级不明确
- ❌ Outliner 的操作会覆盖 Panel 的选择
- ❌ 违反了"Panel 主导"的设计原则

### 解决方案
- ✅ **禁用 Viewport → Panel 同步**
- ✅ **保留 Panel → Viewport 同步**
- ✅ **实现真正的单向数据流**

### 实现建议
- **方案 1（推荐）**: 注释掉 `RegisterViewportSelectionListener()`
- **方案 2**: 改为只读追踪（滚动但不选中）
- **方案 3**: 添加配置选项（过度工程化）

### 最终效果
```
StageEditor Panel = Master (主控)
SceneOutliner = Slave (跟随)
数据流: Panel → Outliner (单向)
```
