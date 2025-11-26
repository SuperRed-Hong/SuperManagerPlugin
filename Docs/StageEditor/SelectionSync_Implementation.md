# StageEditor Panel 单向选择同步 - 实现说明

## 📋 实现总结

已成功实现 **Panel → Viewport 单向同步**，SceneOutliner 现在始终跟随 StageEditor Panel 的选择。

---

## ✅ 修改内容

### 修改的文件
- `Plugins/StageEditor/Source/StageEditor/Private/EditorUI/StageEditorPanel.cpp`

### 具体修改

#### 修改 1: 禁用 Viewport 监听注册 (Line 226-228)

**Before:**
```cpp
RefreshUI();

RegisterViewportSelectionListener();

if (Controller.IsValid())
{
    Controller->Initialize();
}
```

**After:**
```cpp
RefreshUI();

// ❌ Disabled: We want one-way sync (Panel → Viewport only)
// Viewport selection should NOT affect Panel selection
// RegisterViewportSelectionListener();

if (Controller.IsValid())
{
    Controller->Initialize();
}
```

**原因:** 不再监听 SceneOutliner 的选择变化事件

---

#### 修改 2: 禁用析构函数中的注销 (Line 237-241)

**Before:**
```cpp
SStageEditorPanel::~SStageEditorPanel()
{
    UnregisterViewportSelectionListener();
}
```

**After:**
```cpp
SStageEditorPanel::~SStageEditorPanel()
{
    // ❌ Disabled: Viewport listener was not registered
    // UnregisterViewportSelectionListener();
}
```

**原因:** 既然没有注册，也就不需要注销

---

#### 修改 3: 禁用 RefreshUI 中的同步调用 (Line 351-352)

**Before:**
```cpp
    }
}

HandleViewportSelectionChanged(nullptr);
}
```

**After:**
```cpp
    }
}

// ❌ Disabled: One-way sync only (Panel → Viewport)
// HandleViewportSelectionChanged(nullptr);
}
```

**原因:** RefreshUI 时不再主动同步 Viewport 的选择状态

---

## 🎯 最终行为

### 单向同步流程图

```
┌─────────────────────────┐          ┌──────────────────────┐
│  StageEditor Panel      │          │  SceneOutliner       │
│  (Master)               │          │  (Slave)             │
└─────────────────────────┘          └──────────────────────┘
         │                                      ▲
         │  User clicks Prop                   │
         ▼                                      │
  OnSelectionChanged()                          │
         │                                      │
         │  Guard: bUpdatingViewportSelection   │
         │         FromPanel = true             │
         ▼                                      │
  SelectActorInViewport()  ─────────────────────┘
         │                    Sync to Viewport
         │
  GEditor->SelectActor(PropActor)
         │
         ▼
  ✅ Actor selected in Outliner


User clicks Actor in Outliner
         │
         ▼
  ❌ NO sync back to Panel
  ✅ Panel keeps its selection unchanged
```

---

## 📊 行为对比表

| 用户操作 | 修改前（双向同步）⚠️ | 修改后（单向同步）✅ |
|---------|-------------------|-------------------|
| **在 Panel 中点击 Prop** | Panel 选中 Prop<br/>Outliner 选中 Actor | Panel 选中 Prop<br/>Outliner 选中 Actor |
| **在 Outliner 中点击 Actor** | Panel 跟随选中 Prop<br/>Outliner 选中 Actor | **Panel 保持原样**<br/>Outliner 选中 Actor |
| **在 Panel 中点击空白** | Panel 取消选择<br/>Outliner 取消选择 | Panel 取消选择<br/>Outliner 取消选择 |
| **在 Outliner 中取消选择** | Panel 也取消选择<br/>Outliner 取消选择 | **Panel 保持原样**<br/>Outliner 取消选择 |
| **在 Outliner 中多选 Actors** | Panel 尝试同步（可能失败）| **Panel 不受影响** |

---

## 🧪 测试指南

