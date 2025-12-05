# Prop → Entity 重命名方案

> **创建日期**: 2025-12-04
> **执行状态**: ✅ 已完成
> **完成日期**: 2025-12-05
> **最后更新**: 2025-12-05

---

## 📋 重命名原因

### 问题描述

当前 **Stage-Act-Prop** 架构中，"Prop"（道具）的语义范围过窄：

**当前理解：**
- Prop = 道具（桌椅、装饰品等静态物体）

**实际需求：**
- 需要管理：Prop（道具）、Monster（怪物）、NPC、Enemy（敌人）、Vehicle（载具）等
- Monster/NPC 在技术上是 Prop 的子类（使用 `UStagePropComponent`）
- 但在概念上，Monster/NPC 不是"道具"

**核心矛盾：**
技术实现与概念语义不一致，造成理解困惑。

### 解决方案

**采用 Entity（实体）**：
- ✅ 行业标准术语（ECS 架构）
- ✅ 语义范围合适（任何游戏对象）
- ✅ 技术准确，易于理解
- ✅ 可扩展，无概念冲突

**新架构：Stage-Act-Entity**

---

## 🗺️ 完整重命名映射表

### 类名映射

| 旧名称 | 新名称 | 文件位置 |
|--------|--------|---------|
| `UStagePropComponent` | `UStageEntityComponent` | `StageEditorRuntime/Public/Components/` |
| `AProp` | `AStageEntity` | `StageEditorRuntime/Public/Actors/` |
| `FPropDragDropOp` | `FEntityDragDropOp` | `StageEditor/Public/EditorUI/StageEditorPanel.h` |

### 枚举值映射

| 旧名称 | 新名称 | 文件位置 |
|--------|--------|---------|
| `EStageTreeItemType::Prop` | `EStageTreeItemType::Entity` | `StageEditorPanel.h` |
| `EStageTreeItemType::PropsFolder` | `EStageTreeItemType::EntitiesFolder` | `StageEditorPanel.h` |
| `ETriggerZonePreset::PropStateChange` | `ETriggerZonePreset::EntityStateChange` | `StageCoreTypes.h` |

### SUID 结构体相关

| 旧名称 | 新名称 | 文件位置 |
|--------|--------|---------|
| `FSUID::PropID` | `FSUID::EntityID` | `StageCoreTypes.h` |
| `FSUID::MakePropID()` | `FSUID::MakeEntityID()` | `StageCoreTypes.h` |
| `FSUID::IsPropLevel()` | `FSUID::IsEntityLevel()` | `StageCoreTypes.h` |

### 变量/成员映射

| 旧名称 | 新名称 | 使用位置 |
|--------|--------|---------|
| `PropRegistry` | `EntityRegistry` | `Stage.h` - Stage 的注册表 |
| `PropID` | `EntityID` | `StagePropComponent.h`, `StageCoreTypes.h` (FSUID结构体) |
| `PropState` | `EntityState` | `StagePropComponent.h`, `StageEditorPanel.h` |
| `PreviousPropState` | `PreviousEntityState` | `StagePropComponent.h` |
| `PropStateOverrides` | `EntityStateOverrides` | `StageCoreTypes.h` - FAct 结构体 |
| `PropComponent` | `EntityComponent` | `Prop.h` - AProp 的组件成员 |
| `PropItems` | `EntityItems` | `StageEditorPanel.h` - FPropDragDropOp |
| `PropCount` | `EntityCount` | `DataLayerImporter.h` |
| `RegisteredPropCount` | `RegisteredEntityCount` | `DataLayerImporter.h` |
| `AddedPropCount` | `AddedEntityCount` | `DataLayerSynchronizer.h` |
| `RemovedPropCount` | `RemovedEntityCount` | `DataLayerSynchronizer.h` |
| `TotalPropChanges` | `TotalEntityChanges` | `DataLayerSynchronizer.h` |
| `bHasPropState` | `bHasEntityState` | `StageEditorPanel.h` - FStageTreeItem |
| `bIsCustomPropActorAssetPath` | `bIsCustomEntityActorAssetPath` | `StageEditorPanel.h` - FAssetCreationSettings |
| `PropActorAssetFolderPath` | `EntityActorAssetFolderPath` | `StageEditorPanel.h` - FAssetCreationSettings |
| `bIsCustomPropComponentAssetPath` | `bIsCustomEntityComponentAssetPath` | `StageEditorPanel.h` - FAssetCreationSettings |
| `PropComponentAssetFolderPath` | `EntityComponentAssetFolderPath` | `StageEditorPanel.h` - FAssetCreationSettings |
| `DefaultPropActorBlueprintParentClass` | `DefaultEntityActorBlueprintParentClass` | `StageEditorPanel.h` - FAssetCreationSettings |
| `DefaultPropComponentBlueprintParentClass` | `DefaultEntityComponentBlueprintParentClass` | `StageEditorPanel.h` - FAssetCreationSettings |

