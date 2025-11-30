# Phase 8: DataLayerOutliner UI 扩展预研

> 日期: 2025-11-29
> 状态: ✅ 预研完成，方案已确定
> 决策: 采用方案 B（基于 SceneOutliner 框架构建自定义 DataLayer 浏览器）

---

## 重要澄清

**方案 B 不是"从零自建 TreeView"**，而是**复用 UE SceneOutliner 框架**来构建我们自己的 DataLayer 浏览器。

```
┌─────────────────────────────────────────────────────────────┐
│                    SceneOutliner 框架                        │
│  (UE 提供的公开基础设施：TreeView、渲染、选择、拖拽等)         │
└─────────────────────────────────────────────────────────────┘
                              ↑
              ┌───────────────┴───────────────┐
              │                               │
   ┌──────────┴──────────┐         ┌──────────┴──────────┐
   │  原生 DataLayerBrowser │         │  我们的 StageDataLayer │
   │  (引擎 Private 实现)   │         │  Browser (自建)        │
   ├─────────────────────┤         ├─────────────────────┤
   │ FDataLayerMode      │         │ FStageDataLayerMode │
   │ DataLayerHierarchy  │         │ StageDataLayerHierarchy │
   │ DataLayerTreeItem   │         │ StageDataLayerTreeItem │
   │ 原生 Columns        │         │ 自定义 Columns       │
   └─────────────────────┘         └─────────────────────┘
```

### 我们需要实现的 vs 不需要实现的

| 类别 | 内容 | 是否需要我们实现 |
|------|------|-----------------|
| **TreeView 渲染** | 行绘制、虚拟滚动、缓存 | ❌ SceneOutliner 提供 |
| **选择系统** | 单选/多选、Shift/Ctrl 修饰键 | ❌ SceneOutliner 提供 |
| **展开/折叠** | 树节点状态管理 | ❌ SceneOutliner 提供 |
| **拖拽基础** | 拖拽检测、视觉反馈 | ❌ SceneOutliner 提供 |
| **列系统** | HeaderRow、列布局 | ❌ SceneOutliner 提供 |
| **Mode** | 定义 Outliner 的行为和数据源 | ✅ 需要实现 |
| **Hierarchy** | 定义树的层级结构 | ✅ 需要实现 |
| **TreeItem** | 定义节点类型和显示 | ✅ 需要实现 |
| **自定义 Column** | SyncStatus、SUID、Actions | ✅ 需要实现 |

### 代码量估算

```cpp
// 我们需要实现的核心类（参考引擎实现）
FStageDataLayerMode        // ~200-300 行
FStageDataLayerHierarchy   // ~100-200 行
FStageDataLayerTreeItem    // ~150-250 行
FStageDataLayerLabelColumn // ~50-100 行
FStageDataLayerSyncStatusColumn  // ~80-120 行
FStageDataLayerSUIDColumn  // ~50-80 行
FStageDataLayerActionsColumn // ~100-150 行

// 总计：约 700-1200 行
```

这与"从零自建 TreeView"（可能需要 5000+ 行）是完全不同的工作量级别。

---

## 1. 问题背景

### 1.1 当前实现状态

Phase 4 采用了**包装器方案**：

```cpp
// SStageDataLayerBrowser.cpp
FDataLayerEditorModule& DataLayerEditorModule =
    FModuleManager::LoadModuleChecked<FDataLayerEditorModule>("DataLayerEditor");
TSharedRef<SWidget> NativeDataLayerBrowser = DataLayerEditorModule.CreateDataLayerBrowser();
```

将原生 `DataLayerBrowser` 作为黑盒嵌入。

### 1.2 存在的问题

| 问题 | 影响 |
|------|------|
| 无法添加自定义列 | Sync Status、SUID、Actions 列无法实现 |
| 无法获取选中项 | `SelectedDataLayersSet` 始终为空 |
| 无法 Hook 右键菜单 | 只能通过全局扩展器 |

