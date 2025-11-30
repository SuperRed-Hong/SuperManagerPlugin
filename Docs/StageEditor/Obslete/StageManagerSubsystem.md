# 【运行时】“关卡编辑器”系统 StageManagerSubsystem 详细设计2.0

`AStage` Actor现在拥有一个唯一的、由中心化服务分配的 `int32 StageID`，这成为了它在全局范围内的“身份证”。`StageManagerSubsystem`作为`AStage`的全局管理器，其设计也必须与这个新的ID系统精准对齐。

---

# “关卡编辑器”系统: StageManagerSubsystem 详细设计 (修订版)

**文档版本:** 2.0

| **文档版本** | **作者** | **日期** | **备注** |
| --- | --- | --- | --- |
| 1.0 | 洪硕 | 2025-09-19 | 初次设计，没有集成ID系统 |
| 2.0 | 洪硕 | 2025-09-22 | **本文档根据V4.0核心逻辑设计进行了重大修订。所有基于`FName`的ID引用均已被替换为新的`FStageHierarchicalId`复合结构体ID系统。核心数据结构被整合到新的`StageCoreTypes.h`头文件中，以建立一个统一、稳定的类型定义中心。** |
|  |  |  |  |

**关联文档:**

- 《“关卡编辑器”系统需求说明书 (PRD)》V2.1
- 《“关卡编辑器”系统概要设计文档 (High-Level Design)》V4.1
- 《“关卡编辑器”系统: AStage Actor核心逻辑详细设计》V4.1

## 1. 引言

`UStageManagerSubsystem` 是一个 `UGameInstanceSubsystem`，在“关卡编辑器”系统的运行时架构中扮演着核心辅助与全局管理的角色。根据V4.1的最新设计，其职责被进一步明确和巩固：

1. **维护全局实例注册表:** 实时追踪关卡中所有激活的 `AStage` Actor 实例。它现在将使用 `AStage` Actor上新增的、全局唯一的 `int32 StageID` 作为查找的**唯一关键字**。
2. **提供指令式蓝图API:** 为上层游戏逻辑（如任务系统）提供一套稳定的、解耦的蓝图接口。这些接口允许外部系统通过 `StageID` 来强制控制特定 `AStage` 的资源加载状态，而无需持有其直接的对象引用。

本次修订的核心是确保本子系统的所有设计细节都与新的 `FStageHierarchicalId` 复合ID系统以及 `AStage` 的 `StageID` 属性完全兼容，保证系统的稳定性和数据查找的绝对可靠性。

## 2. 实例注册表设计

为了能从全局通过唯一的 `StageID` 快速、安全地定位到对应的`AStage`实例，注册表的设计如下。

### 2.1 数据结构

我们将使用 `TMap` 来存储所有已注册的 `AStage` 实例。键为 `AStage` 自身的 `StageID`，值为一个指向 `AStage` 实例的弱指针。

**C++ 定义:**

```
// 在 StageManagerSubsystem.h 的 private 部分
private:
    /**
     * @brief 存储当前运行时所有已注册AStage实例的注册表。32222
     * @key int32 - 对应于AStage Actor上配置的、全局唯一的StageID。
     * @value TWeakObjectPtr<AStage> - 指向AStage实例的弱指针，以确保生命周期安全。
     */
    TMap<int32, TWeakObjectPtr<AStage>> StageRegistry;

```

**设计决策说明:**

- **`int32 StageID`** 作为Key**: 根据V4.1设计，`AStage` Actor上的 `StageID` 属性是其在整个项目中的唯一标识。使用它作为`TMap`的Key，提供了最直接、高效的查找方式。
- **`TWeakObjectPtr`** 的必要性**: 这一选择的核心理由保持不变且至关重要。
    - **防止内存泄漏**: `UStageManagerSubsystem` 的生命周期与游戏实例一样长。如果使用强引用指针 (`AStage*` 或 `TObjectPtr`)，当包含`AStage`的关卡被卸载时，由于Subsystem中仍存在对`AStage`的强引用，它将无法被垃圾回收(GC)，从而导致内存泄漏。
    - **避免悬垂指针 (Dangling Pointers)**: 当`AStage`被销毁时，`TWeakObjectPtr` 会自动失效（变为`nullptr`）。在使用前通过 `.IsValid()` 检查，可以100%避免访问无效内存导致的程序崩溃。

