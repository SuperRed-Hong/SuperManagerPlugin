# DataLayer 导入功能 - 开发日志

> 创建日期: 2025-11-29
> 状态: 🔄 Phase 13 进行中（架构重设计讨论）
> 最后更新: 2025-12-01 03:30

---

## 功能概述

让 StageEditor 能够根据已有的 DataLayer 数据自动创建 Stage-Act-Prop 架构，实现"项目中途接管"能力。

```
用户已有 DataLayer → 按规范重命名 → 一键导入 → Stage-Act-Prop 架构
                                         ↓
                                  后续变化自动检测 → 提示同步
```

---

## Phase 索引

| Phase | 任务 | 状态 | 详细文档 |
|-------|------|------|---------|
| 1-2 | 反向查找与状态检测 | ✅ 完成 | [Phase1-2_ReverseLookup.md](Phase1-2_ReverseLookup.md) |
| 1-2 | 性能优化（缓存层） | ✅ 完成 | [Phase1-2_PerformanceOptimization.md](Phase1-2_PerformanceOptimization.md) |
| 3 | 命名解析模块 | ✅ 完成 | [Phase3_Parser.md](Phase3_Parser.md) |
| 4 | DataLayerOutliner UI | ✅ 完成 | [Phase4_UI.md](Phase4_UI.md) |
| 5 | 导入逻辑与预览对话框 | ✅ 完成 | [Phase5_Import.md](Phase5_Import.md) |
| 6 | 同步逻辑 | ✅ 完成 | [Phase6_Sync.md](Phase6_Sync.md) |
| 7 | 本地化（中英文） | ✅ 完成 | [Phase7_Localization.md](Phase7_Localization.md) |
| 8 | UI 扩展预研 | ✅ 完成 | [Phase8_UI_Extension_Research.md](Phase8_UI_Extension_Research.md) |
| 8.1 | SceneOutliner 基础架构 | ✅ 完成 | [Phase8_1_SceneOutliner_Foundation.md](Phase8_1_SceneOutliner_Foundation.md) |
| **8.2** | **自定义列实现** | ✅ 完成 | [Phase8_2_CustomColumns.md](Phase8_2_CustomColumns.md) |
| **8.3** | **集成与测试** | ✅ 完成 | [Phase8_3_Integration.md](Phase8_3_Integration.md) |
| **8.4** | **原生功能迁移** | ✅ 完成 | [Phase8_4_NativeFeatures.md](Phase8_4_NativeFeatures.md) |
| **9** | **架构整合** | ✅ 完成 | [Architecture_Integration_Analysis.md](Architecture_Integration_Analysis.md) |
| **9.5** | **代码质量优化** | ✅ 完成 | 见下方详细说明 |
| **10** | **Import 功能重设计** | ✅ 完成 | [Phase10_ImportRedesign.md](Phase10_ImportRedesign.md) |
| **10.8** | **WP Streaming 同步修复** | ✅ 完成 | 见下方详细说明 |
| **10.9** | **命名规范回退** | ✅ 完成 | 见下方详细说明 |
| **11** | **缓存事件驱动优化** | ✅ 完成 | [Phase11_CacheEventDriven.md](Phase11_CacheEventDriven.md) |
| **12** | **Import/Rename 功能增强** | ✅ 完成 | 见下方详细说明 |
| **13** | **StageRegistry 持久化架构重设计** | 🔄 讨论中 | [讨论文档](../../DiscussionTopics/StageRegistry持久化架构重设计.md) |

---

## 架构决策

| 决策 | 文档 |
|------|------|
| **反向查找方案（采纳）** | [Architecture_ReverseLookup.md](Architecture_ReverseLookup.md) |
| **基于 SceneOutliner 框架方案（采纳）** | [Phase8_UI_Extension_Research.md](Phase8_UI_Extension_Research.md) |
| **架构整合方案（Phase 9）** | [Architecture_Integration_Analysis.md](Architecture_Integration_Analysis.md) |
| 废弃方案存档 | [Obsolete/README.md](Obsolete/README.md) |

---

## 当前进度 (Phase 13 - StageRegistry 持久化架构重设计)

### Phase 13 讨论中 🔄

**详细讨论文档：** [StageRegistry持久化架构重设计.md](../../DiscussionTopics/StageRegistry持久化架构重设计.md)

