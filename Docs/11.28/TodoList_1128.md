# StageEditor 开发任务列表 (2025-11-28)

> 本文档记录 StageEditor 插件当前的开发任务，按优先级排序。
>
> 历史记录见：`../StageEditor/ImplementationPlan/TodoList_1127.md`

---

## 任务总览

| 优先级 | ID | 任务名称 | 状态 | 描述 |
|--------|-----|----------|------|------|
| 高 | H-006 | TriggerZone 架构重构 | ✅ 完成 | 通用触发区域系统 + 预设行为 |
| 中 | M-003 | Stage Debug HUD 工具 | ✅ 完成 | 运行时状态可视化 |
| 中 | M-003.1 | Debug HUD Watch 功能 | ✅ 完成 | 选择性监视 Stage + Panel UI |
| 中 | M-004 | Debug HUD 扩展 - Zone 信息 | 🔴 待开始 | 显示 TriggerZone 注册信息 |
| 中 | M-005 | Act InitialDataLayerState | ✅ 完成 | Act 初始 DataLayer 状态 + Panel 快速编辑 |
| 中 | M-006 | Act Follow Stage State | ✅ 完成 | Act 跟随 Stage 状态选项 |
| 中 | M-007 | 批量从 Act 移除 Prop | ✅ 完成 | Panel 右键多选 Props 批量移除 |
| 低 | L-001 | Prop 状态可视化预览 | 🔴 待设计 | 视口 Prop 状态标签 |
| 低 | L-002 | Stage 单例检查优化 | 🟡 临时禁用 | BP 重编译时单例检查误删 Stage |

---

## ✅ H-006: TriggerZone 架构重构（已完成）

### 实现内容

| Story | 内容 | 文件 |
|-------|------|------|
| Story 1 | 基础类型定义 | `StageCoreTypes.h` |
| | - `EStageState` (3态用户枚举) | |
| | - `ETriggerZonePreset` (预设模板) | |
| | - `ETriggerZoneDefaultAction` (预设行为) | |
| | - `FTriggerZoneDescription` (描述结构体) | |
| Story 2 | `UTriggerZoneComponentBase` | `TriggerZoneComponentBase.h/cpp` |
| | - Description 文档属性 | |
| | - OwnerStage 绑定 | |
| | - OnActorEnter/Exit 蓝图事件 | |
| | - **OnEnterAction/OnExitAction 预设行为** | |
| | - 自动注册到 Stage | |
| Story 3 | `ATriggerZoneActor` | `TriggerZoneActor.h/cpp` |
| | - 继承 AProp (SUID 索引) | |
| | - PropState 控制启用/禁用 | |
| Story 4 | AStage API 扩展 | `Stage.h/cpp` |
| | - `GotoState(EStageState)` 用户 API | |
| | - `InternalGotoState(EStageRuntimeState)` 内部 API | |
| | - `RegisterTriggerZone/UnregisterTriggerZone` | |
| | - `GetAllTriggerZones` | |

### 用户 API

```cpp
// 主要状态控制 API
Stage->GotoState(EStageState::Unloaded);   // 请求卸载
Stage->GotoState(EStageState::Loaded);     // 请求加载
Stage->GotoState(EStageState::Active);     // 请求激活

// 便捷方法 (内部调用 GotoState)
Stage->LoadStage();       // = GotoState(Loaded)
Stage->ActivateStage();   // = GotoState(Active)
Stage->UnloadStage();     // = GotoState(Unloaded)

// 状态查询
EStageState State = Stage->GetStageState();           // 3态 (用户)
EStageRuntimeState RT = Stage->GetRuntimeState();     // 5态 (调试)
```

### 预设行为选项 (简化配置)

```cpp
// TriggerZoneComponentBase 属性
UPROPERTY OnEnterAction = ETriggerZoneDefaultAction::Custom;
UPROPERTY OnExitAction = ETriggerZoneDefaultAction::Custom;

// 预设选项:
// - Custom (Blueprint)  : 无自动操作，使用蓝图事件
// - Load Stage         : 自动调用 Stage->LoadStage()
// - Activate Stage     : 自动调用 Stage->ActivateStage()
// - Unload Stage       : 自动调用 Stage->UnloadStage()
```

### 用户工作流对比

| 场景 | 之前 (需要蓝图) | 现在 (预设行为) |
|------|----------------|----------------|
| 简单 LoadZone | 4-5 步 | **2 步** |
| 简单 ActivateZone | 4-5 步 | **2 步** |
| 复杂自定义逻辑 | 蓝图 | 蓝图 (不变) |