### 2.2 注册与反注册逻辑

`AStage` Actor 将在其生命周期事件中，主动向 `StageManagerSubsystem` 注册和反注册。

- **注册 (**`RegisterStage`)**:
    - **调用时机**: 在 `AStage::BeginPlay()` 中。`AStage`会获取自身的 `StageID` 并将自己传递给Subsystem。
    - **实现细节**:
        
        ```
        // 在 StageManagerSubsystem.cpp 中
        void UStageManagerSubsystem::RegisterStage(AStage* StageToRegister)
        {
            if (!IsValid(StageToRegister))
            {
                return;
            }
        
            const int32 StageID = StageToRegister->GetStageID(); // AStage需要提供一个public的GetStageID()方法
            if (StageID <= 0) // StageID必须是有效的正整数
            {
                UE_LOG(LogDynamicStage, Error, TEXT("AStage '%s' attempted to register with an invalid StageID: %d. Registration failed."), *StageToRegister->GetName(), StageID);
                return;
            }
        
            if (StageRegistry.Contains(StageID))
            {
                UE_LOG(LogDynamicStage, Warning, TEXT("A Stage with ID %d is already registered. Overwriting with new instance '%s'."), StageID, *StageToRegister->GetName());
            }
        
            StageRegistry.Add(StageID, StageToRegister);
        }
        
        ```
        
- **反注册 (**`UnregisterStage`)**:
    - **调用时机**: 在 `AStage::EndPlay()` 中。
    - **实现细节**:
        
        ```
        // 在 StageManagerSubsystem.cpp 中
        void UStageManagerSubsystem::UnregisterStage(AStage* StageToUnregister)
        {
            if (!IsValid(StageToUnregister))
            {
                return;
            }
        
            const int32 StageID = StageToUnregister->GetStageID();
            if (StageID > 0)
            {
                // 为确保安全，只移除与传入实例完全匹配的条目
                if (const TWeakObjectPtr<AStage>* FoundStage = StageRegistry.Find(StageID))
                {
                    if (FoundStage->Get() == StageToUnregister)
                    {
                        StageRegistry.Remove(StageID);
                    }
                }
            }
        }
        
        ```
        

## 3. 蓝图API详细设计

以下API将暴露给蓝图，供策划或任务系统等上层逻辑调用。API的参数将统一使用 `StageID` 来定位目标 `AStage`。

### 3.1 ForceActivateStage

强制激活一个Stage，使其加载资源并进入`Active`状态，无论玩家是否在触发器内。

**UFUNCTION 宏及 C++ 函数签名:**

```
// 在 StageManagerSubsystem.h 的 public 部分
public:
    /**
     * @brief 通过StageID强制激活一个Stage，并可以选择锁定其状态。
     * @param StageID 要操作的Stage的全局唯一ID。
     * @param bLockState 如果为true，Stage的状态将被锁定为Active，不再响应玩家触发。
     */
    UFUNCTION(BlueprintCallable, Category = "Dynamic Stage|Manager", meta = (DisplayName = "Force Activate Stage by ID"))
    void ForceActivateStage(int32 StageID, bool bLockState);

```

**内部逻辑描述:**

1. 通过 `StageID` 在 `StageRegistry` 中查找对应的 `AStage` 实例。
2. 检查查找到的弱指针是否有效。若无效，记录警告日志并返回。
3. 获取 `AStage*` 实例，并调用其公共API `Stage->ForceStateOverride(EStageRuntimeState::Active, bLockState)`。
4. 根据 `bLockState` 的值，更新内部的状态锁映射表。

### 3.2 ForceUnloadStage

强制卸载一个Stage，使其释放资源并进入`Unloaded`状态。

**UFUNCTION 宏及 C++ 函数签名:**

