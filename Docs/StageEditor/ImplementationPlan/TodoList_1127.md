# StageEditor 开发任务列表 (2025-11-27)

> 本文档记录 StageEditor 插件当前的开发任务，按优先级排序。
>
> 历史记录见：`FutureTodoList_Obsolete_Pre1127.md`

---

## 高优先级 (High Priority)

| ID | 任务名称 | 状态 | 描述 |
|----|----------|------|------|
| H-006 | TriggerZone 架构重构 | 🔵 设计完成 | 通用触发区域系统，支持蓝图自定义逻辑 |

### H-006: TriggerZone 架构重构（设计完成）

#### 设计原则

1. **不预设用户行为** - C++ 只提供基础能力，逻辑由用户蓝图决定
2. **Zone 是特殊的 Prop** - 继承 AProp，可被 Act/DataLayer 管理
3. **Component 是核心** - 可挂载到任意 Actor，自动注册到 Stage

#### 最终架构

```
UTriggerZoneComponent (核心组件)
├── OwnerStage: TSoftObjectPtr<AStage>  // 绑定目标
├── TriggerActorTags / bRequirePawn     // 过滤配置
├── OnActorEnter / OnActorExit          // 蓝图事件
├── BeginPlay() → 自动注册到 OwnerStage
└── 可挂载到任意 Actor

ATriggerZoneActor : public AProp (便利类，可选)
├── 继承 UStagePropComponent (SUID 索引、Act 管理)
├── 包含 UTriggerZoneComponent
├── PropState: 0=禁用, 非0=启用
└── 适合需要独立放置的场景

AStage (扩展)
├── RegisteredTriggerZones: TArray<UTriggerZoneComponent*>
├── RegisterTriggerZone() / UnregisterTriggerZone()
└── GetAllTriggerZones() // Debug 用
```

#### 与 Built-in Zones 的关系

| 类型 | 行为 | 适用场景 |
|------|------|---------|
| **Built-in Zones** | 硬编码逻辑（现有设计不变） | 80% 默认场景 |
| **External Zone** | 蓝图事件，用户自定义逻辑 | 任何自定义需求 |

#### 用户工作流

**方式 A：放置独立 Actor**
```
1. Place Actors → Trigger Zone Actor
2. 设置 OwnerStage
3. 创建蓝图子类，绑定 OnActorEnter 事件
4. 在蓝图中实现自定义逻辑
```

**方式 B：挂载 Component 到现有 Actor**
```
1. 选中任意 Actor
2. Add Component → Trigger Zone Component
3. 设置 OwnerStage
4. 在该 Actor 的蓝图中绑定事件
```

#### 蓝图示例

```
Event OnActorEnter (Actor)
    │
    ├─► 检查条件（用户自定义）
    │   ├─ Actor Has Tag "Player"?
    │   ├─ 检查任务状态?
    │   └─ 其他条件?
    │
    └─► 调用 API（用户决定）
        ├─ Stage->GotoState()
        ├─ Stage->ActivateAct()
        └─ Stage->SetPropState()
```

#### 实现任务

| 步骤 | 任务 | 状态 |
|------|------|------|
| 1 | 创建 UTriggerZoneComponentBase 基类（含 Description 模板） | 🔴 待开始 |
| 2 | 重构现有 UStageTriggerZoneComponent 继承基类 | 🔴 待开始 |
| 3 | 创建 ATriggerZoneActor : AProp | 🔴 待开始 |
| 4 | AStage 添加 Zone 注册 API | 🔴 待开始 |
| 5 | Stage 状态 API 简化（3态用户 API） | 🔴 待开始 |
| 6 | 编辑器可视化（Zone 连线） | 🔴 待开始 |

#### Stage 状态 API 简化设计

**问题**：5态枚举对用户来说太复杂

**解决方案**：分层抽象
- 内部状态机：5态（Unloaded, Preloading, Loaded, Active, Unloading）
- 用户 API：3态（Unloaded, Loaded, Active）

