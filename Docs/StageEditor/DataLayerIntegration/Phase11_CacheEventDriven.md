# Phase 11: 缓存事件驱动优化

> 日期: 2025-12-01
> 状态: ✅ 完成
> 优先级: HIGH

---

## 1. 问题背景

### 1.1 原缓存机制的 CPU 消耗问题

`FDataLayerSyncStatusCache` 原先使用 **2秒自动过期 + 后台 Ticker 刷新** 机制：

```
每 2 秒所有缓存过期
        ↓
后台 Ticker 每帧检查队列
        ↓
每帧最多刷新 5 个 DataLayer
        ↓
如果有 50 个未导入的 DataLayer
        ↓
持续无限循环刷新（高 CPU 消耗）
```

**问题场景：**
- 项目有大量未导入的 DataLayers（常见于项目初期）
- 缓存 2 秒过期后重新计算，但结果还是 "NotImported"
- 下次过期后又重新计算...无限循环

### 1.2 PostLoad 注册后缓存不刷新

Phase 10.8 实现了 `PostLoad()` 自注册机制，但存在问题：

```
Stage Actor 通过 PostLoad 注册到 Subsystem
        ↓
DataLayerSyncStatusCache 不知道有新 Stage 注册
        ↓
缓存中仍是旧数据（NotImported）
        ↓
UI 显示错误的同步状态
```

---

## 2. 解决方案

### 2.1 移除自动过期，改为事件驱动

**设计原则：**
- 读取优先：UI 读取直接返回缓存，无需计算
- 延迟刷新：后台 Ticker 刷新，不阻塞 UI
- **事件驱动：监听变化事件，精准失效（无自动过期）**

**移除的代码：**
```cpp
// 删除
static constexpr double CacheValidDuration = 2.0;

// 删除
bool IsCacheExpired(double CacheTime) const
{
    return (FPlatformTime::Seconds() - CacheTime) > CacheValidDuration;
}
```

**GetCachedStatus() 逻辑简化：**
```cpp
FDataLayerSyncStatusInfo FDataLayerSyncStatusCache::GetCachedStatus(const UDataLayerAsset* Asset)
{
    if (FCachedSyncStatus* CachedStatus = Cache.Find(WeakAsset))
    {
        // 缓存有效则直接返回（不再检查过期时间）
        // 刷新由事件驱动：DataLayer 变化、Actor 添加/删除、Stage 注册/注销
        if (CachedStatus->bIsValid)
        {
            return CachedStatus->Info;
        }
        // 缓存已失效，加入刷新队列，但仍返回旧值
        RequestRefresh(Asset);
        return CachedStatus->Info;
    }
    // 无缓存，加入刷新队列，返回默认值
    RequestRefresh(Asset);
    return DefaultInfo;
}
```

### 2.2 Stage 注册/注销委托

**Runtime 模块 (StageManagerSubsystem)：**

```cpp
// StageManagerSubsystem.h
DECLARE_MULTICAST_DELEGATE_OneParam(FOnStageRegistered, AStage*);
FOnStageRegistered OnStageRegistered;

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnStageUnregistered, AStage*, int32);
FOnStageUnregistered OnStageUnregistered;
```

```cpp
// StageManagerSubsystem.cpp - RegisterStage()
if (ResultStageID > 0)
{
    OnStageRegistered.Broadcast(Stage);
}

// UnregisterStage()
if (StageRegistry.Remove(StageID) > 0)
{
    OnStageUnregistered.Broadcast(Stage, StageID);
}
```

**Editor 模块 (DataLayerSyncStatusCache)：**