### 函数名映射

| 旧名称 | 新名称 | 使用位置 |
|--------|--------|---------|
| `RegisterProp()` | `RegisterEntity()` | `Stage.h/.cpp` |
| `UnregisterProp()` | `UnregisterEntity()` | `Stage.h/.cpp`, `StageEditorController.h/.cpp` |
| `RegisterProps()` | `RegisterEntities()` | `StageEditorController.h/.cpp` |
| `UnregisterAllProps()` | `UnregisterAllEntities()` | `StageEditorController.h/.cpp` |
| `SetPropState()` | `SetEntityState()` | `StagePropComponent.h/.cpp`, `Prop.h/.cpp` |
| `GetPropState()` | `GetEntityState()` | `StagePropComponent.h`, `Prop.h` |
| `GetPreviousPropState()` | `GetPreviousEntityState()` | `StagePropComponent.h` |
| `GetPropID()` | `GetEntityID()` | `StagePropComponent.h` |
| `SetPropStateInAct()` | `SetEntityStateInAct()` | `StageEditorController.h/.cpp` |
| `RemovePropFromAct()` | `RemoveEntityFromAct()` | `Stage.h/.cpp`, `StageEditorController.h/.cpp` |
| `RemoveAllPropsFromAct()` | `RemoveAllEntitiesFromAct()` | `StageEditorController.h/.cpp` |
| `SyncPropToDataLayer()` | `SyncEntityToDataLayer()` | `StageEditorController.h/.cpp` |
| `AssignPropToStageDataLayer()` | `AssignEntityToStageDataLayer()` | `StageEditorController.h/.cpp` |
| `RemovePropFromStageDataLayer()` | `RemoveEntityFromStageDataLayer()` | `StageEditorController.h/.cpp` |
| `AssignPropToActDataLayer()` | `AssignEntityToActDataLayer()` | `StageEditorController.h/.cpp` |
| `RemovePropFromActDataLayer()` | `RemoveEntityFromActDataLayer()` | `StageEditorController.h/.cpp` |
| `SetPropStateByID()` | `SetEntityStateByID()` | `Stage.h/.cpp` |
| `GetPropStateByID()` | `GetEntityStateByID()` | `Stage.h/.cpp` |
| `SetMultiplePropStates()` | `SetMultipleEntityStates()` | `Stage.h/.cpp` |
| `GetPropActorByID()` | `GetEntityActorByID()` | `Stage.h/.cpp` |
| `GetPropComponentByID()` | `GetEntityComponentByID()` | `Stage.h/.cpp` |
| `GetAllPropIDs()` | `GetAllEntityIDs()` | `Stage.h/.cpp` |
| `GetAllPropActors()` | `GetAllEntityActors()` | `Stage.h/.cpp` |
| `GetPropCount()` | `GetEntityCount()` | `Stage.h/.cpp` |
| `DoesPropExist()` | `DoesEntityExist()` | `Stage.h/.cpp` |
| `GetPropByID()` | `GetEntityByID()` | `Stage.h/.cpp` |
| `GetEffectivePropState()` | `GetEffectiveEntityState()` | `Stage.h/.cpp` |
| `GetControllingActForProp()` | `GetControllingActForEntity()` | `Stage.h/.cpp` |
| `ApplyActPropStatesOnly()` | `ApplyActEntityStatesOnly()` | `Stage.h/.cpp` |
| `GetActPropStates()` | `GetActEntityStates()` | `Stage.h/.cpp` |
| `OnPropStateChanged` | `OnEntityStateChanged` | `StagePropComponent.h` - 委托 |
| `FOnPropStateChanged` | `FOnEntityStateChanged` | `StagePropComponent.h` - 委托类型 |
| `FOnStagePropStateChanged` | `FOnStageEntityStateChanged` | `Stage.h` - 委托类型 |
| `OnStagePropStateChanged` | `OnStageEntityStateChanged` | `Stage.h` - 委托实例 |
| `OnPropStateChangedHandler()` | `OnEntityStateChangedHandler()` | `TriggerZoneActor.h/.cpp` |
| `CreatePropActorBlueprint()` | `CreateEntityActorBlueprint()` | `StageEditorController.h/.cpp` |
| `CreatePropComponentBlueprint()` | `CreateEntityComponentBlueprint()` | `StageEditorController.h/.cpp` |
| `OnRegisterSelectedPropsClicked()` | `OnRegisterSelectedEntitiesClicked()` | `StageEditorPanel.h/.cpp` |
| `OnCreatePropActorBPClicked()` | `OnCreateEntityActorBPClicked()` | `StageEditorPanel.h/.cpp` |
| `OnCreatePropComponentBPClicked()` | `OnCreateEntityComponentBPClicked()` | `StageEditorPanel.h/.cpp` |
| `ApplyPropStateChange()` | `ApplyEntityStateChange()` | `StageEditorPanel.h/.cpp` |

