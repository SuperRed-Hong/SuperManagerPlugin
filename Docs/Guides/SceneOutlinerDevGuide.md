# Scene Outliner 插件开发速查

> 引擎源码参考：`Docs/UESource/Source/Editor/SceneOutliner/`。

## 1. 模块和入口
- **模块位置**：`SceneOutliner` 编辑器模块（`SceneOutlinerModule.h/.cpp`）。通过 `FSceneOutlinerModule::CreateSceneOutliner` 或 `CreateSceneOutlinerTreeView` 搭建 Outliner。插件通常在 `StartupModule` 中向该模块注册自定义列/扩展。
- **核心接口**：`ISceneOutliner`（`Public/ISceneOutliner.h`）是树视图的控制器。列、过滤器、模式等都通过它访问 Outliner 状态。
- **初始化配置**：`FSceneOutlinerInitializationOptions` 定义 Outliner 行为（显示锁、允许重命名等），位于 `Public/SceneOutlinerInitializationOptions.h`。创建 Outliner 时必须提供。

## 2. 树与项
- **树项类型**：`ISceneOutlinerTreeItem` 是所有行数据的基类（`Public/ISceneOutlinerTreeItem.h`）。常见派生有 `FActorTreeItem`、`FFolderTreeItem` 等，位于 `Private/SceneOutlinerTreeItem...` 文件。
- **Tree 访问**：`ISceneOutliner::GetTree()` 返回 `TSharedPtr<FSceneOutlinerTree>`，可遍历当前行、刷新、折叠等。
- **行数据解析**：`ISceneOutlinerTreeItem::As<T>()`/`CastTo<T>()` 用于把通用项转换为具体类型，获取 Actor/Folder 指针。

## 3. 选择模型
- **接口**：`FSceneOutlinerSelection`（`Public/SceneOutlinerSelection.h`）封装当前选中行，包含 `GetSelectedItems()`, `IsSelected(TreeItem)` 等。
- **选中同步**：列或自定义行可通过 `ISceneOutliner::GetSelection()` 访问模型；更改选中需使用 `FSceneOutlinerSelection::Select(TreeItem, ESelectInfo::Type)`，再调用 `ISceneOutliner::SyncSelection()` 同步到编辑器。
- **交互注意**：Actor Picker 模式下通常禁止改变世界选中。先查询 `ISceneOutliner::GetMode()`（`Public/SceneOutlinerModes.h`）以决定是否允许修改。

## 4. 列系统
- **列接口**：实现 `ISceneOutlinerColumn`（`Public/SceneOutlinerColumn.h`）。构造函数通常接收 `ISceneOutliner&` 以访问树、选中、过滤器等上下文。
- **列注册**：通过 `FSceneOutlinerModule::RegisterDefaultColumnType` 或在 `FSceneOutlinerInitializationOptions` 中 `ColumnMap.Add(ColumnId, MakeShared<FYourColumn>(SceneOutliner))`。
- **列信息**：`FSceneOutlinerColumnInfo` 定义列宽、排序支持、可见性（`Public/SceneOutlinerColumnInfo.h`）。
- **排序**：列若支持排序，需实现 `ISceneOutlinerColumn::SortItems`；Outliner 会传入 `EColumnSortMode::Type` 与当前 `ISceneOutlinerTreeItem&`。

## 5. 过滤与模式
- **过滤器**：`FSceneOutlinerFilters` (`Public/SceneOutlinerFilters.h`) 管理所有过滤规则。`ISceneOutliner::GetFilterCollection()` 返回可修改集合；自定义列可添加 lambda 过滤器。
- **模式**：`ISceneOutlinerMode` 的派生类（`Private/Modes/*`）定义 Outliner 行为。常见模式：`FActorMode`, `FComponentMode`, `FSceneOutlinerActorPickerMode`。列的功能应根据 `ISceneOutliner::GetMode()` 判断可用性。

## 6. 常见上下文 API
| API | 功能 | 文件 |
| --- | --- | --- |
| `ISceneOutliner::GetSelection()` | 取得选中模型 | `ISceneOutliner.h` |
| `ISceneOutliner::Refresh()` | 刷新树 | `ISceneOutliner.h` |
| `ISceneOutliner::AddColumn` | 动态添加列 | `ISceneOutliner.h` |
| `FSceneOutlinerTreeItem::GetDisplayString()` | 行显示文本 | `ISceneOutlinerTreeItem.h` |
| `ISceneOutliner::GetMutableTree()` | 获取可变树引用（例如强制展开/折叠） | `ISceneOutliner.h` |
| `ISceneOutliner::OnItemSelectionChanged()` | 选中委托 | `ISceneOutliner.h` |

## 7. 开发步骤建议
1. **阅读接口**：从 `Public/ISceneOutliner.h`、`SceneOutlinerInitializationOptions.h` 着手，了解 `ISceneOutliner` 提供的上下文。
2. **参考内建列**：引擎自带 `FOutlinerActorLabelColumn`, `FOutlinerGutter`（在 `Private/Columns/`）是最佳示例，展示了如何使用 `ISceneOutliner`、Selection、TreeItem。
3. **实现列**：派生 `ISceneOutlinerColumn`，持有 `ISceneOutliner&`，在 `ConstructRowWidget` 中返回自定义 `SWidget`（通常是 `SHorizontalBox` + 自定义按钮/文本）。
4. **处理交互**：通过 Selection Model、Mode、Filter 等接口读取/写入状态，必要时触发 `SceneOutliner.Refresh()`。
5. **注册/清理**：在插件 `StartupModule` 注册列（`FSceneOutlinerModule::RegisterDefaultColumnType`），在 `ShutdownModule` 反注册。若使用 `FModuleManager::LoadModuleChecked`, 记得包含 `SceneOutlinerModule.h`。

## 8. 常见坑
- **模式限制**：Actor Picker 模式往往禁止修改锁定/隐藏状态，务必检查 `ISceneOutliner::GetMode()`。
- **性能**：列 `ConstructRowWidget` 会频繁调用，避免执行昂贵操作（例如遍历世界所有 Actor）。
- **生命周期**：列持有 `ISceneOutliner&`，不要缓存行对象的裸指针；使用 `TWeakPtr<ISceneOutlinerTreeItem>`。
- **同步**：对选中/过滤器的修改完成后调用 `SceneOutliner.Refresh()` 或 `SyncSelection()`，否则不会立即生效。

## 9. 进一步阅读
- `Docs/UESource/Source/Editor/SceneOutliner/Public/ISceneOutliner.h`
- `SceneOutlinerModule.cpp`（了解列注册流程）
- `Private/Columns/` 下的引擎内建列实现
- `SceneOutlinerFilters.h` & `SceneOutlinerFilterInfo.h`（自定义过滤器）
- `SceneOutlinerTreeItem.h` & `SceneOutlinerTreeItemTypes.h`