### 测试用例 1: Panel → Outliner 同步 ✅

**操作步骤:**
1. 打开 StageEditor Panel
2. 在 Panel 的 "Registered Props" 文件夹下点击一个 Prop

**预期结果:**
- ✅ Panel 中该 Prop 被选中（高亮显示）
- ✅ SceneOutliner 中对应的 Actor 被选中（高亮显示）
- ✅ Viewport 中 Actor 显示为选中状态（橙色边框）

**验证:**
```cpp
// Panel 选择状态
StageTreeView->GetSelectedItems() → 包含该 Prop TreeItem

// Viewport 选择状态
GEditor->GetSelectedActors()->Num() → 1
GEditor->GetSelectedActors()->GetSelectedObject(0) → PropActor
```

---

### 测试用例 2: Panel 控制权验证 ✅

**操作步骤:**
1. 在 Panel 中选中 **Prop A**
   - 验证: Outliner 中 **Actor A** 被选中 ✅
2. 在 SceneOutliner 中点击 **Actor B**
   - 验证: Panel 中仍然显示 **Prop A** 被选中 ✅
   - 验证: Outliner 中 **Actor B** 被选中 ✅
3. 再次在 Panel 中点击 **Prop C**
   - 验证: Panel 选中 **Prop C** ✅
   - 验证: Outliner 切换到 **Actor C** ✅

**预期结果:**
- ✅ Panel 的选择不受 Outliner 影响
- ✅ Panel 可以随时"夺回"Outliner 的控制权

---

### 测试用例 3: 清空选择行为 ✅

**操作步骤:**
1. 在 Panel 中选中任意 Prop
2. 在 Panel 中点击空白区域（或按 Esc）

**预期结果:**
- ✅ Panel 取消选择
- ✅ Outliner 也取消选择

**操作步骤 (反向):**
1. 在 Panel 中选中任意 Prop
2. 在 SceneOutliner 中点击空白区域

**预期结果:**
- ✅ Panel 保持原选择（不变）
- ✅ Outliner 取消选择

---

### 测试用例 4: 多个 Props 切换 ✅

**操作步骤:**
1. 在 Panel 中依次快速点击 Prop A, B, C, D

**预期结果:**
- ✅ Panel 的选择跟随点击顺序变化
- ✅ Outliner 的选择同步跟随 Panel
- ✅ 没有延迟或闪烁

---

### 测试用例 5: Acts 和 Folders ✅

**操作步骤:**
1. 在 Panel 中点击一个 **Act** 节点

**预期结果:**
- ✅ Panel 选中该 Act
- ✅ Controller 的 ActiveStage 没有变化
- ❌ Outliner 不受影响（Act 不是 Actor）

**操作步骤:**
2. 在 Panel 中点击 **"Acts" Folder** 或 **"Registered Props" Folder**

**预期结果:**
- ✅ Panel 可以选中 Folder
- ❌ Outliner 不受影响（Folder 不是 Actor）

---

### 测试用例 6: 在 Act 下的 Prop ✅

**操作步骤:**
1. 在 Panel 中展开 **Acts → Act 1**
2. 点击 Act 1 下的某个 Prop (显示 State 信息)

**预期结果:**
- ✅ Panel 选中该 Prop
- ✅ Outliner 选中对应的 Actor
- ✅ 与 "Registered Props" 下点击同一个 Prop 的效果一致

---

## 🔧 Guard 机制说明

### 防止循环的保护变量

虽然现在只有单向同步，但 Guard 仍然保留用于防止潜在的事件循环：

```cpp
// StageEditorPanel.h
bool bUpdatingTreeSelectionFromViewport = false;  // ❌ 已不再使用（反向同步禁用）
bool bUpdatingViewportSelectionFromPanel = false; // ✅ 仍在使用（正向同步保护）
```

### Guard 使用位置

