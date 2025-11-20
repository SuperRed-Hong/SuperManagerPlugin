# UE Plugin Directory Structure - Best Practices

## SuperManager 插件结构分析

### 整体结构
```
SuperManager/
├── Binaries/
├── Config/
├── Content/
├── Resources/
├── Source/
│   └── SuperManager/              ← 单模块
│       ├── Public/                ← 按功能分类
│       │   ├── ActorActions/
│       │   ├── AssetActions/
│       │   ├── CustomOutlinerColumn/
│       │   ├── CustomStyle/
│       │   ├── CustomUICommands/
│       │   ├── SlateWidgets/
│       │   ├── DebugHeader.h
│       │   ├── SuperManager.h
│       │   └── SuperManagerEnums.h
│       ├── Private/               ← 同样按功能分类
│       │   ├── ActorActions/
│       │   ├── AssetActions/
│       │   ├── CustomOutlinerColumn/
│       │   ├── CustomStyle/
│       │   ├── CustomUICommands/
│       │   ├── SlateWidgets/
│       │   └── SuperManager.cpp
│       └── SuperManager.Build.cs
└── SuperManager.uplugin
```

### 核心原则

✅ **功能导向分组** (Feature-Based Organization)
- **不要**: 所有头文件/cpp扁平放在 Public/Private
- **要**: 按功能模块划分子目录（Actions, Widgets, Styles, Commands...）

✅ **Public/Private 对称**
- Public 和 Private 使用相同的子目录结构
- 同一功能的 .h 和 .cpp 分别放在对应目录

✅ **模块入口文件**
- 模块主文件（如 `SuperManager.h/.cpp`）直接放在 Public/Private 根目录
- 工具类头文件（如 `DebugHeader.h`）也放根目录

---

## StageEditor 当前结构问题

### 当前结构
```
StageEditor/
├── Source/
│   ├── StageEditorRuntime/
│   │   ├── Public/               ← ❌ 所有文件扁平放置
│   │   │   ├── Prop.h
│   │   │   ├── Stage.h
│   │   │   ├── StageCoreTypes.h
│   │   │   ├── StagePropComponent.h
│   │   │   └── StageEditorRuntimeModule.h
│   │   └── Private/
│   │       ├── Prop.cpp
│   │       ├── Stage.cpp
│   │       ├── StagePropComponent.cpp
│   │       └── StageEditorRuntimeModule.cpp
│   └── StageEditor/
│       ├── Public/               ← ❌ 所有文件扁平放置
│       │   ├── StageEditorController.h
│       │   ├── StageEditorModule.h
│       │   └── StageEditorPanel.h
│       └── Private/
│           ├── StageEditorController.cpp
│           ├── StageEditorModule.cpp
│           └── StageEditorPanel.cpp
```

### 问题
1. **缺乏分类** - 所有文件混在一起，难以导航
2. **扩展性差** - 新增文件会让目录更混乱
3. **职责不清** - 无法一眼看出文件的功能分类

---

## StageEditor 重构计划

### 目标结构
```
StageEditor/
├── Source/
│   ├── StageEditorRuntime/
│   │   ├── Public/
│   │   │   ├── Core/                    ← 核心数据类型
│   │   │   │   └── StageCoreTypes.h
│   │   │   ├── Actors/                  ← Actor 类
│   │   │   │   ├── Stage.h
│   │   │   │   └── Prop.h
│   │   │   ├── Components/              ← 组件类
│   │   │   │   └── StagePropComponent.h
│   │   │   └── StageEditorRuntimeModule.h
│   │   └── Private/
│   │       ├── Actors/
│   │       │   ├── Stage.cpp
│   │       │   └── Prop.cpp
│   │       ├── Components/
│   │       │   └── StagePropComponent.cpp
│   │       └── StageEditorRuntimeModule.cpp
│   └── StageEditor/
│       ├── Public/
│       │   ├── EditorUI/                ← Slate UI
│       │   │   └── StageEditorPanel.h
│       │   ├── EditorLogic/             ← MVC Controller
│       │   │   └── StageEditorController.h
│       │   └── StageEditorModule.h
│       └── Private/
│           ├── EditorUI/
│           │   └── StageEditorPanel.cpp
│           ├── EditorLogic/
│           │   └── StageEditorController.cpp
│           └── StageEditorModule.cpp
```

### 重构步骤
1. 创建新的子目录结构
2. 移动文件到对应目录
3. 更新所有 `#include` 路径
4. 更新 `.Build.cs` 的 `PublicIncludePaths`（如果需要）
5. 重新编译验证

### 预期收益
- ✅ 清晰的功能分类
- ✅ 更好的可维护性
- ✅ 易于新增功能（如 DataLayer、EditorMode 等）
- ✅ 符合 UE 插件最佳实践