**简单场景工作流：**
1. 放置 `ATriggerZoneActor`
2. 设置 `OwnerStage` + `OnEnterAction = Load Stage`
3. 完成！

### 新增文件

```
Plugins/StageEditor/Source/StageEditorRuntime/
├── Public/
│   ├── Core/StageCoreTypes.h          (扩展)
│   ├── Components/TriggerZoneComponentBase.h  (NEW)
│   └── Actors/TriggerZoneActor.h              (NEW)
└── Private/
    ├── Components/TriggerZoneComponentBase.cpp (NEW)
    └── Actors/TriggerZoneActor.cpp             (NEW)
```

---

## ✅ M-003: Stage Debug HUD 工具（已完成）

**实现文件：**
- `Debug/StageDebugHUD.h/cpp` - HUD 绘制逻辑
- `Debug/StageDebugSettings.h/cpp` - 配置（DeveloperSettings）

**已实现功能：**

| 功能 | 状态 |
|------|------|
| 显示位置配置 (TopLeft/TopRight/BottomLeft/BottomRight/Custom) | ✅ |
| 简洁/详细模式切换 | ✅ |
| Stage 状态显示（5态颜色区分） | ✅ |
| DataLayer 状态显示 | ✅ |
| Act 信息显示（激活/锁定状态） | ✅ |
| Zone 内 Actor 计数（LoadZone/ActivateZone） | ✅ |
| 文字缩放配置 | ✅ |
| 屏幕边距配置 | ✅ |

**使用方法：**
1. 在 GameMode 中设置 `HUDClass = AStageDebugHUD`
2. Project Settings → Plugins → Stage Editor → Enable Debug HUD
3. 或使用控制台命令 `Stage.Debug 1`

**详细模式显示示例：**
```
=== Stage Debug (2) ===
─────────────────────────
Stage_01 (ID:1)
├─ State: Active
├─ DataLayer: Activated
├─ Acts: [1✓, 2, 3🔒]
├─ LoadZone: 1 actor
└─ ActivateZone: 1 actor

Stage_02 (ID:2)
├─ State: Loaded
├─ DataLayer: Loaded
├─ Acts: [1✓]
├─ LoadZone: 0 actors
└─ ActivateZone: 0 actors
```

---

## ✅ M-003.1: Debug HUD Watch 功能（已完成）

**需求描述：**
Debug HUD 不再显示所有 Stage 信息，而是让用户选择监视哪些 Stage。

### 实现内容

| 入口 | 功能 | 状态 |
|------|------|------|
| 控制台命令 | `Stage.Watch`, `Stage.Unwatch` 等 | ✅ 完成 |
| StageEditor Panel | TreeView 左侧眼睛图标列 | ✅ 完成 |

### 控制台命令

| 命令 | 描述 |
|------|------|
| `Stage.Debug [0\|1]` | 开关 Debug HUD |
| `Stage.Watch <ID> [ID2]...` | 添加 Stage 到监视列表 |
| `Stage.Unwatch <ID> [ID2]...` | 从监视列表移除 Stage |
| `Stage.WatchClear` | 清空监视列表 |
| `Stage.WatchAll` | 监视所有已注册的 Stage |
| `Stage.WatchOnly <ID>` | 清空列表后只监视指定 Stage |
| `Stage.WatchList` | 打印当前监视列表 |

### Panel Watch 列

- **位置**：TreeView 最左侧新增 Watch 列
- **Header**：眼睛图标
- **内容**：仅 Stage 行显示可点击的眼睛图标
- **视觉反馈**：
  - 绿色眼睛 = 正在监视
  - 灰色眼睛 = 未监视
  - 半透明灰色 = PIE 未运行

### API 扩展 (StageManagerSubsystem)

```cpp
// Watch API
bool WatchStage(int32 StageID);
bool UnwatchStage(int32 StageID);
void ClearWatchList();
void WatchAllStages();
bool WatchOnlyStage(int32 StageID);
bool IsStageWatched(int32 StageID) const;
TArray<int32> GetWatchedStageIDs() const;
int32 GetWatchedStageCount() const;
bool IsWatchListEmpty() const;
```

### HUD 显示变化

**空列表时：**
```
=== Stage Debug (0/3) ===
─────────────────────────
(No Stage being tracked)
Use: Stage.Watch <ID>
  or Stage.WatchAll
```

**有监视时：**
```
=== Stage Debug (2/3) ===
─────────────────────────
Stage_01 (ID:1)
├─ State: Active
...

Stage_02 (ID:2)
├─ State: Loaded
...
```

