# Phase 1-2 性能优化设计文档

> 创建日期: 2025-11-29
> 状态: ✅ 实施完成
> 最后更新: 2025-11-29 16:30

---

## 1. 问题分析

### 1.1 当前架构的性能瓶颈

```
┌─────────────────────────────────────────────────────────────────┐
│ DataLayerOutliner UI 刷新                                        │
├─────────────────────────────────────────────────────────────────┤
│  每行调用 DetectStatus()                                         │
│       ↓                                                         │
│  FindStageByDataLayer()     O(S×A) - 可接受                      │
│       ↓                                                         │
│  DetectStageLevelChanges()  O(D + A²) - 字符串匹配效率低          │
│       ↓                                                         │
│  DetectActLevelChanges()    O(W) - 遍历全部 Actor                │
└─────────────────────────────────────────────────────────────────┘

S = Stage 数量 (通常 < 100)
A = 每个 Stage 的 Act 数量 (通常 < 10)
D = DataLayerInstance 总数
W = World 中 Actor 总数 (可能数万个)
```

### 1.2 复杂度分析

| 方法 | 时间复杂度 | 评估 |
|------|-----------|------|
| `FindStageByDataLayer()` | O(S × A) | ✅ 可接受 |
| `IsDataLayerImported()` | O(S × A) | ✅ 可接受 |
| `FindActIDByDataLayer()` | O(A) | ✅ 可接受 |
| `DetectStageLevelChanges()` | O(D + A²) | ⚠️ 需改进 |
| `DetectActLevelChanges()` | O(W) | ❌ 严重问题 |

### 1.3 调用频率与影响

| 场景 | 调用频次 | 单次开销 | 总影响 |
|------|---------|---------|--------|
| UI 刷新（50行） | 高频 | 50 × O(W) | ❌ 严重 |
| Hover 提示 | 中频 | O(W) | ⚠️ 明显 |
| 按钮点击 | 低频 | O(S×A) | ✅ 可接受 |

**核心问题**: DataLayerOutliner 每次刷新，50 行就要调用 50 次 `DetectStatus()`，每次都遍历世界中所有 Actor。

---

## 2. 优化策略

### 2.1 分层优化架构

```
┌─────────────────────────────────────────────────────────────────┐
│                        UI Layer                                  │
│  SStageDataLayerBrowser                                         │
│       ↓ 请求状态                                                 │
├─────────────────────────────────────────────────────────────────┤
│                     Cache Layer (新增)                           │
│  FDataLayerSyncStatusCache                                       │
│  ├── TMap<DataLayerAsset*, FCachedStatus> Cache                 │
│  ├── GetCachedStatus() - O(1) 直接返回                          │
│  ├── InvalidateCache() - 标记失效                                │
│  └── 后台刷新机制                                                │
│       ↓ 缓存未命中时                                             │
├─────────────────────────────────────────────────────────────────┤
│                   Detection Layer (优化)                         │
│  FDataLayerSyncStatusDetector                                    │
│  ├── 批量检测代替逐个检测                                        │
│  ├── 指针比较代替字符串匹配                                      │
│  └── 增量更新代替全量遍历                                        │
│       ↓                                                         │
├─────────────────────────────────────────────────────────────────┤
│                    Data Layer (现有)                             │
│  UStageManagerSubsystem + AStage                                 │
└─────────────────────────────────────────────────────────────────┘
```

### 2.2 优化措施总结

| 优化点 | 措施 | 效果 |
|--------|------|------|
| 缓存层 | 添加 `FDataLayerSyncStatusCache` | UI 读取 O(1) |
| 字符串匹配 | 改为指针比较 | O(A²) → O(A) |
| Actor 遍历 | 批量检测 + 事件驱动 | 50×O(W) → O(W) |
| 刷新策略 | 后台异步刷新 | 不阻塞 UI |

---

## 3. 详细设计

### 3.1 缓存层设计

#### 3.1.1 类定义

