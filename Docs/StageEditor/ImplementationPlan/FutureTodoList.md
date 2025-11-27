# StageEditor 未来功能计划

> 此文档维护 StageEditor 插件的未来功能需求列表，按优先级排序。

---

## 高优先级 (High Priority)

| ID | 功能名称 | 状态 | 描述 | 相关文档 |
|----|----------|------|------|----------|
| H-001 | 多 Act 同时激活架构 | ✅ 已完成 | 支持多个 Act 同时激活，PropState 后激活覆盖 | 本文档 |
| H-002 | Default Act ID 修复 | ✅ 已完成 | Default Act 的 ActID 改为 0 | - |
| H-003 | Default Act DataLayer 自动创建 | ✅ 已完成 | H-002 修复后，原有逻辑正常工作 | - |
| H-004 | Stage DataLayer 运行时状态管理 | ✅ 已完成 | Stage 状态机 + TriggerZone 系统 | 本文档 |
| H-005 | 跨 Stage 通信系统 | ✅ 已完成 | 通过 Subsystem 实现 Stage 间通信，含状态锁机制 | 本文档, StageManagerSubsystem.md |
| **R-001** | **Subsystem 重构** | ✅ **已完成** | StageEditorSubsystem → StageManagerSubsystem | 本文档 |

---

## 中优先级 (Medium Priority)

| ID | 功能名称 | 状态 | 描述 | 相关文档 |
|----|----------|------|------|----------|

---

## 低优先级 (Low Priority)

| ID | 功能名称 | 状态 | 描述 | 相关文档 |
|----|----------|------|------|----------|
| L-001 | Prop 状态可视化预览 | 🔴 待设计 | 视口中显示 Prop 在各 Act 中的状态 | - |

---

## 功能详情

### L-001: Prop 状态可视化预览

**需求描述：**
- **触发时机**：鼠标悬停在 Panel 中的 Prop 条目上
- **显示内容**：该 Prop 在各个 Act 中的不同状态值
- **可视化形式**：视口中 Prop Actor 上方显示文字标签
- **显示范围**：仅显示与默认状态不同的 Act（选项 C）
- **标签生命周期**：鼠标移开后延迟 0.5 秒淡出

**预期效果示例：**
```
┌─────────────────┐
│ Act 战斗: 2     │  ← 只显示与默认状态不同的
│ Act 结束: 0     │
└─────────────────┘
```

**依赖项：**
- 依赖 H-001（多 Act 激活架构）完成后，可能需要调整"当前激活状态高亮"的设计

**技术考量：**
- 使用 UE 的 Debug Draw 或 Widget Component 在视口渲染标签
- 需要与 SStageEditorPanel 的悬停事件集成
- 考虑性能影响（大量 Prop 时的渲染开销）

---

### H-001: 多 Act 同时激活架构

**需求描述：**
- 支持多个 Act 同时处于"激活"状态
- PropState 在多 Act 激活时采用**后激活覆盖**策略
- DataLayer 与 Act 激活状态完全同步（Activated 状态）

**已确认的设计决策：**

| 方面 | 决策 |
|------|------|
| 使用场景 | 状态混合式 - 同一 Prop 可被多个 Act 影响 |
| 冲突处理 | 后激活的 Act 覆盖先前的（激活顺序决定优先级） |
| DataLayer | Act Activated → DataLayer Activated（完全同步） |
| 激活顺序记录 | `TArray<int32> ActiveActIDs` - 数组尾部 = 最高优先级 |
| Deactivate 回退 | 保持当前状态，不自动回退 |
| Default Act 地位 | 与其他 Act 平等对待，无特殊地位 |
| 重复激活行为 | 移到尾部（提升优先级） |
| 状态查询策略 | 实时遍历（Act 变化可能频繁） |

---

#### 数据结构变更

```cpp
//================================================================
// Stage.h - Runtime State 区域
//================================================================

// [废弃] int32 CurrentActID = -1;

/** 当前激活的 Act ID 列表，按激活顺序排列（尾部优先级最高） */
UPROPERTY(Transient, BlueprintReadOnly, Category = "Stage|Runtime")
TArray<int32> ActiveActIDs;

/** 最近一次激活的 Act ID */
UPROPERTY(Transient, BlueprintReadOnly, Category = "Stage|Runtime")
int32 RecentActivatedActID = -1;
```

