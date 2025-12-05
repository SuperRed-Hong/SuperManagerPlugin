# StageEditor - 开发日志总览

> 创建日期: 2025-11-29
> 状态: ✅ Phase 15.5 完成（UI 列宽调整修复）
> 最后更新: 2025-12-05
>
> 📂 **文档导航**: [README.md](README.md) - 完整文档索引
> 📚 **历史进度**: [HistoricalProgress.md](DataLayerIntegration/HistoricalProgress.md) - Phase 1-13 详细归档

---

## 🎭 架构命名

**Stage-Act-Entity** (原 Stage-Act-Prop，于 Phase 14.5 重命名)

| 概念 | 说明 |
|------|------|
| **Stage** | 舞台 - 场景管理的根单位 |
| **Act** | 幕 - 场景状态配置 |
| **Entity** | 实体 - 任何被 Stage 管理的游戏对象（怪物、NPC、道具、载具等） |

---

## 功能概述

让 StageEditor 能够根据已有的 DataLayer 数据自动创建 Stage-Act-Entity 架构，实现"项目中途接管"能力。

```
用户已有 DataLayer → 按规范重命名 → 一键导入 → Stage-Act-Entity 架构
                                         ↓
                                  后续变化自动检测 → 提示同步
```

---

## Phase 索引

