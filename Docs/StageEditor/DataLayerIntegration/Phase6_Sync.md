# Phase 6: 同步逻辑

> 完成日期: 2025-11-29
> 状态: ✅ 完成

---

## 目标

实现 DataLayer 变化同步到 Stage-Act-Prop 架构的功能，包括单个同步和批量同步。

---

## 实现内容

### 1. 同步器类 (DataLayerSynchronizer)

**文件：**
- `Public/DataLayerSync/DataLayerSynchronizer.h`
- `Private/DataLayerSync/DataLayerSynchronizer.cpp`

**核心结构：**

```cpp
// 单个同步结果
USTRUCT(BlueprintType)
struct FDataLayerSyncResult
{
    bool bSuccess = false;
    FText ErrorMessage;
    int32 AddedActCount = 0;
    int32 RemovedActCount = 0;
    int32 AddedPropCount = 0;
    int32 RemovedPropCount = 0;
};

// 批量同步结果
USTRUCT(BlueprintType)
struct FDataLayerBatchSyncResult
{
    int32 SyncedCount = 0;
    int32 FailedCount = 0;
    int32 SkippedCount = 0;
    int32 TotalActChanges = 0;
    int32 TotalPropChanges = 0;
};
```

**API：**

```cpp
class FDataLayerSynchronizer
{
public:
    // 同步单个 DataLayer
    static FDataLayerSyncResult SyncDataLayer(UDataLayerAsset* DataLayerAsset, UWorld* World = nullptr);

    // 同步所有 OutOfSync 的 DataLayer
    static FDataLayerBatchSyncResult SyncAllOutOfSync(UWorld* World = nullptr);

    // 检查是否可同步
    static bool CanSync(UDataLayerAsset* DataLayerAsset, FText& OutErrorMessage);
};
```

### 2. 集成到 SStageDataLayerBrowser

**修改：**

```cpp
FReply SStageDataLayerBrowser::OnSyncAllClicked()
{
    FDataLayerBatchSyncResult Result = FDataLayerSynchronizer::SyncAllOutOfSync();
    // 日志输出结果
    return FReply::Handled();
}

bool SStageDataLayerBrowser::CanSyncAll() const
{
    // 遍历所有 DataLayer，检查是否有 OutOfSync 状态
    // 返回 true 时启用按钮
}
```

---

## 同步场景

### Stage 级别同步

**触发条件：** Stage DataLayer 的子 DataLayer 发生变化

**处理逻辑：**
1. 新增子 DataLayer → 创建对应 Act
2. 删除子 DataLayer → 删除对应 Act（保留默认 Act 0）

```cpp
FDataLayerSyncResult SyncStageLevelChanges(AStage* Stage, const FDataLayerSyncStatusInfo& StatusInfo, UWorld* World)
{
    // 处理新增的子 DataLayer
    for (const FString& NewChildName : StatusInfo.NewChildDataLayers)
    {
        CreateActFromDataLayer(Stage, ChildAsset, ActName, World);
        Result.AddedActCount++;
    }

    // 处理删除的子 DataLayer
    for (const FString& RemovedChildName : StatusInfo.RemovedChildDataLayers)
    {
        // 查找并删除对应 Act（跳过默认 Act 0）
        Result.RemovedActCount++;
    }
}
```

### Act 级别同步

**触发条件：** Act DataLayer 中的 Actor 成员发生变化

**处理逻辑：**
1. 新增 Actor → 注册为 Prop
2. 移除 Actor → 注销 Prop

```cpp
FDataLayerSyncResult SyncActLevelChanges(AStage* Stage, int32 ActID, UDataLayerAsset* ActDataLayer, ...)
{
    // 获取当前 DataLayer 中的 Actors
    TArray<AActor*> CurrentActors = GetActorsInDataLayer(ActDataLayer, World);

    // 注册新 Actors
    for (AActor* Actor : CurrentActors)
    {
        if (!IsRegistered(Actor))
        {
            Stage->RegisterProp(Actor);
            Result.AddedPropCount++;
        }
    }

    // 注销移除的 Actors
    for (int32 PropID : PropsToRemove)
    {
        Stage->UnregisterProp(PropID);
        Result.RemovedPropCount++;
    }
}
```

---

## 同步流程

```
1. 检测同步状态 (FDataLayerSyncStatusDetector::DetectStatus)
2. 判断同步类型：
   - Stage DataLayer → SyncStageLevelChanges
   - Act DataLayer → SyncActLevelChanges
3. 执行同步操作（带 FScopedTransaction）
4. 刷新缓存
5. 返回同步结果
```

---

## 技术要点

### 状态检测与同步的分离

- `FDataLayerSyncStatusDetector` - 只检测，不修改
- `FDataLayerSynchronizer` - 执行实际同步操作

### 默认 Act 保护

Act 0 是默认 Act，不能被删除：
```cpp
for (int32 i = Stage->Acts.Num() - 1; i > 0; --i)  // 从 1 开始，跳过 0
{
    Stage->Acts.RemoveAt(i);
}
```

### 缓存失效

同步后需要刷新缓存：
```cpp
FDataLayerSyncStatusCache::Get().InvalidateCache(DataLayerAsset);
```

---

*完成日期: 2025-11-29 17:15*