---

#### 委托声明

```cpp
/** Act 被激活时广播 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnActActivated, int32, ActID);

/** Act 被停用时广播 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnActDeactivated, int32, ActID);

/** 激活列表变化时广播（用于 UI 刷新） */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnActiveActsChanged, const TArray<int32>&, ActiveActIDs);
```

---

#### Act 激活控制 API

```cpp
/**
 * 激活指定 Act，添加到激活列表尾部（最高优先级）
 * 如果已激活，则移到尾部提升优先级
 * 同时激活对应的 DataLayer (Activated 状态)
 * 更新 RecentActivatedActID
 */
UFUNCTION(BlueprintCallable, Category = "Stage|Acts")
void ActivateAct(int32 ActID);

/**
 * 停用指定 Act，从激活列表移除
 * Prop 状态保持不变（不自动回退）
 * 同时停用对应的 DataLayer (Unloaded 状态)
 */
UFUNCTION(BlueprintCallable, Category = "Stage|Acts")
void DeactivateAct(int32 ActID);

/**
 * 按顺序激活多个 Act（数组尾部优先级最高）
 * 等效于依次调用 ActivateAct
 */
UFUNCTION(BlueprintCallable, Category = "Stage|Acts")
void ActivateActs(const TArray<int32>& ActIDs);

/**
 * 停用所有激活的 Act
 * Prop 状态保持不变
 */
UFUNCTION(BlueprintCallable, Category = "Stage|Acts")
void DeactivateAllActs();
```

---

#### Act 状态查询 API

```cpp
/**
 * 获取当前激活的 Act ID 列表（按激活顺序，尾部优先级最高）
 */
UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Stage|Acts")
TArray<int32> GetActiveActIDs() const;

/**
 * 检查指定 Act 是否处于激活状态
 */
UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Stage|Acts")
bool IsActActive(int32 ActID) const;

/**
 * 获取当前最高优先级的 Act ID
 * 返回 ActiveActIDs 的最后一个元素，如果无激活 Act 则返回 -1
 */
UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Stage|Acts")
int32 GetHighestPriorityActID() const;

/**
 * 获取最近一次激活的 Act ID
 */
UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Stage|Acts")
int32 GetRecentActivatedActID() const;

/**
 * 获取激活 Act 的数量
 */
UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Stage|Acts")
int32 GetActiveActCount() const;
```

---

#### Prop 有效状态查询 API

```cpp
/**
 * 获取 Prop 的最终有效状态
 * 从尾部（最高优先级）向头部遍历 ActiveActIDs
 * 返回第一个定义了该 Prop 状态的值
 * 如果没有任何激活 Act 定义该 Prop，返回 Prop 当前实际状态
 */
UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Stage|Props")
int32 GetEffectivePropState(int32 PropID) const;

/**
 * 获取指定 Prop 被哪个激活的 Act 控制（最高优先级的那个）
 * 返回 ActID，如果无 Act 控制则返回 -1
 */
UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Stage|Props")
int32 GetControllingActForProp(int32 PropID) const;
```

---

#### 核心实现逻辑