```cpp
// BindEvents()
if (UStageManagerSubsystem* Subsystem = World->GetSubsystem<UStageManagerSubsystem>())
{
    StageRegisteredHandle = Subsystem->OnStageRegistered.AddRaw(
        this, &FDataLayerSyncStatusCache::OnStageRegistered);
    StageUnregisteredHandle = Subsystem->OnStageUnregistered.AddRaw(
        this, &FDataLayerSyncStatusCache::OnStageUnregistered);
}

// OnStageRegistered()
void FDataLayerSyncStatusCache::OnStageRegistered(AStage* Stage)
{
    if (Stage->StageDataLayerAsset)
    {
        InvalidateCache(Stage->StageDataLayerAsset);
    }
    for (const FAct& Act : Stage->Acts)
    {
        if (Act.AssociatedDataLayer)
        {
            InvalidateCache(Act.AssociatedDataLayer);
        }
    }
}

// OnStageUnregistered()
void FDataLayerSyncStatusCache::OnStageUnregistered(AStage* Stage, int32 StageID)
{
    if (Stage)
    {
        // 失效 Stage 及其 Acts 的 DataLayer 缓存
        if (Stage->StageDataLayerAsset)
        {
            InvalidateCache(Stage->StageDataLayerAsset);
        }
        for (const FAct& Act : Stage->Acts)
        {
            if (Act.AssociatedDataLayer)
            {
                InvalidateCache(Act.AssociatedDataLayer);
            }
        }
    }
    else
    {
        // Stage 已销毁，失效所有缓存
        InvalidateAll();
    }
}
```

### 2.3 Controller 操作触发失效

在 `StageEditorController` 的关键方法中添加 `InvalidateCache()` 调用：

| 方法 | 失效目标 |
|------|----------|
| `CreateNewAct()` | Stage DataLayer |
| `DeleteAct()` | Stage DataLayer |
| `CreateDataLayerForAct()` | Stage DataLayer + 新 Act DataLayer |
| `AssignDataLayerToAct()` | Stage DataLayer + Act DataLayer |

```cpp
// 示例：CreateNewAct()
void FStageEditorController::CreateNewAct(...)
{
    // ... 创建 Act 逻辑 ...

    // 失效缓存，触发 UI 刷新
    if (Stage->StageDataLayerAsset)
    {
        FDataLayerSyncStatusCache::Get().InvalidateCache(Stage->StageDataLayerAsset);
    }
}
```

### 2.4 Sync All 按钮强化

**功能增强：**
1. 始终可点击（移除 `CanSyncAll()` 条件判断）
2. 点击时执行全局缓存刷新

```cpp
// SStageDataLayerOutliner.cpp
FReply SStageDataLayerOutliner::OnSyncAllClicked()
{
    // 1. 失效所有缓存，触发后台刷新
    FDataLayerSyncStatusCache::Get().InvalidateAll();

    // 2. 执行同步
    FDataLayerBatchSyncResult Result = FDataLayerSynchronizer::SyncAllOutOfSync();

    // 3. 刷新 UI
    if (SceneOutliner.IsValid())
    {
        SceneOutliner->FullRefresh();
    }

    return FReply::Handled();
}

bool SStageDataLayerOutliner::CanSyncAll() const
{
    return true;  // 始终可点击，作为手动刷新入口
}
```

---

## 3. 缓存失效触发点总览

| 事件 | 触发方式 | 失效范围 |
|------|----------|----------|
| DataLayer 变化 | `OnDataLayerChanged` 委托 | 变化的 DataLayer + Parent |
| Actor 添加/删除 | `OnLevelActorAdded/Deleted` | Actor 关联的 DataLayers |
| **Stage 注册** | `OnStageRegistered` 委托 | Stage + 所有 Acts DataLayers |
| **Stage 注销** | `OnStageUnregistered` 委托 | Stage + 所有 Acts DataLayers |
| 创建 Act | Controller 调用 | Stage DataLayer |
| 删除 Act | Controller 调用 | Stage DataLayer |
| 创建/分配 DataLayer | Controller 调用 | Stage + Act DataLayer |
| **点击 Sync All** | 用户手动 | 全部缓存 |

---

## 4. 性能对比

| 指标 | 改进前 | 改进后 |
|------|--------|--------|
| 50 个未导入 DL 的 CPU 消耗 | 持续 100% 循环刷新 | 仅首次计算，后续 O(1) |
| UI 刷新响应 | 直接返回缓存 O(1) | 直接返回缓存 O(1) |
| 事件响应延迟 | 最多 2 秒等待过期 | 即时失效，下帧刷新 |
| 后台 Ticker 负载 | 每帧检查过期 + 刷新 | 仅处理失效队列 |