### 文件名映射

| 旧文件名 | 新文件名 | 路径 |
|---------|---------|------|
| `StagePropComponent.h` | `StageEntityComponent.h` | `StageEditorRuntime/Public/Components/` |
| `StagePropComponent.cpp` | `StageEntityComponent.cpp` | `StageEditorRuntime/Private/Components/` |
| `Prop.h` | `StageEntity.h` | `StageEditorRuntime/Public/Actors/` |
| `Prop.cpp` | `StageEntity.cpp` | `StageEditorRuntime/Private/Actors/` |

### 蓝图资产重命名

| 旧资产名 | 新资产名 | 路径 |
|---------|---------|------|
| `BP_BasePropActor` | `BP_BaseEntityActor` | `/StageEditor/PropsBP/PropBaseBP/` → `/StageEditor/EntitiesBP/EntityBaseBP/` |
| `BPC_BasePropComponent` | `BPC_BaseEntityComponent` | `/StageEditor/PropsBP/PropBaseBP/` → `/StageEditor/EntitiesBP/EntityBaseBP/` |

### Content 文件夹重命名

| 旧路径 | 新路径 |
|--------|--------|
| `Plugins/StageEditor/Content/PropsBP/` | `Plugins/StageEditor/Content/EntitiesBP/` |
| `Plugins/StageEditor/Content/PropsBP/PropBaseBP/` | `Plugins/StageEditor/Content/EntitiesBP/EntityBaseBP/` |

### 蓝图事件映射

| 旧名称 | 新名称 |
|--------|--------|
| `On Prop State Changed` | `On Entity State Changed` |
| `Register Prop` | `Register Entity` |
| `Unregister Prop` | `Unregister Entity` |

### 本地化字符串映射 (LOCTEXT/NSLOCTEXT)

| 旧文本 | 新文本 | 文件位置 |
|--------|--------|---------|
| `"Register Selected Props"` | `"Register Selected Entities"` | `StageEditorPanel.cpp` |
| `"Create Prop Actor BP"` | `"Create Entity Actor BP"` | `StageEditorPanel.cpp` |
| `"Create Prop Component BP"` | `"Create Entity Component BP"` | `StageEditorPanel.cpp` |
| `"Registered Props"` | `"Registered Entities"` | `StageEditorPanel.cpp` |
| `"Unregister All Props"` | `"Unregister All Entities"` | `StageEditorPanel.cpp` |
| `"Remove All Props from Act"` | `"Remove All Entities from Act"` | `StageEditorPanel.cpp` |
| `"RemovePropFromActInline_Tooltip"` | `"RemoveEntityFromActInline_Tooltip"` | `StageEditorPanel.cpp` |
| `"UnregisterPropInline_Tooltip"` | `"UnregisterEntityInline_Tooltip"` | `StageEditorPanel.cpp` |
| `"ConfirmUnregisterPropInline"` | `"ConfirmUnregisterEntityInline"` | `StageEditorPanel.cpp` |
| `"PropStateInlineEdit_Tooltip"` | `"EntityStateInlineEdit_Tooltip"` | `StageEditorPanel.cpp` |
| `"X Props"` (格式化字符串) | `"X Entities"` | `StageEditorPanel.cpp` |
| `"Stage Prop"` (Category) | `"Stage Entity"` | `StagePropComponent.h`, `Prop.h` |
| `"Stage\|Props"` (Category) | `"Stage\|Entities"` | `Stage.h` |