### 编辑模式预设（方案 B）

用户可在编辑模式下预设 watch 状态，PIE 启动时自动同步：

1. **编辑模式**：
   - Panel 眼睛图标（浅蓝色）→ 修改 `Stage->bEditorWatched`
   - Details 面板 "Watch in Debug HUD" 属性
   - 支持 Undo/Redo

2. **PIE 启动时**：
   - `RegisterStage()` 检查 `bEditorWatched`
   - 自动加入 `WatchedStageIDs`

3. **PIE 运行中**：
   - Panel 眼睛图标（绿色）→ 同时修改 Subsystem 和 Stage
   - 修改会同步回 `bEditorWatched`，下次 PIE 保持设置

### 修改文件

| 文件 | 变更 |
|------|------|
| `Stage.h` | 新增 `bEditorWatched` 属性 |
| `StageManagerSubsystem.h/cpp` | 新增 Watch API + RegisterStage 同步 watch 状态 |
| `StageEditorRuntimeModule.cpp` | 新增控制台命令 |
| `StageDebugHUD.cpp` | 修改 DrawHUD 使用监视列表 |
| `StageEditorPanel.h` | 新增 Watch 列名 |
| `StageEditorPanel.cpp` | 新增 Watch 列 + 编辑模式/PIE 模式双逻辑 |

---

## ✅ M-005: Act InitialDataLayerState 功能（已完成）

**需求描述：**
当 Stage 变为 Active 时，每个 Act 可以配置其 DataLayer 的初始状态。这允许用户控制哪些 Act 在 Stage 激活时自动加载/激活。

### 实现内容

| 组件 | 变更 | 文件 |
|------|------|------|
| FAct 结构体 | 新增 `InitialDataLayerState` 属性 | `StageCoreTypes.h` |
| AStage | 新增 `ApplyInitialActDataLayerStates()` 方法 | `Stage.h/cpp` |
| Panel UI | Act 行新增 InitialDataLayerState 下拉选择器 | `StageEditorPanel.cpp` |

### InitialDataLayerState 选项

| 状态 | 描述 |
|------|------|
| `Unloaded` | DataLayer 不加载，等待 `ActivateAct()` 调用 |
| `Loaded` | DataLayer 预加载但不激活（资源在内存中） |
| `Activated` | 随 Stage 激活时自动激活 DataLayer |

### 默认值

- **Default Act (ID=1)**：默认为 `Activated`（随 Stage 状态走）
- **其他 Act**：默认为 `Unloaded`（按需加载）

### Panel UI 快速编辑

在 StageEditor Panel 的 TreeView 中，Act 行的 Actions 列新增下拉选择器：

```
Stage_01 (S_1.0.0)
├─ Acts
│   ├─ A_1.1.0  Default Act  [Activated ▼] [🗑️]
│   └─ A_1.2.0  Battle Act   [Unloaded ▼]  [🗑️]
└─ Props
```

- 点击下拉框可快速切换 InitialDataLayerState
- 支持 Undo/Redo（使用 FScopedTransaction）
- 实时显示当前状态

### 代码示例

```cpp
// FAct 结构体
USTRUCT(BlueprintType)
struct FAct
{
    // ... existing properties ...

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Act",
        meta = (DisplayName = "Initial DataLayer State"))
    EDataLayerRuntimeState InitialDataLayerState = EDataLayerRuntimeState::Unloaded;
};

// Stage 应用初始状态
void AStage::ApplyInitialActDataLayerStates()
{
    for (const FAct& Act : Acts)
    {
        switch (Act.InitialDataLayerState)
        {
        case EDataLayerRuntimeState::Activated:
            ActivateAct(Act.SUID.ActID);
            break;
        case EDataLayerRuntimeState::Loaded:
            SetActDataLayerState(Act.SUID.ActID, EDataLayerRuntimeState::Loaded);
            break;
        // Unloaded: do nothing
        }
    }
}
```

---

## 🔴 M-004: Debug HUD 扩展 - Zone 信息（待开始）

**需求描述：**
扩展现有 Debug HUD，显示 H-006 新增的 TriggerZone 注册信息。

**扩展内容：**
```
Stage_01 (ID:1) [Active]
├─ State: Active
├─ DataLayer: Activated
├─ Acts: [1✓, 2]
├─ Registered Zones (3):        ← 新增
│   ├─ TriggerZone_Load_North   ← 新增
│   ├─ TriggerZone_Load_East    ← 新增
│   └─ TriggerZone_Activate     ← 新增
├─ LoadZone: 1 actor
└─ ActivateZone: 1 actor
```

