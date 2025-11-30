# Phase 8.1: SceneOutliner 基础架构实现

> 日期: 2025-11-29
> 状态: ✅ 完成
> 任务: 基于 SceneOutliner 框架构建自定义 DataLayer 浏览器的基础架构

---

## 完成内容

### 1. FStageDataLayerTreeItem (树节点类型)

**文件**:
- `Public/DataLayerSync/StageDataLayerTreeItem.h`
- `Private/DataLayerSync/StageDataLayerTreeItem.cpp`

**功能**:
- 表示单个 DataLayer 节点
- 实现 `ISceneOutlinerTreeItem` 接口
- 提供 DataLayer 名称显示和编辑
- 支持可见性状态切换
- 高亮显示包含选中 Actor 的 DataLayer
- 自定义 Label Widget（带图标和颜色）

**关键实现**:
```cpp
// 静态类型定义
const FSceneOutlinerTreeItemType FStageDataLayerTreeItem::Type(&ISceneOutlinerTreeItem::Type);

// Label Widget 生成
TSharedRef<SWidget> GenerateLabelWidget(ISceneOutliner& Outliner, const STableRow<FSceneOutlinerTreeItemPtr>& InRow);

// 可见性控制
void OnVisibilityChanged(const bool bNewVisibility);
bool GetVisibility() const;
```

### 2. FStageDataLayerHierarchy (数据层级结构)

**文件**:
- `Public/DataLayerSync/StageDataLayerHierarchy.h`
- `Private/DataLayerSync/StageDataLayerHierarchy.cpp`

**功能**:
- 定义 DataLayer 树的层级结构
- 从 UDataLayerManager 枚举所有 DataLayer
- 支持 DataLayer 父子关系
- 监听 DataLayer 变化事件并刷新

**关键实现**:
```cpp
// 工厂方法
static TUniquePtr<FStageDataLayerHierarchy> Create(FStageDataLayerMode* Mode, const TWeakObjectPtr<UWorld>& World);

// 创建树项
void CreateItems(TArray<FSceneOutlinerTreeItemPtr>& OutItems) const;

// 查找/创建父项
FSceneOutlinerTreeItemPtr FindOrCreateParentItem(const ISceneOutlinerTreeItem& Item, ...);
```

**事件订阅**:
- `GEngine->OnLevelActorListChanged()` - Actor 列表变化
- `DataLayerEditorSubsystem->OnDataLayerChanged()` - DataLayer 变化
- `DataLayerEditorSubsystem->OnActorDataLayersChanged()` - Actor DataLayer 成员变化

### 3. FStageDataLayerMode (Outliner 模式)

**文件**:
- `Public/DataLayerSync/StageDataLayerMode.h`
- `Private/DataLayerSync/StageDataLayerMode.cpp`

**功能**:
- 定义 Outliner 的行为和数据源
- 处理选择、双击、键盘事件
- 提供右键菜单
- 管理筛选和状态显示

**关键实现**:
```cpp
// 构造函数中初始化
FStageDataLayerMode(const FStageDataLayerModeParams& Params);

// 创建 Hierarchy
TUniquePtr<ISceneOutlinerHierarchy> CreateHierarchy();

// 选择处理
void OnItemSelectionChanged(FSceneOutlinerTreeItemPtr TreeItem, ESelectInfo::Type SelectionType, const FSceneOutlinerItemSelection& Selection);

// 右键菜单
TSharedPtr<SWidget> CreateContextMenu();

// 获取选中项
TArray<UDataLayerInstance*> GetSelectedDataLayers() const;
```

**右键菜单功能**:
- Toggle Visibility - 切换可见性
- Make Visible - 显示
- Make Hidden - 隐藏

---

## 设计决策

### 1. 不显示 Actor 子节点

与原生 DataLayerBrowser 不同，我们的实现只显示 DataLayer 节点，不显示 Actor 子节点。

**原因**:
- 简化实现
- 聚焦于 DataLayer 管理而非 Actor 成员查看
- 减少性能开销

### 2. 事件驱动刷新

使用 `FullRefreshEvent()` 进行完整刷新，而非逐项增量更新。

**原因**:
- 实现更简单可靠
- 对于中小规模的 DataLayer 列表足够高效
- 避免复杂的增量同步逻辑

### 3. 选中状态缓存

在 `OnItemSelectionChanged` 中缓存选中的 DataLayer，用于在树刷新后恢复选择。

---

## 编译验证

✅ 编译成功，无警告

---

## 下一步: Phase 8.2 - 自定义列

待实现:
- [ ] FStageDataLayerLabelColumn - 名称列
- [ ] FStageDataLayerSyncStatusColumn - 同步状态列
- [ ] FStageDataLayerSUIDColumn - SUID 列
- [ ] FStageDataLayerActionsColumn - 操作按钮列

---

## 代码统计

| 文件 | 行数 |
|------|------|
| StageDataLayerTreeItem.h | ~115 |
| StageDataLayerTreeItem.cpp | ~175 |
| StageDataLayerHierarchy.h | ~95 |
| StageDataLayerHierarchy.cpp | ~110 |
| StageDataLayerMode.h | ~170 |
| StageDataLayerMode.cpp | ~490 |
| **总计** | **~1155** |

---

## 重命名记录 (2025-11-29)

将 `SStageDataLayerBrowser` 重命名为 `SStageDataLayerOutliner`，因为现在基于 SceneOutliner 框架构建，不再是包装器方案。

| 旧名称 | 新名称 |
|--------|--------|
| `SStageDataLayerBrowser` | `SStageDataLayerOutliner` |
| `StageDataLayerBrowserTabName` | `StageDataLayerOutlinerTabName` |
| `StageDataLayerBrowser.ContextMenu` | `StageDataLayerOutliner.ContextMenu` |

---

*文档创建: 2025-11-29*
