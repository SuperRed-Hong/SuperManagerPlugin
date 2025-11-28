# TriggerZone Component 继承体系分析

## 概述

StageEditor 使用两层 TriggerZone 组件设计：

```
UBoxComponent
    └── UTriggerZoneComponentBase     (通用基类)
            └── UStageTriggerZoneComponent  (Stage专用派生类)
```

## 类职责对比

| 功能领域 | UTriggerZoneComponentBase (基类) | UStageTriggerZoneComponent (派生类) |
|---------|----------------------------------|-------------------------------------|
| **主要职责** | 提供通用TriggerZone功能，面向Blueprint用户 | 提供Stage状态管理专用功能 |
| **使用场景** | 独立TriggerZoneActor、用户自定义触发器 | Stage内置Zone、Stage外置Zone |
| **事件处理** | 通过委托暴露给Blueprint | 转发给Stage进行状态转换 + 预设动作 |

---

## 基类功能 (UTriggerZoneComponentBase)

### 1. Blueprint 事件委托
```cpp
UPROPERTY(BlueprintAssignable)
FOnTriggerZoneActorEnter OnActorEnter;  // Actor进入时广播

UPROPERTY(BlueprintAssignable)
FOnTriggerZoneActorExit OnActorExit;    // Actor离开时广播
```

**用途**: 用户在Blueprint中绑定这些事件，实现自定义触发逻辑。

### 2. Stage 绑定 (软引用)
```cpp
UPROPERTY(EditAnywhere)
TSoftObjectPtr<AStage> OwnerStage;  // 编辑器配置的Stage引用
```

**用途**: 在编辑器中配置关联的Stage，支持跨关卡引用。

### 3. Actor 过滤
```cpp
UPROPERTY(EditAnywhere)
TArray<FName> TriggerActorTags;  // 允许触发的Actor标签

UPROPERTY(EditAnywhere)
bool bMustBePawn = false;         // 必须是Pawn
```

**过滤逻辑**:
1. 如果 `TriggerActorTags` 为空 → 只允许 Pawn 触发
2. 如果设置了 Tags → 只允许带这些标签的 Actor
3. 如果同时设置 `bMustBePawn` → 必须是 Pawn 且带标签

### 4. Zone 启用/禁用
```cpp
UPROPERTY(EditAnywhere)
bool bZoneEnabled = true;

UFUNCTION(BlueprintCallable)
void SetZoneEnabled(bool bEnabled);
```

### 5. 描述文档系统
```cpp
UPROPERTY(EditAnywhere)
FTriggerZoneDescription Description;  // 用于调试/可视化的描述信息
```

### 6. 可扩展的虚函数
```cpp
protected:
    // 派生类可重写以添加自定义行为
    virtual void HandleActorEnter(AActor* Actor);
    virtual void HandleActorExit(AActor* Actor);
```

---

## 派生类扩展 (UStageTriggerZoneComponent)

### 1. Zone 类型
```cpp
UPROPERTY(EditAnywhere)
EStageTriggerZoneType ZoneType;  // LoadZone 或 ActivateZone
```

| 类型 | 颜色 | 职责 |
|-----|------|------|
| LoadZone | 黄色 | 外层区域，触发 Stage DataLayer 加载 |
| ActivateZone | 绿色 | 内层区域，触发 Stage 激活 |

### 2. 预设动作 (Stage专用)
```cpp
UPROPERTY(EditAnywhere)
ETriggerZoneDefaultAction OnEnterAction;  // 进入时执行的预设动作

UPROPERTY(EditAnywhere)
ETriggerZoneDefaultAction OnExitAction;   // 离开时执行的预设动作
```

可选值：
- `Custom` - 不执行预设，用户自行处理
- `LoadStage` - 调用 Stage->LoadStage()
- `ActivateStage` - 调用 Stage->ActivateStage()
- `UnloadStage` - 调用 Stage->UnloadStage()

**为什么放在派生类**: 这些预设动作都是针对 Stage 的操作，不属于通用基类的职责。

### 3. 直接 Stage 绑定 (运行时)
```cpp
private:
    TWeakObjectPtr<AStage> BoundStage;  // 运行时绑定的Stage (弱引用)

public:
    void BindToStage(AStage* Stage);    // 绑定到Stage
    void UnbindFromStage();             // 解绑
    AStage* GetBoundStage() const;      // 获取绑定的Stage
```

**与基类 OwnerStage 的区别**:
- 基类 `OwnerStage`: 软引用，编辑器配置
- 派生类 `BoundStage`: 弱引用，运行时设置，用于状态转发

