# 01 - 项目背景和当前状态

## 📋 项目概述

### 项目名称
**Stage Editor** - Unreal Engine 5 关卡编辑器插件

### 项目目标
创建一个强大的编辑器工具，用于管理大型开放世界游戏中的场景状态和流式加载。

### 核心概念

#### Stage (舞台)
- `AStage` Actor - 区域管理器
- 负责加载/卸载 DataLayer
- 向 Prop 发送状态指令
- 持有全局唯一的 `StageID`

#### Act (幕)
- `FAct` 结构体 - 场景状态定义
- 定义一组 Prop 的目标状态
- 关联特定的 DataLayer
- 在 Stage 内有唯一的 `ActID`

#### Prop (道具)
- `UStagePropComponent` - 核心组件
- 可以附加到任何 Actor
- 存储 `PropID` 和 `PropState`
- 响应 Stage 的状态指令

---

## 🎯 当前项目状态

### 开发阶段
**Phase 3** - DataLayer Creation Uniqueness (进行中)

### 已完成的 Phase

#### Phase 1: Stage DataLayer Asset Reference ✅
**完成时间**: 2025-11-21

**主要成果**:
1. 在 `AStage` 中添加了 `StageDataLayerAsset` (TObjectPtr)
2. 保留 `StageDataLayerName` 用于显示
3. 实现了 `PostEditChangeProperty` 自动同步
4. 修复了所有编译错误

**关键文件**:
- `Stage.h` - 添加了 `StageDataLayerAsset` 字段
- `Stage.cpp` - 实现了属性同步逻辑

**文档**:
- `Docs/Artifacts/MultiUser_And_DataLayer_Configuration/README.md`

---

#### Phase 2: Controller Logic Migration ✅
**完成时间**: 2025-11-21

**主要成果**:
1. 重写了 `FindStageDataLayerInstance` 使用 Asset 引用
2. 更新了所有 DataLayer 操作方法
3. 迁移了所有 `StageDataLayerName` 的使用
4. 添加了 DataLayer 资产路径配置功能

**关键文件**:
- `StageEditorController.h` - 添加了 DataLayer 路径配置
- `StageEditorController.cpp` - 更新了所有 DataLayer 操作
- `StageEditorPanel.h` - 添加了路径配置 UI

**文档**:
- `Docs/Artifacts/MultiUser_And_DataLayer_Configuration/DataLayer_Path_Configuration.md`

---

#### Phase 2.5: DataLayer 功能修复 ✅
**完成时间**: 2025-11-26

**主要成果**:
1. 重新启用了 DataLayer 自动创建功能
2. 添加了 External Objects 安全检查（防止崩溃）
3. 添加了 `OnCreated()` 初始化调用
4. 替换为 UE 原生 World Partition 转换功能

**关键修复**:
- **问题**: DataLayer 自动创建被禁用，点击 "Add Act" 不创建 DataLayer
- **原因**: 之前因为 WorldDataLayers 配置问题被注释掉
- **解决**: 添加安全检查，重新启用功能

**关键文件**:
- `StageEditorController.cpp` - 重新启用自动创建，添加安全检查
- `StageEditor.Build.cs` - 添加 WorldPartitionEditor 模块依赖

**文档**:
- `Docs/11.26/Session_Summary.md`
- `Docs/11.26/Quick_Reference.md`
- `Docs/Artifacts/MultiUser_And_DataLayer_Configuration/DataLayer_Crash_Fix.md`
- `Docs/Artifacts/MultiUser_And_DataLayer_Configuration/WorldPartition_Conversion_Update.md`

---

## 🔧 技术栈

### Unreal Engine
- **版本**: 5.6
- **关键系统**:
  - World Partition
  - DataLayer System
  - Editor Subsystems
  - Custom Details Panel

### 开发工具
- **IDE**: Visual Studio 2022
- **编译**: UnrealBuildTool
- **版本控制**: Git

### 插件结构
```
StageEditor/
├── Source/
│   ├── StageEditor/              # 编辑器模块
│   │   ├── Public/
│   │   │   ├── EditorLogic/      # Controller 逻辑
│   │   │   ├── EditorUI/         # UI 面板
│   │   │   └── Subsystems/       # Editor Subsystems
│   │   └── Private/
│   └── StageEditorRuntime/       # 运行时模块
│       ├── Public/
│       │   ├── Actors/           # Stage, Prop
│       │   ├── Components/       # StagePropComponent
│       │   └── Core/             # 核心数据结构
│       └── Private/
└── Content/                      # 蓝图和资产
```

---

## 📊 当前代码统计

### 核心文件数量
- **头文件**: ~15 个
- **实现文件**: ~15 个
- **总代码行数**: ~5000 行

### 关键类和结构

#### Runtime (StageEditorRuntime)
- `AStage` - Stage Actor
- `AProp` - Prop 基类
- `UStagePropComponent` - Prop 组件
- `FStageHierarchicalId` - 层级 ID 结构
- `FAct` - Act 数据结构

#### Editor (StageEditor)
- `FStageEditorModule` - 插件模块
- `FStageEditorController` - 核心控制器
- `SStageEditorPanel` - UI 面板
- `UStageEditorSubsystem` - ⏳ **待实现**