---

## 5. 代码变更清单

| 文件 | 变更类型 | 说明 |
|------|----------|------|
| `StageManagerSubsystem.h` | 修改 | 添加 `OnStageRegistered`, `OnStageUnregistered` 委托 |
| `StageManagerSubsystem.cpp` | 修改 | 在 `RegisterStage()`, `UnregisterStage()` 中广播委托 |
| `DataLayerSyncStatusCache.h` | 修改 | 移除过期常量，添加 Stage 委托句柄和回调声明 |
| `DataLayerSyncStatusCache.cpp` | 修改 | 移除过期检查，添加 Stage 委托绑定和回调实现 |
| `StageEditorController.cpp` | 修改 | 在关键方法中添加 `InvalidateCache()` 调用 |
| `SStageDataLayerOutliner.cpp` | 修改 | 强化 Sync All 按钮，添加全局缓存刷新 |

---

## 6. 测试验证

### 6.1 功能测试

- [x] Stage PostLoad 注册后，DataLayer 状态正确显示
- [x] Stage Unload 后，DataLayer 状态正确更新
- [x] 创建/删除 Act 后，状态正确刷新
- [x] 点击 Sync All 按钮，全部状态刷新
- [x] PIE 模式不触发缓存失效（编辑器数据隔离）

### 6.2 性能测试

- [x] 50+ 未导入 DataLayers 场景，CPU 无异常消耗
- [x] UI 响应流畅，无卡顿

---

## 7. 架构图

```
┌─────────────────────────────────────────────────────────────┐
│                    缓存失效触发源                             │
├─────────────────────────────────────────────────────────────┤
│                                                             │
│  ┌──────────────────┐  ┌──────────────────┐                │
│  │ StageManager     │  │ DataLayerEditor  │                │
│  │ Subsystem        │  │ Subsystem        │                │
│  ├──────────────────┤  ├──────────────────┤                │
│  │ OnStageRegistered│  │ OnDataLayerChanged                │
│  │ OnStageUnregister│  └────────┬─────────┘                │
│  └────────┬─────────┘           │                          │
│           │                     │                          │
│           ▼                     ▼                          │
│  ┌──────────────────────────────────────────┐              │
│  │        DataLayerSyncStatusCache          │              │
│  ├──────────────────────────────────────────┤              │
│  │ BindEvents():                            │              │
│  │   - OnStageRegistered → InvalidateCache  │              │
│  │   - OnStageUnregistered → InvalidateCache│              │
│  │   - OnDataLayerChanged → InvalidateCache │              │
│  │   - OnLevelActorAdded → InvalidateCache  │              │
│  │                                          │              │
│  │ 无自动过期，纯事件驱动                     │              │
│  └────────────────────┬─────────────────────┘              │
│                       │                                    │
│                       ▼                                    │
│  ┌──────────────────────────────────────────┐              │
│  │           后台 Ticker                     │              │
│  │  处理 PendingRefreshQueue                │              │
│  │  每帧最多刷新 MaxRefreshPerTick (5)      │              │
│  └──────────────────────────────────────────┘              │
│                                                             │
│  ┌──────────────────┐                                      │
│  │ StageEditor      │                                      │
│  │ Controller       │                                      │
│  ├──────────────────┤                                      │
│  │ CreateNewAct()   │──────► InvalidateCache()             │
│  │ DeleteAct()      │──────► InvalidateCache()             │
│  │ CreateDataLayer..│──────► InvalidateCache()             │
│  │ AssignDataLayer..│──────► InvalidateCache()             │
│  └──────────────────┘                                      │
│                                                             │
│  ┌──────────────────┐                                      │
│  │ Sync All 按钮    │──────► InvalidateAll()               │
│  │ (始终可点击)     │        (用户手动刷新入口)             │
│  └──────────────────┘                                      │
│                                                             │
└─────────────────────────────────────────────────────────────┘
```

---

*文档创建: 2025-12-01 01:30*