```cpp
void AStage::ActivateAct(int32 ActID)
{
    // 1. 验证 Act 存在
    if (!DoesActExist(ActID)) return;

    // 2. 如果已激活，先移除（后面会添加到尾部）
    ActiveActIDs.Remove(ActID);

    // 3. 添加到尾部（最高优先级）
    ActiveActIDs.Add(ActID);

    // 4. 更新 RecentActivatedActID
    RecentActivatedActID = ActID;

    // 5. 激活 DataLayer
    SetActDataLayerState(ActID, EDataLayerRuntimeState::Activated);

    // 6. 应用该 Act 的 PropState（覆盖之前的）
    ApplyActPropStates(ActID);

    // 7. 广播事件
    OnActActivated.Broadcast(ActID);
    OnActiveActsChanged.Broadcast(ActiveActIDs);
}

void AStage::DeactivateAct(int32 ActID)
{
    // 1. 检查是否在激活列表中
    if (!ActiveActIDs.Contains(ActID)) return;

    // 2. 从列表移除
    ActiveActIDs.Remove(ActID);

    // 3. 停用 DataLayer
    SetActDataLayerState(ActID, EDataLayerRuntimeState::Unloaded);

    // 4. Prop 状态保持不变（不回退）

    // 5. 广播事件
    OnActDeactivated.Broadcast(ActID);
    OnActiveActsChanged.Broadcast(ActiveActIDs);
}

int32 AStage::GetEffectivePropState(int32 PropID) const
{
    // 从尾部（最高优先级）向头部遍历
    for (int32 i = ActiveActIDs.Num() - 1; i >= 0; --i)
    {
        int32 ActID = ActiveActIDs[i];
        if (const FAct* Act = FindActByID(ActID))
        {
            if (const int32* State = Act->PropStateOverrides.Find(PropID))
            {
                return *State;
            }
        }
    }
    // 回退：返回 Prop 当前实际状态
    return GetPropStateByID(PropID);
}

int32 AStage::GetControllingActForProp(int32 PropID) const
{
    // 从尾部（最高优先级）向头部遍历
    for (int32 i = ActiveActIDs.Num() - 1; i >= 0; --i)
    {
        int32 ActID = ActiveActIDs[i];
        if (const FAct* Act = FindActByID(ActID))
        {
            if (Act->PropStateOverrides.Contains(PropID))
            {
                return ActID;
            }
        }
    }
    return -1;
}
```

**状态**：✅ 已实现（2025-11-26）

---

### H-002: Default Act ID 修复

**问题描述：**
- 当前 Default Act 的 ActID 是 1
- 应该改为 0，保持与文档和直觉一致

**受影响代码：**
- `Stage.cpp:13` - 构造函数中 `DefaultAct.SUID.ActID = 1`
- `Stage.cpp:192-193` - RegisterProp 中查找 Default Act
- 其他引用 ActID == 1 的地方

**注意事项：**
- 需要同步修改 `StageCoreTypes.h` 中的 `IsActLevel()` 检查逻辑（当前 `ActID > 0`）
- 考虑数据迁移：已有 Stage 的 Default Act ID 如何处理

---

### H-003: Default Act DataLayer 自动创建

**问题描述：**
- 创建 Stage 时会自动创建 `StageDataLayerAsset`
- 但 Default Act 没有自动创建对应的 `AssociatedDataLayer`

**期望行为：**
- 当 Stage DataLayer 创建时，同时为 Default Act 创建 DataLayer
- DataLayer 命名约定：`DL_Stage_{StageName}_Act_0` 或类似

**实现位置：**
- 可能在 `StageEditorController` 中处理（与 Stage DataLayer 创建逻辑相邻）

---

### H-004: Stage DataLayer 运行时状态管理

**状态**：🟠 实现中

**需求描述：**
- Stage 根 DataLayer 的运行时生命周期管理（加载-激活-卸载）
- 自动触发机制（玩家进入/离开区域）
- 外部系统（Subsystem）强制控制能力

**参考文档：**
- `Docs/StageEditor/Obslete/AStage_CoreLogic_V4.md.md` - V4.2 原始设计

---

#### 实现进度追踪

| 子任务 | 状态 | 描述 |
|--------|------|------|
| H-004.1 | ✅ 已完成 | EStageRuntimeState 5态枚举 |
| H-004.2 | ✅ 已完成 | 状态机核心函数 (GotoState, OnEnterState, OnExitState) |
| H-004.3 | ✅ 已完成 | 双 TriggerZone 组件（临时使用 UBoxComponent） |
| H-004.4 | ✅ 已完成 | Overlap 回调和状态转换逻辑 |
| H-004.5 | ✅ 已完成 | UStageTriggerZoneComponent 重构 |
| H-004.6 | ✅ 已完成 | 分层锁机制（Stage 锁 + Act 锁） |

---

#### 已完成功能

**1. EStageRuntimeState 5态枚举** ✅
```cpp
// StageCoreTypes.h
enum class EStageRuntimeState : uint8
{
    Unloaded,      // 完全卸载，DataLayer 未加载
    Preloading,    // 正在加载 DataLayer（过渡态）
    Loaded,        // DataLayer 已加载，但未激活（预加载缓冲区）
    Active,        // DataLayer 已激活，完全可交互
    Unloading      // 正在卸载 DataLayer（过渡态）
};
```