**实现任务：**

| 步骤 | 任务 | 状态 |
|------|------|------|
| 1 | 在 DrawStageDetailed 中添加 Registered Zones 信息 | 🔴 待开始 |
| 2 | 显示 Zone 名称和启用状态 | 🔴 待开始 |
| 3 | 显示 Zone 内 Actor 数量（可选） | 🔴 待开始 |

---

## ✅ M-006: Act Follow Stage State（已完成）

**需求描述：**
当 Act 勾选 `bFollowStageState` 时，Act 的 DataLayer 状态完全跟随 Stage 状态变化，无需手动配置 `InitialDataLayerState`。

### 实现内容

| 组件 | 变更 | 文件 |
|------|------|------|
| FAct 结构体 | 新增 `bFollowStageState` 属性 | `StageCoreTypes.h` |
| AStage | 新增 `ApplyFollowingActStates()` 方法 | `Stage.h/cpp` |
| Panel UI | Act 行新增 Follow Checkbox | `StageEditorPanel.cpp` |

### 行为对比

| bFollowStageState | Stage Loaded | Stage Active | Stage Unloaded |
|-------------------|--------------|--------------|----------------|
| ✅ 勾选 | Act → Loaded | Act → Activated | Act → Unloaded |
| ❌ 不勾选 | 无变化 | 使用 InitialDataLayerState | 无变化 |

### 默认值

- **Default Act (ID=1)**：默认勾选 `bFollowStageState = true`
- **其他 Act**：默认不勾选

### Panel UI

```
Stage_01 (S_1.0.0)
├─ Acts
│   ├─ A_1.1.0  Default Act  [✓ Follow] [(Follow) ▼] [🗑️]
│   └─ A_1.2.0  Battle Act   [☐ Follow] [Unloaded ▼] [🗑️]
```

- Checkbox 勾选时，下拉框显示 "(Follow)" 并禁用
- 支持 Undo/Redo

---

## ✅ M-007: 批量从 Act 移除 Prop（已完成）

**需求描述：**
在 StageEditor Panel 中，允许用户多选同一 Act 下的 Props，然后右键批量移除。

### 实现内容

| 功能 | 描述 |
|------|------|
| 批量从 Act 移除 | 多选同一 Act 下的 Props，显示 "Remove X Props from Act" |
| 批量从 Stage 注销 | 多选任意 Props，显示 "Unregister X Props from Stage" |

### 右键菜单示例

```
选中 3 个 Props 在 Act_Battle 下:
┌─────────────────────────────────┐
│ Remove 3 Props from Act         │
│ ─────────────────────────────── │
│ Unregister 3 Props from Stage   │
└─────────────────────────────────┘
```

### Bug 修复：Prop DataLayer 分配

同时修复了 Prop 注册时的 DataLayer 分配问题：

- **之前**：Props 同时获得 Stage DataLayer + Default Act DataLayer 标签
- **之后**：Props 只获得 Default Act DataLayer 标签

**原因**：Stage DataLayer 始终为 Active，带有该标签的 Actor 会绕过 Act 管理。

---

## 🟡 L-002: Stage 单例检查优化（临时禁用）

**问题描述：**
当修改 Stage 蓝图并重新编译时，单例检查会错误地删除新实例（重编译后的版本）。

### 问题原因

1. BP 重编译时，UE 创建新的 Actor 实例
2. 旧实例在短暂时间内仍然存在
3. `OnLevelActorAdded` 触发单例检查
4. 检测到"两个 Stage"，删除新实例（错误）

### 当前状态

**临时禁用单例检查** (`StageEditorController.cpp:672-712`)

### 未来改进方案

| 方案 | 描述 | 优缺点 |
|------|------|--------|
| 检查蓝图来源 | 如果两个 Stage 来自同一个 BP 类，则跳过检查 | 允许 BP 重编译，但不允许同类多实例 |
| 延迟检查 | 使用 `FTimerHandle` 延迟执行检查 | 给 UE 时间清理旧实例 |
| 检查 StageID | 如果 StageID 相同，视为同一个 Stage | 需要可靠的 ID 持久化机制 |
| 完全移除 | 允许多 Stage | 最简单，但改变原设计意图 |

---

## 🔴 L-001: Prop 状态可视化预览（待设计）