### 1.3 目标 UI

```
┌─────────────┬──────────────────────┬────────────┬──────────────┐
│ Sync Status │ Item Label           │ SUID       │ Actions      │
├─────────────┼──────────────────────┼────────────┼──────────────┤
│ ✓ (绿色)    │ DL_Stage_Castle      │ S:1        │ [Sync]       │
│ ⚠ (橙色)    │ DL_Act_Castle_Ext    │ A:1.2      │ [Sync]       │
│ ● (蓝色)    │ DL_Stage_Forest      │            │ [Import]     │
└─────────────┴──────────────────────┴────────────┴──────────────┘
```

---

## 2. 引擎源码分析

### 2.1 SceneOutliner 架构

```
引擎路径: Engine/Source/Editor/SceneOutliner/

FSceneOutlinerModule (Public)
├── RegisterColumnType<T>()           # 注册全局列类型
├── RegisterDefaultColumnType<T>()    # 注册默认列（会出现在所有 Outliner）
├── FactoryColumn()                   # 工厂方法创建列
├── ColumnMap                         # 列类型 -> 工厂的映射
└── DefaultColumnMap                  # 默认列映射

SSceneOutliner (Public)
├── AddColumn()                       # 运行时添加列
├── RemoveColumn()                    # 运行时移除列
└── RefreshColumns()                  # 刷新列

ISceneOutlinerColumn (Public)
├── GetColumnID()                     # 返回列 ID
├── ConstructHeaderRowColumn()        # 构建表头
└── ConstructRowWidget()              # 构建行单元格
```

### 2.2 DataLayerEditor 架构

```
引擎路径: Engine/Source/Editor/DataLayerEditor/

FDataLayerEditorModule (Public)
├── CreateDataLayerBrowser()          # 创建浏览器 Widget
├── SyncDataLayerBrowserToDataLayer() # 同步选中
└── GetAllDataLayersMenuExtenders()   # 右键菜单扩展器

SDataLayerBrowser (Private)           # ❌ 无法直接使用
├── DataLayerOutliner                 # private 成员
└── OnSelectionChanged()

SDataLayerOutliner : SSceneOutliner (Private)
└── CustomAddToToolbar()              # 自定义工具栏

FDataLayerMode : ISceneOutlinerMode (Private)  # ❌ 无法直接使用
└── 控制 DataLayer 树的行为
```

### 2.3 列注册机制详解

#### 全局注册 vs 实例注册

```cpp
// 方式1: 全局注册（会出现在所有使用 UseDefaultColumns() 的 Outliner）
FSceneOutlinerModule& Module = FModuleManager::LoadModuleChecked<FSceneOutlinerModule>("SceneOutliner");
Module.RegisterDefaultColumnType<FMyColumn>(ColumnInfo);

// 方式2: 实例注册（仅在特定 Outliner 实例中）
FSceneOutlinerInitializationOptions InitOptions;
InitOptions.ColumnMap.Add(FMyColumn::GetID(), ColumnInfo);
```

#### 为什么全局注册对 DataLayerOutliner 无效？

查看 `SSceneOutliner::SetupColumns()`（引擎源码 SSceneOutliner.cpp:531）：

```cpp
if (SharedData->ColumnMap.Num() == 0)
{
    SharedData->UseDefaultColumns();  // 只有 ColumnMap 为空时才加载默认列
}
```

查看 `SDataLayerBrowser::Construct()`（引擎源码 SDataLayerBrowser.cpp:149-165）：

```cpp
FSceneOutlinerInitializationOptions InitOptions;
// DataLayerBrowser 明确添加了自己的列
InitOptions.ColumnMap.Add(FDataLayerOutlinerIsVisibleColumn::GetID(), ...);
InitOptions.ColumnMap.Add(FDataLayerOutlinerIsLoadedInEditorColumn::GetID(), ...);
InitOptions.ColumnMap.Add(FSceneOutlinerBuiltInColumnTypes::Label(), ...);
InitOptions.ColumnMap.Add(FDataLayerOutlinerDeleteButtonColumn::GetID(), ...);
// ... 更多列
```

