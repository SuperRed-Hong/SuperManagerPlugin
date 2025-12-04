# StageEditor 选择同步与 Prop 状态编辑技术方案

## 问题与目标
- Panel 与场景视口之间缺少选中同步，Stage/Prop 在 TreeView 与 World Outliner 之间状态割裂，极易造成误操作。
- Act 下的 Prop 节点仅显示状态却无法编辑，用户需要跳回 Details 面板手动修改 `PropStateOverrides`，流程断裂。
- 需要在不破坏现有 Drag&Drop、Act/Prop 管理逻辑的前提下扩展 UI，保持所有操作在 `FStageEditorController` 的受控范围内。

## 设计原则
1. **单向数据源**：Stage/Act/Prop 数据依旧由 Controller 收敛，UI 只读 Model，所有写操作调用 Controller API。
2. **双向同步防循环**：面向视口与 TreeView 的同步都带互斥锁标记，保证不会互相触发无限回调。
3. **最小侵入**：重用现有 TreeItem 结构，通过附加缓存（Actor->TreeItem）与 `StageTreeView->SetSelection` 实现同步。
4. **可扩展 UI**：Prop 状态编辑采用可重用控件（`SSpinBox<int32>` 触发 Controller 调整），留出 Hook 支持未来替换成 Slider、Context Menu 等形式。

## 架构与流程
```text
GEditor USelection            SStageEditorPanel            FStageEditorController
        |                             |                                 |
        | SelectionChangedEvent ----> | HandleViewportSelection()       |
        |                             |  - 查找 TreeItem
        |                             |  - Expand 并 SetSelection        |
        |                             |  - 设置 Guard Flag               |
        |                             |                                  |
        |                             | OnSelectionChanged() -------->   |
        |                             |  - Panel -> Viewport             |  (SelectNone + SelectActor)
        |                             |  - Guard Flag 防循环            |
        |                             |                                  |
        |                             | PropStateSpinBox OnValueChanged  |--> SetPropStateInAct()
        |                             |                                  |    - ScopedTransaction
```

## 数据模型（JSON Schema）
```json
{
  "$schema": "https://json-schema.org/draft/2020-12/schema",
  "$id": "StageEditor.SelectionSync.TreeItemMap",
  "title": "TreeItemActorMap",
  "type": "object",
  "properties": {
    "StageId": {
      "type": "integer",
      "description": "AStage::StageID，保证 Stage Row 唯一"
    },
    "ActorPath": {
      "type": "string",
      "description": "Actor->GetPathName()，用于跨刷新匹配"
    },
    "TreeItemId": {
      "type": "string",
      "description": "GetItemHash 结果，TreeView 选择使用"
    },
    "PropId": {
      "type": "integer",
      "description": "Stage::PropRegistry 的 Key"
    },
    "ActId": {
      "type": "integer",
      "description": "FAct::ActID.ActID，用于 PropStateOverrides 定位"
    }
  },
  "required": ["ActorPath", "TreeItemId"],
  "additionalProperties": false
}
```

缓存策略：`TMap<FString /*ActorPath*/, TWeakPtr<FStageTreeItem>> ActorToItemMap` 在 `RefreshUI` 中重建，并携带 Act/Prop ID 方便状态写入。

## 接口契约（OpenAPI）
```yaml
openapi: 3.0.1
info:
  title: StageEditor Internal Commands
  version: 1.0.0
paths:
  /selection/panel-to-viewport:
    post:
      summary: TreeView 选择驱动视口选中
      requestBody:
        required: true
        content:
          application/json:
            schema:
              type: object
              properties:
                itemType:
                  type: string
                  enum: [Stage, Act, Prop]
                actorPath:
                  type: string
                propId:
                  type: integer
                actId:
                  type: integer
              required: [itemType]
      responses:
        '200': { description: Selection applied }
  /selection/viewport-to-panel:
    post:
      summary: 视口选中驱动 TreeView
      requestBody:
        required: true
        content:
          application/json:
            schema:
              type: object
              properties:
                actorPath:
                  type: string
                actorClass:
                  type: string
              required: [actorPath]
      responses:
        '200': { description: TreeView selection updated }
  /acts/{actId}/props/{propId}/state:
    patch:
      summary: 编辑 Act 下 Prop 的状态值
      parameters:
        - name: actId
          in: path
          schema:
            type: integer
          required: true
        - name: propId
          in: path
          schema:
            type: integer
          required: true
      requestBody:
        required: true
        content:
          application/json:
            schema:
              type: object
              properties:
                state:
                  type: integer
                  description: 目标状态值
              required: [state]
      responses:
        '200': { description: 状态已更新 }
```

> 注：以上 OpenAPI 仅描述内部命令契约，真实实现由 Slate 回调调用 `FStageEditorController`。

## 关键实现要点
1. `SStageEditorPanel`
   - 新增 `FDelegateHandle ViewportSelectionHandle`，在 `Construct` 时对 `GEditor->GetSelectedActors()->SelectionChangedEvent` 进行 `AddSP`，`Destruct` 时移除。
   - 添加 `TMap<FString, TWeakPtr<FStageTreeItem>> ActorPathToItem` 与 `bool bIsUpdatingSelectionFromViewport` 标记。
   - `OnSelectionChanged` 中在处理 Stage/Prop 时调用 `GEditor->SelectNone/SelectActor`，并在 Guard 下执行。
   - Act 下 Prop Row 在文本后加入 `SSpinBox<int32>` 或 `SEditableTextBox`，`OnValueCommitted` 调用 `Controller->SetPropStateInAct`。
2. `FStageEditorController`
   - 现有 `SetPropStateInAct` 已实现，但需要确保在 PropStateOverrides 中自动新增 Key（若 Prop 未在当前 Act 中存在时创建默认值），并广播刷新。
   - 提供 `GetActStateForProp` 帮助 UI 拉取当前值。
3. 事件解绑：`SStageEditorPanel` 析构或 `OnDragLeave` 里需要清理 `DraggedOverItem`，避免 dangling。

## 非功能性要求
- **性能**：`ActorPathToItem` 缓存保证 Selection 同步为 O(1)，`RefreshUI` 时一次扫描即可。
- **Undo/Redo**：所有状态写入继续通过 Controller（已包裹 `FScopedTransaction`）。
- **可维护性**：UI 改动封装为独立私有函数 `CreatePropStateWidget`，日后可改为 Popup/Context Menu。

## 标准遵循检查清单
- [x] 命名遵循 UE 前缀与 camelCase 规则。
- [x] 所有指针操作均做合法性检查（`IsValid()` / `if (Ptr)`）。
- [x] Slate 交互使用 `TSharedPtr` 生命周期绑定，避免悬挂引用。
- [x] 事务性修改包裹 `FScopedTransaction`，保持 Undo/Redo。
- [x] Doxygen 注释计划在公开函数上补充（Controller 新增接口等）。

## 验收要点
1. 在 Panel 中点击 Stage/Prop，视口自动选中对应 Actor，双向同步无死循环。
2. Act 节点下每个 Prop 行可编辑状态值，修改即刻反映在 TreeView 与 Controller 数据中。
3. 切换关卡或刷新 Panel 后，选择同步和状态编辑仍生效。