**2. 状态机核心函数** ✅
```cpp
// Stage.h/cpp
void GotoState(EStageRuntimeState NewState);
void OnEnterState(EStageRuntimeState State);
void OnExitState(EStageRuntimeState State);
void OnStageDataLayerLoaded();   // 异步回调
void OnStageDataLayerUnloaded(); // 异步回调
```

**3. 双 TriggerZone 组件** ✅（临时实现，待重构）
```cpp
// Stage.h - 当前使用 UBoxComponent
UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stage|Trigger")
TObjectPtr<UBoxComponent> LoadZoneComponent;

UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stage|Trigger")
TObjectPtr<UBoxComponent> ActivateZoneComponent;
```

**4. Overlap 回调** ✅
```cpp
void OnLoadZoneBeginOverlap(...);
void OnLoadZoneEndOverlap(...);
void OnActivateZoneBeginOverlap(...);
void OnActivateZoneEndOverlap(...);
bool ShouldTriggerForActor(AActor* Actor) const;
```

---

#### 待实现功能

**5. UStageTriggerZoneComponent 重构** 🔵

需要创建独立的 TriggerZone 组件类，替代当前直接使用 UBoxComponent 的临时实现。

详见下方 **UStageTriggerZoneComponent 设计** 章节。

**6. 分层锁机制** 🔴
```cpp
// ========== Stage 级别锁 ==========
UPROPERTY(Transient)
bool bIsStageStateLocked = false;

UPROPERTY(Transient)
EStageRuntimeState LockedStageState;

void ForceStageStateOverride(EStageRuntimeState NewState, bool bLockState);
void ReleaseStageStateOverride();

// ========== Act 级别锁 ==========
UPROPERTY(Transient)
TSet<int32> LockedActIDs;

void LockAct(int32 ActID);
void UnlockAct(int32 ActID);
bool IsActLocked(int32 ActID) const;
```

---

---

### UStageTriggerZoneComponent 设计（H-004.5 重构方案）

**状态**：✅ 已完成（2025-11-27）

**设计背景：**
- 当前 H-004.3/H-004.4 使用 UBoxComponent 临时实现了 TriggerZone
- 需要重构为独立的 Component 类以支持：
  - 外部 TriggerZone 配置（20% 场景）
  - 多 TriggerZone 合并引用计数
  - 通用流程控制（不仅限于 Stage 状态管理）

---

#### 核心设计

##### 1. EStageTriggerZoneType 枚举

```cpp
UENUM(BlueprintType)
enum class EStageTriggerZoneType : uint8
{
    LoadZone,      // 外层：触发 Preloading → Loaded
    ActivateZone,  // 内层：触发 Loaded → Active
};
```

##### 2. UStageTriggerZoneComponent 类

```cpp
UCLASS(ClassGroup=(StageEditor), meta=(BlueprintSpawnableComponent))
class STAGEEDITORRUNTIME_API UStageTriggerZoneComponent : public UBoxComponent
{
    GENERATED_BODY()

public:
    UStageTriggerZoneComponent();

    // === 配置 ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TriggerZone")
    EStageTriggerZoneType ZoneType = EStageTriggerZoneType::LoadZone;

    // === 运行时绑定（由 Stage 设置）===
    UPROPERTY(Transient, BlueprintReadOnly, Category = "TriggerZone")
    TObjectPtr<AStage> OwnerStage;

    // === 方法 ===
    void BindToStage(AStage* Stage);
    void UnbindFromStage();

    // === 过滤（可由 Stage 覆盖）===
    virtual bool ShouldTriggerForActor(AActor* Actor) const;

protected:
    // Overlap 事件直接转发给 Stage
    UFUNCTION()
    void OnZoneBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnZoneEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
};
```

##### 3. AStage TriggerZone 配置（重构后）