### #include 路径更新

| 旧路径 | 新路径 | 需要更新的文件 |
|--------|--------|---------------|
| `#include "Components/StagePropComponent.h"` | `#include "Components/StageEntityComponent.h"` | 所有引用该头文件的文件 |
| `#include "Actors/Prop.h"` | `#include "Actors/StageEntity.h"` | `TriggerZoneActor.h`, `Stage.h` 等 |
| `#include "Prop.generated.h"` | `#include "StageEntity.generated.h"` | `Prop.h` → `StageEntity.h` |
| `#include "StagePropComponent.generated.h"` | `#include "StageEntityComponent.generated.h"` | `StagePropComponent.h` → `StageEntityComponent.h` |

### 默认资产路径更新

| 旧路径 | 新路径 | 文件位置 |
|--------|--------|---------|
| `"/StageEditor/PropsBP"` | `"/StageEditor/EntitiesBP"` | `StageEditorPanel.cpp` |
| `"/StageEditor/PropsBP/PropBaseBP/BP_BasePropActor.BP_BasePropActor_C"` | `"/StageEditor/EntitiesBP/EntityBaseBP/BP_BaseEntityActor.BP_BaseEntityActor_C"` | `StageEditorPanel.h` |
| `"/StageEditor/PropsBP/PropBaseBP/BPC_BasePropComponent.BPC_BasePropComponent_C"` | `"/StageEditor/EntitiesBP/EntityBaseBP/BPC_BaseEntityComponent.BPC_BaseEntityComponent_C"` | `StageEditorPanel.h` |

---

## 📁 影响文件清单（完整版）

### Runtime 模块 (StageEditorRuntime)

#### 需要重命名的文件

```
Source/StageEditorRuntime/Public/Components/StagePropComponent.h     → StageEntityComponent.h
Source/StageEditorRuntime/Private/Components/StagePropComponent.cpp  → StageEntityComponent.cpp
Source/StageEditorRuntime/Public/Actors/Prop.h                       → StageEntity.h
Source/StageEditorRuntime/Private/Actors/Prop.cpp                    → StageEntity.cpp
```

#### 需要修改内容的文件

| 文件 | 修改类型 | 预估修改处数 |
|------|---------|-------------|
| `Public/Core/StageCoreTypes.h` | FSUID结构体, 枚举, PropStateOverrides | ~15 处 |
| `Public/Components/StagePropComponent.h` | 类名, 成员变量, 函数, 委托 | ~25 处 |
| `Private/Components/StagePropComponent.cpp` | 类名, 函数实现 | ~5 处 |
| `Public/Actors/Prop.h` | 类名, include, 成员变量, 函数 | ~15 处 |
| `Private/Actors/Prop.cpp` | 类名, 函数实现 | ~5 处 |
| `Public/Actors/Stage.h` | PropRegistry, 所有Prop相关函数, 委托 | ~60 处 |
| `Private/Actors/Stage.cpp` | 所有Prop相关函数实现 | ~30 处 |
| `Public/Actors/TriggerZoneActor.h` | include, 注释, PropState相关 | ~20 处 |
| `Private/Actors/TriggerZoneActor.cpp` | 函数实现 | ~10 处 |
| `Public/Subsystems/StageManagerSubsystem.h` | 注释 | ~2 处 |

**Runtime 模块统计：**
- 重命名文件：4 个
- 修改文件：10 个
- 预估修改处：~187 处

