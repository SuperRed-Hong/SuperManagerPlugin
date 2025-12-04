# RecentActivatedActID 语义讨论

> 状态：待讨论
> 创建日期：2025-11-26
> 相关代码：`Stage.h` - `RecentActivatedActID`

---

## 背景

在实现多 Act 同时激活架构（H-001）时，引入了 `RecentActivatedActID` 变量。现在需要讨论是否有必要维护这个独立变量，还是用 getter 方法即可。

---

## 当前实现

```cpp
// Stage.h
UPROPERTY(Transient, BlueprintReadOnly, Category = "Stage|Runtime")
int32 RecentActivatedActID = -1;

// Stage.cpp - ActivateAct()
RecentActivatedActID = ActID;  // 每次激活时更新
```

---

## 两种语义对比

### 方案 A：删除变量，只用 getter

使用现有的 `GetHighestPriorityActID()` 替代：

```cpp
int32 GetHighestPriorityActID() const
{
    return ActiveActIDs.Num() > 0 ? ActiveActIDs.Last() : -1;
}
```

**特点**：返回当前激活列表中优先级最高的 Act，如果没有激活的 Act 则返回 -1

### 方案 B：保留独立变量（当前实现）

**特点**：记录"历史上最后一次激活操作的目标 ActID"，即使该 Act 已被 Deactivate

---

## 行为差异示例

| 操作 | RecentActivatedActID (方案B) | GetHighestPriorityActID (方案A) |
|------|------------------------------|----------------------------------|
| `ActivateAct(1)` | 1 | 1 |
| `ActivateAct(2)` | 2 | 2 |
| `DeactivateAct(2)` | **2** (保留) | **1** (回退) |
| `DeactivateAllActs()` | **2** (保留) | **-1** (无激活) |

---

## 讨论要点

1. **是否需要在所有 Act 都 Deactivate 后，仍能查询"最后激活过的是哪个 Act"？**
   - 如果需要：保留 `RecentActivatedActID`
   - 如果不需要：删除，统一用 `GetHighestPriorityActID()`

2. **潜在使用场景**：
   - UI 显示"最近操作的 Act"
   - 调试/日志记录
   - 某些游戏逻辑可能需要知道"上一次激活的是什么"

3. **简化 vs 功能完整性**：
   - 删除可以减少维护的状态变量
   - 保留提供更完整的历史信息

---

## 决策记录

| 日期 | 决策 | 原因 |
|------|------|------|
| 2025-11-26 | 暂时保留 | 等待团队讨论后决定 |

---

## 相关 API

- `GetHighestPriorityActID()` - 获取当前最高优先级的激活 Act
- `GetActiveActIDs()` - 获取所有激活的 Act 列表
- `GetRecentActivatedActID()` - 获取最近一次激活的 Act（当前实现）
