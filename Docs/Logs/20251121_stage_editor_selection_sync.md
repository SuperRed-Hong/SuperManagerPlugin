# StageEditor 选择同步与 Prop 状态编辑

## 问题描述
- StageEditor Panel 与视口/World Outliner 之间没有任何选择同步，用户在面板和关卡间来回切换会不断丢失上下文。
- Act 节点下的 Prop 只能查看状态值，无法在同一界面里编辑 `PropStateOverrides`，导致流程反复跳转 Details 面板。
- 影响：Stage 管线核心交互不可用，容易导致错误操作或漏改状态。

## 解决方案
- `SStageEditorPanel` 内新增 Selection Sync 逻辑：缓存 ActorPath→TreeItem，对接 `USelection::SelectObjectEvent`，并在 TreeView 选择时调用 `GEditor->SelectActor`，双向同步且使用 Guard 标记避免递归。
- 为 Act 下的 Prop 行追加 `SNumericEntryBox<int32>`，直接调用 `FStageEditorController::SetPropStateInAct` 修改状态，保持事务与数据刷新。
- 保留已有 drag&drop 与 Context Menu 逻辑，同时记录 Stage/Prop TreeItem 的 ActorPath、PropState 用于映射。

### 修改的文件列表
- `Docs/StageEditor/SelectionSyncAndPropStateEdit_TechPlan.md`
- `Plugins/StageEditor/Source/StageEditor/Public/EditorUI/StageEditorPanel.h`
- `Plugins/StageEditor/Source/StageEditor/Private/EditorUI/StageEditorPanel.cpp`

## 实施步骤
1. **技术方案**：在 `Docs/StageEditor/SelectionSyncAndPropStateEdit_TechPlan.md` 给出数据模型、OpenAPI/契约、规范检查清单。
2. **UI 数据扩展**：为 `FStageTreeItem` 扩展 ActorPath/状态字段，`RefreshUI` 时构建 `ActorPathToTreeItem`，并在 Act/Props/Stage 节点写入。
3. **交互实现**：绑定 `USelection->SelectObjectEvent`，实现 `Register/UnregisterViewportSelectionListener`、Selection Guard、Tree→Viewport/Viewport→Tree 同步，新增内联 `SNumericEntryBox` 状态编辑与相关 Helper。

## 测试验证
- ✅ 代码级验证：通过阅读 `USelection` 现有使用（SuperManager）确认 `SelectObjectEvent` 签名，与 TGuardValue 组合避免递归。
- ⚠️ 未做实际运行：本地缺少 UE Editor 运行环境，无法在编辑器中点击验证；需在引擎内手动确认 Prop 状态编辑与双向选中能正确刷新。

## 注意事项
- 由于 TreeView 中同一 Prop 会在多个节点出现，目前仅对首次注册的节点（通常是 Act 下的条目）做 Viewport→Panel 映射，后续若需精准定位不同 Act 需扩展 key。
- `SNumericEntryBox` 使用 `OnValueCommitted`，滑块操作在失焦或回车时才提交，这是为了减少事务数量；若需要实时预览，可在后续迭代中增加 `OnValueChanged` + 批处理机制。

## 问题提醒
- 多选场景仍未处理：Viewport 多选会只取第一项进行同步，若团队需要批量同步需追加策略。
- 当前实现默认 Tree 选择会清空视口选择，如果仍需配合引擎原生多选，需要额外判断 `ESelectInfo::Type` 并保留已有选择集。

## 下一步建议
1. **可配置自动聚焦**：为自动选择/聚焦视口添加 Editor Preference，允许用户关闭 Tree→Viewport 的 SelectNone 行为。
2. **多选与高亮扩展**：完善多选同步策略，并允许同时高亮 Registered Props 与 Act 节点，避免不同视角下的定位困难。