#### 问题背景

发现 `StageManagerSubsystem` 的注册表设计存在根本性缺陷：
- `NextStageID` 不持久化，编辑器重启后从 1 开始
- 依赖 `ScanWorldForExistingStages()` 遍历已加载 Stage 来恢复
- WP Streaming 卸载的 Stage 无法被遍历到
- 可能导致 StageID 冲突，违反"全局唯一"设计目标

#### 解决方案

采用**双层架构**：

1. **持久化层 (DataAsset)**
   - `UStageRegistryAsset` - 每个 Level 一个
   - 存储 `NextStageID`、`StageEntries[]` 完整列表
   - 使用 `TSoftObjectPtr<UWorld>` 关联 Level（跟随移动/重命名）

2. **运行时层 (Subsystem)**
   - 加载/管理 RegistryAsset
   - 维护运行时缓存 `RuntimeStageMap`
   - 支持 LevelInstance（使用原生 `FLevelInstanceID`）

#### 待确定事项

- [ ] RegistryAsset 查找效率优化
- [ ] 多人协作场景处理
- [ ] Cross-Stage 通信 API 变更

---

## 历史进度

### Phase 12 完成 ✅

本阶段完成了导入和重命名功能的增强。

#### 12.1 Import Preview - DefaultAct 选项简化

Import Preview 对话框的 DefaultAct 下拉框只显示已有的 Act DataLayers：

**选项：**
1. Act1 (第一个子 DataLayer) ← 默认选中
2. Act2, Act3, ...

用户选中的 Act 成为 DefaultAct (ID=1)，其他 Acts 从 ID=2 开始。

**修改文件：**
- `SDataLayerImportPreviewDialog.cpp` - 单个 Stage 导入预览
- `SBatchImportPreviewDialog.cpp` - 批量导入预览

#### 12.2 Std Rename - Act DataLayer 前缀修复

**问题：** Act DataLayer 的 Std Rename 对话框只显示 `DL_Act_` 前缀，用户需要手动输入 `StageName_ActName`。

**修复：**
- Stage 前缀：`DL_Stage_`
- Act 前缀：`DL_Act_<StageName>_`（自动从父 DataLayer 获取 StageName）

用户只需输入 ActName 部分，StageName 会自动保持正确。

**修改文件：**
- `SStdRenamePopup.cpp`

#### 12.3 Stage DataLayer 重命名 - 级联更新子 Act

**问题：** 重命名 Stage DataLayer 后，子 Act DataLayer 的 StageName 部分不会更新，导致命名不一致。

**修复：** 重命名 Stage DataLayer 时，自动级联更新所有子 Act DataLayer：

```
重命名前:
├── DL_Stage_OldName
│   ├── DL_Act_OldName_Combat
│   └── DL_Act_OldName_Stealth

重命名 Stage 为 DL_Stage_NewName:
├── DL_Stage_NewName          ← 用户修改
│   ├── DL_Act_NewName_Combat  ← 自动级联
│   └── DL_Act_NewName_Stealth ← 自动级联
```

**修改文件：**
- `StageEditorController.cpp` - `RenameStageDataLayer()` 方法

---

## 历史进度

### Phase 11 缓存事件驱动优化（✅ 完成）

详见: [Phase11_CacheEventDriven.md](Phase11_CacheEventDriven.md)

本阶段完成了缓存系统的事件驱动优化，移除了自动过期机制，大幅降低 CPU 消耗。

### Phase 10 Import 功能重设计（✅ 完成）

详见: [Phase10_ImportRedesign.md](Phase10_ImportRedesign.md)

| 子任务 | 状态 | 说明 |
|--------|------|------|
| 10.1 移除 Act 行 Import 按钮 | ✅ 完成 | 只有 Stage DataLayer 显示 Import |
| 10.2 子 Act 跟随导入 + 命名警告 | ✅ 完成 | 按层级判断，检查命名规范 |
| 10.3 DefaultAct 适配 | ✅ 完成 | 有子 Acts 时用户选择，无则自动创建 |
| 10.4 Preview 对话框重构 | ✅ 完成 | 添加 DefaultAct 下拉选择 |
| 10.5 批量 Import Preview 新建 | ✅ 完成 | SBatchImportPreviewDialog 树状布局 |
| 10.6 选择通知修复 | ✅ 完成 | FStageDataLayerMode 调用 OnSelectionChanged |
| 10.7 命名规范简化 | ⚠️ 回退 | 见 Phase 10.9 |
| 10.8 WP Streaming 同步修复 | ✅ 完成 | PostLoad 自注册，详见下方 |
| 10.9 命名规范回退 | ✅ 完成 | 回退到详细格式，详见下方 |

