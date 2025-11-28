# StageEditor Blueprint API 文档

> 版本: Phase 1.2
> 更新日期: 2025-11-28

本文档详细说明 StageEditor 插件暴露给蓝图的所有 API。

---

## 目录

- [AStage (舞台Actor)](#astage-舞台actor)
  - [事件/委托](#事件委托)
  - [Stage 状态控制 API (3态)](#stage-状态控制-api-3态)
  - [Stage 状态查询 API](#stage-状态查询-api)
  - [Act 控制 API](#act-控制-api)
  - [Act 查询 API](#act-查询-api)
  - [Act 锁定 API](#act-锁定-api)
  - [Prop 状态控制 API](#prop-状态控制-api)
  - [Prop 查询 API](#prop-查询-api)
  - [DataLayer 控制 API](#datalayer-控制-api)
  - [TriggerZone API](#triggerzone-api)
- [UStagePropComponent (道具组件)](#ustagepropcomponent-道具组件)
  - [事件/委托](#事件委托-1)
  - [状态控制 API](#状态控制-api)
  - [Stage 交互 API](#stage-交互-api)
- [UTriggerZoneComponentBase (触发区域组件)](#utriggerzonecomponentbase-触发区域组件)
  - [事件/委托](#事件委托-2)
  - [Stage 绑定 API](#stage-绑定-api)
  - [过滤 API](#过滤-api)
  - [预设行为](#预设行为)
  - [状态控制 API](#状态控制-api-1)
  - [文档 API](#文档-api)
- [数据类型](#数据类型)
- [使用示例](#使用示例)

---

## AStage (舞台Actor)

舞台的"导演"，管理 Acts 列表和 Props 注册表，负责将 Act 状态应用到 Props。

### 核心属性 (BlueprintReadOnly)

| 属性 | 类型 | 说明 |
|------|------|------|
| `SUID` | `FSUID` | 舞台唯一标识符，包含 StageID |
| `Acts` | `TArray<FAct>` | 所有已定义的 Act 列表 |
| `PropRegistry` | `TMap<int32, AActor*>` | 已注册的 Props (PropID → Actor) |
| `ActiveActIDs` | `TArray<int32>` | 当前激活的 Act ID 列表（按激活顺序） |
| `RecentActivatedActID` | `int32` | 最近激活的 Act ID |
| `CurrentStageState` | `EStageRuntimeState` | 当前运行时状态（5态） |
| `StageDataLayerAsset` | `UDataLayerAsset*` | Stage 的根 DataLayer 资产 |
| `RegisteredTriggerZones` | `TArray<UTriggerZoneComponentBase*>` | 已注册的触发区域 |

---

### 事件/委托

#### OnActActivated
```
委托签名: FOnActActivated(int32 ActID)
```
当 Act 被激活时广播。

**参数:**
- `ActID` - 被激活的 Act ID

---

#### OnActDeactivated
```
委托签名: FOnActDeactivated(int32 ActID)
```
当 Act 被停用时广播。

**参数:**
- `ActID` - 被停用的 Act ID

---

#### OnActiveActsChanged
```
委托签名: FOnActiveActsChanged()
```
当激活的 Acts 列表变更时广播（用于 UI 刷新）。

---

#### OnStagePropStateChanged
```
委托签名: FOnStagePropStateChanged(int32 PropID, int32 OldState, int32 NewState)
```
当 Stage 内任意 Prop 状态变更时广播。

**参数:**
- `PropID` - 状态变更的 Prop ID
- `OldState` - 变更前的状态值
- `NewState` - 变更后的状态值

---

### Stage 状态控制 API (3态)

这是主要的用户 API，使用简化的 3 态模型。

#### GotoState
```cpp
void GotoState(EStageState TargetState)
```
请求 Stage 转换到目标状态。**这是控制 Stage 状态的主要 API**。

**参数:**
- `TargetState` - 目标状态 (`Unloaded` / `Loaded` / `Active`)

**状态转换:**
- `GotoState(Unloaded)` → 触发卸载序列
- `GotoState(Loaded)` → 触发加载序列（预加载 → 已加载）
- `GotoState(Active)` → 触发完全激活（必要时先加载 → 激活）

**安全性:**
- 可在任何状态下调用
- 无效转换会被优雅忽略

---

#### LoadStage
```cpp
void LoadStage()
```
便捷方法：请求 Stage 加载。等同于 `GotoState(EStageState::Loaded)`。

---

#### ActivateStage
```cpp
void ActivateStage()
```
便捷方法：请求 Stage 激活。等同于 `GotoState(EStageState::Active)`。

---

#### UnloadStage
```cpp
void UnloadStage()
```
便捷方法：请求 Stage 卸载。等同于 `GotoState(EStageState::Unloaded)`。

---

### Stage 状态查询 API

#### GetStageState
```cpp
EStageState GetStageState() const
```
获取简化的 Stage 状态（3态用户视图）。

**状态映射:**
| 内部状态 | 用户状态 |
|----------|----------|
| Unloaded, Unloading | `Unloaded` |
| Preloading, Loaded | `Loaded` |
| Active | `Active` |

**返回值:**
- 简化的 Stage 状态

---

#### GetRuntimeState (调试用)
```cpp
EStageRuntimeState GetRuntimeState() const
```
获取内部运行时状态（5态开发者视图）。用于调试和详细状态检查。

**返回值:**
- `Unloaded` / `Preloading` / `Loaded` / `Active` / `Unloading`

---

#### GetCurrentStageState
```cpp
EStageRuntimeState GetCurrentStageState() const
```
获取当前 Stage 运行时状态（同 `GetRuntimeState`）。

---

#### IsStageLoaded
```cpp
bool IsStageLoaded() const
```
检查 Stage DataLayer 是否已加载（Loaded 或 Active 状态）。

---

#### IsStageActive
```cpp
bool IsStageActive() const
```
检查 Stage 是否完全激活。

---

#### IsInTransitionState
```cpp
bool IsInTransitionState() const
```
检查 Stage 是否处于过渡状态（Preloading 或 Unloading）。

---

#### GetStageID
```cpp
int32 GetStageID() const
```
获取 Stage 的唯一 ID（便捷方法，等同于 `SUID.StageID`）。

---

### Act 控制 API

#### ActivateAct
```cpp
void ActivateAct(int32 ActID)
```
激活指定 Act，应用其 PropState 配置并激活关联的 DataLayer。

**参数:**
- `ActID` - 要激活的 Act ID

**行为:**
1. 将 ActID 添加到 `ActiveActIDs`（如果尚未激活）
2. 将 Act 的 PropStateOverrides 应用到所有相关 Props
3. 激活目标 Act 的 DataLayer
4. 更新 `RecentActivatedActID`
5. 广播 `OnActActivated` 事件

---

#### DeactivateAct
```cpp
void DeactivateAct(int32 ActID)
```
停用指定 Act，卸载其 DataLayer。

**参数:**
- `ActID` - 要停用的 Act ID

**行为:**
1. 从 `ActiveActIDs` 移除 ActID
2. 卸载 Act 的 DataLayer
3. 重新应用剩余 Acts 的 PropState（按优先级）
4. 广播 `OnActDeactivated` 事件

**注意:** 锁定的 Act 无法停用（见 `LockAct`）

---

#### ApplyActPropStatesOnly
```cpp
bool ApplyActPropStatesOnly(int32 ActID)
```
仅应用 Act 的 PropState 配置，**不切换 DataLayer**。适用于预览或不需要流式加载的场景。

**参数:**
- `ActID` - 要应用的 Act ID

**返回值:**
- `true` - 成功找到并应用
- `false` - Act 不存在

---

### Act 查询 API

#### IsActActive
```cpp
bool IsActActive(int32 ActID) const
```
检查指定 Act 是否处于激活状态。

**参数:**
- `ActID` - Act ID

**返回值:**
- `true` - Act 在 `ActiveActIDs` 中
- `false` - Act 未激活

---

#### GetActiveActIDs
```cpp
TArray<int32> GetActiveActIDs() const
```
获取当前激活的 Act ID 列表（按激活顺序）。

**返回值:**
- 激活的 ActID 数组，最后一个是最高优先级

---

#### GetActiveActCount
```cpp
int32 GetActiveActCount() const
```
获取当前激活的 Act 数量。

---

#### GetHighestPriorityActID
```cpp
int32 GetHighestPriorityActID() const
```
获取最高优先级（最近激活）的 Act ID。

**返回值:**
- 最高优先级 ActID，无激活时返回 -1

---

#### GetActDisplayName
```cpp
FString GetActDisplayName(int32 ActID) const
```
获取 Act 的显示名称。

**参数:**
- `ActID` - Act ID

**返回值:**
- Act 的 DisplayName，未找到则返回空字符串

---

#### GetActPropStates
```cpp
TMap<int32, int32> GetActPropStates(int32 ActID) const
```
获取 Act 配置的 PropState 映射表。

**参数:**
- `ActID` - Act ID

**返回值:**
- PropID → State 的映射表，未找到则返回空 Map

---

#### GetAllActIDs
```cpp
TArray<int32> GetAllActIDs() const
```
获取 Stage 内所有 Act ID。

**返回值:**
- 所有 ActID 的数组

---

#### GetActCount
```cpp
int32 GetActCount() const
```
获取 Act 总数。

---

#### DoesActExist
```cpp
bool DoesActExist(int32 ActID) const
```
检查指定 Act 是否存在。

**参数:**
- `ActID` - 要检查的 Act ID

**返回值:**
- `true` - 存在
- `false` - 不存在

---

### Act 锁定 API

#### LockAct
```cpp
void LockAct(int32 ActID)
```
锁定 Act，防止其被停用。

**参数:**
- `ActID` - 要锁定的 Act ID

---

#### UnlockAct
```cpp
void UnlockAct(int32 ActID)
```
解锁 Act，允许其被停用。

**参数:**
- `ActID` - 要解锁的 Act ID

---

#### IsActLocked
```cpp
bool IsActLocked(int32 ActID) const
```
检查指定 Act 是否被锁定。

**参数:**
- `ActID` - Act ID

**返回值:**
- `true` - 已锁定
- `false` - 未锁定

---

#### GetLockedActIDs
```cpp
TArray<int32> GetLockedActIDs() const
```
获取所有已锁定的 Act ID。

---

### Prop 状态控制 API

#### SetPropStateByID
```cpp
bool SetPropStateByID(int32 PropID, int32 NewState, bool bForce = false)
```
通过 ID 设置单个 Prop 的状态。

**参数:**
- `PropID` - Prop ID
- `NewState` - 新状态值
- `bForce` - 是否强制触发更新（即使状态相同）

**返回值:**
- `true` - 成功
- `false` - Prop 不存在或无 PropComponent

---

#### GetPropStateByID
```cpp
int32 GetPropStateByID(int32 PropID) const
```
获取指定 Prop 的当前状态。

**参数:**
- `PropID` - Prop ID

**返回值:**
- 当前状态值，未找到返回 -1

---

#### SetMultiplePropStates
```cpp
void SetMultiplePropStates(const TMap<int32, int32>& PropStates)
```
批量设置多个 Prop 的状态。

**参数:**
- `PropStates` - PropID → State 的映射表

---

### Prop 查询 API

#### GetPropActorByID
```cpp
AActor* GetPropActorByID(int32 PropID) const
```
通过 ID 获取 Prop Actor。

**参数:**
- `PropID` - Prop ID

**返回值:**
- Prop Actor 指针，未找到返回 nullptr

---

#### GetPropComponentByID
```cpp
UStagePropComponent* GetPropComponentByID(int32 PropID) const
```
通过 ID 获取 Prop 的 StagePropComponent。

**参数:**
- `PropID` - Prop ID

**返回值:**
- UStagePropComponent 指针，未找到返回 nullptr

---

#### GetAllPropIDs
```cpp
TArray<int32> GetAllPropIDs() const
```
获取所有已注册的 Prop ID。

---

#### GetAllPropActors
```cpp
TArray<AActor*> GetAllPropActors() const
```
获取所有已注册的 Prop Actor。

---

#### GetPropCount
```cpp
int32 GetPropCount() const
```
获取已注册 Prop 总数。

---

#### DoesPropExist
```cpp
bool DoesPropExist(int32 PropID) const
```
检查指定 Prop 是否存在。

**参数:**
- `PropID` - 要检查的 Prop ID

**返回值:**
- `true` - 存在且有效
- `false` - 不存在或已失效

---

### DataLayer 控制 API

#### SetActDataLayerState
```cpp
bool SetActDataLayerState(int32 ActID, EDataLayerRuntimeState NewState)
```
设置 Act 关联 DataLayer 的运行时状态。**不会**改变 `ActiveActIDs` 或应用 PropState。

**参数:**
- `ActID` - Act ID
- `NewState` - 目标状态 (Unloaded/Loaded/Activated)

**返回值:**
- `true` - 成功
- `false` - Act 不存在或无关联 DataLayer

---

#### GetActDataLayerState
```cpp
EDataLayerRuntimeState GetActDataLayerState(int32 ActID) const
```
获取 Act 关联 DataLayer 的当前运行时状态。

**参数:**
- `ActID` - Act ID

**返回值:**
- 当前状态，未找到返回 `Unloaded`

---

#### IsActDataLayerLoaded
```cpp
bool IsActDataLayerLoaded(int32 ActID) const
```
检查 Act 的 DataLayer 是否已加载或激活。

**参数:**
- `ActID` - Act ID

**返回值:**
- `true` - Loaded 或 Activated 状态
- `false` - Unloaded 或不存在

---

#### LoadActDataLayer / UnloadActDataLayer
```cpp
bool LoadActDataLayer(int32 ActID)
bool UnloadActDataLayer(int32 ActID)
```
便捷函数：加载/卸载 Act 的 DataLayer。

---

#### SetStageDataLayerState / GetStageDataLayerState
```cpp
bool SetStageDataLayerState(EDataLayerRuntimeState NewState)
EDataLayerRuntimeState GetStageDataLayerState() const
```
控制 Stage 根 DataLayer 的状态。

---

#### GetActDataLayerAsset
```cpp
UDataLayerAsset* GetActDataLayerAsset(int32 ActID) const
```
获取 Act 关联的 DataLayer 资产。

---

### TriggerZone API

#### RegisterTriggerZone
```cpp
void RegisterTriggerZone(UTriggerZoneComponentBase* Zone)
```
注册一个 TriggerZone 到此 Stage。
由 `UTriggerZoneComponentBase` 在 `OwnerStage` 设置时自动调用。

**参数:**
- `Zone` - 要注册的区域

---

#### UnregisterTriggerZone
```cpp
void UnregisterTriggerZone(UTriggerZoneComponentBase* Zone)
```
从此 Stage 取消注册 TriggerZone。
由 `UTriggerZoneComponentBase` 在 `EndPlay` 时自动调用。

**参数:**
- `Zone` - 要取消注册的区域

---

#### GetAllTriggerZones
```cpp
TArray<UTriggerZoneComponentBase*> GetAllTriggerZones() const
```
获取所有已注册的 TriggerZone。

**返回值:**
- 所有已注册区域的数组

---

#### GetTriggerZoneCount
```cpp
int32 GetTriggerZoneCount() const
```
获取已注册的 TriggerZone 数量。

---

#### ForceStageStateOverride
```cpp
void ForceStageStateOverride(EStageRuntimeState NewState, bool bLockState = false)
```
强制 Stage 进入指定状态，绕过正常的 TriggerZone 转换。
用于 StageManagerSubsystem 跨 Stage 协调。

**参数:**
- `NewState` - 强制进入的状态
- `bLockState` - 如果为 true，Stage 将保持此状态直到调用 `ReleaseStageStateOverride`

---

#### ReleaseStageStateOverride
```cpp
void ReleaseStageStateOverride()
```
释放 Stage 状态锁定，允许正常的 TriggerZone 转换。

---

#### IsStageStateLocked
```cpp
bool IsStageStateLocked() const
```
检查 Stage 状态是否当前被锁定。

---

## UStagePropComponent (道具组件)

使任意 Actor 成为可控的舞台道具。可添加到任何 Actor 使其响应 Stage 状态变更。

### 核心属性 (BlueprintReadOnly)

| 属性 | 类型 | 说明 |
|------|------|------|
| `SUID` | `FSUID` | 道具唯一标识符，包含 StageID 和 PropID |
| `OwnerStage` | `AStage*` | 所属的 Stage Actor |
| `PropState` | `int32` | 当前状态值 |
| `PreviousPropState` | `int32` | 上一个状态值（用于过渡逻辑） |

---

### 事件/委托

#### OnPropStateChanged
```
委托签名: FOnPropStateChanged(int32 NewState, int32 OldState)
```
当 PropState 变更时广播。在蓝图中绑定此事件实现状态响应逻辑。

**参数:**
- `NewState` - 新状态值
- `OldState` - 旧状态值

---

### 状态控制 API

#### SetPropState
```cpp
void SetPropState(int32 NewState, bool bForce = false)
```
设置 Prop 的状态。

**参数:**
- `NewState` - 新状态值
- `bForce` - 强制触发更新（即使状态相同）

**行为:**
1. 保存当前状态到 PreviousPropState
2. 更新 PropState
3. 广播 OnPropStateChanged 事件
4. (Editor) 重新运行 Construction Scripts

---

#### GetPropState
```cpp
int32 GetPropState() const
```
获取当前状态值。

---

#### GetPreviousPropState
```cpp
int32 GetPreviousPropState() const
```
获取上一个状态值。用于实现过渡动画或条件逻辑。

---

#### IncrementState
```cpp
void IncrementState()
```
状态值 +1。适用于简单的顺序状态机。

---

#### DecrementState
```cpp
void DecrementState()
```
状态值 -1。适用于简单的顺序状态机。

---

#### ToggleState
```cpp
void ToggleState(int32 StateA, int32 StateB)
```
在两个状态之间切换。

**参数:**
- `StateA` - 状态 A
- `StateB` - 状态 B

**行为:**
- 当前为 StateA → 切换到 StateB
- 当前为 StateB 或其他 → 切换到 StateA

---

### Stage 交互 API

#### GetOwnerStage
```cpp
AStage* GetOwnerStage() const
```
获取所属的 Stage Actor。

**返回值:**
- Stage Actor 指针，未注册返回 nullptr

---

#### GetPropID
```cpp
int32 GetPropID() const
```
获取 Prop 在 Stage 内的唯一 ID（便捷方法，等同于 `SUID.PropID`）。

---

#### GetOwnerStageID
```cpp
int32 GetOwnerStageID() const
```
获取所属 Stage 的 ID（便捷方法，等同于 `SUID.StageID`）。

---

#### IsRegisteredToStage
```cpp
bool IsRegisteredToStage() const
```
检查此 Prop 是否已注册到 Stage。

**返回值:**
- `true` - 已注册（有有效的 OwnerStage 且 PropID > 0）
- `false` - 未注册

---

## UTriggerZoneComponentBase (触发区域组件)

通用触发区域组件，提供 overlap 检测、蓝图事件和预设行为。

### 核心属性 (BlueprintReadWrite)

| 属性 | 类型 | 说明 |
|------|------|------|
| `OwnerStage` | `TSoftObjectPtr<AStage>` | 绑定的 Stage |
| `TriggerActorTags` | `TArray<FName>` | 触发 Actor 标签过滤 |
| `bRequirePawn` | `bool` | 是否要求触发者为 Pawn |
| `OnEnterAction` | `ETriggerZoneDefaultAction` | 进入时的预设行为 |
| `OnExitAction` | `ETriggerZoneDefaultAction` | 离开时的预设行为 |
| `bZoneEnabled` | `bool` | 区域是否启用 |
| `Description` | `FTriggerZoneDescription` | 区域描述（用于文档/调试） |

---

### 事件/委托

#### OnActorEnter
```
委托签名: FOnTriggerZoneActorEnter(UTriggerZoneComponentBase* Zone, AActor* Actor)
```
当有效 Actor 进入区域时广播。在蓝图中绑定此事件实现自定义触发逻辑。

**参数:**
- `Zone` - 触发的区域
- `Actor` - 进入的 Actor

**使用场景:**
- 检查额外条件
- 调用 Stage API (LoadStage, ActivateAct 等)
- 播放效果、声音等

---

#### OnActorExit
```
委托签名: FOnTriggerZoneActorExit(UTriggerZoneComponentBase* Zone, AActor* Actor)
```
当有效 Actor 离开区域时广播。

**参数:**
- `Zone` - 触发的区域
- `Actor` - 离开的 Actor

---

### Stage 绑定 API

#### IsBoundToStage
```cpp
bool IsBoundToStage() const
```
检查此区域是否绑定到 Stage。

---

#### GetOwnerStage
```cpp
AStage* GetOwnerStage() const
```
获取绑定的 Stage（从软指针解析）。

**返回值:**
- Stage Actor 指针，未绑定或无效返回 nullptr

---

### 过滤 API

#### ShouldTriggerForActor
```cpp
bool ShouldTriggerForActor(AActor* Actor) const
```
判断 Actor 是否应该触发区域事件。可在蓝图或 C++ 中覆盖以实现自定义过滤。

**默认行为:**
- 如果 `TriggerActorTags` 为空：仅允许 Pawn
- 如果设置了标签：Actor 必须有任一标签
- 如果 `bRequirePawn` 为 true：额外要求是 Pawn

**参数:**
- `Actor` - 要检查的 Actor

**返回值:**
- `true` - 应该触发事件
- `false` - 应该忽略

---

### 预设行为

预设行为简化了常见场景的配置，无需编写蓝图逻辑。

#### OnEnterAction / OnExitAction

设置 `OnEnterAction` 或 `OnExitAction` 属性为以下值之一：

| 值 | 描述 |
|----|------|
| `Custom` | 无自动操作，使用 `OnActorEnter`/`OnActorExit` 事件 |
| `LoadStage` | 自动调用 `Stage->LoadStage()` |
| `ActivateStage` | 自动调用 `Stage->ActivateStage()` |
| `UnloadStage` | 自动调用 `Stage->UnloadStage()` |

**典型配置:**

| 场景 | OnEnterAction | OnExitAction |
|------|---------------|--------------|
| 加载区域 | `LoadStage` | `Custom` 或 `UnloadStage` |
| 激活区域 | `ActivateStage` | `Custom` |
| 卸载区域 | `Custom` | `UnloadStage` |

---

### 状态控制 API

#### SetZoneEnabled
```cpp
void SetZoneEnabled(bool bEnabled)
```
设置区域启用状态。禁用时不会触发任何事件。

**参数:**
- `bEnabled` - 是否启用

---

#### IsZoneEnabled
```cpp
bool IsZoneEnabled() const
```
检查区域是否启用。

---

### 文档 API

#### GetDescriptionText
```cpp
FString GetDescriptionText() const
```
获取可读的描述文本（用于调试显示）。

**格式:**
```
"When [Trigger] enters this zone, and [Condition], execute [Action] on [Target] to [Effect]."
```

---

#### GetDescription
```cpp
const FTriggerZoneDescription& GetDescription() const
```
获取描述结构体（用于详细解析）。

---

#### ApplyDescriptionPreset
```cpp
void ApplyDescriptionPreset(ETriggerZonePreset Preset)
```
应用预设模板快速填充描述字段。

**参数:**
- `Preset` - 预设模板类型

---

## 数据类型

### FSUID (Stage Unique ID)
```cpp
USTRUCT(BlueprintType)
struct FSUID
{
    int32 StageID;  // Stage 唯一标识
    int32 ActID;    // Act 唯一标识 (Stage 内)
    int32 PropID;   // Prop 唯一标识 (Stage 内)
};
```

### FAct
```cpp
USTRUCT(BlueprintType)
struct FAct
{
    FSUID SUID;                              // Act 的唯一标识
    FString DisplayName;                     // 显示名称
    EDataLayerRuntimeState InitialDataLayerState;  // Stage 激活时的初始状态
    UDataLayerAsset* AssociatedDataLayer;    // 关联的 DataLayer
    TMap<int32, int32> PropStateOverrides;   // PropID → State 映射
};
```

**InitialDataLayerState 选项:**

| 值 | 描述 |
|----|------|
| `Unloaded` | 不加载，等待 `ActivateAct()` 调用 |
| `Loaded` | 预加载但不激活（资源在内存中） |
| `Activated` | 随 Stage 激活时自动激活 |

**默认值:**
- Default Act (ID=1)：`Activated`
- 其他 Act：`Unloaded`

### EStageState (用户 3 态)
```cpp
enum class EStageState : uint8
{
    Unloaded,   // 未加载
    Loaded,     // 已加载（预加载完成）
    Active      // 已激活（可交互）
};
```

### EStageRuntimeState (内部 5 态)
```cpp
enum class EStageRuntimeState : uint8
{
    Unloaded,    // 未加载
    Preloading,  // 正在加载（过渡状态）
    Loaded,      // 已加载
    Active,      // 已激活
    Unloading    // 正在卸载（过渡状态）
};
```

### ETriggerZoneDefaultAction
```cpp
enum class ETriggerZoneDefaultAction : uint8
{
    Custom,         // 无自动操作，使用蓝图事件
    LoadStage,      // 自动调用 Stage->LoadStage()
    ActivateStage,  // 自动调用 Stage->ActivateStage()
    UnloadStage     // 自动调用 Stage->UnloadStage()
};
```

### ETriggerZonePreset
```cpp
enum class ETriggerZonePreset : uint8
{
    Custom,             // 自定义
    StageLoad,          // Stage 加载触发
    StageActivate,      // Stage 激活触发
    ActActivate,        // Act 激活触发
    ActDeactivate,      // Act 停用触发
    PropStateChange,    // Prop 状态变更触发
    ConditionalTrigger  // 条件触发
};
```

### FTriggerZoneDescription
```cpp
USTRUCT(BlueprintType)
struct FTriggerZoneDescription
{
    ETriggerZonePreset Preset;  // 预设模板
    FString Trigger;            // 谁/什么触发？(e.g., "Player")
    FString Condition;          // 何时触发？(可选条件)
    FString Target;             // 影响什么？(e.g., "Stage_BossRoom")
    FString Action;             // 执行什么？(e.g., "LoadStage()")
    FString Effect;             // 为什么？(e.g., "预加载Boss房资源")
};
```

### EDataLayerRuntimeState
```cpp
enum class EDataLayerRuntimeState : uint8
{
    Unloaded,   // 未加载
    Loaded,     // 已加载（在内存中）
    Activated   // 已激活（可见/交互）
};
```

---

## 使用示例

### 示例 1: 基本 Stage 状态控制（推荐方式）
```
// 在关卡蓝图中
Event BeginPlay
    → Get Stage Reference
    → Activate Stage   // 或 Load Stage / Unload Stage

// 触发器中切换状态
Event On Actor Begin Overlap
    → Get Stage Reference
    → Branch (Stage State == Active)
        True: Unload Stage
        False: Activate Stage
```

### 示例 2: 使用 GotoState API
```
// 更灵活的状态控制
Event Custom: ChangeStageState
    → Get Stage Reference
    → Goto State (TargetState: 从变量获取)

// 状态查询
Event Tick
    → Get Stage Reference
    → Get Stage State
    → Switch on Enum
        Unloaded: Show "Not Loaded" UI
        Loaded: Show "Ready" UI
        Active: Show "Active" UI
```

### 示例 3: Multi-Act 激活
```
// 同时激活多个 Act
Event Custom: SetupBattleScene
    → Get Stage Reference
    → Activate Act (ActID: 1)  // 基础场景
    → Activate Act (ActID: 3)  // 战斗配置
    → Lock Act (ActID: 1)      // 防止意外停用

// 检查 Act 状态
Event Custom: CheckActStatus
    → Get Stage Reference
    → Is Act Active (ActID: 3)
    → Branch
        True: "Battle mode active"
        False: "Normal mode"
```

### 示例 4: 使用 TriggerZone 预设行为（零代码）
```
// 在编辑器中配置 TriggerZoneActor:
// 1. 放置 TriggerZoneActor
// 2. 设置 Owner Stage = Stage_BossRoom
// 3. 设置 On Enter Action = Load Stage
// 4. 设置 On Exit Action = Unload Stage
// 完成！无需任何蓝图代码
```

### 示例 5: 使用 TriggerZone 蓝图事件（自定义逻辑）
```
// TriggerZone Actor 蓝图
Event On Actor Enter (Zone, Actor)
    → Cast to Character (Actor)
    → Branch (Has Item "BossKey")
        True:
            → Get Owner Stage
            → Activate Stage
            → Play Sound "DoorUnlock"
        False:
            → Display Message "Need Boss Key"
```

### 示例 6: 监听 Prop 状态变化
```
// 在带有 StagePropComponent 的 Actor 蓝图中
Event BeginPlay
    → Get Component by Class (StagePropComponent)
    → Bind Event to OnPropStateChanged

Custom Event: OnStateChanged (NewState, OldState)
    → Switch on Int (NewState)
        Case 0: Play "Close" Animation
        Case 1: Play "Open" Animation
        Case 2: Play "Locked" Animation
```

### 示例 7: 手动控制 DataLayer 预加载
```
// 预加载下一个 Act 的资源
Event Custom: PrepareNextAct
    → Get Stage Reference
    → Load Act DataLayer (ActID: NextActID)
    → (等待加载完成后)
    → Apply Act Prop States Only (ActID: NextActID)
```

### 示例 8: 简单开关门
```
// Door Actor 蓝图
// Components: Static Mesh, StagePropComponent

Event On Prop State Changed (NewState, OldState)
    → Branch (NewState == 1)
        True: Set Relative Rotation (0, 90, 0)  // 开门
        False: Set Relative Rotation (0, 0, 0)  // 关门

// 或使用 Toggle
Event On Interact
    → Get StagePropComponent
    → Toggle State (0, 1)
```

---

## 版本历史

| 版本 | 日期 | 变更 |
|------|------|------|
| 1.0 | 2024-11 | 初始 Blueprint API 文档 |
| 1.1 | 2025-11-27 | 添加 TriggerZone 系统 API |
| 1.2 | 2025-11-28 | 添加 Stage 状态控制 API (3态/5态)、Multi-Act 系统、TriggerZoneComponentBase、预设行为、FAct.InitialDataLayerState |