| Phase | 任务 | 状态 | 详细文档 |
|-------|------|------|---------|
| 1-2 | 反向查找与状态检测 | ✅ 完成 | [Phase1-2_ReverseLookup.md](DataLayerIntegration/Phase1-2_ReverseLookup.md) |
| 1-2 | 性能优化（缓存层） | ✅ 完成 | [Phase1-2_PerformanceOptimization.md](DataLayerIntegration/Phase1-2_PerformanceOptimization.md) |
| 3 | 命名解析模块 | ✅ 完成 | [Phase3_Parser.md](DataLayerIntegration/Phase3_Parser.md) |
| 4 | DataLayerOutliner UI | ✅ 完成 | [Phase4_UI.md](DataLayerIntegration/Phase4_UI.md) |
| 5 | 导入逻辑与预览对话框 | ✅ 完成 | [Phase5_Import.md](DataLayerIntegration/Phase5_Import.md) |
| 6 | 同步逻辑 | ✅ 完成 | [Phase6_Sync.md](DataLayerIntegration/Phase6_Sync.md) |
| 7 | 本地化（中英文） | ✅ 完成 | [Phase7_Localization.md](DataLayerIntegration/Phase7_Localization.md) |
| 8 | UI 扩展预研 | ✅ 完成 | [Phase8_UI_Extension_Research.md](DataLayerIntegration/Phase8_UI_Extension_Research.md) |
| 8.1 | SceneOutliner 基础架构 | ✅ 完成 | [Phase8_1_SceneOutliner_Foundation.md](DataLayerIntegration/Phase8_1_SceneOutliner_Foundation.md) |
| **8.2** | **自定义列实现** | ✅ 完成 | [Phase8_2_CustomColumns.md](DataLayerIntegration/Phase8_2_CustomColumns.md) |
| **8.3** | **集成与测试** | ✅ 完成 | [Phase8_3_Integration.md](DataLayerIntegration/Phase8_3_Integration.md) |
| **8.4** | **原生功能迁移** | ✅ 完成 | [Phase8_4_NativeFeatures.md](DataLayerIntegration/Phase8_4_NativeFeatures.md) |
| **9** | **架构整合** | ✅ 完成 | [Architecture_Integration_Analysis.md](DataLayerIntegration/Architecture_Integration_Analysis.md) |
| **9.5** | **代码质量优化** | ✅ 完成 | [历史进度](DataLayerIntegration/HistoricalProgress.md#phase-95) |
| **10** | **Import 功能重设计** | ✅ 完成 | [Phase10_ImportRedesign.md](DataLayerIntegration/Phase10_ImportRedesign.md) |
| **10.8** | **WP Streaming 同步修复** | ✅ 完成 | [历史进度](DataLayerIntegration/HistoricalProgress.md#phase-108) |
| **10.9** | **命名规范回退** | ✅ 完成 | [历史进度](DataLayerIntegration/HistoricalProgress.md#phase-109) |
| **11** | **缓存事件驱动优化** | ✅ 完成 | [Phase11_CacheEventDriven.md](DataLayerIntegration/Phase11_CacheEventDriven.md) |
| **12** | **Import/Rename 功能增强** | ✅ 完成 | [历史进度](DataLayerIntegration/HistoricalProgress.md#phase-12) |
| **13** | **StageRegistry 持久化架构重设计** | 🔄 设计完成 | [讨论文档](CoreArchitecture/Phase13_StageRegistry_Discussion.md) |
| **14** | **Import 蓝图类支持 + TriggerZone 蓝图化** | ✅ 完成 | [ImportBlueprintClassSupport.md](DataLayerIntegration/ImportBlueprintClassSupport.md) |
| **14.5** | **Prop → Entity 架构重命名** | ✅ 完成 | [PropToEntity_RenamingPlan.md](Refactoring/PropToEntity_RenamingPlan.md) |
| **15** | **Entity 管理安全性增强** | ✅ 完成 | [Phase15_EntityManagement_SafetyEnhancements.md](EditorFeatures/Phase15_EntityManagement_SafetyEnhancements.md) |
| **15.5** | **DataLayerOutliner 列宽调整修复** | ✅ 完成 | [SSplitter_ResizeMode_Explained.md](TechNotes/SSplitter_ResizeMode_Explained.md) |

---

## 架构决策

| 决策 | 文档 |
|------|------|
| **反向查找方案（采纳）** | [Architecture_ReverseLookup.md](DataLayerIntegration/Architecture_ReverseLookup.md) |
| **基于 SceneOutliner 框架方案（采纳）** | [Phase8_UI_Extension_Research.md](DataLayerIntegration/Phase8_UI_Extension_Research.md) |
| **架构整合方案（Phase 9）** | [Architecture_Integration_Analysis.md](DataLayerIntegration/Architecture_Integration_Analysis.md) |
| **StageRegistry 持久化方案（Phase 13）** | [Phase13_StageRegistry_Discussion.md](CoreArchitecture/Phase13_StageRegistry_Discussion.md) |
| 废弃方案存档 | [Obsolete/DataLayerIntegration/README.md](Obsolete/DataLayerIntegration/README.md) |

---

## 当前进度

### Phase 15.5 完成 ✅ - DataLayerOutliner 列宽调整修复
**完成日期:** 2025-12-05
**详细文档:** [SSplitter_ResizeMode_Explained.md](TechNotes/SSplitter_ResizeMode_Explained.md)

#### 问题描述

SStageDataLayerOutliner 的列宽手动调整行为与 UE 原生 SceneOutliner 相反：
- **预期行为:** 向左拖拽列边缘时，左侧列变窄，右侧列变宽
- **实际行为:** 向左拖拽时，右侧列变窄，左侧列变宽（方向相反）
- **影响范围:** 所有列分隔线（Label|SUID 和 SUID|Actions）

#### 根本原因

使用 `ManualWidth` 模式时，`SHeaderRow` 为每列创建 **两个** `SSplitter` 槽位：
1. 列内容槽（不可调整大小）
2. 列宽调整手柄槽（可调整大小）

这导致：
- 视觉列数：3（Label、SUID、Actions）
- 实际 SSplitter 槽位数：6（每列有内容槽 + 手柄槽）
- 索引错位导致拖拽操作影响错误的列，表现为方向相反

#### 解决方案

切换到 `Fill` 模式（比例宽度）：
- 每列只创建 **一个** `SSplitter` 槽位
- 宽度使用比例值（0.0-1.0）而非像素值
- 槽位索引与视觉列一一对应
- 拖拽行为与 UE 原生 SceneOutliner 保持一致

#### 技术实现

**修改文件:**
1. `SStageDataLayerOutliner.cpp:127-167` - 在 `FSceneOutlinerColumnInfo` 中添加 `FillSize` 参数
   - Label 列：`1.0f`（填充剩余空间）
   - SUID 列：`0.2f`（20% 总宽度）
   - Actions 列：`0.4f`（40% 总宽度）

2. `StageDataLayerColumns.cpp:177-190, 548-561` - 移除 SUID 和 Actions 列的 `ManualWidth()` 调用
   - 让 SceneOutliner 自动应用 `FillWidth` 模式

**编译状态:** ✅ 通过（5.44 秒）

**验证结果:** 用户确认正常工作

---

### Phase 15 完成 ✅ - Entity 管理安全性增强
**完成日期:** 2025-12-05
**详细文档:** [Phase15_EntityManagement_SafetyEnhancements.md](EditorFeatures/Phase15_EntityManagement_SafetyEnhancements.md)

#### 核心改进

本阶段针对 Entity 生命周期管理和 Stage 删除安全性进行全面增强，解决潜在的数据完整性风险。

**主要功能:**
1. **孤立 Entity 检测与清理系统**
   - `IsOrphaned()` - 检测 Entity 的 OwnerStage 是否已删除
   - `ClearOrphanedState()` - 重置孤立 Entity 到未注册状态
   - `CleanOrphanedEntities()` - 批量扫描并清理孤立 Entities
   - UI 工具栏添加 "Clean Orphaned" 按钮

2. **单 Stage 注册约束强制执行**
   - 注册时检测 Entity 是否已属于其他 Stage
   - 冲突时显示确认对话框，可选择移动或跳过
   - 防止一个 Entity 同时注册到多个 Stages

3. **显式 Stage 删除确认对话框**
   - Stage 行添加 Delete 按钮（红色垃圾桶图标）
   - 删除前显示详细信息：
     - 将删除的 DataLayer 列表
     - 注册的 Entity 数量
   - 三选项对话框：
     - **Yes:** 删除 Stage + DataLayers
     - **No:** 仅删除 Stage（保留 DataLayers）
     - **Cancel:** 取消操作

4. **事务支持完善**
   - 修复 `DeleteDataLayerForAct()` 事务顺序错误
   - 所有操作支持完整的 Undo/Redo
   - Entity 自动注销保留（低风险，支持 Undo）

5. **Blueprint 重构安全检查**
   - 添加 `GIsReconstructingBlueprintInstances` 检测
   - 防止 Blueprint 编译时误触发 Entity 注销

**架构决策:**
- ❌ 移除 `OnLevelActorDeleted` 中的 DataLayer 自动删除（高风险）
- ✅ 保留 Entity 自动注销（低风险，支持 Undo）
- ✅ Stage 删除移至显式 UI 按钮（用户明确操作）
- ✅ 孤立 Entity 手动清理（避免误删数据）

**修改统计:**
- 新增 API：8 个公共方法
- 修改文件：6 个
- 新增代码：~500 行
- 修改代码：~150 行
- 编译状态：✅ 通过（12.17 秒）

---

### Phase 14.5 完成 ✅ - Prop → Entity 架构重命名
**完成日期:** 2025-12-05
**详细文档:** [PropToEntity_RenamingPlan.md](Refactoring/PropToEntity_RenamingPlan.md)

#### 重命名原因

"Prop"（道具）语义范围过窄，无法准确描述 Monster、NPC、Enemy 等被 Stage 管理的对象。采用 "Entity"（实体）作为通用术语，符合 ECS 架构的行业标准。

#### 主要变更

| 类别 | 旧名称 | 新名称 |
|------|--------|--------|
| 架构 | Stage-Act-Prop | Stage-Act-Entity |
| 组件 | `UStagePropComponent` | `UStageEntityComponent` |
| Actor | `AProp` | `AStageEntity` |
| 变量 | `PropRegistry`, `PropID`, `PropState` | `EntityRegistry`, `EntityID`, `EntityState` |
| 文件 | `StagePropComponent.h`, `Prop.h` | `StageEntityComponent.h`, `StageEntity.h` |

#### 修改统计

- 重命名文件：4 个
- 修改源文件：~19 个
- 代码修改处：~400 处
- 编译状态：✅ 通过

---

### Phase 14 完成 ✅
**完成日期:** 2025-12-04
**详细文档:** [ImportBlueprintClassSupport.md](DataLayerIntegration/ImportBlueprintClassSupport.md)

---

### Phase 13 进行中 🔄
**状态:** 设计评审完成，待实施
**完成日期:** 2025-12-04 专家评审
**详细文档:** [Phase13_StageRegistry_Discussion.md](CoreArchitecture/Phase13_StageRegistry_Discussion.md)
**评审报告:** [Phase13_ExpertReview_Report.md](CoreArchitecture/Phase13_ExpertReview_Report.md)

**评审结论:**
- 可实施性：7.5/10 → 补充后预期 9/10
- ✅ P0-1 完成：FLevelInstanceID 稳定性验证
- 🔄 待处理：P0-2/P0-3 事务一致性和迁移方案
- 📋 待办事项：10 个（3P0 + 3P1 + 4P2）

---

### Phase 14 详细内容（归档）

本阶段完成了 DataLayer Import 功能的蓝图类支持系统，解决了使用 C++ 基类直接实例化 Stage 的核心问题，并优化了整体工作流。同时实现了 TriggerZone 组件的蓝图继承支持。

#### 14.1 核心问题解决

**问题1:** Import 创建的 Stage Actor 直接使用 AStage C++ 基类，无法自定义行为
**解决:** 实现完整的蓝图类选择和创建流程

**问题2:** 工作流复杂度过高，需要多次点击和手动选择
**解决:** 自动触发 Blueprint 创建，简化操作流程

**问题3:** UTriggerZoneComponentBase 无法蓝图继承
**解决:** 添加 Blueprintable 说明符支持蓝图子类创建

#### 14.2 主要功能实现

**Import 工作流优化:**
1. ✅ 移除 Import Preview 对话框中的 Blueprint 类选择器
2. ✅ 用户点击 OK 后自动触发 Blueprint 创建对话框
3. ✅ 使用 FAssetCreationSettings 统一配置默认路径和父类
4. ✅ 创建完成后自动使用新 Blueprint 执行 Import
5. ✅ 用户取消创建则中止 Import 操作

**Blueprint 创建增强:**
1. ✅ CreateStageBlueprint() 返回 UBlueprint* 而非 void
2. ✅ 添加默认参数支持便捷调用
3. ✅ Class Picker 自动预选配置的默认父类
4. ✅ 从 Settings 读取默认文件夹路径

**TSoftClassPtr 加载问题修复:**
- ✅ 发现并修复 TSoftClassPtr 加载机制问题
- ✅ 使用 IsNull() 检查路径而非 IsValid()
- ✅ IsValid() 只对已加载资源返回 true，不适合加载前检查

**TriggerZone 蓝图化支持:**
- ✅ 为 UTriggerZoneComponentBase 添加 Blueprintable 说明符
- ✅ 用户现在可以在蓝图中创建 TriggerZone 子类
- ✅ 提升 TriggerZone 系统的扩展性和灵活性

**日志系统规范化:**
- ✅ 声明全局 LogStageEditor 分类
- ✅ 移除局部 DEFINE_LOG_CATEGORY_STATIC
- ✅ 统一使用模块专属日志

#### 14.3 修改文件统计

| 模块 | 文件 | 修改量 | 说明 |
|------|------|--------|------|
| UI | SDataLayerImportPreviewDialog.h/cpp | 删除 ~100 行<br>修改 ~80 行 | 移除选择器 UI，重构 Import 逻辑 |
| Controller | StageEditorController.h/cpp | +166 行 | Blueprint 创建返回值 + Settings 加载 |
| Importer | DataLayerImporter.h/cpp | +32 行 | Blueprint 类参数支持 |
| Settings | StageEditorPanel.h/cpp | +102 行 | 加载逻辑修复 + 调试日志 |
| Runtime | TriggerZoneComponentBase.h | 修改 1 行 | 添加 Blueprintable |
| Module | StageEditorModule.h/cpp | +2 行 | 全局日志声明 |

**总修改量:** 85 files changed, 3093 insertions(+), 1803 deletions(-)

#### 14.4 技术要点

**关键技术决策:**
1. **显式 > 隐式原则** - 用户必须显式创建 Blueprint 类
2. **TSoftClassPtr 加载机制理解** - IsNull() vs IsValid() vs LoadSynchronous()
3. **UE 原生 UI 复用** - 使用 SClassPickerDialog 和 UBlueprintFactory
4. **事务支持** - 所有修改包裹在 FScopedTransaction

**关键经验:**
- TSoftClassPtr::IsValid() ≠ "路径有效"，而是 "资源已加载"
- 需要理解 UE 惰性加载机制
- 详细日志输出快速定位问题
- 文档先行避免返工

#### 14.5 新增资产

- ✅ BP_BaseStage 蓝图基类
- ✅ BP_BaseEntityActor 蓝图基类 (原 BP_BasePropActor)
- ✅ BPC_BaseEntityComponent 蓝图组件基类 (原 BPC_BasePropComponent)
- ✅ BP_BaseTriggerZoneActor 蓝图基类
- ✅ BPC_BaseTriggerZone 蓝图组件基类
- ✅ 4 个测试 DataLayer 资产（DL_TEST1-4）

#### 14.6 编译状态

```
Result: Succeeded
Total execution time: 14.16 seconds
```

---

## 历史进度归档

**Phase 1-13 的详细进度信息已迁移到:** [HistoricalProgress.md](DataLayerIntegration/HistoricalProgress.md)

### 快速索引

| Phase | 任务概要 | 文档链接 |
|-------|---------|---------|
| 1-2 | 反向查找与状态检测 + 性能优化 | [Phase1-2 详细文档](DataLayerIntegration/Phase1-2_ReverseLookup.md) |
| 3 | 命名解析模块 | [Phase3 详细文档](DataLayerIntegration/Phase3_Parser.md) |
| 4 | DataLayerOutliner UI | [Phase4 详细文档](DataLayerIntegration/Phase4_UI.md) |
| 5 | 导入逻辑与预览对话框 | [Phase5 详细文档](DataLayerIntegration/Phase5_Import.md) |
| 6 | 同步逻辑 | [Phase6 详细文档](DataLayerIntegration/Phase6_Sync.md) |
| 7 | 本地化（中英文） | [Phase7 详细文档](DataLayerIntegration/Phase7_Localization.md) |
| 8-8.4 | SceneOutliner 架构 + 原生功能迁移 | [Phase8 详细文档](DataLayerIntegration/Phase8_UI_Extension_Research.md) |
| 9-9.5 | 架构整合 + 代码质量优化 | [Architecture Analysis](DataLayerIntegration/Architecture_Integration_Analysis.md) |
| 10 | Import 功能重设计 | [Phase10 详细文档](DataLayerIntegration/Phase10_ImportRedesign.md) |
| 11 | 缓存事件驱动优化 | [Phase11 详细文档](DataLayerIntegration/Phase11_CacheEventDriven.md) |
| 12 | Import/Rename 功能增强 | [历史进度归档](DataLayerIntegration/HistoricalProgress.md#phase-12) |
| 13 | StageRegistry 持久化架构重设计 🔄 | [讨论文档](CoreArchitecture/Phase13_StageRegistry_Discussion.md) |

---

## 待办事项

### 🔲 TODO: StageEditorPanel 重构评估

> **触发条件**: Phase 8 完成后

Phase 8 完成 DataLayerBrowser 基于 SceneOutliner 框架的重构后，评估是否将 StageEditorPanel 也迁移到同一框架。

**潜在收益**: 代码统一性、维护性、扩展性
**需解决问题**: FAct 是 struct 非 UObject、文件夹是纯 UI 概念

详见: [UI_Development.md 第10节](../UI_Development.md#10-未来待评估)

---

## 关键文件索引

```
Plugins/StageEditor/Source/
├── StageEditorRuntime/
│   ├── Public/
│   │   ├── Subsystems/StageManagerSubsystem.h  # 反向查找 API + Stage 注册委托 (Phase 11)
│   │   ├── Actors/Stage.h                      # Stage Actor (PostLoad 自注册 Phase 10.8)
│   │   ├── Actors/StageEntity.h                # Entity 基类 (Phase 14.5 重命名自 Prop.h)
│   │   └── Components/StageEntityComponent.h   # Entity 组件 (Phase 14.5 重命名自 StagePropComponent.h)
│   └── Private/
│       ├── Subsystems/StageManagerSubsystem.cpp
│       ├── Actors/Stage.cpp
│       ├── Actors/StageEntity.cpp              # Phase 14.5 重命名自 Prop.cpp
│       └── Components/StageEntityComponent.cpp # Phase 14.5 重命名自 StagePropComponent.cpp
│
├── StageEditor/
│   ├── Public/DataLayerSync/
│   │   ├── SStageDataLayerOutliner.h          # Phase 4/8.3 - 主 UI 控件
│   │   ├── StageDataLayerNameParser.h         # Phase 3 - 命名解析 (Phase 10.9 回退详细格式)
│   │   ├── DataLayerSyncStatus.h              # Phase 1-2 - 状态枚举和检测器
│   │   ├── DataLayerSyncStatusCache.h         # Phase 1-2/11 - 缓存层（事件驱动）
│   │   ├── DataLayerImporter.h                # Phase 5 - 导入逻辑
│   │   ├── SDataLayerImportPreviewDialog.h    # Phase 5/10.4 - 导入预览对话框
│   │   ├── SBatchImportPreviewDialog.h        # Phase 10.5 - 批量导入预览对话框
│   │   ├── DataLayerSynchronizer.h            # Phase 6 - 同步逻辑
│   │   ├── StageDataLayerTreeItem.h           # Phase 8.1 - 树节点类型
│   │   ├── StageDataLayerHierarchy.h          # Phase 8.1 - 数据层级结构
│   │   ├── StageDataLayerMode.h               # Phase 8.1 - Outliner 模式
│   │   ├── StageDataLayerColumns.h            # Phase 8.2 - 自定义列
│   │   └── SStdRenamePopup.h                  # Phase 8.4 - 标准重命名精简对话框
│   │
│   ├── Private/DataLayerSync/
│   │   ├── SStageDataLayerOutliner.cpp
│   │   ├── StageDataLayerNameParser.cpp
│   │   ├── DataLayerSyncStatus.cpp
│   │   ├── DataLayerSyncStatusCache.cpp       # Phase 11 - 事件驱动缓存
│   │   ├── DataLayerImporter.cpp
│   │   ├── SDataLayerImportPreviewDialog.cpp
│   │   ├── SBatchImportPreviewDialog.cpp      # Phase 10.5
│   │   ├── DataLayerSynchronizer.cpp
│   │   ├── StageDataLayerTreeItem.cpp
│   │   ├── StageDataLayerHierarchy.cpp
│   │   ├── StageDataLayerMode.cpp
│   │   ├── StageDataLayerColumns.cpp
│   │   └── SStdRenamePopup.cpp
│   │
│   ├── Private/EditorLogic/
│   │   └── StageEditorController.cpp          # Phase 11 - 添加 InvalidateCache 调用
│   │
│   └── Private/StageEditorModule.cpp          # Tab 注册 + 缓存生命周期
```

---

## 快速恢复（给 Claude）

```
继续开发或回顾 DataLayer 导入功能：
1. 读取此文档了解整体状态和 Phase 索引
2. 查看对应 Phase 详细文档了解实现细节
3. 查看 Architecture_ReverseLookup.md 了解核心架构
```

---

## 已删除的废弃文件

> 记录于 2025-11-29 15:40，元数据存储方案废弃后删除

```
├── Public/DataLayerSync/
│   ├── StageEditorDataLayerUserData.h     # ❌ 已删除
│   ├── StageDataLayerSyncSubsystem.h      # ❌ 已删除
│   └── StageEditorDataLayerUtils.h        # ❌ 已删除
├── Private/DataLayerSync/
│   ├── StageDataLayerSyncSubsystem.cpp    # ❌ 已删除
│   └── StageEditorDataLayerUtils.cpp      # ❌ 已删除
```

---

*最后更新: 2025-12-05 - Phase 15.5 DataLayerOutliner 列宽调整修复完成*