```cpp
// === 内置 TriggerZone（80% 场景使用）===
UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stage|Trigger")
TObjectPtr<UStageTriggerZoneComponent> BuiltInLoadZone;

UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stage|Trigger")
TObjectPtr<UStageTriggerZoneComponent> BuiltInActivateZone;

// === 外部 TriggerZone 引用（20% 场景，可选）===
UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stage|Trigger|Advanced")
TArray<TSoftObjectPtr<UStageTriggerZoneComponent>> ExternalTriggerZones;

// === 禁用内置选项（不推荐）===
UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stage|Trigger|Advanced",
    meta = (DisplayName = "Disable Built-in Zones (Not Recommended)"))
bool bDisableBuiltInZones = false;

// === 已注册的 Zone（运行时）===
UPROPERTY(Transient)
TArray<TObjectPtr<UStageTriggerZoneComponent>> RegisteredLoadZones;

UPROPERTY(Transient)
TArray<TObjectPtr<UStageTriggerZoneComponent>> RegisteredActivateZones;

// === 合并的 Actor 追踪（状态机用）===
UPROPERTY(Transient)
TSet<TObjectPtr<AActor>> OverlappingLoadZoneActors;

UPROPERTY(Transient)
TSet<TObjectPtr<AActor>> OverlappingActivateZoneActors;
```

##### 4. Stage 主导的初始化流程

```cpp
// Stage 发起注册，而非 Component 主动注册
UFUNCTION(BlueprintCallable, Category = "Stage")
void InitializeTriggerZones();
```

**流程：**
```
Stage::InitializeTriggerZones()
├── 1. 清空已注册列表
├── 2. 处理内置 Zone（如未禁用）
│   ├── BuiltInLoadZone->BindToStage(this)
│   └── BuiltInActivateZone->BindToStage(this)
├── 3. 处理外部 Zone
│   └── 遍历 ExternalTriggerZones，绑定有效的 Zone
├── 4. 验证配置
│   └── 无 LoadZone 时输出 Error 日志
└── 5. 完成状态机初始化
```

##### 5. Zone 事件处理流程

```cpp
// Component 调用 Stage 的处理方法
void AStage::HandleZoneBeginOverlap(UStageTriggerZoneComponent* Zone, AActor* OtherActor);
void AStage::HandleZoneEndOverlap(UStageTriggerZoneComponent* Zone, AActor* OtherActor);
```

**处理逻辑（合并引用计数）：**
1. Zone 的 Overlap 事件触发
2. Zone 调用 `OwnerStage->HandleZoneBeginOverlap(this, OtherActor)`
3. Stage 根据 `Zone->ZoneType` 更新对应的 Actor 集合
4. 当集合从 0→1 或 N→0 时触发状态转换

---

#### 设计决策说明

| 问题 | 决策 | 理由 |
|------|------|------|
| Component 所属模块 | StageEditorRuntime | 运行时需要，不依赖编辑器 |
| 注册流程发起者 | Stage 主导 | Stage 有 Initialize 方法，显式控制绑定时机 |
| 禁用内置 Zone | 允许但警告 | 灵活性 vs 安全性平衡，提示用户非推荐操作 |
| 多 Zone 合并逻辑 | 合并引用计数 | 任意 Zone 触发算进入，全部离开才算离开 |
| Actor 集合存储位置 | 仅在 Stage | Component 不存储，避免数据重复 |

---

#### 设计决策（已确定）

| 问题 | 决策 | 理由 |
|------|------|------|
| 状态数量 | ✅ **5态** | 为未来扩展保留空间 + 阻止频繁触发 |
| 触发方式 | ✅ **自动 + 手动都支持** | 覆盖无缝流送和手动控制两种场景 |
| 触发器位置 | ✅ **内置为主 + 可选外部覆盖** | 80/20原则，开箱即用且可扩展 |
| 离开 ActivateZone 行为 | ✅ **保持 Active** | 简单稳定，避免边界抖动，职责单一 |
| 状态锁粒度 | ✅ **分层锁（Stage + Act）** | 更细粒度控制 |
| DataLayer 激活策略 | ✅ **按需激活** | 资源高效，现有设计 |

---

#### 已确定的设计细节

##### 1. EStageRuntimeState 五态模型

```cpp
enum class EStageRuntimeState : uint8
{
    Unloaded,      // 完全卸载，DataLayer 未加载
    Preloading,    // 正在加载 DataLayer（过渡态，阻止重复触发）
    Loaded,        // DataLayer 已加载，但未激活（预加载缓冲区）
    Active,        // DataLayer 已激活，完全可交互
    Unloading      // 正在卸载 DataLayer（过渡态，阻止重复触发）
};
```

