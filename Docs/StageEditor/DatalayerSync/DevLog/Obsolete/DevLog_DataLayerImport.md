# DataLayer 导入功能 - 开发流程日志

> 创建日期: 2025-11-29
> 状态: 开发中
> 关联文档: PRD_DataLayerImport.md, TechSpec_DataLayerImport.md

---

## 目录

1. [功能概述](#1-功能概述)
2. [Phase 4 预研：DataLayerOutliner 扩展机制](#2-phase-4-预研datalayeroutliner-扩展机制)
3. [实施记录](#3-实施记录)
4. [问题与解决方案](#4-问题与解决方案)
5. [待办事项](#5-待办事项)

---

## 1. 功能概述

### 1.1 目标

让 StageEditor 能够根据已有的 DataLayer 数据自动创建 Stage-Act-Prop 架构，实现"项目中途接管"能力。

### 1.2 核心流程

```
用户已有 DataLayer → 按规范重命名 → 一键导入 → Stage-Act-Prop 架构
                                         ↓
                                  后续变化自动检测 → 提示同步
```

### 1.3 任务分解（7 Phases）

| Phase | 任务 | 状态 | 说明 |
|-------|------|------|------|
| 1 | 元数据基础设施 | 待开始 | UAssetUserData 存储导入状态 |
| 2 | 状态检测与提示系统 | 待开始 | 三态检测：未导入/已同步/不同步 |
| 3 | 命名解析模块 | 待开始 | 正则解析 DL_Stage_*/DL_Act_* |
| **4** | **DataLayerOutliner UI 扩展** | **进行中** | 新增列和按钮 |
| 5 | 导入逻辑与预览对话框 | 待开始 | 导入执行和 UI |
| 6 | 同步逻辑 | 待开始 | 增量同步 |
| 7 | 本地化（中英文） | 待开始 | LOCTEXT |

---

## 2. Phase 4 预研：DataLayerOutliner 扩展机制

> 日期: 2025-11-29 12:00-12:15
> 耗时: 约 15 分钟

### 2.1 预研目标

确定如何在 DataLayerOutliner 中添加自定义列（Sync Status、SUID、Actions）。

### 2.2 源码分析

#### 2.2.1 查阅的源码文件

引擎源码路径：`C:\Program Files\Epic Games\UE_5.6\Engine\Source\Editor\DataLayerEditor\`

| 文件 | 关键发现 |
|------|---------|
| `Private/DataLayer/SDataLayerOutliner.h` | `SDataLayerOutliner : SSceneOutliner` - DataLayerOutliner 继承自 SceneOutliner |
| `Private/DataLayer/SDataLayerBrowser.cpp` | **关键文件** - 列注册代码在此，通过 `InitOptions.ColumnMap.Add()` |
| `Private/DataLayer/DataLayerMode.h` | `FDataLayerMode : ISceneOutlinerMode` - 模式定义 |
| `Private/DataLayer/DataLayerOutlinerIsLoadedInEditorColumn.h/cpp` | 列实现示例，实现 `ISceneOutlinerColumn` 接口 |
| `Public/DataLayer/DataLayerEditorSubsystem.h` | **关键文件** - 事件广播机制 |

#### 2.2.2 架构理解

```
SDataLayerBrowser (主容器 Widget)
└── SDataLayerOutliner : SSceneOutliner (TreeView)
    └── FDataLayerMode : ISceneOutlinerMode (模式，控制行为)
        └── ISceneOutlinerColumn (列接口)
```

#### 2.2.3 列注册机制

**关键代码位置**：`SDataLayerBrowser.cpp` 第 149-165 行

```cpp
FSceneOutlinerInitializationOptions InitOptions;
InitOptions.OutlinerIdentifier = TEXT("DataLayerEditorOutliner");
InitOptions.bShowHeaderRow = true;
InitOptions.bShowParentTree = true;
InitOptions.bShowCreateNewFolder = false;
InitOptions.bShowTransient = true;
InitOptions.ModeFactory = FCreateSceneOutlinerMode::CreateLambda([this](SSceneOutliner* Outliner) {
    return new FDataLayerMode(FDataLayerModeParams(Outliner, this, nullptr));
});

// 列注册示例（第 156-164 行）
InitOptions.ColumnMap.Add(
    FDataLayerOutlinerIsVisibleColumn::GetID(),
    FSceneOutlinerColumnInfo(
        ESceneOutlinerColumnVisibility::Visible,  // 可见性
        0,                                         // 优先级（越小越靠左）
        FCreateSceneOutlinerColumn::CreateLambda([](ISceneOutliner& InSceneOutliner) {
            return MakeShareable(new FDataLayerOutlinerIsVisibleColumn(InSceneOutliner));
        })
    )
);
```

**发现要点**：
- 列通过 `InitOptions.ColumnMap.Add()` 注册
- `FSceneOutlinerColumnInfo` 包含可见性、优先级、工厂函数
- 优先级数字越小，列越靠左

#### 2.2.4 列实现接口

**关键代码位置**：`SceneOutliner/Public/ISceneOutlinerColumn.h`

```cpp
class ISceneOutlinerColumn : public TSharedFromThis<ISceneOutlinerColumn>
{
public:
    virtual FName GetColumnID() = 0;
    virtual SHeaderRow::FColumn::FArguments ConstructHeaderRowColumn() = 0;
    virtual const TSharedRef<SWidget> ConstructRowWidget(FSceneOutlinerTreeItemRef TreeItem, const STableRow<FSceneOutlinerTreeItemPtr>& Row) = 0;
    virtual bool SupportsSorting() const { return false; }
};
```

#### 2.2.5 列实现示例

**关键代码位置**：`DataLayerOutlinerIsLoadedInEditorColumn.cpp` 第 40-54 行（表头构建）

```cpp
SHeaderRow::FColumn::FArguments FDataLayerOutlinerIsLoadedInEditorColumn::ConstructHeaderRowColumn()
{
    return SHeaderRow::Column(GetColumnID())
        .FixedWidth(24.f)
        .HAlignHeader(HAlign_Center)
        .VAlignHeader(VAlign_Center)
        .HAlignCell(HAlign_Center)
        .VAlignCell(VAlign_Center)
        .DefaultTooltip(FText::FromName(GetColumnID()))
        [
            SNew(SImage)
            .Image(FAppStyle::GetBrush(TEXT("DataLayer.LoadedInEditor")))
            .ColorAndOpacity(FSlateColor::UseForeground())
        ];
}
```

**关键代码位置**：`DataLayerOutlinerIsLoadedInEditorColumn.cpp` 第 56-129 行（行内容构建）

```cpp
const TSharedRef<SWidget> FDataLayerOutlinerIsLoadedInEditorColumn::ConstructRowWidget(FSceneOutlinerTreeItemRef TreeItem, const STableRow<FSceneOutlinerTreeItemPtr>& Row)
{
    if (TreeItem->IsA<FDataLayerTreeItem>())
    {
        return SNew(SHorizontalBox)
            + SHorizontalBox::Slot()
            .Padding(0, 0, 0, 0)
            .AutoWidth()
            .VAlign(VAlign_Center)
            [
                SNew(SCheckBox)
                .IsEnabled_Lambda([TreeItem]() { /* ... */ })
                .IsChecked_Lambda([TreeItem]() { /* ... */ })
                .OnCheckStateChanged_Lambda([this, TreeItem](ECheckBoxState NewState) { /* ... */ })
                // ...
            ];
    }
    return SNullWidget::NullWidget;
}
```

**发现要点**：
- 使用 `TreeItem->IsA<FDataLayerTreeItem>()` 检查项类型
- 使用 Lambda 绑定动态状态
- 使用 `FScopedTransaction` 包装修改操作

#### 2.2.6 事件广播机制

**关键代码位置**：`DataLayerEditorSubsystem.h` 第 149-168 行

```cpp
/* Broadcasts whenever one or more DataLayers are modified
 *
 * Actions
 * Add    : The specified ChangedDataLayer is a newly created UDataLayerInstance
 * Modify : The specified ChangedDataLayer was just modified
 * Delete : A DataLayer was deleted
 * Rename : The specified ChangedDataLayer was just renamed
 * Reset  : A large amount of changes have occurred
 */
DECLARE_EVENT_ThreeParams(UDataLayerEditorSubsystem, FOnDataLayerChanged,
    const EDataLayerAction /*Action*/,
    const TWeakObjectPtr<const UDataLayerInstance>& /*ChangedDataLayer*/,
    const FName& /*ChangedProperty*/);
FOnDataLayerChanged& OnDataLayerChanged() { return DataLayerChanged; }

/** Broadcasts whenever one or more Actors changed UDataLayerInstances*/
DECLARE_EVENT_OneParam(UDataLayerEditorSubsystem, FOnActorDataLayersChanged,
    const TWeakObjectPtr<AActor>& /*ChangedActor*/);
FOnActorDataLayersChanged& OnActorDataLayersChanged() { return ActorDataLayersChanged; }
```

**发现要点**：
- `OnDataLayerChanged()` 事件包含 Action 类型（Add/Modify/Delete/Rename/Reset）
- `OnActorDataLayersChanged()` 事件用于 Actor 成员变化
- 可通过 `AddSP()` 订阅事件

### 2.3 方案讨论

#### 2.3.1 两个备选方案

| 方案 | 描述 | 优点 | 缺点 |
|------|------|------|------|
| **A: 扩展原有 DataLayerOutliner** | 使用 Hook 注入列到现有窗口 | 用户无需切换窗口 | 需要 hack 私有类，升级风险高 |
| **B: 创建独立 Tab** | 创建 "Stage DataLayer Browser" 独立窗口 | 完全可控，维护简单 | 用户需要打开新窗口 |

#### 2.3.2 关键讨论：同步问题

**用户提问**：如果创建独立 Tab，跟原生 DataLayerOutliner 会有同步问题吗？

**分析结论**：**不会有同步问题**

1. **数据层面**：两个 Tab 操作同一份底层数据（`UDataLayerInstance`, `UDataLayerAsset`）
2. **UI 刷新**：引擎提供完善的事件广播机制

**同步机制**：

| 场景 | 原生 Outliner | Stage Tab | 同步机制 |
|------|--------------|-----------|---------|
| 原生 Outliner 创建 DataLayer | 调用 API | 收到 `OnDataLayerChanged(Add)` | 自动刷新 |
| Stage Tab 导入 Stage | 调用相同 API | 自己刷新 | 原生收到事件 |
| 原生 Outliner 删除 DataLayer | 调用 API | 收到 `OnDataLayerChanged(Delete)` | 自动刷新 |

### 2.4 最终决策

**选择方案 B：创建独立的 Stage DataLayer Tab**

理由：
1. 代码更干净，不依赖引擎私有 API
2. 可以在 StageEditor Panel 旁边显示，工作流更顺畅
3. 未来 UE 升级不会破坏功能
4. 引擎事件机制保证数据同步

### 2.5 实施计划

Phase 4 细分为子任务：

| 子任务 | 描述 | 状态 |
|--------|------|------|
| 4.1 | 创建 SStageDataLayerBrowser 框架 | ✅ 完成 |
| 4.2 | 编译验证并测试 | ✅ 完成 |

---

## 3. 实施记录

### 3.1 Phase 4.1：创建 SStageDataLayerBrowser 框架

> 日期: 2025-11-29 12:25-12:32

#### 3.1.1 创建的文件

| 文件 | 说明 |
|------|------|
| `Public/DataLayerSync/SStageDataLayerBrowser.h` | 主 Widget 头文件 |
| `Private/DataLayerSync/SStageDataLayerBrowser.cpp` | 主 Widget 实现 |

#### 3.1.2 实现要点

**问题发现**：尝试直接创建自定义 `SSceneOutliner` 时遇到问题：
- `FDataLayerMode` 在 `Private/` 目录，无法直接使用
- 创建完整的自定义 `ISceneOutlinerMode` 实现工作量较大
- 需要实现 `CreateHierarchy()`、树形结构管理等复杂逻辑

**方案调整**：采用"包装器"方案
- 使用 `FDataLayerEditorModule::CreateDataLayerBrowser()` 创建原生 DataLayerBrowser
- 在其外部包装一层，添加我们的工具栏和额外 UI
- 保留原生 Outliner 的所有功能（树形视图、拖放、右键菜单等）

**核心代码**：

```cpp
void SStageDataLayerBrowser::Construct(const FArguments& InArgs)
{
    // 订阅事件
    SubscribeToEvents();

    // 获取原生 Browser
    FDataLayerEditorModule& DataLayerEditorModule = FModuleManager::LoadModuleChecked<FDataLayerEditorModule>("DataLayerEditor");
    TSharedRef<SWidget> NativeDataLayerBrowser = DataLayerEditorModule.CreateDataLayerBrowser();

    // 包装 UI
    ChildSlot
    [
        SNew(SVerticalBox)
        + SVerticalBox::Slot().AutoHeight()[ /* 标题 */ ]
        + SVerticalBox::Slot().AutoHeight()[ BuildToolbar() ]
        + SVerticalBox::Slot().AutoHeight()[ SNew(SSeparator) ]
        + SVerticalBox::Slot().FillHeight(1.0f)[ NativeDataLayerBrowser ]
    ];
}

void SStageDataLayerBrowser::SubscribeToEvents()
{
    if (UDataLayerEditorSubsystem* Subsystem = UDataLayerEditorSubsystem::Get())
    {
        OnDataLayerChangedHandle = Subsystem->OnDataLayerChanged().AddSP(
            this, &SStageDataLayerBrowser::OnDataLayerChanged);
        OnActorDataLayersChangedHandle = Subsystem->OnActorDataLayersChanged().AddSP(
            this, &SStageDataLayerBrowser::OnActorDataLayersChanged);
    }
}
```

**优点**：
1. 实现简单，复用原生功能
2. 与原生 DataLayerOutliner 保持一致体验
3. 可以在后续 Phase 根据需要扩展

**缺点**：
1. 无法添加自定义列（SyncStatus、SUID、Actions）
2. 需要通过其他 UI 元素（工具栏、弹窗）实现这些功能

#### 3.1.3 修改的文件

- `Public/StageEditorModule.h` - 添加新 Tab 名称和 spawn 函数
- `Private/StageEditorModule.cpp` - 注册新 Tab spawner

#### 3.1.4 编译验证

> 时间: 2025-11-29 12:32-12:35

```
Building 6 action(s)
Result: Succeeded
Total execution time: 17.33 seconds
```

#### 3.1.5 运行测试

> 时间: 2025-11-29 12:35

- ✅ Window > Tools > Stage DataLayer Browser
- ✅ Tab 正常打开
- ✅ 工具栏显示正常（Sync All、Import Selected）
- ✅ 原生 DataLayerBrowser 嵌入正常

**测试截图**：
> 📷 请将截图放入 `Docs/StageEditor/DatalayerSync/Screenshots/` 目录
>
> ![Phase 4.1 测试截图](Screenshots/Phase4_1_StageDataLayerBrowser.png)
>
> *截图说明：Stage DataLayer Browser Tab 首次打开界面*

---

## 4. 问题与解决方案

### 4.1 问题列表

| # | 问题 | 状态 | 解决方案 |
|---|------|------|---------|
| 1 | SDataLayerBrowser 在 Private 目录，无法直接继承 | 已解决 | 创建独立 Tab，复用引擎公开 API |
| 2 | 两个 Tab 数据同步 | 已解决 | 使用 UDataLayerEditorSubsystem 事件机制 |
| 3 | FDataLayerMode 在 Private 目录 | 已解决 | 采用包装器方案，使用 CreateDataLayerBrowser() |

### 4.2 技术决策记录

| 决策点 | 选择 | 原因 | 日期 |
|--------|------|------|------|
| UI 扩展方案 | 独立 Tab（包装器） | 可维护性、升级安全性、FDataLayerMode 不可用 | 2025-11-29 12:25 |

---

## 5. 待办事项

### 5.1 当前待办

- [x] Phase 4.1: 创建 SStageDataLayerBrowser 框架
- [x] Phase 4.2: 编译验证并测试
- [ ] Phase 3: 命名解析模块（推荐下一步）

### 5.2 后续 Phases

- [ ] Phase 1: 元数据基础设施
- [ ] Phase 2: 状态检测与提示系统
- [ ] Phase 3: 命名解析模块
- [ ] Phase 5: 导入逻辑与预览对话框
- [ ] Phase 6: 同步逻辑
- [ ] Phase 7: 本地化

---

## 6. 会话恢复信息

> ⚠️ 此节用于 Claude 上下文 compact 后快速恢复状态

### 6.1 当前进度（2025-11-29 12:40）

**已完成**：
- ✅ Phase 4.1: 创建 `SStageDataLayerBrowser` Widget（包装器方案）
- ✅ Phase 4.2: 编译验证并测试通过

**关键文件**：
- `Plugins/StageEditor/Source/StageEditor/Public/DataLayerSync/SStageDataLayerBrowser.h`
- `Plugins/StageEditor/Source/StageEditor/Private/DataLayerSync/SStageDataLayerBrowser.cpp`
- `Plugins/StageEditor/Source/StageEditor/Public/StageEditorModule.h`
- `Plugins/StageEditor/Source/StageEditor/Private/StageEditorModule.cpp`

**技术决策**：
- 采用"包装器"方案而非自定义 Outliner（因为 FDataLayerMode 在 Private 目录）
- 使用 `FDataLayerEditorModule::CreateDataLayerBrowser()` 创建原生 Browser
- 通过 `UDataLayerEditorSubsystem::OnDataLayerChanged()` 事件同步

### 6.2 下一步建议

**推荐执行 Phase 3: 命名解析模块**

需要实现：
```cpp
// 文件：StageDataLayerNameParser.h/cpp
struct FDataLayerNameParseResult
{
    bool bIsValid;
    bool bIsStage;      // DL_Stage_*
    bool bIsAct;        // DL_Act_*
    FString StageName;  // Stage 名称
    FString ActName;    // Act 名称（仅当 bIsAct=true）
};

// 解析函数
FDataLayerNameParseResult ParseDataLayerName(const FString& DataLayerName);
```

正则模式：
- Stage: `^DL_Stage_(.+)$`
- Act: `^DL_Act_([^_]+)_(.+)$`

### 6.3 关键文档

- PRD: `Docs/StageEditor/DatalayerSync/PRD_DataLayerImport.md`
- 技术规格: `Docs/StageEditor/DatalayerSync/TechSpec_DataLayerImport.md`
- 本开发日志: `Docs/StageEditor/DatalayerSync/DevLog_DataLayerImport.md`

---

## 附录

### A. 关键源码引用

#### A.1 列注册示例（SDataLayerBrowser.cpp:156-164）

```cpp
InitOptions.ColumnMap.Add(
    FDataLayerOutlinerIsVisibleColumn::GetID(),
    FSceneOutlinerColumnInfo(
        ESceneOutlinerColumnVisibility::Visible,
        0,
        FCreateSceneOutlinerColumn::CreateLambda([](ISceneOutliner& InSceneOutliner) {
            return MakeShareable(new FDataLayerOutlinerIsVisibleColumn(InSceneOutliner));
        })
    )
);
```

#### A.2 事件订阅示例

```cpp
UDataLayerEditorSubsystem::Get()->OnDataLayerChanged().AddSP(
    this, &SStageDataLayerBrowser::OnDataLayerChanged);

void SStageDataLayerBrowser::OnDataLayerChanged(
    EDataLayerAction Action,
    const TWeakObjectPtr<const UDataLayerInstance>& ChangedDataLayer,
    const FName& ChangedProperty)
{
    RefreshTree();
}
```

### B. 文件结构

```
Plugins/StageEditor/Source/StageEditor/
├── Private/
│   └── DataLayerSync/
│       └── SStageDataLayerBrowser.cpp
└── Public/
    └── DataLayerSync/
        └── SStageDataLayerBrowser.h
```

---

**文档维护说明**：
- 每次开发会话后更新此文档
- 记录关键决策和思路，包括具体代码位置
- 时间精确到分钟
- 问题解决后标记状态
- 保持待办事项最新
- 测试后添加截图到 Screenshots/ 目录

---

*最后更新: 2025-11-29 12:40*
