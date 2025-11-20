# 【通用】“关卡编辑器”系统：AStage Actor数据结构与核心逻辑详细设计_V4.2

---

### **“关卡编辑器”系统：AStage Actor数据结构与核心逻辑详细设计_V4.2**

| **文档版本** | **作者** | **日期** | **备注** |
| --- | --- | --- | --- |
| 3.0 | 洪硕 | 2025-09-19 | 基于“事件驱动模型”共识编写 |
| 4.0 | 洪硕 | 2025-09-22 | **重大修订：全面集成`FStageHierarchicalId`系统，替换所有基于`FName`的ID引用。** |
| 4.1 | 洪硕 | 2025-09-22 | 此版本将FAct核心设计内容整合进此文档 |
| 4.2 | 洪硕 | 2025-09-22 | **`本次修正已根据最终共识，添加了ActivatedActs有序数组。`** |

### 关联文档：

[“动态舞台”系统概要设计文档 (High-Level Design)_V4.1](https://o082e0vcfd2.feishu.cn/wiki/FjM7wwfOeiHmU7kKaYiciJn8n0N)

[《中心化ID分配与本地索引系统详细设计》](https://o082e0vcfd2.feishu.cn/wiki/UJZhwHeyKiZvP7k8U8TcQSjGnec)

### 1. 引言

本文档为 `AStage` Actor 的运行时核心功能提供详尽的技术设计。`AStage` Actor 作为“关卡编辑器”系统的心脏，是一个自治的“区域导演”，负责管理其管辖范围内的资源加载，并响应游戏逻辑事件来**触发**场景状态的批量变更。

本设计严格遵循“**事件驱动**”的核心原则，实现一个高效、职责明确的`Actor`。设计将覆盖其C++类定义、用于管理资源加载的状态机、与触发器和Data Layer系统的交互，以及为上层游戏逻辑（策划蓝图）提供的指令式API。

**本次修订的核心是将系统的数据关联基础从`FName`全面迁移到新的`FStageHierarchicalId`复合结构体ID系统，以确保数据关联的绝对稳定和高效查询。**

### 2. AStage类数据结构定义

### 2.1 核心类型定义头文件 (`StageCoreTypes.h`)

为实现高内聚低耦合，系统的核心数据结构`FStageHierarchicalId`和`FAct`被定义在独立的`StageCoreTypes.h`头文件中。

### 核心类型: FAct

`FAct`是一个纯数据结构体，它定义了一个“场景状态的蓝图”。每个`FAct`代表一种特定的场景样貌（例如“和平时期”、“被攻击时”、“已摧毁”），并包含了驱动这种样貌所需的所有数据。
`FAct`

**设计说明:**

- **文件位置:** 根据V4.0设计，我们创建一个新的核心头文件`StageCoreTypes.h`。它将包含系统中所有通用的、基础的数据结构定义，包括新的ID结构体`FStageHierarchicalId`和更新后的`FAct`结构体。此文件将成为其他系统模块（如`AStage`）依赖的基础。
- **FStageHierarchicalId:** 这是一个由`StageID`, `ActID`, `PropID`三个`int32`组成的复合ID。它能唯一地标识系统中的每一个Stage、Act和Prop。关键在于，我们为其实现了`operator==`和`GetTypeHash`，这是让它能够作为`TMap`键值进行高效哈希查找的必要条件。
- **USTRUCT(BlueprintType):** 标记为`BlueprintType`使得策划可以在蓝图中创建、读取或修改`FAct`变量，增加了系统的灵活性。
- **优先级变更:** 根据`AStage Actor`3.0核心设计，`FAct`不再包含`Priority`成员。状态冲突的解决由`AStage` Actor的运行时逻辑负责，它将采用**“最新优先”**的策略。这意味着最后被激活并应用的`Act`将决定场景的基础状态，而更具体的、临时的状态变化将通过`PropStateOverrides`机制来处理。
- **FAct 成员变量:**
    - **`ActID`成员已从`FName`更新为`FStageHierarchicalId`，使其自身的标识符融入了新的ID体系。**
    - **DataLayer:** `TSoftObjectPtr<UDataLayerAsset>`，软对象指针用于关联一个数据层资源，实现了“按需加载”的核心需求。
    - **TargetPropStates:** `TMap<FName, int32>`，这是`FAct`的核心数据，定义了在此Act激活时，每个`Prop`（由其`FName`类型的ID标识）应该被设置成的目标`PropState`值。

```
// StageCoreTypes.h

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "StageCoreTypes.generated.h"

class UDataLayerAsset;

/**
 * 动态舞台系统的层级化复合ID。
 * 用于唯一、稳定地标识系统中的每一个Stage、Act和Prop。
 * 实现了`operator==`和`GetTypeHash`，可作为TMap的Key。
 */
USTRUCT(BlueprintType)
struct FStageHierarchicalId
{
    GENERATED_BODY()

    // 全局唯一的Stage ID，由中心化服务分配。
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ID")
    int32 StageID = 0;

    // Stage内唯一的Act ID，由AStage Actor本地分配。
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ID")
    int32 ActID = 0;

    // Stage内唯一的Prop ID，由AStage Actor本地分配。
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ID")
    int32 PropID = 0;

    // 为了能作为TMap的Key，需要重载等于操作符。
    bool operator==(const FStageHierarchicalId& Other) const
    {
        return StageID == Other.StageID && ActID == Other.ActID && PropID == Other.PropID;
    }

    // 默认构造函数
    FStageHierarchicalId() = default;
};

// 为TMap提供哈希函数。
inline uint32 GetTypeHash(const FStageHierarchicalId& ID)
{
    return HashCombine(HashCombine(GetTypeHash(ID.StageID), GetTypeHash(ID.ActID)), GetTypeHash(ID.PropID));
}

/**
 * 代表一个“幕”或“场景状态蓝图”的纯数据结构。
 * (已更新) 全面采用FStageHierarchicalId进行数据关联。
 */
USTRUCT(BlueprintType)
struct FAct : public FTableRowBase
{
    GENERATED_BODY()

    /**
     * Act的唯一层级ID。
     * 其StageID部分应与所属的AStage Actor的StageID一致。
     */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Act Definition")
    FStageHierarchicalId ActID;

    /**
     * 与此Act关联的数据层。当此Act激活时，该数据层将被加载。
     */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Act Definition")
    TSoftObjectPtr<UDataLayerAsset> DataLayer;

    /**
     * 定义了此Act下每个Prop的目标PropState。
     * - Key (FStageHierarchicalId): Prop的唯一层级ID。
     * - Value (int32): 该Prop在此Act激活时应该被设置的目标PropState值。
     */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Act Definition", meta = (DisplayName = "Target Prop States"))
    TMap<FStageHierarchicalId, int32> TargetPropStates;

    FAct() = default;
};

```

### 2.2 AStage类成员变量定义 (`AStage.h` - 修订后)

`AStage.h包含StageCoreTypes.h并定义AStage Actor自身的数据结构。**本次修正已根据最终共识，添加了ActivatedActs有序数组。**`

```
// AStage.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "StageCoreTypes.h" // 包含 FStageHierarchicalId 和 FAct 的定义
#include "AStage.generated.h"

// 前向声明
class AVolume;
class AProp;

/**
 * 运行时状态枚举，仅用于管理资源的加载与卸载生命周期。
 */
UENUM(BlueprintType)
enum class EStageRuntimeState : uint8
{
    Unloaded,      // 完全卸-载，DataLayer未激活
    Preloading,    // 正在请求加载DataLayer
    Active,        // 资源和逻辑都已就绪
    Unloading      // 正在请求卸载DataLayer
};

/**
 * “关卡编辑器”系统的核心Actor，扮演“区域导演”的角色。
 * 负责管理其管辖范围内的资源加载，并响应游戏逻辑事件来触发场景状态的批量变更。
 * 严格遵循“事件驱动”模型。
 */
UCLASS(Blueprintable)
class YOURPROJECT_API AStage : public AActor
{
    GENERATED_BODY()

public:
    AStage();

protected:
    //~ Begin AActor Interface
    virtual void BeginPlay() override;
    //~ End AActor Interface

    // -- 核心ID与注册表 --

    /** 由中心化ID服务分配的全局唯一Stage ID。*/
    UPROPERTY(VisibleAnywhere, Category = "Stage|ID System")
    int32 StageID;

    /** 用于在编辑器中分配新ActID的本地计数器。*/
    UPROPERTY(VisibleAnywhere, Category = "Stage|ID System")
    int32 NextAvailableActId = 1;

    /** 用于在编辑器中分配新PropID的本地计数器。*/
    UPROPERTY(VisibleAnywhere, Category = "Stage|ID System")
    int32 NextAvailablePropId = 1;

    // 在 AStage.h 的 protected 部分

    /**
     * @brief 道具注册表 (The Prop Registry)
     * 核心数据结构，存储了本Stage管理的所有Prop的ID与其Actor实例之间的稳定映射。
     */
    UPROPERTY(VisibleAnywhere, Category = "Stage|ID System", meta=(DisplayName="Prop Registry", AllowPrivateAccess = "true"))
    TMap<FStageHierarchicalId, TSoftObjectPtr<AProp>> PropRegistry;

    // -- 配置属性 --

    /** 负责触发本Stage资源加载/卸载生命周期的触发区。*/
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stage|Configuration")
    TArray<TSoftObjectPtr<AVolume>> TriggerZones;

    /** 本Stage可上演的所有剧本(Act)的定义列表。*/
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stage|Configuration", meta=(TitleProperty="ActID"))
    TArray<FAct> Acts;

    // -- 运行时状态 --

    /** 当前的资源加载状态。*/
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stage|Runtime", Transient)
    EStageRuntimeState CurrentState = EStageRuntimeState::Unloaded;

    /**
     * 【有序列表】记录了所有已激活Act的历史顺序。
     * 主要用于调试和辅助策划进行逻辑判断。此列表不直接参与PropState的计算。
     */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stage|Runtime", Transient)
    TArray<FStageHierarchicalId> ActivatedActs;

    /**
     * 最近被设置的Act ID，是基础状态变化的唯一驱动者。
     */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stage|Runtime", Transient)
    FStageHierarchicalId RecentActID;

    /**
     * 策划通过指令覆写的Prop状态，拥有最高优先级。
     */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stage|Runtime", Transient)
    TMap<FStageHierarchicalId, int32> PropStateOverrides;

private:
    /** 运行时追踪进入触发区的Actor。*/
    UPROPERTY(Transient)
    TSet<TObjectPtr<AActor>> OverlappingActors;

    // -- 状态机核心函数 --
    void GotoState(EStageRuntimeState NewState);
    void OnEnterState(EStageRuntimeState State);
    void OnExitState(EStageRuntimeState State);

    // -- 事件处理 --
    UFUNCTION()
    void OnTriggerZoneBeginOverlap(AActor* OverlappedActor, AActor* OtherActor);
    UFUNCTION()
    void OnTriggerZoneEndOverlap(AActor* OverlappedActor, AActor* OtherActor);

    // -- 内部逻辑 --
    void UpdateAllDataLayersForState(bool bShouldBeActive);
    const FAct* FindActByID(const FStageHierarchicalId& ActID) const;
    void ApplyPropStatesFromAct(const FAct& Act);

    // -- 指令式API支持 --
    bool bIsStateLocked = false;
    EStageRuntimeState LockedState;

public:
    // -- 蓝图API --

    /** 告知系统一个Act已进入激活状态，并将其设为最新的基础状态驱动者。*/
    UFUNCTION(BlueprintCallable, Category="Dynamic Stage|Actions")
    void ActivateAct(FStageHierarchicalId ActID);

    /** 告知系统一个Act已离开激活状态（注意：不会自动回退状态）。*/
    UFUNCTION(BlueprintCallable, Category="Dynamic Stage|Actions")
    void DeactivateAct(FStageHierarchicalId ActID);

    /** 触发一次基础层的PropState批量变更。*/
    UFUNCTION(BlueprintCallable, Category="Dynamic Stage|Actions")
    void SetRecentActID(FStageHierarchicalId ActID);

    /** 触发一次覆写层的PropState变更，拥有最高优先级。*/
    UFUNCTION(BlueprintCallable, Category="Dynamic Stage|Actions")
    void OverridePropState(FStageHierarchicalId PropID, int32 NewState);

    /** 清除一个Prop的覆写状态，使其恢复到由RecentActID控制的基础状态。*/
    UFUNCTION(BlueprintCallable, Category="Dynamic Stage|Actions")
    void ClearPropStateOverride(FStageHierarchicalId PropID);

    // -- 公共API (供StageManagerSubsystem调用) --

    /** 从外部系统强制控制Stage的资源加载状态。*/
    void ForceStateOverride(EStageRuntimeState NewState, bool bLockState);

    /** 解除资源加载状态的锁定。*/
    void ReleaseStateOverride();
};

```

### 3. 状态机设计

本系统的状态机**仅负责管理资源的加载/卸载生命周期**，不参与`PropState`的逻辑计算。

### 3.1 状态定义 (EStageRuntimeState)

该C++枚举已在头文件中定义，简化了原有的状态，专注于资源层面：

- `Unloaded`: `Stage`处于休眠状态，其关联的`Data Layer`未激活。
- `Preloading`: 正在请求加载`Data Layer`，这是一个过渡状态。
- `Preloaded`: `Data Layer`已加载，但`Stage`可能还未完全激活。
- `Active`: `Stage`完全激活，资源和逻辑都已就绪。
- `Unloading`: 正在请求卸载`Data Layer`，过渡状态。

### 3.2 状态机核心函数

- `void GotoState(EStageRuntimeState NewState)`
    - **职责**: 安全地将状态机从当前状态转换到新状态。
    - **逻辑**:
        - 检查`bIsStateLocked`标志。如果为`true`且`NewState`与`LockedState`不匹配，则阻止状态转换。
        - 如果`NewState`与`CurrentState`相同，则直接返回。
        - 调用`OnExitState(CurrentState)`执行清理逻辑。
        - 更新`CurrentState = NewState`。
        - 调用`OnEnterState(NewState)`执行进入新状态的初始化逻辑。
- `void OnEnterState(EStageRuntimeState State)`
    - **职责**: 处理进入每个状态时的核心逻辑。
    - **逻辑 (Switch-Case)**:
        - `Preloading`: 调用`UpdateAllDataLayersForState(true)`。一旦`Data Layer`加载完成（通过回调或轮询确认），自动调用`GotoState(EStageRuntimeState::Active)`。
        - `Active`: `Stage`进入可交互状态。
        - `Unloading`: 调用`UpdateAllDataLayersForState(false)`。一旦`Data Layer`卸载完成，自动调用`GotoState(EStageRuntimeState::Unloaded)`。
        - `Unloaded`, `Preloaded`: 保持静默。
- `void OnExitState(EStageRuntimeState State)`
    - **职责**: 预留用于未来的清理工作。

### 4. 事件处理

`Stage`的资源生命周期由与`AVolume`触发器的交互自动管理。

### 4.1 BeginPlay中的事件绑定

在`BeginPlay`中，将执行以下操作：

1. 遍历`TriggerZones`数组。
2. 对每个有效的`AVolume`，将其`OnActorBeginOverlap`和`OnActorEndOverlap`委托动态绑定到`AStage`的`OnTriggerZoneBeginOverlap`和`OnTriggerZoneEndOverlap`函数。

### 4.2 回调函数实现

- `OnTriggerZoneBeginOverlap(..., AActor* OtherActor)`
    - 过滤`OtherActor`，确保是目标类型（如玩家Pawn）。
    - 将`OtherActor`添加到`OverlappingActors`集合中。
    - 如果`OverlappingActors`的数量从0变为1，并且当前状态为`Unloaded`，则调用`GotoState(EStageRuntimeState::Preloading)`。
- `OnTriggerZoneEndOverlap(..., AActor* OtherActor)`
    - 从`OverlappingActors`集合中移除`OtherActor`。
    - 如果集合数量变为0，并且当前状态为`Active`，则调用`GotoState(EStageRuntimeState::Unloading)`。

### 5. 与Data Layer系统交互

- **实现函数**: `void UpdateAllDataLayersForState(bool bShouldBeActive)`
- **调用时机**:
    - 在进入`Preloading`状态时调用`UpdateAllDataLayersForState(true)`。
    - 在进入`Unloading`状态时调用`UpdateAllDataLayersForState(false)`。
- **具体逻辑**:
    - 获取`UDataLayerManager`实例。
    - 遍历本`Stage`配置的`Acts`数组中的所有`FAct`。
    - 收集所有不重复的`UDataLayerAsset`。
    - 根据`bShouldBeActive`参数，将这些`Data Layer`的状态统一设置为`EDataLayerState::Activated`或`EDataLayerState::Unloaded`。

### 6. 指令式API接口

### 6.1 蓝图API (供策划使用)

- **`void ActivateAct(FStageHierarchicalId ActID)`**:
    - **职责:** 告知系统一个`Act`已进入激活状态，并可选地将其设为最新的基础状态驱动者。
    - **逻辑:**
    a. 将`ActID`添加到`ActivatedActs`**有序数组**的末尾，用于记录激活历史。
    b. **自动调用**`SetRecentActID(ActID)`，触发该`Act`的状态应用。
- **`void DeactivateAct(FStageHierarchicalId ActID)`**:
    - **职责:** 告知系统一个`Act`已离开激活状态。
    - **逻辑:**
    a. 从`ActivatedActs`数组中移除`ActID`。
    b. **注意：** 此函数**不会**自动触发任何状态回退。如需恢复到上一个状态，策划需要自己负责调用`SetRecentActID`。
- **`void SetRecentActID(FStageHierarchicalId ActID)`**:
    - **职责:** 触发一次**基础层**的`PropState`批量变更。
    - **逻辑:**
    a. 更新`this->RecentActID = ActID;`。
    b. 查找`ActID`对应的`FAct`数据。
    c. 如果找到，则调用内部函数`ApplyPropStatesFromAct(FoundAct)`。
- **`void OverridePropState(FStageHierarchicalId PropID, int32 NewState)`**:
    - **职责:** 触发一次**覆写层**的`PropState`变更，拥有最高优先级。
    - **逻辑:**
    a. 更新`PropStateOverrides.FindOrAdd(PropID) = NewState;`。
    b. 通过`PropRegistry`找到`AProp`实例并**立即**更新其`PropState`。
- **`void ClearPropStateOverride(FStageHierarchicalId PropID)`**:
    - **职责:** 清除一个`Prop`的覆写状态，使其恢复到由`RecentActID`控制的基础状态。
    - **逻辑:**
    a. 从`PropStateOverrides`中移除`PropID`。
    b. 查找当前`RecentActID`对应的`FAct`，并从中找到`PropID`应有的基础状态值，然后应用它。

### 7.(新增) 不做清单 (V1.0 - MVP)

为明确系统边界，以下内容将不会在本版本中实现：

1. **不做状态的持续控制:** 系统只在事件触发时修改`PropState`一次。如果`Prop`的`PropState`后续因自身逻辑或其他系统而改变，本系统不会干预，也不会尝试将其“恢复”到`Act`定义的状态。
2. **不做`Act`卸载时的自动状态回退:** 当策划停用一个`Act`时（例如从`ActivatedActs`中移除），系统不会自动将场景恢复到上一个`Act`的状态。状态的回退必须由策划通过显式调用`SetRecentActID`来完成。
3. **不做复杂的`Act`优先级计算:** 状态的优先级只有两层：`PropStateOverrides`（最高）和`RecentActID`（基础）。系统不会去计算多个已激活`Act`之间的组合或优先级关系。