```cpp
// DataLayerSyncStatusCache.h

/**
 * 缓存的同步状态
 */
struct FCachedSyncStatus
{
    FDataLayerSyncStatusInfo Info;
    double CacheTime = 0;       // 缓存时间戳
    bool bIsValid = false;      // 是否有效
};

/**
 * DataLayer 同步状态缓存管理器 (单例)
 *
 * 设计原则:
 * - 读取优先：UI 读取直接返回缓存，无需计算
 * - 延迟刷新：后台定时刷新，不阻塞 UI
 * - 事件驱动：监听变化事件，精准失效
 */
class STAGEEDITOR_API FDataLayerSyncStatusCache
{
public:
    static FDataLayerSyncStatusCache& Get();

    //----------------------------------------------------------------
    // 缓存读取 API - O(1)
    //----------------------------------------------------------------

    /** 获取缓存的状态（始终立即返回，不阻塞） */
    FDataLayerSyncStatusInfo GetCachedStatus(const UDataLayerAsset* Asset) const;

    /** 检查缓存是否存在且有效 */
    bool HasValidCache(const UDataLayerAsset* Asset) const;

    //----------------------------------------------------------------
    // 缓存管理 API
    //----------------------------------------------------------------

    /** 使单个条目失效 */
    void InvalidateCache(const UDataLayerAsset* Asset);

    /** 使所有缓存失效 */
    void InvalidateAll();

    /** 请求刷新（加入队列，后台处理） */
    void RequestRefresh(const UDataLayerAsset* Asset = nullptr);

    /** 强制同步刷新（阻塞，用于按钮点击） */
    FDataLayerSyncStatusInfo ForceRefresh(const UDataLayerAsset* Asset);

    //----------------------------------------------------------------
    // 生命周期
    //----------------------------------------------------------------

    void Initialize();
    void Shutdown();

private:
    FDataLayerSyncStatusCache() = default;

    /** 缓存存储 */
    TMap<TWeakObjectPtr<const UDataLayerAsset>, FCachedSyncStatus> Cache;

    /** 缓存有效期（秒） */
    static constexpr double CacheValidDuration = 2.0;

    /** 后台刷新定时器 */
    FTSTicker::FDelegateHandle TickerHandle;

    /** 待刷新队列 */
    TArray<TWeakObjectPtr<const UDataLayerAsset>> PendingRefreshQueue;

    /** 后台刷新 Tick */
    bool OnTick(float DeltaTime);

    /** 事件绑定句柄 */
    FDelegateHandle DataLayerChangedHandle;
    FDelegateHandle ActorAddedHandle;
    FDelegateHandle ActorDeletedHandle;

    /** 事件回调 */
    void OnDataLayerChanged(const UDataLayerInstance* Instance);
    void OnActorAddedToWorld(AActor* Actor);
    void OnActorRemovedFromWorld(AActor* Actor);
};
```

#### 3.1.2 缓存策略

```
┌─────────────────────────────────────────────────────────────────┐
│                     缓存失效触发条件                              │
├─────────────────────────────────────────────────────────────────┤
│ 1. 时间过期: CacheTime + 2秒 < CurrentTime                       │
│ 2. DataLayer 变化: OnDataLayerChanged() 事件                     │
│ 3. Actor 变化: OnActorAddedToWorld/RemovedFromWorld              │
│ 4. 手动失效: InvalidateCache() / InvalidateAll() 调用            │
└─────────────────────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────────────────────┐
│                     缓存刷新策略                                  │
├─────────────────────────────────────────────────────────────────┤
│ GetCachedStatus() 调用时:                                        │
│   ├── 有效缓存 → 直接返回 O(1)                                   │
│   ├── 无效缓存 → 返回旧值 + 加入刷新队列                          │
│   └── 无缓存   → 返回 NotImported + 加入刷新队列                  │
│                                                                 │
│ 后台 Tick (每帧):                                                │
│   └── 处理刷新队列，每帧最多处理 3 个                             │
│                                                                 │
│ ForceRefresh() 调用时:                                           │
│   └── 立即计算并更新缓存（用于按钮点击）                          │
└─────────────────────────────────────────────────────────────────┘
```

### 3.2 检测逻辑优化

#### 3.2.1 指针比较代替字符串匹配

