# Stage Editor Plugin - 完整开发计划

## 📋 当前状态审计 (Current Status Audit)

### ✅ 已完成 (Completed)
1. **插件基础结构**
   - ✅ 双模块结构（StageEditorRuntime + StageEditor）
   - ✅ 目录结构重组（功能导向）
   - ✅ Tab 注册到 Window 菜单

2. **Runtime 核心**
   - ✅ `StageCoreTypes.h` - FAct, FStageHierarchicalId
   - ✅ **混合 Prop 架构**
     - ✅ `UStagePropComponent` - 核心组件
     - ✅ `AProp` - 便利基类
   - ✅ `AStage` - 基础注册和状态管理

3. **Editor 基础**
   - ✅ `FStageEditorController` - Mock ID 生成
   - ✅ `SStageEditorPanel` - **基础框架（但不符合规格）**

### ❌ 规格偏差 (Spec Deviations)

| 规格要求 | 当前实现 | 状态 |
|---------|---------|------|
| **TreeView 层级结构** | ListView 平铺 | ❌ |
| Acts 文件夹 | 无文件夹概念 | ❌ |
| Default Act 自动创建 | 未实现 | ❌ |
| Prop 自动加入 Default Act | 未实现 | ❌ |
| Registered Props 文件夹 | 无 | ❌ |
| 拖拽注册 | 未实现 | ❌ |
| 吸管注册 | 未实现 | ❌ |
| PropState 预览按钮 | 未实现 | ❌ |
| 选择同步 | 未实现 | ❌ |
| 连线可视化 | 未实现 | ❌ |

---

## 🎯 三阶段开发计划

### Phase 1: 原型 (Prototype) - 核心流程验证

**目标**: 实现 Stage-Act-Prop 核心循环，**严格按照规格**实现 TreeView UI

#### 1.1 Runtime 核心修复
- [x] ✅ `UStagePropComponent` + `AProp` 混合架构
- [x] ✅ `AStage` 基础注册逻辑
- [ ] **🔴 `AStage` 自动创建 Default Act**
  - 在构造函数中添加
  - ActID = {StageID, 0, 0}
  - DisplayName = "Default Act"
- [ ] **🔴 注册 Prop 时自动加入 Default Act**
  - 修改 `RegisterProp()` 逻辑
  - 自动添加到 Default Act 的 PropStateOverrides

#### 1.2 Editor Panel 完全重构
- [ ] **🔴 实现 TreeView 层级结构**
  - 替换 `SListView` 为 `STreeView`
  - 定义 `FStageTreeItem` 数据结构（节点类型：Folder/Act/Prop）
  - 实现层级生成逻辑
  
- [ ] **🔴 Acts 文件夹 + Default Act 显示**
  - 根节点：Stage 名称
  - 子节点：Acts 文件夹
    - Default Act（显示所有 Prop）
    - 其他自定义 Act（仅显示有覆写的 Prop）
  
- [ ] **🔴 Registered Props 文件夹**
  - 平铺显示所有已注册 Prop
  - 显示 PropID 和名称

#### 1.3 基础交互
- [ ] **🔴 Quick Create Buttons**
  - 实现 `CreateStageBlueprint()`
  - 实现 `CreatePropBlueprint()`
  - 使用 `IAssetTools` 和 `FKismetEditorUtilities`
  
- [ ] **🔴 DebugHeader 集成**
  - 全局替换 `UE_LOG` 为 `DebugHeader::Print` 或 `PrintLog`
  
- [ ] **🔴 Prop 注册逻辑（按钮版）**
  - "Register Selected Props" 按钮
  - 检测 `UStagePropComponent`
  - 自动加入 Default Act
  
- [ ] **🔴 Create Act 按钮**
  - 创建新 Act
  - 添加到 TreeView

- [ ] ⚠️ PropState 预览（Phase 1 暂不强制）
  - 简单的 "Preview" 按钮
  - 调用 ConstructionScript 刷新

#### 1.4 验收标准
1. 打开 Panel，能看到 TreeView 结构
2. 能看到 "Acts" 和 "Registered Props" 两个文件夹
3. Default Act 自动存在
4. 注册 Prop 后，能在 Default Act 下看到
5. 创建新 Act 能正常显示

---

### Phase 2: 功能填充 (Feature Filling) - 完整交互

**目标**: 实现所有编辑器交互功能，DataLayer 集成

