# Stage Editor Plugin - Development Tasks (Updated)

## Phase 0: 基础设施 (Infrastructure) ✅
- [x] 插件结构创建（双模块）
- [x] 目录结构重组（功能导向）
- [x] Tab 注册到 Window 菜单
- [x] Runtime 核心类型（StageCoreTypes.h）
- [x] 混合 Prop 架构（Component + Base Class）

---

## Phase 1: 原型 (Prototype) - 核心流程 ✅
- [x] **Runtime 核心修复**
  - [x] `AStage` 自动创建 Default Act
  - [x] 注册 Prop 时自动加入 Default Act
- [x] **Panel UI 重构**
  - [x] `STreeView` 实现
  - [x] Acts/Registered Props 文件夹结构
- [x] **基础交互**
  - [x] Quick Create Buttons (Stage/Prop BP)
  - [x] Register Selected Props 按钮
  - [x] Create Act 按钮
  - [x] **Drag & Drop 注册** (从 Outliner 拖入 Panel)
    - [x] 支持拖拽到特定 Stage/Act (Context-aware Drop)
    - [x] 注册成功/失败反馈 (Notify Info)

---

## Phase 1.5: 核心补全 (Core Completion) 🔴
**当前重点：让编辑器真正可用**

### 1.5.1 Prop State 编辑 (P0)
- [ ] **UI 交互**
  - [ ] 右键菜单 "Set State" 或双击编辑
  - [ ] 输入框对话框
- [ ] **Controller 逻辑**
  - [ ] `SetPropStateInAct` 实现
  - [ ] 更新 `AStage` 数据并标记 Dirty

### 1.5.2 选择同步 (P1)
- [ ] **Panel -> Viewport**
  - [ ] `OnSelectionChanged` 触发 `GEditor->SelectActor`
- [ ] **Viewport -> Panel**
  - [ ] 监听 `USelection::SelectionChangedEvent`
  - [ ] 同步 TreeView 选中项

---

## Phase 2: 交互增强 (Feature Filling) ⏸️

### 2.1 视口交互
- [ ] P2: 连线可视化（选中时绘制虚线）
- [ ] P3: 吸管/拾取注册 (已有 Drag&Drop，优先级降低)

### 2.2 DataLayer 集成
- [ ] P1: Act 绑定 `UDataLayerAsset`
- [ ] P2: DataLayer 预览控制

### 2.3 Editor Mode (可选)
- [ ] P2: 创建 `FStageEditorMode`

---

## Phase 3: 打磨 (Polish) ⏸️
- [x] **UX 改进**
  - [x] TreeView 刷新时保持展开状态
- [ ] P2: 多选支持
- [ ] P1: Undo/Redo 完整支持
- [ ] P2: 搜索/过滤
- [ ] P2: 大量 Prop 虚拟化优化

---

## 🚨 立即行动 (Next Steps)
1. [x] **Enhance Drag & Drop Highlight** (Brighter color + Hierarchy highlight)
2. [/] **实现 Prop State 编辑功能** (UI + Logic)
3. **实现选择同步**