### 4. 事件转发到 Stage
```cpp
virtual void HandleActorEnter(AActor* Actor) override
{
    // 转发到 Stage 处理状态转换
    if (AStage* Stage = BoundStage.Get())
    {
        Stage->HandleZoneBeginOverlap(this, Actor);
    }

    // 执行预设动作
    ExecutePresetAction(OnEnterAction);
}
```

---

## 事件处理流程

```
┌─────────────────────────────────────────────────────────────────────┐
│                     基类 OnZoneBeginOverlap                          │
├─────────────────────────────────────────────────────────────────────┤
│ 1. bZoneEnabled 检查                                                 │
│ 2. ShouldTriggerForActor() 过滤                                      │
│ 3. HandleActorEnter(Actor)  ←── 派生类重写                           │
│    ├── 转发给 Stage (状态转换)                                        │
│    └── ExecutePresetAction() (派生类执行预设动作)                      │
│ 4. OnActorEnter.Broadcast() ←── 广播Blueprint事件                    │
└─────────────────────────────────────────────────────────────────────┘
```

**派生类重写 `HandleActorEnter`**:
1. 转发给 Stage 处理状态转换
2. 执行预设动作 (LoadStage/ActivateStage/UnloadStage)

---

## 使用场景对比

### 场景 1: Stage 内置 Zone (80% 用例)

```cpp
// Stage 构造函数中创建
BuiltInLoadZone = CreateDefaultSubobject<UStageTriggerZoneComponent>(TEXT("BuiltInLoadZone"));
BuiltInLoadZone->ZoneType = EStageTriggerZoneType::LoadZone;

// BeginPlay 时绑定
BuiltInLoadZone->BindToStage(this);
```

**特点**:
- ZoneType 不可编辑 (内置锁定)
- 自动转发给 Stage 处理状态
- 可设置 OnEnterAction/OnExitAction 预设动作
- 同时继承基类的委托功能 (用户可扩展)

### 场景 2: 独立 TriggerZoneActor (使用基类)

```cpp
// TriggerZoneActor 使用基类组件
TriggerZone = CreateDefaultSubobject<UTriggerZoneComponentBase>(TEXT("TriggerZone"));
TriggerZone->OwnerStage = TargetStage;
```

**特点**:
- 在编辑器配置 OwnerStage
- 只能通过 OnActorEnter 委托处理逻辑
- 适合完全自定义的触发逻辑

### 场景 3: 自定义 Blueprint 触发器 (使用基类)

在Blueprint中:
1. 添加 `UTriggerZoneComponentBase` 组件
2. 绑定 `OnActorEnter` 事件
3. 在事件中实现自定义逻辑 (播放特效、声音等)

---

## 关键设计决策

### 为什么派生类有两个 Stage 引用？

| 引用 | 类型 | 来源 | 用途 |
|-----|------|------|------|
| OwnerStage (基类) | TSoftObjectPtr | 编辑器配置 | 预设动作执行的后备 |
| BoundStage (派生类) | TWeakObjectPtr | 运行时绑定 | 状态转发 + 预设动作执行 |

**原因**:
1. **内置 Zone**: 不需要编辑器配置，运行时由 Stage 调用 `BindToStage()`
2. **外置 Zone**: 可以两者都用，或只用其一
3. **灵活性**: 预设动作优先使用 BoundStage，回退到 OwnerStage

### 为什么不合并成一个？

- 内置 Zone 的 Stage 引用是 Transient 的 (不序列化)
- 外置 Zone 需要 EditAnywhere 的软引用 (序列化)
- 两者生命周期和用途不同

### 为什么预设动作放在派生类？

- `LoadStage`、`ActivateStage`、`UnloadStage` 都是 Stage 专用操作
- 基类应该保持通用性，不应包含 Stage 相关逻辑
- 这样基类可以用于非 Stage 场景 (如通用触发器)

---

## 总结

| 方面 | 基类 | 派生类 |
|-----|------|--------|
| 继承 | UBoxComponent | UTriggerZoneComponentBase |
| 主要功能 | 通用触发器，Blueprint友好 | Stage状态管理专用 |
| 事件处理 | 仅委托 | 转发给Stage + 预设动作 + 继承委托 |
| Stage引用 | 软引用 (编辑器配置) | 弱引用 (运行时绑定) |
| 预设动作 | ❌ 无 | ✅ LoadStage/ActivateStage/UnloadStage |
| 扩展方式 | 绑定OnActorEnter委托 | 重写HandleActorEnter |
| 适用场景 | 独立触发器、自定义逻辑 | Stage内置/外置Zone |