##### 2. 双 TriggerZone 模式

```
┌─────────────────────────────────────────────────────┐
│                LoadZone (外层/大)                    │
│                                                     │
│    ┌───────────────────────────────┐               │
│    │      ActivateZone (内层/小)    │               │
│    │                               │               │
│    │        Stage 核心区域         │               │
│    │                               │               │
│    └───────────────────────────────┘               │
│                                                     │
│                  预加载缓冲区                        │
└─────────────────────────────────────────────────────┘
```

**状态转换流程：**
- 进入 LoadZone → Unloaded → Preloading → Loaded
- 进入 ActivateZone → Loaded → Active
- 离开 ActivateZone（仍在 LoadZone）→ **保持 Active**
- 离开 LoadZone → Active/Loaded → Unloading → Unloaded

##### 3. TriggerZone 实现（内置 + 可选外部覆盖）

```cpp
// ========== 内置触发器（默认使用）==========
UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stage|Trigger")
TObjectPtr<UBoxComponent> LoadZoneComponent;

UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stage|Trigger")
TObjectPtr<UBoxComponent> ActivateZoneComponent;

// ========== 高级选项：外部触发器覆盖 ==========
UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stage|Trigger|Advanced")
bool bUseExternalLoadZone = false;

UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stage|Trigger|Advanced",
    meta = (EditCondition = "bUseExternalLoadZone"))
TSoftObjectPtr<AVolume> ExternalLoadZone;

UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stage|Trigger|Advanced")
bool bUseExternalActivateZone = false;

UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stage|Trigger|Advanced",
    meta = (EditCondition = "bUseExternalActivateZone"))
TSoftObjectPtr<AVolume> ExternalActivateZone;
```

**外部覆盖的处理要点：**
- BeginPlay 时检查 `bUseExternal*` 标志
- 外部 Volume 有效时，绑定其 Overlap 事件
- 外部 Volume 无效/被删除时，回退到内置 Component
- 需要处理外部 Volume 的生命周期（弱引用 + 有效性检查）

##### 4. 分层锁机制（供 Subsystem 使用）

```cpp
// ========== Stage 级别锁 ==========
UPROPERTY(Transient)
bool bIsStageStateLocked = false;

UPROPERTY(Transient)
EStageRuntimeState LockedStageState;

// 强制设置 Stage 状态并可选锁定
void ForceStageStateOverride(EStageRuntimeState NewState, bool bLockState);

// 释放 Stage 状态锁
void ReleaseStageStateOverride();

// ========== Act 级别锁 ==========
UPROPERTY(Transient)
TSet<int32> LockedActIDs;  // 被锁定的 Act 不能被 Deactivate

// 锁定特定 Act（不能被 Deactivate）
void LockAct(int32 ActID);

// 解锁特定 Act
void UnlockAct(int32 ActID);

// 检查 Act 是否被锁定
bool IsActLocked(int32 ActID) const;
```

**锁的行为：**
- Stage 锁：锁定后 TriggerZone 自动触发被忽略，只响应 Subsystem 指令
- Act 锁：锁定后该 Act 不能被 `DeactivateAct()` 停用

##### 5. Default Act 自动激活

```cpp
void AStage::BeginPlay()
{
    Super::BeginPlay();

    // 自动激活 Default Act (ID = 1)
    if (DoesActExist(1) && !IsActActive(1))
    {
        ActivateAct(1);
    }
}
```

---

#### 讨论记录

**2025-11-27 讨论 1：状态数量**
- 确认 5 态设计（添加 Loaded 状态）
- Preloading/Unloading 用于阻止频繁触发
- Loaded 状态作为预加载缓冲区

**2025-11-27 讨论 2：离开 ActivateZone 行为**
- 决定：保持 Active，直到离开 LoadZone
- 理由：简单稳定，避免边界抖动，职责单一

**2025-11-27 讨论 3：TriggerZone 实现**
- 决定：内置 BoxComponent 为主 + 可选外部 Volume 覆盖
- 80/20 原则，但 20% 的外部覆盖也必须完善处理