**需求描述：**
- **触发时机**：鼠标悬停在 Panel 中的 Prop 条目上
- **显示内容**：该 Prop 在各个 Act 中的不同状态值
- **可视化形式**：视口中 Prop Actor 上方显示文字标签
- **显示范围**：仅显示与默认状态不同的 Act
- **标签生命周期**：鼠标移开后延迟 0.5 秒淡出

---

## 2025-11-28 完成任务

| 任务 | 描述 |
|------|------|
| ✅ H-006 Story 1 | 基础类型 (EStageState, ETriggerZonePreset, FTriggerZoneDescription) |
| ✅ H-006 Story 2 | UTriggerZoneComponentBase (事件、绑定、过滤、自动注册) |
| ✅ H-006 Story 3 | ATriggerZoneActor (继承 AProp，PropState 控制) |
| ✅ H-006 Story 4 | AStage API 扩展 (RegisterTriggerZone, GetAllTriggerZones) |
| ✅ 预设行为 | OnEnterAction/OnExitAction (Load/Activate/Unload Stage) |
| ✅ API 重命名 | GotoState(EStageState) 用户 API，InternalGotoState 内部 API |
| ✅ M-003.1 Watch API | StageManagerSubsystem Watch 系列方法 |
| ✅ M-003.1 控制台命令 | Stage.Watch/Unwatch/WatchAll/WatchClear/WatchOnly/WatchList |
| ✅ M-003.1 HUD 更新 | 仅显示监视列表中的 Stage |
| ✅ M-003.1 Panel UI | TreeView 左侧眼睛图标列，点击切换监视状态 |
| ✅ M-003.1 编辑模式支持 | bEditorWatched 属性 + PIE 启动时同步 |
| ✅ 代码简化 | 移除 PlayerStart 检测，延迟一帧策略已足够 |
| ✅ Act InitialDataLayerState | 每个 Act 可配置初始 DataLayer 状态 |
| ✅ Panel 快速编辑 | Act 行新增 InitialDataLayerState 下拉选择器 |
| ✅ M-006 Follow Stage | Act 跟随 Stage 状态选项 (bFollowStageState) |
| ✅ M-007 批量移除 | Panel 右键多选 Props 批量从 Act 移除/从 Stage 注销 |
| ✅ Bug 修复 | Prop 注册 DataLayer 分配修复（只分配 Default Act DataLayer）|
| 🟡 L-002 单例检查 | 临时禁用单例检查（BP 重编译误删问题）|

---

## 2025-11-27 完成任务

| 任务 | 描述 |
|------|------|
| ~~✅ PlayerStart 检测~~ | ~~修复玩家出生点在 TriggerZone 内时 Stage 状态不触发~~ (11.28 移除，延迟一帧策略已足够) |
| ✅ InitialStageState | 禁用 Built-in Zones 时可配置初始状态 |
| ✅ TriggerActorTags | TriggerActorTag 改为数组，支持多标签过滤 |
| ✅ Extent 同步修复 | LoadZoneExtent/ActivateZoneExtent 修改后正确同步 |
| ✅ H-006 设计 | TriggerZone 架构重构设计完成 |
| ✅ M-003 Debug HUD | Stage Debug HUD 基础功能实现 |

---

## 更新日志

| 日期 | 更新内容 |
|------|----------|
| 2025-11-28 | 创建新文档，H-006 全部完成 |
| 2025-11-28 | H-006: 预设行为 OnEnterAction/OnExitAction |
| 2025-11-28 | API 重命名: GotoState(EStageState) vs InternalGotoState |
| 2025-11-28 | M-003.1: Debug HUD Watch 功能 - 控制台命令 + Watch API |
| 2025-11-28 | M-003.1: StageEditor Panel 新增 Watch 列（眼睛图标） |
| 2025-11-28 | M-003.1: 编辑模式支持 - bEditorWatched 属性 + RegisterStage 同步 |
| 2025-11-28 | 简化: 移除 PlayerStart 检测，CheckInitialOverlaps 精简 |
| 2025-11-28 | Act InitialDataLayerState: FAct 新增属性 + Stage 应用初始状态 |
| 2025-11-28 | Panel 快速编辑: Act 行新增 InitialDataLayerState 下拉选择器 |
| 2025-11-28 | M-006: Act bFollowStageState 选项 + ApplyFollowingActStates() |
| 2025-11-28 | M-007: Panel 右键批量移除 Props from Act / Unregister from Stage |
| 2025-11-28 | Bug 修复: Prop 注册只分配 Default Act DataLayer (不再分配 Stage DataLayer) |
| 2025-11-28 | L-002: 临时禁用 Stage 单例检查 (BP 重编译误删问题) |