**结论**: `ColumnMap.Num() != 0`，不会调用 `UseDefaultColumns()`，全局注册的列不会出现。

### 2.4 SuperManager 的 LockedActorColumn 为什么能工作？

```cpp
// SuperManager.cpp:729
SceneOutlinerModule.RegisterDefaultColumnType<FOutlinerSelectionLockColumn>(SelectionLockColumnInfo);
```

这个列出现在 **World Outliner (ActorBrowser)** 中，因为：
- World Outliner 使用 `UseDefaultColumns()` 路径
- 它没有预设 `ColumnMap`

---

## 3. 备选方案评估

### 方案 A: 运行时 AddColumn

**思路**: 获取 `SDataLayerOutliner` 实例，调用 `AddColumn()` 动态添加列。

**实现**:
```cpp
void SStageDataLayerBrowser::Construct(...)
{
    // 创建原生 Browser
    TSharedRef<SWidget> NativeBrowser = DataLayerEditorModule.CreateDataLayerBrowser();

    // 遍历 Widget 树找到 SDataLayerOutliner
    TSharedPtr<SDataLayerOutliner> Outliner = FindDataLayerOutliner(NativeBrowser);

    // 动态添加列
    Outliner->AddColumn(TEXT("SyncStatus"), SyncStatusColumnInfo);
    Outliner->AddColumn(TEXT("SUID"), SUIDColumnInfo);
}
```

**问题**:
- `SDataLayerBrowser` 和 `SDataLayerOutliner` 都在 Private 目录
- 需要遍历 Widget 树，脆弱且可能被引擎更新破坏
- 仍然无法获取选中回调

**评估**: ⚠️ 可行但不稳定

---

### 方案 B: 基于 SceneOutliner 框架构建自定义浏览器

**思路**: 复用 UE 的 SceneOutliner 框架（公开 API），实现自定义的 Mode/Hierarchy/TreeItem/Column。

> **重要澄清**: 这不是"从零自建 TreeView"，而是利用引擎提供的成熟框架。
> 原生 DataLayerBrowser 也是基于同一框架构建的，只是它的实现在 Private 目录我们无法直接使用。

**架构**:
```
SStageDataLayerBrowser (自建)
├── 工具栏 [Sync All] [Import Selected]
└── SSceneOutliner (UE 公开框架，提供 TreeView 基础能力)
    ├── FStageDataLayerMode : ISceneOutlinerMode
    │   ├── 数据源：UDataLayerManager
    │   └── 行为：选择、展开、右键菜单
    │
    ├── FStageDataLayerHierarchy : ISceneOutlinerHierarchy
    │   └── 构建 DataLayer 树结构
    │
    ├── FStageDataLayerTreeItem : ISceneOutlinerTreeItem
    │   └── 表示单个 DataLayer 节点
    │
    └── 自定义列
        ├── FStageDataLayerSyncStatusColumn
        ├── FStageDataLayerSUIDColumn
        └── FStageDataLayerActionsColumn
```

**SceneOutliner 框架提供的能力**（我们不需要实现）:
- TreeView 渲染（行绘制、虚拟滚动、缓存）
- 选择系统（单选/多选、Shift/Ctrl 修饰键）
- 展开/折叠状态管理
- 拖拽基础设施
- 列系统（HeaderRow、列布局、列排序）

**我们需要实现的**（业务逻辑层，约 700-1200 行）:
- Mode: 定义数据源和行为
- Hierarchy: 定义树的层级结构
- TreeItem: 定义节点类型
- Columns: 自定义列渲染

**优点**:
- 完全可控，可添加任意列
- 可获取选中回调
- 不依赖 Private API
- 升级安全（使用公开框架）
- 工作量可控（复用框架能力）

**缺点**:
- 需要理解 SceneOutliner 框架的接口设计
- 需要与原生 DataLayer 系统保持数据同步