**SelectActorInViewport() - Line 1614:**
```cpp
void SStageEditorPanel::SelectActorInViewport(AActor* ActorToSelect)
{
    if (!GEditor || !ActorToSelect)
    {
        return;
    }

    // 设置 Guard: 防止 GEditor->SelectActor 触发 SelectObjectEvent
    // 进而调用已禁用的 HandleViewportSelectionChanged
    TGuardValue<bool> Guard(bUpdatingViewportSelectionFromPanel, true);
    GEditor->SelectNone(false, true);
    GEditor->SelectActor(ActorToSelect, true, true);
}
```

**OnSelectionChanged() - Line 668:**
```cpp
void SStageEditorPanel::OnSelectionChanged(TSharedPtr<FStageTreeItem> Item, ESelectInfo::Type SelectInfo)
{
    // 防止从 Viewport 触发的 Panel 更新再次触发 Viewport 同步
    // 虽然反向同步已禁用，但此检查作为额外保护
    if (bUpdatingTreeSelectionFromViewport || !Item.IsValid() || !Controller.IsValid())
    {
        return;
    }

    // ... 同步到 Viewport
}
```

---

## 📝 未使用的代码

以下函数/变量现在不再被调用，但保留以便将来可能需要：

### 可以删除的代码（可选）

1. **RegisterViewportSelectionListener()** - Line 1471
2. **UnregisterViewportSelectionListener()** - Line 1491
3. **HandleViewportSelectionChanged()** - Line 1502
4. **bUpdatingTreeSelectionFromViewport** - 成员变量
5. **ActorPathToTreeItem** - TMap（部分使用，主要用于反向同步）
6. **ViewportSelectionDelegateHandle** - FDelegateHandle
7. **ActorSelectionPtr** - TWeakObjectPtr<USelection>

### 建议

**保留代码的理由:**
- ✅ 未来可能需要可选的双向同步模式
- ✅ 代码已被注释禁用，不影响性能
- ✅ 保留作为参考实现

**删除代码的理由:**
- ✅ 减少代码体积
- ✅ 避免维护未使用的代码
- ✅ 使代码意图更清晰

**当前决策:** **保留但禁用**（已注释）

---

## 🎯 设计原则

### Master-Slave 架构

```
StageEditor Panel = Master (主控者)
    │
    ├── 职责: 管理 Stage、Act、Prop 的层级关系
    ├── 权限: 完全控制选择状态
    └── 行为: 主动推送选择到 Viewport

SceneOutliner = Slave (跟随者)
    │
    ├── 职责: 显示场景中的所有 Actor
    ├── 权限: 被动接受来自 Panel 的选择
    └── 行为: 反映 Panel 的选择状态，但不影响 Panel
```

### 单向数据流

```
用户交互 → Panel 状态变化 → Viewport 同步更新
    ↓                          │
    └─────────────────────────┘
          (不再回流)
```

---

## 🚀 优势

### 1. **清晰的控制权** ✅
- Panel 是唯一的"真相源"
- 用户明确知道 Panel 主导一切

### 2. **避免选择冲突** ✅
- 不会出现"Panel 和 Outliner 选择不一致"的困惑
- 不会出现"点击 Outliner 导致 Panel 跳转"的意外行为

### 3. **工作流优化** ✅
- 用户可以在 Outliner 中查看其他 Actor
- 同时保持 Panel 中的工作状态
- Panel 和 Outliner 可以"各司其职"

### 4. **符合 Stage 编辑理念** ✅
- StageEditor 是专门的工具，应该主导选择
- Outliner 是通用工具，应该配合专门工具

---

## 📚 相关文档

- **SelectionSync_Analysis.md** - 详细的同步机制分析
- **StageEditorPanel.h** - Panel 接口定义
- **StageEditorPanel.cpp** - Panel 实现（包含修改）

---

## 版本历史

**v1.0 (2025-01-21)**
- ✅ 禁用 Viewport → Panel 反向同步
- ✅ 保留 Panel → Viewport 正向同步
- ✅ 实现真正的单向数据流
- ✅ StageEditor Panel 成为选择的唯一主控者