---

### Editor 模块 (StageEditor)

| 文件 | 修改类型 | 预估修改处数 |
|------|---------|-------------|
| `Public/EditorLogic/StageEditorController.h` | 函数名, 注释 | ~25 处 |
| `Private/EditorLogic/StageEditorController.cpp` | 函数实现, LOCTEXT, 事务描述 | ~35 处 |
| `Public/EditorUI/StageEditorPanel.h` | 枚举, 类, 变量, 函数, 设置结构体 | ~35 处 |
| `Private/EditorUI/StageEditorPanel.cpp` | UI文本, 函数实现, 资产路径 | ~80 处 |
| `Public/DataLayerSync/DataLayerImporter.h` | 变量, 注释 | ~8 处 |
| `Private/DataLayerSync/DataLayerImporter.cpp` | 实现 | ~5 处 |
| `Public/DataLayerSync/DataLayerSynchronizer.h` | 变量, 注释 | ~10 处 |
| `Private/DataLayerSync/DataLayerSynchronizer.cpp` | 实现 | ~5 处 |
| `Public/DataLayerSync/SDataLayerImportPreviewDialog.h` | 注释 | ~2 处 |

**Editor 模块统计：**
- 修改文件：9 个
- 预估修改处：~205 处

---

### 文档文件

**需要更新的文档：**

1. **设计文档**
   - `Docs/StageEditor/HighLevelDesign.md`
   - `Docs/StageEditor/StageEditorController.md`
   - `Docs/StageEditor/SStageEditorPanel与TreeView详细设计文档.md`
   - `Docs/StageEditor/Overview.md`

2. **Phase 文档**
   - 所有 Phase 相关文档中的术语

3. **核心文档**
   - `Docs/CLAUDE.md` - 项目概述

---

## 🔧 实施步骤

### Phase 1: 文件重命名（优先处理）

**原因：先重命名文件，避免后续编辑时文件名不一致**

```bash
# Runtime 模块
Prop.h                  → StageEntity.h
Prop.cpp                → StageEntity.cpp
StagePropComponent.h    → StageEntityComponent.h
StagePropComponent.cpp  → StageEntityComponent.cpp

# 使用 git mv 保留历史记录
git mv Plugins/StageEditor/Source/StageEditorRuntime/Public/Actors/Prop.h \
       Plugins/StageEditor/Source/StageEditorRuntime/Public/Actors/StageEntity.h

git mv Plugins/StageEditor/Source/StageEditorRuntime/Private/Actors/Prop.cpp \
       Plugins/StageEditor/Source/StageEditorRuntime/Private/Actors/StageEntity.cpp

git mv Plugins/StageEditor/Source/StageEditorRuntime/Public/Components/StagePropComponent.h \
       Plugins/StageEditor/Source/StageEditorRuntime/Public/Components/StageEntityComponent.h

git mv Plugins/StageEditor/Source/StageEditorRuntime/Private/Components/StagePropComponent.cpp \
       Plugins/StageEditor/Source/StageEditorRuntime/Private/Components/StageEntityComponent.cpp
```

### Phase 2: Runtime 模块代码重命名

**顺序：从底层到上层**

1. ✅ `StageCoreTypes.h` - 修改结构体定义
2. ✅ `StageEntityComponent.h/.cpp` - 修改组件类
3. ✅ `StageEntity.h/.cpp` - 修改 Actor 基类
4. ✅ `Stage.h/.cpp` - 修改 Stage 管理逻辑
5. ✅ `TriggerZoneActor.h` - 修改相关引用

### Phase 3: Editor 模块代码重命名

**顺序：从 Controller 到 UI**

1. ✅ `StageEditorController.h/.cpp` - 修改 Controller API
2. ✅ `StageEditorPanel.h/.cpp` - 修改 UI 逻辑
3. ✅ `DataLayerImporter.h` - 修改导入逻辑
4. ✅ `DataLayerSynchronizer.h/.cpp` - 修改同步逻辑
5. ✅ `SDataLayerImportPreviewDialog.h` - 修改对话框

### Phase 4: 文档更新

1. ✅ 更新 `CLAUDE.md` - 项目概述
2. ✅ 更新设计文档
3. ✅ 更新 Overview.md
4. ✅ 创建术语对照表