#### 2.1 高级注册逻辑
- [ ] **拖拽注册**
  - 从 World Outliner 拖拽 Actor 到 Panel
  - 拖拽到 Acts 文件夹或特定 Act
  
- [ ] **吸管/拾取注册**
  - "Add Prop" 按钮进入拾取模式
  - 点击视口中的 Actor 注册

#### 2.2 视口交互
- [ ] **选择同步**
  - 点击 Panel 中的 Prop → 选中视口 Actor
  - 选中视口 Actor → 高亮 Panel 条目
  
- [ ] **连线可视化**
  - 选中 Stage/Act 时绘制虚线连接 Props
  - 使用 HitProxy 或 Gizmo

#### 2.3 状态管理
- [ ] **Act 状态覆写**
  - 双击 Act 下的 Prop 设置 PropState
  - 显示当前状态值
  
- [ ] **PropState 预览完善**
  - 点击眼睛图标强制刷新状态
  - 支持 ConstructionScript 调用

#### 2.4 DataLayer 集成
- [ ] **Act 绑定 DataLayer**
  - Act 属性中选择 UDataLayerAsset
  - 显示 DataLayer 名称
  
- [ ] **DataLayer 预览**
  - 预览时控制 DataLayer 可见性
  - 异步加载状态处理

#### 2.5 Editor Mode 支持（可选）
- [ ] **创建 FStageEditorMode**
  - 注册自定义编辑模式
  - 模式切换时自动打开 Panel
  
- [ ] **模式工具栏**
  - "Open Panel" 按钮
  - 拾取工具图标

#### 2.6 验收标准
1. 拖拽和吸管两种注册方式都可用
2. 选择同步正常工作
3. 能看到连线可视化
4. 能设置 Act 的 Prop 状态
5. DataLayer 绑定和预览可用

---

### Phase 3: 打磨 (Polish) - 生产就绪

**目标**: UX 优化，错误处理，性能调优

#### 3.1 UX 改进
- [ ] **多选支持**
  - Ctrl+点击多选 Props
  - 批量注册/修改状态
  
- [ ] **右键菜单**
  - TreeView 节点右键菜单
  - "Delete", "Rename", "Duplicate"

- [ ] **搜索/过滤**
  - 按名称搜索 Prop
  - 按 State 过滤

#### 3.2 健壮性
- [ ] **Undo/Redo 完整支持**
  - 所有操作通过 FScopedTransaction
  - 测试撤销/重做流程
  
- [ ] **错误处理**
  - Prop 缺少 Component 的提示
  - ID 冲突检测和警告
  
- [ ] **脏状态追踪**
  - 修改后提示保存
  - 防止意外丢失数据

#### 3.3 性能优化
- [ ] **大量 Prop 性能**
  - 虚拟化 TreeView（大数据集）
  - 延迟刷新 UI
  
- [ ] **DataLayer 异步加载**
  - 进度条显示
  - 加载失败容错

#### 3.4 文档和测试
- [ ] **用户文档**
  - 使用指南（Markdown）
  - 视频教程脚本
  
- [ ] **自动化测试**
  - Editor 单元测试
  - 集成测试用例

#### 3.5 验收标准
1. 所有操作支持 Undo/Redo
2. 大量 Prop 场景不卡顿
3. 错误情况有友好提示
4. 文档完整可读

---

## 🚀 立即行动计划

### 第一步：修复当前 Panel
**优先级：P0（最高）**

1. **重构 SStageEditorPanel 使用 TreeView**
   - 定义 `FStageTreeItem` 结构
   - 实现 `OnGetChildren()` 回调
   - 构建 Acts/Props 文件夹层级
   
2. **AStage 自动创建 Default Act**
   - 修改构造函数
   - 确保 PropRegistry 非空时也有 Default Act

3. **注册逻辑修复**
   - `RegisterProp()` 自动加入 Default Act
   - 更新 Controller 逻辑

### 第二步：验证核心流程
**优先级：P1**

1. **编译并测试**
   - 打开 Panel 看到正确的树形结构
   - 注册 Prop 验证自动加入 Default Act
   
2. **创建演示关卡**
   - 放置 Stage 和几个 Prop
   - 录制工作流程截图

### 第三步：逐步推进 Phase 2
**优先级：P2**

- 按照 Phase 2 任务列表逐个实现
- 每完成一个功能立即验证

---

## 📊 任务追踪
所有任务应同步更新到 `task.md`，使用以下状态标记：
- `[ ]` 待开始
- `[/]` 进行中
- `[x]` 已完成
- `[!]` 有问题/阻塞