**2025-11-27 讨论 4：状态锁粒度**
- 决定：分层锁（Stage 级别 + Act 级别独立锁）
- Stage 锁：控制 Stage 的 Load/Unload/Activate 状态
- Act 锁：可以锁定特定 Act 不能被 Deactivate

**2025-11-27 讨论 5：DataLayer 激活策略**
- 决定：按需激活（只激活 ActiveActIDs 中的 Act）
- Default Act 应该默认在 ActiveActIDs 中

**2025-11-27 发现问题：Default Act 未自动激活** 🐛
- 现状：构造函数创建 Default Act，但未添加到 ActiveActIDs
- 修复：在 BeginPlay 中自动激活 Default Act (ID=1)
- 优先级：高（影响基础功能）
- 状态：✅ 已修复

**2025-11-27 讨论 6：UStageTriggerZoneComponent 设计**
- 决定：创建独立的 Component 类继承 UBoxComponent
- 支持 ZoneType 枚举（LoadZone/ActivateZone）
- 设计为可复用的通用流程控制组件

**2025-11-27 讨论 7：TriggerZone 注册流程**
- 决定：Stage 主导注册（通过 InitializeTriggerZones 方法）
- 而非 Component 主动注册到 Stage
- 理由：显式控制，时序清晰

**2025-11-27 讨论 8：多 Zone 合并逻辑**
- 决定：合并引用计数方案
- 任意 LoadZone 进入 → Actor 加入 OverlappingLoadZoneActors
- 所有 LoadZone 离开 → Actor 从集合移除
- Actor 集合仅存储在 Stage，Component 不存储

**2025-11-27 讨论 9：禁用内置 Zone**
- 决定：允许禁用，但在 Details 面板显示警告提示
- 用户需要确保配置了外部 Zone，否则状态机不工作

---

### H-005: 跨 Stage 通信系统

**状态**：✅ 已完成（2025-11-27）

**需求描述：**
- 通过 `StageManagerSubsystem` 实现 Stage 之间的通信
- 所有跨 Stage 通信必须经过 Subsystem（唯一中介）
- 支持状态锁机制实现优先级控制

**架构设计：**

```
┌─────────────────────────────────────────────────────────────┐
│                     Subsystem (唯一中介)                     │
│                                                             │
│   Stage A ───请求───▶ Subsystem ───指令───▶ Stage B        │
│                           │                                 │
│                      状态锁判断                              │
│                           │                                 │
│                    ┌──────┴──────┐                          │
│                    ▼             ▼                          │
│               有锁：拒绝     无锁：执行                       │
└─────────────────────────────────────────────────────────────┘
```

**关键设计决策：**

| 方面 | 决策 |
|------|------|
| 通信路径 | Stage 间无直接通信，必须经过 Subsystem |
| 优先级模型 | 二级优先级：Subsystem(带锁) > Stage自身逻辑 |
| Stage Actor 生命周期 | 永久存在于 Persistent Level，作为区域锚点 |
| 状态锁作用 | 锁定后 Stage 只响应 Subsystem 指令，忽略自身触发逻辑 |

**二级状态模型：**

| 状态 | Subsystem 请求 | Stage 自身逻辑(玩家触发等) |
|------|---------------|---------------------------|
| 无锁 | ✅ 执行 | ✅ 响应 |
| 有锁 | ✅ 执行 | ❌ 忽略 |

**核心 API（参考 StageManagerSubsystem.md）：**

```cpp
// 强制激活 + 可选锁定
void ForceActivateStage(int32 StageID, bool bLockState);

// 强制卸载 + 可选锁定
void ForceUnloadStage(int32 StageID, bool bLockState);

// 释放覆写，恢复自动管理
void ReleaseStageOverride(int32 StageID);
```

**依赖项：**
- 依赖 H-004（Stage 内部区域流程控制）设计完成后，明确"Stage 自身逻辑"的具体行为

**相关文档：**
- `Docs/StageEditor/StageManagerSubsystem.md` - 详细 API 设计

---

### R-001: Subsystem 重构（最高优先级）

**状态**：✅ 已完成（2025-11-27）