### Phase 5: 编译测试

1. ✅ 清理编译输出
2. ✅ 重新编译 Runtime 模块
3. ✅ 重新编译 Editor 模块
4. ✅ 运行编辑器验证功能

---

## 📊 进度跟踪

| 阶段 | 状态 | 文件数 | 预计时间 |
|------|------|--------|---------|
| Phase 1: 文件重命名 | 🔄 进行中 | 4 | 5 分钟 |
| Phase 2: Runtime 代码 | ⏳ 待开始 | 8 | 20 分钟 |
| Phase 3: Editor 代码 | ⏳ 待开始 | 8 | 20 分钟 |
| Phase 4: 文档更新 | ⏳ 待开始 | 10+ | 15 分钟 |
| Phase 5: 编译测试 | ⏳ 待开始 | - | 10 分钟 |
| **总计** | **0%** | **30+** | **~70 分钟** |

---

## ⚠️ 风险和注意事项

### 潜在风险

1. **蓝图资产兼容性**
   - 现有蓝图使用了 `UStagePropComponent`
   - 重命名后可能需要手动修复蓝图引用
   - **缓解措施**：先备份项目，测试蓝图兼容性

2. **#include 路径**
   - 文件名变化会导致 `#include` 路径失效
   - **缓解措施**：使用 IDE 的全局搜索替换

3. **Git 历史记录**
   - 文件重命名可能影响 Git Blame
   - **缓解措施**：使用 `git mv` 保留历史

### 测试检查清单

- [ ] 编译无错误
- [ ] 编译无警告
- [ ] 编辑器可正常启动
- [ ] StageEditorPanel 可正常打开
- [ ] 可创建新 Stage
- [ ] 可注册 Entity（原 Prop）
- [ ] Entity 状态切换正常
- [ ] DataLayer 导入功能正常

---

## 📝 术语对照表

### 中文翻译

| 英文 | 旧翻译 | 新翻译 | 说明 |
|------|--------|--------|------|
| Prop | 道具 | 道具/实体 | 保留在特定上下文 |
| Entity | - | 实体 | 新术语 |
| Stage-Act-Prop | 舞台-幕-道具 | 舞台-幕-实体 | 新架构名称 |
| Entity State | - | 实体状态 | 新术语 |

### 概念说明

**Entity（实体）在 StageEditor 中的定义：**

> Entity 是舞台中任何可被 Stage 管理、可在不同 Act 中改变状态的游戏对象，包括但不限于：
> - 传统道具（Prop）：桌椅、装饰品
> - 角色（Character）：玩家角色、NPC
> - 敌人（Enemy）：怪物、Boss
> - 载具（Vehicle）：车辆、飞船
> - 触发器（Trigger）：区域触发
> - 环境对象：灯光、特效

---

## 🎯 完成标准

### 代码层面

- ✅ 所有 "Prop" 相关类名已重命名为 "Entity"
- ✅ 所有函数名、变量名已更新
- ✅ 所有注释已更新
- ✅ 编译无错误无警告

### 文档层面

- ✅ 所有设计文档已更新
- ✅ CLAUDE.md 已更新
- ✅ Overview.md 已更新
- ✅ 术语对照表已创建

### 功能层面

- ✅ 编辑器可正常启动
- ✅ StageEditor 所有功能正常
- ✅ 蓝图资产兼容性正常

---

## 📚 参考资料

- **决策讨论**：Claude 对话记录（2025-12-04）
- **原架构文档**：`Docs/StageEditor/HighLevelDesign.md`
- **ECS 架构参考**：Entity-Component-System 设计模式

---

## 🔧 IDE 批量替换指南

### 推荐替换顺序

为避免部分匹配问题，建议按以下顺序进行替换（先替换长的、具体的，再替换短的、通用的）：

#### 第一批：文件重命名（使用 git mv）