```cpp
// 优化前: O(A²) 字符串匹配
for (const FString& Name : CurrentChildNames)
{
    for (const FString& RegisteredName : RegisteredActDataLayers)
    {
        if (RegisteredName.Contains(Name) || Name.Contains(RegisteredName))
        {
            // ...
        }
    }
}

// 优化后: O(A) 指针集合运算
TSet<UDataLayerAsset*> CurrentChildAssets;
TSet<UDataLayerAsset*> RegisteredActAssets;

// 新增 = 当前有但注册没有
for (UDataLayerAsset* Asset : CurrentChildAssets.Difference(RegisteredActAssets))
{
    OutInfo.NewChildDataLayers.Add(Asset->GetName());
}

// 删除 = 注册有但当前没有
for (UDataLayerAsset* Asset : RegisteredActAssets.Difference(CurrentChildAssets))
{
    OutInfo.RemovedChildDataLayers.Add(Asset->GetName());
}
```

#### 3.2.2 事件驱动精准失效

```cpp
void FDataLayerSyncStatusCache::OnActorAddedToWorld(AActor* Actor)
{
    if (!Actor || !Actor->HasDataLayers()) return;

    // 只失效相关的 DataLayer 缓存
    for (const UDataLayerInstance* Instance : Actor->GetDataLayerInstances())
    {
        if (UDataLayerAsset* Asset = Instance->GetAsset())
        {
            InvalidateCache(Asset);

            // 也失效父 DataLayer（Stage 级别）
            if (const UDataLayerInstance* Parent = Instance->GetParent())
            {
                InvalidateCache(Parent->GetAsset());
            }
        }
    }
}
```

---

## 4. 文件结构

```
Plugins/StageEditor/Source/StageEditor/
├── Public/DataLayerSync/
│   ├── DataLayerSyncStatus.h           # 现有 - 状态枚举和结构体
│   ├── DataLayerSyncStatusCache.h      # 新增 - 缓存管理器
│   └── ...
├── Private/DataLayerSync/
│   ├── DataLayerSyncStatus.cpp         # 优化 - 检测逻辑
│   ├── DataLayerSyncStatusCache.cpp    # 新增 - 缓存实现
│   └── ...
```

---

## 5. 优化效果预估

| 场景 | 优化前 | 优化后 | 提升 |
|------|--------|--------|------|
| UI 刷新 50 行 | 50 × O(W) | 50 × O(1) | **~99%** |
| Hover 提示 | O(W) | O(1) | **~99%** |
| 首次加载 | N × O(W) | 后台 O(W) | 不阻塞 UI |
| 内存占用 | 0 | ~10KB | 可接受 |

---

## 6. 实施计划

| 步骤 | 任务 | 优先级 | 状态 |
|------|------|--------|------|
| 1 | 创建 `FDataLayerSyncStatusCache` 类 | P0 | ✅ |
| 2 | 实现缓存读写和失效逻辑 | P0 | ✅ |
| 3 | 优化 `DetectStageLevelChanges` 为指针比较 | P0 | ✅ |
| 4 | 绑定事件实现精准失效 | P1 | ✅ |
| 5 | 添加后台刷新 Tick | P1 | ✅ |
| 6 | 集成到 Module 生命周期 | P0 | ✅ |
| 7 | 编译验证 | P0 | ✅ |

---

## 7. 实施摘要

**实施日期**: 2025-11-29

**新增文件**:
- `Public/DataLayerSync/DataLayerSyncStatusCache.h` - 缓存管理器头文件
- `Private/DataLayerSync/DataLayerSyncStatusCache.cpp` - 缓存管理器实现

**修改文件**:
- `Private/DataLayerSync/DataLayerSyncStatus.cpp` - 优化 `DetectStageLevelChanges()` 为指针比较
- `Private/StageEditorModule.cpp` - 在 Module 生命周期中初始化/关闭缓存

**核心改进**:
1. **缓存层**: `FDataLayerSyncStatusCache` 单例，UI 读取直接返回缓存 O(1)
2. **指针比较**: 使用 `TSet<UDataLayerAsset*>` 代替字符串匹配，O(A²) → O(A)
3. **事件驱动**: 监听 `OnDataLayerChanged`、`OnLevelActorAdded/Deleted` 精准失效
4. **后台刷新**: Ticker 每帧处理最多 3 个刷新请求，不阻塞 UI

---

*文档版本: v1.1*
*最后更新: 2025-11-29 16:30*
