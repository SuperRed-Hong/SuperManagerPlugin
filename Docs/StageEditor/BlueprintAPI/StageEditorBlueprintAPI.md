# StageEditor Blueprint API 文档

> 版本: Phase 1
> 更新日期: 2024-11

本文档详细说明 StageEditor 插件暴露给蓝图的所有 API。

---

## 目录

- [AStage (舞台Actor)](#astage-舞台actor)
  - [事件/委托](#事件委托)
  - [Act 控制 API](#act-控制-api)
  - [Act 查询 API](#act-查询-api)
  - [Prop 状态控制 API](#prop-状态控制-api)
  - [Prop 查询 API](#prop-查询-api)
  - [DataLayer 控制 API](#datalayer-控制-api)
- [UStagePropComponent (道具组件)](#ustagepropcomponent-道具组件)
  - [事件/委托](#事件委托-1)
  - [状态控制 API](#状态控制-api)
  - [Stage 交互 API](#stage-交互-api)
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
| `CurrentActID` | `int32` | 当前激活的 Act ID，-1 表示无激活 |
| `StageDataLayerAsset` | `UDataLayerAsset*` | Stage 的根 DataLayer 资产 |

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

### Act 控制 API

#### ActivateAct
```cpp
void ActivateAct(int32 ActID)
```
激活指定 Act，应用其 PropState 配置并激活关联的 DataLayer。

**参数:**
- `ActID` - 要激活的 Act ID

**行为:**
1. 将 Act 的 PropStateOverrides 应用到所有相关 Props
2. 卸载当前 DataLayer（如果不同）
3. 激活目标 Act 的 DataLayer
4. 更新 CurrentActID
5. 广播 OnActActivated 事件

---

#### DeactivateAct
```cpp
void DeactivateAct()
```
停用当前激活的 Act，卸载其 DataLayer。

**行为:**
1. 卸载当前 DataLayer
2. 重置 CurrentActID 为 -1
3. 广播 OnActDeactivated 事件

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

#### GetCurrentActID
```cpp
int32 GetCurrentActID() const
```
获取当前激活的 Act ID。

**返回值:**
- 当前 ActID，如果没有激活则返回 -1

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

**返回值:**
- Act 数量

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

**返回值:**
- 所有 PropID 的数组

---

#### GetAllPropActors
```cpp
TArray<AActor*> GetAllPropActors() const
```
获取所有已注册的 Prop Actor。

**返回值:**
- 所有 Prop Actor 的数组

---

#### GetPropCount
```cpp
int32 GetPropCount() const
```
获取已注册 Prop 总数。

**返回值:**
- Prop 数量

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
设置 Act 关联 DataLayer 的运行时状态。**不会**改变 CurrentActID 或应用 PropState。

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

#### GetStageID
```cpp
int32 GetStageID() const
```
获取 Stage 的唯一 ID（便捷方法，等同于 `SUID.StageID`）。

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

**返回值:**
- 当前 PropState

---

#### GetPreviousPropState
```cpp
int32 GetPreviousPropState() const
```
获取上一个状态值。用于实现过渡动画或条件逻辑。

**返回值:**
- 上一个 PropState

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
    UDataLayerAsset* AssociatedDataLayer;    // 关联的 DataLayer
    TMap<int32, int32> PropStateOverrides;   // PropID → State 映射
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

### 示例 1: 基本 Act 切换
```
// 在关卡蓝图中
Event BeginPlay
    → Get Stage Reference
    → Activate Act (ActID: 1)

// 触发器中
Event On Actor Begin Overlap
    → Get Stage Reference
    → Activate Act (ActID: 2)
```

### 示例 2: 监听 Prop 状态变化
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

### 示例 3: 手动控制 DataLayer 预加载
```
// 预加载下一个 Act 的资源
Event Custom: PrepareNextAct
    → Get Stage Reference
    → Load Act DataLayer (ActID: NextActID)
    → (等待加载完成后)
    → Apply Act Prop States Only (ActID: NextActID)
```

### 示例 4: 简单开关门
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
