# Phase 8.3: SceneOutliner 集成

> 日期: 2025-11-29
> 状态: ✅ 完成
> 任务: 将自定义 SceneOutliner 集成到 SStageDataLayerOutliner，替换原生 DataLayerBrowser

---

## 完成内容

### SStageDataLayerOutliner 重构

**文件**:
- `Public/DataLayerSync/SStageDataLayerOutliner.h`
- `Private/DataLayerSync/SStageDataLayerOutliner.cpp`

**变更**:
1. 移除对 `FDataLayerEditorModule` 的依赖
2. 使用 `FSceneOutlinerModule` 创建自定义 SceneOutliner
3. 集成 Phase 8.1 的 `FStageDataLayerMode` 作为模式
4. 集成 Phase 8.2 的自定义列

---

## 关键实现

### 1. 头文件新增成员

```cpp
// 新增 include
#include "SceneOutlinerFwd.h"

// 新增成员
TSharedPtr<SSceneOutliner> SceneOutliner;
FStageDataLayerMode* DataLayerMode = nullptr;
TWeakObjectPtr<UWorld> RepresentingWorld;
```

### 2. InitializeOutliner 实现

```cpp
void SStageDataLayerOutliner::InitializeOutliner()
{
    FSceneOutlinerModule& SceneOutlinerModule = FModuleManager::LoadModuleChecked<FSceneOutlinerModule>("SceneOutliner");

    // 配置初始化选项
    FSceneOutlinerInitializationOptions InitOptions;
    InitOptions.bShowHeaderRow = true;
    InitOptions.bShowSearchBox = true;
    InitOptions.bShowCreateNewFolder = false;
    InitOptions.bFocusSearchBoxWhenOpened = false;

    // 模式工厂 - 返回原始指针，SceneOutliner 接管所有权
    InitOptions.ModeFactory = FCreateSceneOutlinerMode::CreateLambda([this](SSceneOutliner* Outliner) -> ISceneOutlinerMode*
    {
        FStageDataLayerModeParams Params(Outliner, this, RepresentingWorld);
        FStageDataLayerMode* NewMode = new FStageDataLayerMode(Params);
        DataLayerMode = NewMode;
        return NewMode;
    });

    // 注册自定义列
    InitOptions.ColumnMap.Add(
        FStageDataLayerSyncStatusColumn::GetID(),
        FSceneOutlinerColumnInfo(
            ESceneOutlinerColumnVisibility::Visible,
            1,
            FCreateSceneOutlinerColumn::CreateLambda([](ISceneOutliner& Outliner)
            {
                return TSharedRef<ISceneOutlinerColumn>(MakeShared<FStageDataLayerSyncStatusColumn>(Outliner));
            }),
            true
        )
    );

    InitOptions.ColumnMap.Add(
        FStageDataLayerActionsColumn::GetID(),
        FSceneOutlinerColumnInfo(...)
    );

    // 创建 SceneOutliner
    TSharedRef<ISceneOutliner> OutlinerRef = SceneOutlinerModule.CreateSceneOutliner(InitOptions);
    SceneOutliner = StaticCastSharedRef<SSceneOutliner>(OutlinerRef);
}
```

### 3. UI 构建

```cpp
void SStageDataLayerOutliner::Construct(const FArguments& InArgs)
{
    RepresentingWorld = GEditor ? GEditor->GetEditorWorldContext().World() : nullptr;
    SubscribeToEvents();
    InitializeOutliner();

    ChildSlot
    [
        SNew(SVerticalBox)
        + SVerticalBox::Slot().AutoHeight()
        [
            // Title
        ]
        + SVerticalBox::Slot().AutoHeight()
        [
            // Toolbar (Sync All, Import Selected)
        ]
        + SVerticalBox::Slot().FillHeight(1.0f)
        [
            // Custom SceneOutliner
            SceneOutliner.IsValid() ? SceneOutliner.ToSharedRef() : SNullWidget::NullWidget
        ]
    ];
}
```

---

## API 说明

### FSceneOutlinerInitializationOptions 关键字段

| 字段 | 类型 | 说明 |
|------|------|------|
| `ModeFactory` | `FCreateSceneOutlinerMode` | 模式工厂委托，返回 `ISceneOutlinerMode*` |
| `ColumnMap` | `TMap<FName, FSceneOutlinerColumnInfo>` | 列注册表 |
| `bShowHeaderRow` | bool | 是否显示表头 |
| `bShowSearchBox` | bool | 是否显示搜索框 |
| `bShowCreateNewFolder` | bool | 是否显示创建文件夹按钮 |

### FSceneOutlinerColumnInfo 构造函数

```cpp
FSceneOutlinerColumnInfo(
    ESceneOutlinerColumnVisibility InVisibility,  // 可见性
    uint8 InPriorityIndex,                        // 优先级
    const FCreateSceneOutlinerColumn& InFactory,  // 列工厂
    bool inCanBeHidden = true,                    // 是否可隐藏
    TOptional<float> InFillSize = {},            // 填充宽度
    TAttribute<FText> InColumnLabel = {},        // 列标题
    EHeaderComboVisibility = OnHover,            // 表头下拉可见性
    FOnGetContent = {}                           // 表头菜单
)
```

---

## 编译修复记录

### 错误 1: FSceneOutlinerColumnInfo 参数不匹配
```cpp
// 错误: 第5个参数传了 FName (想当作 "insert after")
FSceneOutlinerColumnInfo(Visible, 1, Factory, true, FName("Label"))

// 修复: 只传必需参数
FSceneOutlinerColumnInfo(Visible, 1, Factory, true)
```

### 错误 2: ModeFactory 返回类型错误
```cpp
// 错误: 返回 TSharedRef
[](SSceneOutliner* Outliner) -> TSharedRef<ISceneOutlinerMode>
{
    return MakeShared<FStageDataLayerMode>(Params);
}

// 修复: 返回原始指针 (SceneOutliner 接管所有权)
[](SSceneOutliner* Outliner) -> ISceneOutlinerMode*
{
    return new FStageDataLayerMode(Params);
}
```

### 错误 3: CreateSceneOutliner 返回类型转换
```cpp
// 错误: 直接赋值给 TSharedPtr<SSceneOutliner>
SceneOutliner = SceneOutlinerModule.CreateSceneOutliner(InitOptions);

// 修复: 显式转换
TSharedRef<ISceneOutliner> OutlinerRef = SceneOutlinerModule.CreateSceneOutliner(InitOptions);
SceneOutliner = StaticCastSharedRef<SSceneOutliner>(OutlinerRef);
```

---

## 架构总览

```
SStageDataLayerOutliner (Slate Widget)
├── Header (Title)
├── Toolbar
│   ├── Sync All Button
│   └── Import Selected Button
└── SSceneOutliner
    ├── FStageDataLayerMode (Mode)
    │   └── FStageDataLayerHierarchy (Data Source)
    │       └── FStageDataLayerTreeItem (Tree Nodes)
    └── Columns
        ├── Label (Built-in)
        ├── SyncStatus (Custom)
        └── Actions (Custom)
```

---

## 代码统计

| 文件 | 修改行数 |
|------|----------|
| SStageDataLayerOutliner.h | +12 |
| SStageDataLayerOutliner.cpp | +60, -20 |

---

*文档创建: 2025-11-29*