**评估**: ✅ 推荐，最佳平衡点

---

### 方案 C: Hook InitOptions

**思路**: 在 `CreateDataLayerBrowser()` 创建过程中注入自定义列。

**问题**:
- `FDataLayerEditorModule::CreateDataLayerBrowser()` 无参数无回调
- 无法在创建前/后注入 InitOptions

**评估**: ❌ 不可行

---

### 方案 D: 右键菜单扩展

**思路**: 通过 `GetAllDataLayersMenuExtenders()` 扩展右键菜单。

**实现**:
```cpp
FDataLayerEditorModule& Module = FModuleManager::LoadModuleChecked<FDataLayerEditorModule>("DataLayerEditor");
Module.GetAllDataLayersMenuExtenders().Add(
    FDataLayerEditorModule::FDataLayersMenuExtender::CreateLambda(
        [](const TSharedRef<FUICommandList>& Commands,
           const TSet<TWeakObjectPtr<const UDataLayerInstance>>& SelectedDataLayers,
           const TSet<FDataLayerActor>& SelectedDataLayerActors)
        {
            // 添加 Import / Sync 菜单项
            return MakeShareable(new FExtender);
        }
    )
);
```

**优点**:
- 官方支持的扩展点
- 实现简单

**缺点**:
- 无法添加自定义列
- 无状态可视化（Sync Status 图标）
- 用户体验差

**评估**: ⚠️ 可作为补充，但不满足需求

---

## 4. 方案对比

| 方案 | 自定义列 | 选中回调 | 稳定性 | 工作量 | 推荐度 |
|------|---------|---------|--------|--------|-------|
| A. 运行时 AddColumn | ✅ | ❌ | ⚠️ 低 | 中 | ⭐⭐ |
| **B. 基于 SceneOutliner 框架** | ✅ | ✅ | ✅ 高 | **中** | ⭐⭐⭐⭐ |
| C. Hook InitOptions | - | - | - | - | ❌ |
| D. 右键菜单扩展 | ❌ | ⚠️ 有限 | ✅ 高 | 低 | ⭐⭐ |

> **注**: 方案 B 的工作量是"中"而非"高"，因为我们复用 SceneOutliner 框架，
> 只需实现业务逻辑层（约 700-1200 行），而非从零实现 TreeView（5000+ 行）。

---

## 5. 最终决策

### 选择方案 B: 基于 SceneOutliner 框架构建自定义浏览器

**理由**:
1. **灵活度**: 完全可控，可实现所有目标 UI 功能
2. **稳定性**: 使用公开 API，不依赖 Private 实现
3. **扩展性**: 未来可添加更多功能（拖拽、批量操作等）
4. **工作量可控**: 复用 SceneOutliner 框架，只需实现业务逻辑层
5. **用户明确要求**: 用户不接受方案 A/D 的灵活度限制

**接受的代价**:
- 需要理解 SceneOutliner 框架的 Mode/Hierarchy/TreeItem 接口
- 需要与原生 DataLayer 系统保持数据同步

**预计工作量**:
- 核心类实现: 约 700-1200 行代码
- 开发周期: 中等（框架学习曲线后可快速开发）

---

## 6. 实施路线图

### Phase 8.1: 基础架构
- [ ] 创建 `FStageDataLayerHierarchy` - 数据层级结构
- [ ] 创建 `FStageDataLayerTreeItem` - 树节点类型
- [ ] 创建 `FStageDataLayerMode` - Outliner 模式

### Phase 8.2: 自定义列
- [ ] 创建 `FStageDataLayerLabelColumn` - 名称列
- [ ] 创建 `FStageDataLayerSyncStatusColumn` - 同步状态列
- [ ] 创建 `FStageDataLayerSUIDColumn` - SUID 列
- [ ] 创建 `FStageDataLayerActionsColumn` - 操作按钮列