---

## 🎯 Phase 3 的背景

### 为什么需要 Phase 3？

#### 问题
在 Phase 2.5 完成后，DataLayer 创建功能已经可以工作，但存在一个根本性问题：

**DataLayer 命名基于用户可见的名称（DisplayName）**

```cpp
// 当前的命名方式
FString AssetName = FString::Printf(TEXT("DL_Stage_%s"), *Stage->GetName());
FString AssetName = FString::Printf(TEXT("DL_Act_%s_%d_%s"), 
    *Stage->GetName(), ActID, *TargetAct->DisplayName);
```

**问题**:
- ❌ 多用户可能创建同名 Stage/Act
- ❌ 名称冲突需要复杂的后缀处理
- ❌ 用户重命名会导致混乱
- ❌ 不符合 ID 系统的设计理念

#### 解决方案
**使用 ID 系统代替名称系统**

我们已经有完善的 ID 架构（见 `detailed_specification.md`），但之前没有应用到 DataLayer 命名上。

Phase 3 的目标就是：
1. 创建 `UStageEditorSubsystem` 统一管理 Stage 注册
2. 实现 StageID 的自动分配
3. 所有 DataLayer 命名基于 ID 而不是名称

---

## 📚 重要设计文档

### 核心规格文档
**文件**: `Docs/Artifacts/detailed_specification.md`

**关键章节**:
- 1.1 核心数据结构 - `FStageHierarchicalId`
- 1.2 ID 分配与关联策略 - 引用关联模式
- 1.3 核心概念 - Stage, Act, Prop

### ID 系统设计
**文件**: `Docs/11.26/Phase3_ID_Based_Design.md`

**关键内容**:
- ID 命名规则
- 离线工作流（Phase 2 实现）
- 对比分析：名称方案 vs ID 方案

### 最终实施方案
**文件**: `Docs/11.26/Phase3_Final_Implementation_Plan.md`

**关键内容**:
- StageEditorSubsystem 详细设计
- Stage 自动注册逻辑
- DataLayer 创建流程更新

---

## 🔍 代码风格和约定

### 命名约定
- **类**: `AStage`, `UStageEditorSubsystem`
- **结构体**: `FAct`, `FStageHierarchicalId`
- **枚举**: `EStageRuntimeState`
- **成员变量**: `StageID`, `PropRegistry`
- **函数**: `RegisterStage`, `CreateDataLayerForAct`

### 代码组织
使用 `#pragma region` 组织代码：
```cpp
#pragma region Imports
// 包含文件
#pragma endregion Imports

#pragma region Construction
// 构造函数
#pragma endregion Construction

#pragma region Core Logic
// 核心逻辑
#pragma endregion Core Logic
```

### 注释风格
使用 Doxygen 风格：
```cpp
/**
 * @brief Brief description.
 * 
 * Detailed description.
 * 
 * @param ParamName - Parameter description
 * @return Return value description
 */
```

### 日志和通知
```cpp
// 日志
UE_LOG(LogTemp, Log, TEXT("✅ Success message"));
UE_LOG(LogTemp, Warning, TEXT("⚠️ Warning message"));
UE_LOG(LogTemp, Error, TEXT("❌ Error message"));

// 用户通知
DebugHeader::ShowNotifyInfo(TEXT("User-facing message"));
```

---

## 🧪 测试环境

### 测试关卡要求
1. **World Partition 关卡**
   - 必须是 World Partition 类型
   - 启用 "Use External Actors"
   - 启用 "Use Actor Folder Objects"

2. **DataLayer 配置**
   - 关卡必须支持 External Objects
   - 有 WorldDataLayers Actor

### 测试数据
- **Stage 数量**: 至少 3 个
- **Act 数量**: 每个 Stage 至少 2 个
- **Prop 数量**: 每个 Stage 至少 5 个

---

## 📈 项目里程碑

### 已完成
- ✅ Phase 1: DataLayer Asset Reference (2025-11-21)
- ✅ Phase 2: Controller Logic Migration (2025-11-21)
- ✅ Phase 2.5: DataLayer 功能修复 (2025-11-26)

### 当前
- ⏳ Phase 3: StageEditorSubsystem + ID 系统 (2025-11-26)

### 计划中
- ⏳ Phase 4: UI Display Name Handling
- ⏳ Phase 5: Backward Compatibility
- ⏳ Phase 6: Testing

---

## 🎯 Phase 3 的重要性

Phase 3 是整个 ID 系统的**基础设施**：

1. **为后续 Phase 铺路**
   - Phase 4 需要基于 ID 的显示逻辑
   - Phase 5 需要 ID 迁移功能
   - Phase 6 需要稳定的 ID 系统

2. **解决根本问题**
   - 不是临时方案，而是架构级改进
   - 符合原始设计文档的理念
   - 为多用户协作打下基础

3. **简化未来开发**
   - 不需要处理名称冲突
   - 不需要复杂的重命名逻辑
   - 代码更简洁可维护

---

**现在您已经了解了项目的完整背景，可以继续阅读其他文档开始实施了！** 🚀