```bash
# 在项目根目录执行
git mv "Plugins/StageEditor/Source/StageEditorRuntime/Public/Components/StagePropComponent.h" \
       "Plugins/StageEditor/Source/StageEditorRuntime/Public/Components/StageEntityComponent.h"

git mv "Plugins/StageEditor/Source/StageEditorRuntime/Private/Components/StagePropComponent.cpp" \
       "Plugins/StageEditor/Source/StageEditorRuntime/Private/Components/StageEntityComponent.cpp"

git mv "Plugins/StageEditor/Source/StageEditorRuntime/Public/Actors/Prop.h" \
       "Plugins/StageEditor/Source/StageEditorRuntime/Public/Actors/StageEntity.h"

git mv "Plugins/StageEditor/Source/StageEditorRuntime/Private/Actors/Prop.cpp" \
       "Plugins/StageEditor/Source/StageEditorRuntime/Private/Actors/StageEntity.cpp"
```

#### 第二批：长标识符替换（先长后短）

**IDE 全局搜索替换设置：**
- 搜索范围：`Plugins/StageEditor/Source/**/*.{h,cpp}`
- 区分大小写：是
- 使用正则表达式：否（普通文本替换）

| 顺序 | 搜索 | 替换 | 说明 |
|------|------|------|------|
| 1 | `UStagePropComponent` | `UStageEntityComponent` | 类名 |
| 2 | `StagePropComponent` | `StageEntityComponent` | include路径、生成文件 |
| 3 | `FOnStagePropStateChanged` | `FOnStageEntityStateChanged` | 委托类型 |
| 4 | `OnStagePropStateChanged` | `OnStageEntityStateChanged` | 委托实例 |
| 5 | `FOnPropStateChanged` | `FOnEntityStateChanged` | 委托类型 |
| 6 | `OnPropStateChanged` | `OnEntityStateChanged` | 委托实例 |
| 7 | `PropStateOverrides` | `EntityStateOverrides` | FAct成员 |
| 8 | `PropRegistry` | `EntityRegistry` | Stage成员 |
| 9 | `PropComponent` | `EntityComponent` | AProp成员 |
| 10 | `PropItems` | `EntityItems` | DragDrop |
| 11 | `FPropDragDropOp` | `FEntityDragDropOp` | 类名 |
| 12 | `PreviousPropState` | `PreviousEntityState` | 成员变量 |
| 13 | `PropState` | `EntityState` | 状态变量（小心！） |
| 14 | `PropID` | `EntityID` | ID变量 |
| 15 | `PropCount` | `EntityCount` | 计数变量 |
| 16 | `AProp` | `AStageEntity` | 类名 |

#### 第三批：函数名替换

| 顺序 | 搜索 | 替换 |
|------|------|------|
| 1 | `RegisterProps` | `RegisterEntities` |
| 2 | `UnregisterAllProps` | `UnregisterAllEntities` |
| 3 | `UnregisterProp` | `UnregisterEntity` |
| 4 | `RegisterProp` | `RegisterEntity` |
| 5 | `RemoveAllPropsFromAct` | `RemoveAllEntitiesFromAct` |
| 6 | `RemovePropFromAct` | `RemoveEntityFromAct` |
| 7 | `SetPropStateInAct` | `SetEntityStateInAct` |
| 8 | `SetPropStateByID` | `SetEntityStateByID` |
| 9 | `GetPropStateByID` | `GetEntityStateByID` |
| 10 | `SetMultiplePropStates` | `SetMultipleEntityStates` |
| 11 | `SetPropState` | `SetEntityState` |
| 12 | `GetPropState` | `GetEntityState` |
| 13 | `GetPreviousPropState` | `GetPreviousEntityState` |
| 14 | `GetEffectivePropState` | `GetEffectiveEntityState` |
| 15 | `GetControllingActForProp` | `GetControllingActForEntity` |
| 16 | `ApplyActPropStatesOnly` | `ApplyActEntityStatesOnly` |
| 17 | `GetActPropStates` | `GetActEntityStates` |
| 18 | `GetPropActorByID` | `GetEntityActorByID` |
| 19 | `GetPropComponentByID` | `GetEntityComponentByID` |
| 20 | `GetAllPropIDs` | `GetAllEntityIDs` |
| 21 | `GetAllPropActors` | `GetAllEntityActors` |
| 22 | `GetPropCount` | `GetEntityCount` |
| 23 | `DoesPropExist` | `DoesEntityExist` |
| 24 | `GetPropByID` | `GetEntityByID` |
| 25 | `GetPropID` | `GetEntityID` |
| 26 | `SyncPropToDataLayer` | `SyncEntityToDataLayer` |
| 27 | `AssignPropToStageDataLayer` | `AssignEntityToStageDataLayer` |
| 28 | `RemovePropFromStageDataLayer` | `RemoveEntityFromStageDataLayer` |
| 29 | `AssignPropToActDataLayer` | `AssignEntityToActDataLayer` |
| 30 | `RemovePropFromActDataLayer` | `RemoveEntityFromActDataLayer` |
| 31 | `CreatePropActorBlueprint` | `CreateEntityActorBlueprint` |
| 32 | `CreatePropComponentBlueprint` | `CreateEntityComponentBlueprint` |

