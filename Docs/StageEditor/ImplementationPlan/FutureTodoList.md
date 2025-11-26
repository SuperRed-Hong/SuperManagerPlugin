# StageEditor 未来功能计划

> 此文档维护 StageEditor 插件的未来功能需求列表，按优先级排序。

---

## 高优先级 (High Priority)

| ID | 功能名称 | 状态 | 描述 | 相关文档 |
|----|----------|------|------|----------|
| H-001 | 多 Act 同时激活架构 | ✅ 已完成 | 支持多个 Act 同时激活，PropState 后激活覆盖 | 本文档 |
| H-002 | Default Act ID 修复 | ✅ 已完成 | Default Act 的 ActID 改为 0 | - |
| H-003 | Default Act DataLayer 自动创建 | ✅ 已完成 | H-002 修复后，原有逻辑正常工作 | - |

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

## 更新日志

| 日期 | 更新内容 |
|------|----------|
| 2025-11-26 | 创建文档，添加 L-001 Prop 状态可视化预览、H-001 多 Act 激活架构 |
| 2025-11-26 | 添加 H-002 Default Act ID 修复、H-003 Default Act DataLayer 自动创建 |
| 2025-11-26 | 更新 H-001 设计决策：激活顺序、冲突处理、DataLayer 同步策略 |
| 2025-11-26 | ✅ 完成 H-002、H-003 实现，编译通过 |
| 2025-11-26 | ✅ 完成 H-001 多 Act 激活架构实现，编译通过 |
| 2025-11-26 | 🧹 清理：移除废弃的 CurrentActID 和 GetCurrentActID() |
