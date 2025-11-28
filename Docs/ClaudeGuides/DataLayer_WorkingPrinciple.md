# DataLayer 工作原理：World Partition vs 传统 Level

## 概述

你的 StageEditor 插件使用了 UE5 的 **DataLayer 系统**。这个系统在两种不同的关卡架构下有不同的工作方式：

1. **World Partition 模式** (UE5 新架构)
2. **传统 Level Streaming 模式** (UE4 兼容模式)

---

## 关键区别

### 1. World Partition Level (推荐，UE5 原生)

**特点：**
- 世界被自动分割成网格（Cells）
- **DataLayer 是主要的流式加载控制机制**
- Actor 通过 DataLayer 分组和管理
- 运行时根据玩家位置和 DataLayer 状态自动加载/卸载

**你的代码行为：**
```cpp
// Runtime (Stage.cpp:62-84)
UDataLayerSubsystem* DataLayerSubsystem = GetWorld()->GetSubsystem<UDataLayerSubsystem>();
DataLayerSubsystem->SetDataLayerRuntimeState(Instance, EDataLayerRuntimeState::Activated);
```

- ✅ `SetDataLayerRuntimeState` **直接控制资源加载/卸载**
- ✅ Activated → 加载 DataLayer 中的所有 Actor 和资源
- ✅ Unloaded → 卸载 DataLayer 中的所有 Actor 和资源
- ✅ **效果立竿见影**：切换 Act 时，相关场景资源会实时加载/卸载

**编辑器预览：**
```cpp
// Editor (StageEditorController.cpp:205-226)
DataLayerSubsystem->SetDataLayerVisibility(Instance, bShouldBeActive);
```

- ✅ `SetDataLayerVisibility` **控制编辑器中的可见性**
- ✅ 不加载/卸载资源，只是显示/隐藏
- ✅ 快速预览不同 Act 的场景状态

---

### 2. 传统 Level Streaming (非 World Partition)

**特点：**
- 使用传统的 Level Streaming Volumes 或手动代码控制
- **DataLayer 是辅助的可见性管理工具**
- 资源加载主要由 Level Streaming 机制控制
- DataLayer 只控制已加载 Actor 的显示/隐藏

**你的代码行为：**

#### **Runtime 模式 (运行游戏时)：**
```cpp
// Stage.cpp:62-84
DataLayerSubsystem->SetDataLayerRuntimeState(Instance, EDataLayerRuntimeState::Activated);
```

**⚠️ 行为差异：**
- ❌ **不会触发资源加载/卸载**（因为没有 World Partition Cells）
- ✅ **只控制 Actor 的可见性和 Tick 状态**
- ✅ 适用于：场景中已存在的 Actor，通过 DataLayer 分组管理显示
- ❌ 不适用于：动态加载大量资源（需要配合 Level Streaming）

**实际效果：**
- 切换 Act 时：
  - ✅ 属于该 Act 的 DataLayer 中的 Actor **变为可见**
  - ✅ 其他 Act 的 DataLayer 中的 Actor **变为不可见**
  - ❌ **不会加载/卸载资源到内存**

#### **Editor 模式 (编辑器预览时)：**
```cpp
// StageEditorController.cpp:224
DataLayerSubsystem->SetDataLayerVisibility(Instance, bShouldBeActive);
```

**✅ 两种模式下行为一致：**
- 仅控制可见性
- 快速预览不同场景状态

---

## 你的 StageEditor 在两种模式下的适用场景

### ✅ World Partition Level (完全适用)

**推荐使用场景：**
- 大型开放世界
- 需要动态加载/卸载大量资源
- 多个 Act 之间资源差异很大（例如：白天场景 vs 夜晚场景）

**工作流程：**
1. 创建 World Partition Level
2. 为每个 Act 创建独立的 DataLayer
3. 将 Props 添加到对应的 DataLayer
4. 运行时调用 `ActivateAct(ActID)`
   - → `SetDataLayerRuntimeState(Activated)`
   - → **自动加载该 Act 的所有资源**

**优势：**
- ✅ 内存高效：只加载当前 Act 需要的资源
- ✅ 性能优秀：自动管理流式加载
- ✅ 艺术家友好：在编辑器中直观预览

---

### ⚠️ 传统 Level (部分适用)

**适用场景：**
- 小型场景（所有资源可以一次性加载到内存）
- 同一场景下的**可见性切换**（例如：显示/隐藏某些道具）
- 多个 Act 共享大部分资源，只切换少量 Actor 的显示状态

