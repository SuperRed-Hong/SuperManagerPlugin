# UE5 Level Instance 与 World Partition 技术指南

> 本文档介绍 UE5 原生类 LevelInstance、LevelBounds、PackedLevelActor 的概念与用法，以及 Level Instance 与 World Partition 组合使用的最佳实践。

---

## 目录

1. [核心类介绍](#核心类介绍)
   - [ALevelInstance](#1-alevelinstance-level-instance-actor)
   - [ALevelBounds](#2-alevelbounds-level-bounds-actor)
   - [APackedLevelActor](#3-apackedlevelactor-packed-level-actor)
2. [三者关系总结](#三者关系总结)
3. [Level Instance + World Partition 组合使用](#level-instance--world-partition-组合使用)
4. [关卡蓝图问题](#关卡蓝图问题level-instance-的关卡蓝图是否会执行)
5. [替代方案](#替代方案)
6. [最佳实践总结](#最佳实践总结)

---

## 核心类介绍

### 1. ALevelInstance (Level Instance Actor)

#### 概念

**Level Instance** 是 UE5 引入的一种**预制件 (Prefab) 工作流**，允许你将一个完整的关卡作为可复用的"模块"放置到世界中。

#### 核心用途

```
┌─────────────────────────────────────────────┐
│  Level Instance = "关卡级别的预制件"          │
├─────────────────────────────────────────────┤
│  • 将一组 Actor 打包成一个可复用单元           │
│  • 可在世界中多次放置同一个 Level Instance     │
│  • 支持"就地编辑" (Edit in Place)            │
│  • 修改源关卡后，所有实例自动更新              │
└─────────────────────────────────────────────┘
```

#### 使用场景

- **建筑模块化**：房间、楼层、建筑可重复使用
- **环境拼装**：岩石群、树木群、街道模块
- **游戏关卡设计**：可复用的关卡片段

#### 关键属性

```cpp
// AActor 上的相关属性
bIsMainWorldOnly  // 如果为 true，该 Actor 只在主世界加载，不会通过 Level Instance 加载
LevelInstanceType // 标识 Actor 属于哪种类型的 Level Instance
```

#### 工作流

1. 创建一个关卡，放入你想复用的 Actors
2. 在主世界中放置 `ALevelInstance` Actor
3. 设置其引用的关卡资产
4. 可以双击进入"Edit in Place"模式编辑

---

### 2. ALevelBounds (Level Bounds Actor)

#### 概念

**Level Bounds** 是一个**自动计算关卡边界**的 Actor，它会根据关卡内所有相关 Actor 的 Bounds 来确定整个关卡的包围盒。

#### 核心用途

```
┌─────────────────────────────────────────────┐
│  LevelBounds = 关卡的"边界框计算器"          │
├─────────────────────────────────────────────┤
│  • 自动收集所有 Actor 的 Bounds              │
│  • 计算关卡的整体包围盒 (Bounding Box)        │
│  • 用于流送、裁剪、编辑器可视化等             │
└─────────────────────────────────────────────┘
```

#### 关键机制

```cpp
// ALevelBounds 核心函数
void UpdateLevelBounds();           // 更新边界 - 遍历所有 Actor 计算总 Bounds
void OnLevelActorAddedRemoved();    // Actor 增删时触发
void OnLevelActorMoved();           // Actor 移动时触发
void BroadcastLevelBoundsUpdated(); // 广播边界更新事件

// 关键变量
bAutoUpdateBounds  // 是否自动更新边界
```

#### 哪些 Actor 参与边界计算？

```cpp
// AActor 上的属性
bRelevantForLevelBounds  // 如果为 true，该 Actor 的 Bounds 会被纳入关卡边界计算
```

#### 使用场景

- **World Partition**：用于确定 Cell 的空间范围
- **Level Streaming**：判断关卡是否在视野内
- **编辑器工具**：关卡可视化、快速定位

---

### 3. APackedLevelActor (Packed Level Actor)

#### 概念

**Packed Level Actor** 是 Level Instance 的一种**优化版本**，它将 Level Instance 中的所有内容"打包"成一个更高效的运行时表示。

#### 核心用途

```
┌─────────────────────────────────────────────────────┐
│  PackedLevelActor = "烘焙后的 Level Instance"        │
├─────────────────────────────────────────────────────┤
│  • 将 Level Instance 转换为单个 Actor               │
│  • 合并 Static Mesh、减少 Draw Call                 │
│  • 类似于 HLOD 的概念，但针对 Level Instance         │
│  • 运行时性能更好，但失去单独编辑能力                │
└─────────────────────────────────────────────────────┘
```

#### 工作原理

```
Level Instance (编辑时)          PackedLevelActor (运行时)
┌──────────────────┐             ┌──────────────────┐
│  Actor 1         │             │                  │
│  Actor 2         │  ──Pack──>  │  单个合并后的    │
│  Actor 3         │             │  Actor + Mesh    │
│  ...             │             │                  │
└──────────────────┘             └──────────────────┘
```

#### 相关工具类

```cpp
// FPackedLevelActorUtils - 打包工具
static bool CanPack();                    // 检查是否可以打包
static bool CreateOrUpdateBlueprint(...); // 创建/更新打包后的蓝图
static void PackAllLoadedActors();        // 打包所有已加载的 Actor
```

#### 使用场景

- **大世界优化**：减少远距离 Level Instance 的开销
- **结合 HLOD**：作为 HLOD 系统的一部分使用
- **发布优化**：将编辑时的模块化结构转换为运行时高效结构

---

## 三者关系总结

```
┌─────────────────────────────────────────────────────────────┐
│                    关系图                                    │
├─────────────────────────────────────────────────────────────┤
│                                                             │
│   LevelBounds ──────> 计算关卡边界                          │
│        │                  │                                 │
│        ▼                  ▼                                 │
│   ┌─────────┐      ┌─────────────┐      ┌────────────────┐ │
│   │ ULevel  │ ───> │LevelInstance│ ───> │PackedLevelActor│ │
│   │ (关卡)  │      │  (可复用)    │ Pack │   (优化后)     │ │
│   └─────────┘      └─────────────┘      └────────────────┘ │
│                                                             │
│   编辑阶段              模块化阶段           运行时优化阶段   │
└─────────────────────────────────────────────────────────────┘
```

| 类 | 主要职责 | 使用时机 |
|---|---|---|
| `ALevelBounds` | 计算关卡空间边界 | 自动创建，用于流送/裁剪 |
| `ALevelInstance` | 关卡级预制件复用 | 模块化世界构建 |
| `APackedLevelActor` | Level Instance 的烘焙优化 | 运行时性能优化 |

---

## Level Instance + World Partition 组合使用

### 官方态度：**支持，但有限制**

```
┌────────────────────────────────────────────────────────────┐
│  World Partition 关卡                                       │
│  ┌──────────────────────────────────────────────────────┐  │
│  │  Cell A          Cell B          Cell C              │  │
│  │  ┌─────────┐    ┌─────────┐    ┌─────────┐          │  │
│  │  │LevelInst│    │LevelInst│    │ Actors  │          │  │
│  │  │   #1    │    │   #2    │    │         │          │  │
│  │  └─────────┘    └─────────┘    └─────────┘          │  │
│  └──────────────────────────────────────────────────────┘  │
│                                                            │
│  ✅ 可以组合使用                                            │
│  ⚠️ 但有流送、蓝图等方面的限制                              │
└────────────────────────────────────────────────────────────┘
```

### 推荐场景

| 场景 | 推荐度 | 说明 |
|-----|-------|------|
| 可复用的建筑模块 | ✅ 推荐 | 房间、楼层等静态结构 |
| 环境装饰组合 | ✅ 推荐 | 岩石群、植被组合 |
| 带复杂逻辑的预制件 | ⚠️ 谨慎 | 关卡蓝图不会执行 |
| 需要独立流送的区域 | ❌ 不推荐 | 用 Data Layer 代替 |

---

## 关卡蓝图问题：Level Instance 的关卡蓝图是否会执行？

### 核心结论

```
┌─────────────────────────────────────────────────────────┐
│  ❌ Level Instance 的关卡蓝图 (Level Blueprint)          │
│     在被实例化时 **不会执行**                             │
└─────────────────────────────────────────────────────────┘
```

### 原因分析

```cpp
// Level Instance 的加载方式
┌─────────────────────────────────────────────────────────┐
│                                                         │
│  普通 Level Streaming:                                  │
│  ┌─────────────┐                                       │
│  │ ULevel      │ ──> ALevelScriptActor (关卡蓝图)      │
│  │             │     ✅ BeginPlay 会执行                │
│  └─────────────┘                                       │
│                                                         │
│  Level Instance:                                        │
│  ┌─────────────┐                                       │
│  │ LevelInst   │ ──> 只加载 Actor 数据                  │
│  │             │     ❌ 不实例化 ALevelScriptActor      │
│  └─────────────┘                                       │
│                                                         │
└─────────────────────────────────────────────────────────┘
```

Level Instance 的设计哲学是**只复用 Actor 布局数据**，而不是完整的关卡运行时行为。

### 技术细节

```cpp
// AActor 上的相关设置
bIsMainWorldOnly = true;
// 如果设为 true，该 Actor 只在主世界(Persistent Level)加载
// 不会通过 Level Instance 加载

// ALevelScriptActor 默认就是 bIsMainWorldOnly = true
// 所以关卡蓝图 Actor 不会被 Level Instance 实例化
```

---

## 替代方案

### 方案 1：使用 Actor 蓝图代替关卡蓝图（推荐）

```
┌─────────────────────────────────────────────────────────┐
│  推荐做法                                                │
├─────────────────────────────────────────────────────────┤
│                                                         │
│  Level Instance 关卡内:                                  │
│  ┌────────────────────────────────────────┐            │
│  │  Static Meshes (场景几何)               │            │
│  │  BP_RoomController (Actor 蓝图) ✅      │            │
│  │    - BeginPlay 逻辑                    │            │
│  │    - 交互逻辑                          │            │
│  │    - 事件响应                          │            │
│  └────────────────────────────────────────┘            │
│                                                         │
│  ✅ Actor 蓝图会正常实例化并执行                         │
└─────────────────────────────────────────────────────────┘
```

**示例结构：**
```
MyRoom_LevelInstance.umap
├── SM_Wall_01
├── SM_Floor_01
├── SM_Door_01
└── BP_RoomLogicController    <-- 把逻辑放这里
    ├── BeginPlay()
    ├── OnPlayerEnter()
    └── TriggerEvent()
```

### 方案 2：使用事件驱动架构

```cpp
// 在主世界监听 Level Instance 加载事件
// 然后从外部触发逻辑

// 主世界 GameMode 或 Subsystem 中
void OnLevelInstanceLoaded(ALevelInstance* LoadedInstance)
{
    // 查找实例内的特定 Actor 并初始化
    TArray<AActor*> ChildActors;
    LoadedInstance->GetLevelInstanceSubsystem()->GetLevelInstanceActors(ChildActors);

    for (AActor* Actor : ChildActors)
    {
        if (IInitializable* Initializable = Cast<IInitializable>(Actor))
        {
            Initializable->Initialize();
        }
    }
}
```

### 方案 3：如果必须要关卡蓝图逻辑

```
┌─────────────────────────────────────────────────────────┐
│  使用传统 Level Streaming 而非 Level Instance            │
├─────────────────────────────────────────────────────────┤
│                                                         │
│  World Partition + Data Layers + Streaming Levels       │
│                                                         │
│  • 关卡蓝图会正常执行                                    │
│  • 但失去 Level Instance 的"多实例复用"能力             │
│  • 适合独特的、不需要复用的关卡区域                      │
└─────────────────────────────────────────────────────────┘
```

---

## 最佳实践总结

```
┌─────────────────────────────────────────────────────────────┐
│  Level Instance + World Partition 最佳实践                  │
├─────────────────────────────────────────────────────────────┤
│                                                             │
│  ✅ DO:                                                     │
│  • 用于纯视觉/几何的可复用模块                               │
│  • 逻辑放在 Actor 蓝图中，不用关卡蓝图                       │
│  • 配合 Packed Level Actor 优化运行时性能                   │
│                                                             │
│  ❌ DON'T:                                                  │
│  • 不要依赖 Level Instance 的关卡蓝图                       │
│  • 不要在 Level Instance 中放置需要独立流送控制的内容        │
│  • 不要过度嵌套 Level Instance                              │
│                                                             │
│  架构建议:                                                  │
│  ┌─────────────────────────────────────────────────────┐   │
│  │  World Partition (主世界)                            │   │
│  │    ├── Data Layers (功能区域划分)                    │   │
│  │    ├── Level Instances (可复用视觉模块)              │   │
│  │    │     └── Actor Blueprints (模块内逻辑)           │   │
│  │    └── Streaming Levels (独特区域+关卡蓝图)          │   │
│  └─────────────────────────────────────────────────────┘   │
│                                                             │
└─────────────────────────────────────────────────────────────┘
```

---

## 参考资料

- [Unreal Engine Documentation - Level Instancing](https://dev.epicgames.com/documentation/en-us/unreal-engine/level-instancing-in-unreal-engine)
- [Unreal Engine Documentation - World Partition](https://dev.epicgames.com/documentation/en-us/unreal-engine/world-partition-in-unreal-engine)
- [Unreal Engine Documentation - Actor Editor Context](https://dev.epicgames.com/documentation/en-us/unreal-engine/actor-editor-context-in-unreal-engine)
- [UE5 API Reference - ALevelBounds](https://dev.epicgames.com/documentation/en-us/unreal-engine/API/Runtime/Engine/ALevelBounds)

---

*文档创建日期：2025-11-29*
*适用引擎版本：UE 5.x*