### Phase 10.8 WP Streaming 同步修复（✅ 完成）

#### 问题描述
在 World Partition 关卡中，如果 Stage Actor 处于卸载状态（未选择 Load Region），DataLayer 的同步状态会错误显示为"未导入"。

**根因分析：**
```
Stage Actor 被 WP 卸载
        ↓
ScanWorldForExistingStages() 使用 TActorIterator 扫描
        ↓
TActorIterator 只能遍历已加载的 Actor
        ↓
StageRegistry 中没有该 Stage 的条目
        ↓
FindStageByDataLayer() 找不到关联
        ↓
同步状态错误显示为 "NotImported"
```

#### 解决方案
让 Stage Actor 在加载后主动注册到 Subsystem：

**方案演进：**
1. 最初尝试 `PostInitializeComponents()` → 发现 PIE 模式也会触发，污染编辑器数据
2. 最终使用 `PostLoad()` + PIE 检测，只在 Editor 模式下注册

```cpp
// Stage.cpp
void AStage::PostLoad()
{
    Super::PostLoad();
#if WITH_EDITOR
    UWorld* World = GetWorld();
    if (!World) return;

    // 跳过 PIE 和游戏模式 - 避免污染编辑器数据
    if (World->IsPlayInEditor() || World->IsGameWorld()) return;

    EnsureRegisteredWithSubsystem();
#endif
}
```

**Stage 注册流程总览：**
| 场景 | 处理方式 |
|------|----------|
| 新创建 Stage | `OnLevelActorAdded` → Controller 注册 |
| 关卡加载 Stage | `ScanWorldForExistingStages` → Subsystem 扫描注册 |
| **WP streaming 加载** | `PostLoad()` → Stage 自注册 ✅ |
| PIE / 游戏模式 | 不注册（避免污染编辑器数据） |

### Phase 10.9 命名规范回退（✅ 完成）

#### 背景
Phase 10.7 简化了命名规范（`DL_Act_<ActName>`），但用户反馈需要回退到详细格式以支持更清晰的关联关系。

#### 变更内容
| 项目 | 简化格式 (10.7) | 详细格式 (10.9) |
|------|-----------------|-----------------|
| Stage DataLayer | `DL_Stage_<StageName>` | `DL_Stage_<StageName>` (不变) |
| Act DataLayer | `DL_Act_<ActName>` | `DL_Act_<StageName>_<ActName>` |

**修改文件：**
- `StageDataLayerNameParser.h/cpp` - 解析逻辑恢复 StageName 检查
- `DataLayerImporter.cpp` - 导入时检查 StageName 一致性

---

## 历史进度

### Phase 9 架构整合（已完成）

| 子任务 | 状态 | 说明 |
|--------|------|------|
| 9.1 修复 OnLevelActorAdded | ✅ 完成 | `FScopedImportBypass` RAII 模式，避免重复创建 |
| 9.2 Controller API 扩展 | ✅ 完成 | `ImportStageFromDataLayer()`, `RenameDataLayer()`, 重构调用链 |
| 9.3 事件同步机制 | ✅ 完成 | 订阅 `OnDataLayerChanged` 事件，同步外部重命名 |
| 9.4 冗余字段清理 | ✅ 完成 | 添加 `GetStageDataLayerDisplayName()` getter，保留字段兼容性 |

详见: [Architecture_Integration_Analysis.md](Architecture_Integration_Analysis.md)

### Phase 9.5 代码质量优化（已完成）

> 详细说明保留在下方，供回顾参考

#### Bug 修复

| 问题 | 修复 | 文件 |
|------|------|------|
| Stage 单例逻辑错误 | 修正为"同 BP 类型单例"而非"每关卡单例" | `StageEditorController.cpp:696-744` |

**修复说明**: 原逻辑阻止所有 Stage Actor 创建，实际应为：
- 同一 Blueprint 类型的 Stage → 一个关卡只能有一个实例
- 不同 Blueprint 类型的 Stage → 可以共存多个