```
// 在 StageManagerSubsystem.h 的 public 部分
public:
    /**
     * @brief 通过StageID强制卸载一个Stage，并可以选择锁定其状态。
     * @param StageID 要操作的Stage的全局唯一ID。
     * @param bLockState 如果为true，Stage的状态将被锁定为Unloaded，不再响应玩家触发。
     */
    UFUNCTION(BlueprintCallable, Category = "Dynamic Stage|Manager", meta = (DisplayName = "Force Unload Stage by ID"))
    void ForceUnloadStage(int32 StageID, bool bLockState);

```

**内部逻辑描述:**

1. 与 `ForceActivateStage` 逻辑一致，首先通过 `StageID` 查找并验证 `AStage` 实例。
2. 调用 `AStage` 实例的公共API `Stage->ForceStateOverride(EStageRuntimeState::Unloaded, bLockState)`。
3. 根据 `bLockState` 的值，更新内部的状态锁映射表。

### 3.3 ReleaseStageOverride

解除对一个Stage的状态强制覆写，使其行为恢复由`TriggerZone`的Overlap事件自动管理。

**UFUNCTION 宏及 C++ 函数签名:**

```
// 在 StageManagerSubsystem.h 的 public 部分
public:
    /**
     * @brief 通过StageID解除对一个Stage的状态强制覆写。
     * Stage将恢复其默认的、由Overlap事件驱动的资源加载行为。
     * @param StageID 要操作的Stage的全局唯一ID。
     */
    UFUNCTION(BlueprintCallable, Category = "Dynamic Stage|Manager", meta = (DisplayName = "Release Stage Override by ID"))
    void ReleaseStageOverride(int32 StageID);

```

**内部逻辑描述:**

1. 通过 `StageID` 查找并验证 `AStage` 实例。
2. 调用 `AStage` 实例的公共API `Stage->ReleaseStateOverride()`。
3. 从内部的状态锁映射表中移除该 `StageID` 对应的条目。

## 4. 状态锁内部实现

为了清晰地追踪由本Subsystem发起的强制状态，我们需要一个内部数据结构来记录这些被锁定的Stage。

### 4.1 数据结构

我们将继续使用一个 `TMap` 来存储被覆写的Stage及其被强制设定的目标状态。

**C++ 定义:**

```
// 在 StageManagerSubsystem.h 的 private 部分
private:
    /**
     * @brief 存储被本Subsystem强制覆写状态的Stage及其目标状态。
     * Key是Stage的唯一ID，Value是被强制设定的目标状态。
     */
    TMap<int32, EStageRuntimeState> OverriddenStageStates;

```

**设计说明:**

这个映射表是 `AStage` 内部 `bIsStateLocked` 状态在Subsystem层的一个镜像记录。它清晰地表明了某个`AStage`的锁定状态是由`StageManagerSubsystem`发起的。这对于调试和维护至关重要，可以快速定位状态控制的来源。

### 4.2 状态覆写逻辑

在 `ForceActivateStage` 和 `ForceUnloadStage` 函数中，当 `bLockState` 为 `true` 时，我们将更新此映射表。

**示例实现 (在 ForceActivateStage 中):**

```
// ... 查找到有效的 Stage 实例 ...
Stage->ForceStateOverride(EStageRuntimeState::Active, bLockState);

if (bLockState)
{
    OverriddenStageStates.Add(StageID, EStageRuntimeState::Active);
}
else
{
    // 如果没有锁定，则它只是触发一次，不算持续覆写，应从我们的追踪记录中移除
    OverriddenStageStates.Remove(StageID);
}

```

### 4.3 状态锁释放逻辑

在 `ReleaseStageOverride` 函数中，无论之前状态如何，都将从映射表中移除对应条目，表示Subsystem放弃控制。

**示例实现 (在 ReleaseStageOverride 中):**

```
// ... 查找到有效的 Stage 实例 ...
Stage->ReleaseStateOverride();

// 从我们的追踪记录中移除，表示已放弃强制控制
OverriddenStageStates.Remove(StageID);

```