# Stage Editor Plugin - 完整开发计划 (Updated)

## 📋 当前状态审计 (Current Status Audit)

### ✅ 已完成 (Completed)
1. **插件基础结构**
   - ✅ 双模块结构（StageEditorRuntime + StageEditor）
   - ✅ Tab 注册到 Window 菜单

2. **Runtime 核心**
   - ✅ `StageCoreTypes.h` - FAct, FStageHierarchicalId
   - ✅ 混合 Prop 架构 (`UStagePropComponent` + `AProp`)
   - ✅ `AStage` 基础注册和状态管理
   - ✅ **Default Act 逻辑** (自动创建 Default Act, 注册 Prop 自动加入)

3. **Editor UI & 交互**
   - ✅ `FStageEditorController`
   - ✅ `SStageEditorPanel` 重构为 **STreeView**
   - ✅ **文件夹结构** (Acts, Registered Props)
   - ✅ **快速创建按钮** (Create Stage BP, Create Prop BP)
   - ✅ **注册功能** (Register Selected, **Drag & Drop from Outliner**)
     - ✅ 支持拖拽到特定 Stage (Context-aware Drop)
     - ✅ 注册反馈 (Notify Info)
   - ✅ **Create Act** 功能
   - ✅ **UX 优化** (TreeView 保持展开状态)
   - ✅ `DebugHeader` 集成

### ❌ 核心缺失 (Critical Missing)

| 缺失功能 | 说明 | 优先级 |
|---------|------|-------|
| **Prop State 编辑** | 目前 Panel 中仅显示 State 数值，无法修改 | 🔴 P0 |
| **选择同步** | Panel 与 Viewport 选择互不通 | 🟡 P1 |
| **Prop 预览** | 点击 Act 虽然触发 Activate，但缺乏明确的预览模式反馈 | 🟡 P1 |

---

## 🎯 剩余开发计划

### Phase 1.5: 核心补全 (Core Completion)
**目标**: 完成 "Stage-Act-Prop" 闭环，确保用户能在编辑器中完整配置演出。

#### 1.5.1 Prop State 编辑 (🔴 P0)
- [ ] **UI 支持编辑状态**
  - 方案 A: 右键菜单 "Set State..." -> 弹出输入框
  - 方案 B: 双击条目 -> 变为可编辑文本框
  - 方案 C: 属性栏 (Details View) 显示选中 Item 的详细信息 (推荐)
- [ ] **Controller 逻辑**
  - 实现 `SetPropStateInAct(ActID, PropID, NewState)`
  - 更新 `AStage` 数据

#### 1.5.2 选择同步 (🟡 P1)
- [ ] **Panel -> Viewport**
  - 点击 TreeView Item -> 选中场景中对应的 Actor
- [ ] **Viewport -> Panel**
  - 选中场景 Actor -> 自动展开并高亮 Panel 中的对应节点

### Phase 2: 交互增强 (Interaction)
**目标**: 提升易用性和可视化反馈。

#### 2.1 视口交互
- [ ] **连线可视化**
  - 选中 Stage/Act 时，绘制 Stage -> Props 的连线
  - 使用 `FComponentVisualizer` 或 `DrawDebug`
- [ ] **吸管/拾取注册** (已由 Drag & Drop 部分替代，优先级降低)
  - "Add Prop" 按钮进入拾取模式

#### 2.2 DataLayer 集成
- [ ] **Act 绑定 DataLayer**
  - Act 属性中增加 `UDataLayerAsset*` 字段
  - 激活 Act 时加载/显示对应 DataLayer

### Phase 3: 打磨 (Polish)
**目标**: 生产就绪，性能优化。

- [ ] **多选支持**: 批量修改状态
- [ ] **Undo/Redo**: 完善所有操作的事务支持
- [ ] **搜索/过滤**: 快速查找 Prop
- [ ] **文档与测试**

---

## 🚀 立即行动计划 (Next Steps)

### 第一步：实现 Prop State 编辑 (P0)
1.  **修改 `SStageEditorPanel`**:
    - 为 Prop 节点添加右键菜单 "Change State"。
    - 或者集成一个简单的属性面板区域。
2.  **实现逻辑**:
    - 弹出对话框输入新的 State (Int)。
    - 调用 Controller 更新 Stage 数据。
    - 刷新 TreeView 显示。

### 第二步：实现选择同步 (P1)
1.  **Panel -> Viewport**: 
    - 在 `OnSelectionChanged` 中调用 `GEditor->SelectActor`。
2.  **Viewport -> Panel**:
    - 监听 `USelection::SelectionChangedEvent`。
    - 在 TreeView 中查找并 `SetSelection`。

### 第三步：DataLayer 调研
1.  研究如何在 Runtime/Editor 中动态控制 DataLayer 的激活状态。