**背景：**
- 当前代码中存在 `UStageEditorSubsystem`（继承 `UEditorSubsystem`）
- 文档中描述了 `StageManagerSubsystem` 的跨 Stage 通信设计
- 两者实际上是同一个概念，需要统一

**重构内容：**

| 项目 | 变更前 | 变更后 |
|------|--------|--------|
| 类名 | `UStageEditorSubsystem` | `UStageManagerSubsystem` |
| 父类 | `UEditorSubsystem` | `UWorldSubsystem` |
| 模块 | StageEditor (Editor) | StageEditorRuntime (Runtime) |
| 生命周期 | 仅编辑器 | 编辑器 + 运行时 |

**统一后的职责：**
```
UStageManagerSubsystem : UWorldSubsystem
├── Stage 注册/查询（编辑器 + 运行时）
├── ID 分配（编辑时完成，ID 存在 Stage Actor 上）
├── 跨 Stage 通信（运行时，H-005）
└── 状态锁机制（运行时，H-005）
```

**为什么合并更合理：**
1. 功能内聚 - 所有 Stage 管理功能集中一处
2. 数据一致 - 注册表只有一份
3. 运行时需要 - 跨 Stage 通信需要知道有哪些 Stage
4. 简化架构 - 一个类管所有事

**WorldPartition 环境说明：**
- 项目使用 WorldPartition，地图切换很少发生
- `UWorldSubsystem` 生命周期问题基本不用担心
- Stage 在 `BeginPlay` 时自动注册即可

**受影响的文件：**
- `StageEditor/Public/Subsystems/StageEditorSubsystem.h` → 移动到 Runtime 模块并重命名
- `StageEditor/Private/Subsystems/StageEditorSubsystem.cpp` → 同上
- `StageEditorController.cpp` - 更新 Subsystem 引用
- `Stage.cpp` - 更新 BeginPlay 中的注册逻辑
- `Docs/StageEditor/StageManagerSubsystem.md` - 更新文档

**实现步骤：**
1. 在 StageEditorRuntime 模块创建 `UStageManagerSubsystem`
2. 迁移现有功能（注册、ID 分配、查询）
3. 更新所有引用位置
4. 删除旧的 `UStageEditorSubsystem`
5. 编译验证

---

## 更新日志

| 日期 | 更新内容 |
|------|----------|
| 2025-11-26 | 创建文档，添加 L-001 Prop 状态可视化预览、H-001 多 Act 激活架构 |
| 2025-11-26 | 添加 H-002 Default Act ID 修复、H-003 Default Act DataLayer 自动创建 |
| 2025-11-26 | 更新 H-001 设计决策：激活顺序、冲突处理、DataLayer 同步策略 |
| 2025-11-26 | ✅ 完成 H-002、H-003 实现，编译通过 |
| 2025-11-26 | ✅ 完成 H-001 多 Act 激活架构实现，编译通过 |
| 2025-11-26 | 🧹 清理：移除废弃的 CurrentActID 和 GetCurrentActID() |
| 2025-11-27 | 添加 H-004 Stage 内部区域流程控制（待讨论）|
| 2025-11-27 | 添加 H-005 跨 Stage 通信系统设计 |
| 2025-11-27 | ✅ 完成 H-004.1~H-004.4 实现（5态枚举、状态机、双TriggerZone、Overlap回调）|
| 2025-11-27 | 🐛 修复 Default Act 未自动激活问题 |
| 2025-11-27 | 🔵 设计 UStageTriggerZoneComponent 重构方案（H-004.5）|
| 2025-11-27 | 📝 整理讨论记录：TriggerZone 设计决策 6-9 |
| 2025-11-27 | 🔴 添加 R-001 Subsystem 重构（StageEditorSubsystem → StageManagerSubsystem）|
| 2025-11-27 | 📝 决策：合并为 UWorldSubsystem，统一编辑器和运行时功能 |
| 2025-11-27 | ✅ 完成 R-001 Subsystem 重构，编译通过 |
| 2025-11-27 | ✅ 完成 H-004.5 UStageTriggerZoneComponent 重构，编译通过 |
| 2025-11-27 | ✅ 完成 H-004.6 分层锁机制（Stage 锁 + Act 锁），编译通过 |
| 2025-11-27 | ✅ 完成 H-005 跨 Stage 通信系统，编译通过 |
