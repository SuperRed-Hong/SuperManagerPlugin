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

### 1.5.1 Prop 管理功能 (P0) [x]
- [x] **Runtime Layer**
  - [x] 增强 `AStage::UnregisterProp` - 清理所有 Acts 中的 PropStateOverrides
  - [x] 添加 `AStage::RemovePropFromAct` - 从特定 Act 移除 Prop
- [x] **Controller Layer**
  - [x] `SetPropStateInAct` - 设置 Prop 状态
  - [x] `RemovePropFromAct` - 从 Act 移除 Prop
  - [x] `RemoveAllPropsFromAct` - 移除 Act 的所有 Props
  - [x] `UnregisterProp` - 注销 Prop
  - [x] `UnregisterAllProps` - 注销所有 Props
- [x] **UI Layer**
  - [x] Stage 右键菜单 → "Unregister All Props"
  - [x] Act 右键菜单 → "Remove All Props from Act"
  - [x] Prop (在 Act 下) → "Set State..." / "Remove from Act" / "Unregister from Stage"
  - [x] Prop (在 Registered Props 下) → "Unregister from Stage"

### 1.5.2 Act 管理功能 (P0) [x]
- [x] **Runtime Layer**
  - [x] `AStage::RemoveAct` - 删除 Act
- [x] **Controller Layer**
  - [x] `DeleteAct` - 删除 Act（保护 Default Act）
### 1.5.2 Act 管理功能 (P0) [x]
- [x] **Runtime Layer**
  - [x] `AStage::RemoveAct` - 删除 Act
- [x] **Controller Layer**
  - [x] `DeleteAct` - 删除 Act（保护 Default Act）
- [x] **UI Layer**
  - [x] Act Row 最右侧删除按钮 (AppStyle Delete Icon)
  - [x] Act 右键菜单 → "Delete Act"
  - [x] **Internal Drag & Drop**: Registered Props → Act (Add to Act)
  - [x] **Highlight**: Updated to 80% White for better visibility

### 1.5.3 Prop 内联删除按钮 (P0) [x]
- [x] **UI Layer**
  - [x] Prop Row 在 Act 下 → x 按钮（移除出 Act）
  - [x] Prop Row 在 Registered Props 下 → x 按钮（注销）
- [x] **Controller Layer**
  - [x] 监听 level 中 actor 删除事件
  - [x] 自动注销被删除的 Prop

### 1.5.4 选择同步 (P1)
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
1. [x] **Enhance Drag & Drop Highlight** (80% White + Internal Drag Support)
2. [x] **实现 Prop 管理功能** (State editing + Unregistration)
3. [x] **实现 Act 删除功能** (Inline button + Context menu)
4. [x] **Prop 内联删除按钮** (Context-aware + Auto cleanup)
5. **实现选择同步**