#### 代码重复清理

| 优先级 | 问题 | 状态 | 说明 |
|--------|------|------|------|
| **HIGH** | FindStageAncestor 内联循环 | ✅ 完成 | 10+ 处 while 循环重构为调用 `FindStageAncestor()` |
| **HIGH** | 确认对话框模式 | ✅ 完成 | 新增 `ShowConfirmDialog()` 辅助方法，重构 7 处调用 |
| **HIGH** | SStageDataLayerOutliner UI 重复 | ✅ 完成 | 提取 `RebuildUI()` 方法，统一 Construct/OnMapChanged |
| MEDIUM | 对话框窗口设置 | ✅ 评估 | 重复量小（4-5行/处），不值得提取 |
| MEDIUM | FStageTreeItem 工厂方法 | ✅ 评估 | 需改头文件+后续设置逻辑差异大，暂缓 |
| LOW | 按钮/文本辅助 | ✅ 评估 | 影响小，暂缓 |

#### 新增辅助方法

**StageEditorPanel.h/cpp:**
```cpp
/** 显示 Yes/No 确认对话框，返回 true 表示用户点击 Yes */
bool ShowConfirmDialog(const FText& Title, const FText& Message) const;
```

**SStageDataLayerOutliner.h/cpp:**
```cpp
/** 重建整个 UI（初始化和关卡切换时调用） */
void RebuildUI();
```

#### 代码影响统计

- 减少重复代码约 **150 行**
- 提高可维护性和一致性
- 编译通过，功能正常

---

## Phase 8.4 已完成功能

### 已完成的自定义功能
- ✅ SyncStatus 列（同步状态图标，FixedWidth 24px）
- ✅ SUID 列（Stage Unique ID，ManualWidth 60px 可调整）
- ✅ Actions 列（Import/Sync 按钮，ManualWidth 60px 可调整）
- ✅ Toolbar（Sync All / Import Selected 按钮）
- ✅ Label 列（DataLayer 名称 + 图标，FillWidth 自动填充）

### 原生功能迁移状态

| 功能 | 状态 | 说明 |
|------|------|------|
| IsVisible 列（眼睛图标） | ✅ 完成 | 控制 DataLayer 编辑器可见性 |
| IsLoadedInEditor 列（勾选框） | ✅ 完成 | 控制 DataLayer 编辑器加载状态 |
| Ctrl+B 定位 Content Browser | ✅ 完成 | FindInContentBrowser 命令 |
| F2 重命名 | ✅ 完成 | 快捷键重命名 |
| F5 刷新 | ✅ 完成 | 强制刷新树 |
| 右键菜单扩展 | ✅ 完成 | Rename, Toggle/Load/Unload, Find in CB |
| Actions/SUID 列宽度可调 | ✅ 完成 | ManualWidth 支持用户拖拽调整 |
| 关卡切换同步 | ✅ 完成 | 订阅 MapChange 事件，自动更新 DataLayer 列表 |
| SUID 列 | ✅ 完成 | 显示 S:X (Stage) 或 A:X.Y (Act) |
| **Std Rename 按钮** | ✅ 完成 | 标准重命名精简对话框（根据层级自动判断 Stage/Act） |
| **Import 按钮改进** | ✅ 完成 | 始终显示 + 不规范命名时弹出警告确认 |

### 已移除/排除的功能

| 功能 | 原因 |
|------|------|
| 双击设置 Current DataLayer | 用户反馈：功能不实用 |
| Delete 键删除 DataLayer | 不需要删除功能 |
| Make Current DataLayer 按钮 | 不需要此功能 |
| 层级删除按钮列 | 不需要删除功能 |
| Create DataLayer 按钮 | StageEditor 通过 Import 创建 |

### 低优先级待评估

| 功能 | 说明 |
|------|------|
| WorldDataLayers 根节点 | 显示 WorldDataLayers 作为根容器 |
| 拖放设置父子关系 | DataLayer 层级调整 |
| 筛选选项 | Only Selected Actors / Highlight Selected |

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
│   │   └── Actors/Stage.h                      # Stage Actor (PostLoad 自注册 Phase 10.8)
│   └── Private/
│       ├── Subsystems/StageManagerSubsystem.cpp
│       └── Actors/Stage.cpp
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

*最后更新: 2025-12-01 03:30*