### Phase 8.3: 集成
- [ ] 重构 `SStageDataLayerBrowser` 使用自建 Outliner
- [ ] 实现选中回调
- [ ] 实现右键菜单
- [ ] 事件同步（DataLayer 变化时刷新）

### Phase 8.4: 功能完善
- [ ] 拖拽支持（可选）
- [ ] 搜索过滤
- [ ] 列可见性持久化

---

## 7. 参考资源

### 引擎源码（只读参考）

| 文件 | 路径 | 用途 |
|------|------|------|
| SDataLayerBrowser.cpp | `DataLayerEditor/Private/DataLayer/` | 了解原生实现 |
| FDataLayerMode.cpp | `DataLayerEditor/Private/DataLayer/` | Mode 实现参考 |
| DataLayerHierarchy.cpp | `DataLayerEditor/Private/DataLayer/` | Hierarchy 实现参考 |
| DataLayerTreeItem.cpp | `DataLayerEditor/Private/DataLayer/` | TreeItem 实现参考 |

### 项目内参考

| 文件 | 路径 | 用途 |
|------|------|------|
| OutlinerSelectionColumn.h | `SuperManager/Public/CustomOutlinerColumn/` | Column 实现参考 |
| SuperManager.cpp | `SuperManager/Private/` | Column 注册参考 |

### 公开 API

| 文件 | 路径 | 用途 |
|------|------|------|
| ISceneOutlinerMode.h | `SceneOutliner/Public/` | Mode 接口 |
| ISceneOutlinerColumn.h | `SceneOutliner/Public/` | Column 接口 |
| ISceneOutlinerHierarchy.h | `SceneOutliner/Public/` | Hierarchy 接口 |
| ISceneOutlinerTreeItem.h | `SceneOutliner/Public/` | TreeItem 接口 |
| SSceneOutliner.h | `SceneOutliner/Public/` | Outliner Widget |

---

## 8. 关键代码片段

### 8.1 SceneOutliner 初始化示例

```cpp
// 参考 SDataLayerBrowser.cpp
FSceneOutlinerInitializationOptions InitOptions;
InitOptions.OutlinerIdentifier = TEXT("StageDataLayerOutliner");
InitOptions.bShowHeaderRow = true;
InitOptions.bShowParentTree = true;
InitOptions.bShowCreateNewFolder = false;
InitOptions.bShowTransient = true;

// 设置 Mode 工厂
InitOptions.ModeFactory = FCreateSceneOutlinerMode::CreateLambda(
    [this](SSceneOutliner* Outliner)
    {
        return new FStageDataLayerMode(Outliner, this);
    }
);

// 添加列
InitOptions.ColumnMap.Add(
    FStageDataLayerSyncStatusColumn::GetID(),
    FSceneOutlinerColumnInfo(
        ESceneOutlinerColumnVisibility::Visible,
        0,  // 优先级
        FCreateSceneOutlinerColumn::CreateLambda(
            [](ISceneOutliner& InSceneOutliner)
            {
                return MakeShareable(new FStageDataLayerSyncStatusColumn(InSceneOutliner));
            }
        )
    )
);

// 创建 Outliner
DataLayerOutliner = SNew(SSceneOutliner, InitOptions);
```

### 8.2 Column 实现示例

```cpp
// 参考 SuperManager/OutlinerSelectionColumn.h
class FStageDataLayerSyncStatusColumn : public ISceneOutlinerColumn
{
public:
    FStageDataLayerSyncStatusColumn(ISceneOutliner& SceneOutliner);

    virtual FName GetColumnID() override { return GetID(); }
    static FName GetID() { return FName("SyncStatus"); }

    virtual SHeaderRow::FColumn::FArguments ConstructHeaderRowColumn() override;
    virtual const TSharedRef<SWidget> ConstructRowWidget(
        FSceneOutlinerTreeItemRef TreeItem,
        const STableRow<FSceneOutlinerTreeItemPtr>& Row) override;
};
```

---

*文档结束*
*创建日期: 2025-11-29*