**工作流程：**
1. 创建传统 Level
2. 所有 Props 提前放置在场景中
3. 为每个 Act 创建 DataLayer
4. 将 Props 添加到对应的 DataLayer
5. 运行时调用 `ActivateAct(ActID)`
   - → `SetDataLayerRuntimeState(Activated)`
   - → **只控制可见性，不加载/卸载**

**限制：**
- ❌ 所有资源始终在内存中（无法动态加载/卸载）
- ❌ 不适合大型场景（内存占用高）
- ✅ 适合快速切换场景状态（例如：开门/关门、灯光开关）

**解决方案（如果需要动态加载）：**
需要配合 **Level Streaming** 手动控制：

```cpp
// 在 AStage::ActivateAct() 中添加：
if (!IsWorldPartitionActive())
{
    // Manually load/unload streaming levels
    ULevelStreaming* StreamingLevel = GetLevelStreamingByActID(ActID);
    if (StreamingLevel)
    {
        StreamingLevel->SetShouldBeLoaded(true);
        StreamingLevel->SetShouldBeVisible(true);
    }

    // Unload previous Act's level
    if (PreviousStreamingLevel)
    {
        PreviousStreamingLevel->SetShouldBeLoaded(false);
    }
}
```

---

## 当前代码的检测逻辑

你的代码已经包含了 World Partition 检测：

```cpp
// StageEditorController.h:102-103
bool IsWorldPartitionActive() const;

// StageEditorController.cpp:748-750
bool FStageEditorController::IsWorldPartitionActive() const
{
    if (UWorld* World = GEditor->GetEditorWorldContext().World())
    {
        return World->GetWorldPartition() != nullptr;
    }
    return false;
}
```

**建议增强：**
在编辑器中提示用户当前模式：

```cpp
void FStageEditorController::Initialize()
{
    BindEditorDelegates();
    FindStageInWorld();

    // 提示用户当前关卡模式
    if (IsWorldPartitionActive())
    {
        DebugHeader::ShowNotifyInfo(TEXT("World Partition detected: Full DataLayer support enabled"));
    }
    else
    {
        DebugHeader::ShowNotifyInfo(TEXT("Traditional Level: DataLayer will only control visibility (not streaming)"));
    }
}
```

---

## 实际测试建议

### 测试 1：传统 Level 模式
1. 创建新 Level（不启用 World Partition）
2. 在场景中放置多个 Static Mesh Actor
3. 创建 Stage，注册这些 Actor 为 Props
4. 创建两个 Act，分配不同的 Props
5. 运行游戏，切换 Act
   - **预期结果**：Props 显示/隐藏切换，但所有资源始终在内存中

### 测试 2：World Partition 模式
1. 创建新 World Partition Level
2. 重复上述步骤
3. 运行游戏，切换 Act
   - **预期结果**：Props 显示/隐藏切换 + 资源动态加载/卸载

---

## 推荐配置

### 对于小型项目/原型：
- ✅ 使用传统 Level
- ✅ DataLayer 仅用于可见性管理
- ✅ 简单快速，适合测试

### 对于生产环境/大型项目：
- ✅ 使用 World Partition Level
- ✅ DataLayer 用于完整的资源流式加载
- ✅ 内存高效，适合上线

---

## 参考文档

- **World Partition 官方文档**: [UE5 World Partition](https://docs.unrealengine.com/5.0/en-US/world-partition-in-unreal-engine/)
- **DataLayer 系统**: [Data Layers in UE5](https://docs.unrealengine.com/5.0/en-US/data-layers-in-unreal-engine/)
- **Level Streaming**: [Level Streaming Overview](https://docs.unrealengine.com/5.0/en-US/level-streaming-in-unreal-engine/)

---

## 总结

| 特性 | World Partition Level | 传统 Level |
|------|----------------------|-----------|
| DataLayer 控制资源加载 | ✅ 是 | ❌ 否（仅可见性） |
| 适合大型场景 | ✅ 是 | ❌ 否 |
| 内存效率 | ✅ 高（动态加载） | ⚠️ 低（全部加载） |
| 设置复杂度 | ⚠️ 中等 | ✅ 简单 |
| 你的代码支持 | ✅ 完全支持 | ⚠️ 部分支持（仅可见性） |

**核心结论：**
- 你的 StageEditor 在 **World Partition Level** 下功能完整
- 在**传统 Level** 下，DataLayer 只控制**可见性**，不控制**资源加载**
- 如需在传统 Level 下实现动态加载，需额外集成 **Level Streaming** 机制