#### 第四批：枚举和设置

| 搜索 | 替换 |
|------|------|
| `EStageTreeItemType::PropsFolder` | `EStageTreeItemType::EntitiesFolder` |
| `EStageTreeItemType::Prop` | `EStageTreeItemType::Entity` |
| `ETriggerZonePreset::PropStateChange` | `ETriggerZonePreset::EntityStateChange` |
| `PropsFolder` | `EntitiesFolder` |
| `bIsCustomPropActorAssetPath` | `bIsCustomEntityActorAssetPath` |
| `PropActorAssetFolderPath` | `EntityActorAssetFolderPath` |
| `bIsCustomPropComponentAssetPath` | `bIsCustomEntityComponentAssetPath` |
| `PropComponentAssetFolderPath` | `EntityComponentAssetFolderPath` |
| `DefaultPropActorBlueprintParentClass` | `DefaultEntityActorBlueprintParentClass` |
| `DefaultPropComponentBlueprintParentClass` | `DefaultEntityComponentBlueprintParentClass` |

#### 第五批：SUID 相关

| 搜索 | 替换 |
|------|------|
| `MakePropID` | `MakeEntityID` |
| `IsPropLevel` | `IsEntityLevel` |

#### 第六批：UI 文本和路径

| 搜索 | 替换 |
|------|------|
| `"Register Selected Props"` | `"Register Selected Entities"` |
| `"Create Prop Actor BP"` | `"Create Entity Actor BP"` |
| `"Create Prop Component BP"` | `"Create Entity Component BP"` |
| `"Registered Props"` | `"Registered Entities"` |
| `"Unregister All Props"` | `"Unregister All Entities"` |
| `"Remove All Props from Act"` | `"Remove All Entities from Act"` |
| `"/StageEditor/PropsBP"` | `"/StageEditor/EntitiesBP"` |
| `BP_BasePropActor` | `BP_BaseEntityActor` |
| `BPC_BasePropComponent` | `BPC_BaseEntityComponent` |
| `PropBaseBP` | `EntityBaseBP` |

#### 第七批：注释中的 Prop（手动审查）

建议手动搜索并审查以下模式：
- `// Prop` → `// Entity`
- `* Prop` → `* Entity`
- `@brief Prop` → `@brief Entity`
- `Stage Prop` → `Stage Entity`（作为 Category 名称）
- `Stage|Props` → `Stage|Entities`（作为 Category 路径）

### ⚠️ 替换注意事项

1. **备份代码**：执行前先提交当前状态或创建分支
2. **分批验证**：每批替换后编译验证，确保无错误
3. **检查 UPROPERTY**：确保 Category 中的文本也被正确替换
4. **检查 LOCTEXT**：UI 文本需要手动核对
5. **检查 DisplayName**：UPROPERTY 的 meta 中的显示名称

---

## 📊 总体统计

| 项目 | 数量 |
|------|------|
| 需要重命名的文件 | 4 个 |
| 需要修改的源文件 | ~19 个 |
| 需要修改的文档文件 | 10+ 个 |
| 预估代码修改处 | ~400 处 |
| 蓝图资产重命名 | 2 个 |
| Content 文件夹重命名 | 2 个 |

---

*创建时间: 2025-12-04*
*完成时间: 2025-12-05*
*执行者: 用户手动批量重命名 + Claude Code 辅助检查*