```cpp
// 用户 API（简化）
void LoadStage();       // 请求 Loaded 状态
void ActivateStage();   // 请求 Active 状态
void UnloadStage();     // 请求 Unloaded 状态
EStageState GetStageState();  // 返回 3 态

// 开发者 API（详细）
EStageRuntimeState GetStageRuntimeState();  // 返回 5 态
```

**详细设计**：`Docs/11.28/H-006_TriggerZone_Redesign_Handoff.md`

---

## 中优先级 (Medium Priority)

| ID | 任务名称 | 状态 | 描述 |
|----|----------|------|------|
| M-003 | Stage Debug HUD 工具 | 🔵 设计中 | 运行时显示 Stage/DataLayer/Zone 状态 |

### M-003: Stage Debug HUD 工具

**需求描述：**
运行时 Debug 工具，实时显示所有 Stage 和 DataLayer 的状态，方便测试和调试。

**核心功能：**
1. 屏幕角落显示所有 Stage 状态列表
2. 显示位置可配置（预设：左上/右上/左下/右下 + 自定义坐标）
3. 简洁/详细模式可切换
4. 控制台命令开关 `Stage.Debug 1/0`

**Zone Debug 扩展（H-006 关联）：**
```
【详细模式 - Zone 信息】
Stage_01 (ID:1)
├─ State: Active
├─ TriggerZones (5):
│   ├─ BuiltIn_Load: 1 actor inside
│   ├─ BuiltIn_Activate: 1 actor inside ✓
│   ├─ Zone_East: 0 actors (on Door_01)
│   └─ Zone_West: disabled
└─ Last Triggered: Zone_East (2.3s ago)
```

**技术方案：**
- `UStageDebugSettings` (UDeveloperSettings) - 配置存储
- `AStageDebugHUD` : AHUD - 绘制逻辑
- 控制台命令 `Stage.Debug 0/1`

**详细设计文档：** `M-003_StageDebugHUD_Handoff.md`

---

## 低优先级 (Low Priority)

| ID | 任务名称 | 状态 | 描述 |
|----|----------|------|------|
| L-001 | Prop 状态可视化预览 | 🔴 待设计 | 视口中显示 Prop 在各 Act 中的状态 |

### L-001: Prop 状态可视化预览

**需求描述：**
- **触发时机**：鼠标悬停在 Panel 中的 Prop 条目上
- **显示内容**：该 Prop 在各个 Act 中的不同状态值
- **可视化形式**：视口中 Prop Actor 上方显示文字标签
- **显示范围**：仅显示与默认状态不同的 Act
- **标签生命周期**：鼠标移开后延迟 0.5 秒淡出

---

## 已完成任务摘要 (2025-11-27)

| 任务 | 描述 |
|------|------|
| ✅ PlayerStart 检测 | 修复玩家出生点在 TriggerZone 内时 Stage 状态不触发 |
| ✅ InitialStageState | 禁用 Built-in Zones 时可配置初始状态 |
| ✅ TriggerActorTags | TriggerActorTag 改为数组，支持多标签过滤 |
| ✅ Extent 同步修复 | LoadZoneExtent/ActivateZoneExtent 修改后正确同步到组件 |
| ✅ ZoneType 编辑限制 | Built-in Zone 的 ZoneType 只读，External Zone 可编辑 |
| ✅ Filtering DisplayName | 简化 Per-Zone Override 的显示名称 |
| ✅ H-006 设计 | TriggerZone 架构重构设计完成 |

---

## 更新日志

| 日期 | 更新内容 |
|------|----------|
| 2025-11-27 | 创建新文档，从旧文档迁移待办任务 |
| 2025-11-27 | H-006 设计完成：TriggerZone 架构重构 |
| 2025-11-28 | H-006 增加：Description 模板系统设计 |
| 2025-11-28 | H-006 增加：Stage 状态 API 简化设计（5态内部 vs 3态用户） |
| 2025-11-28 | 创建交接文档：`Docs/11.28/H-006_TriggerZone_Redesign_Handoff.md` |
