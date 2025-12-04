# Phase 2: 状态检测与提示系统

> 日期: 2025-11-29 13:25-13:32
> 状态: ✅ 完成

---

## 1. 目标

实现 DataLayer 同步状态的检测和提示生成，用于显示：
- 未导入（蓝色）
- 已同步（绿色）
- 不同步（橙色）

---

## 2. 实施记录

### 2.1 创建的文件

| 文件 | 说明 |
|------|------|
| `Public/DataLayerSync/DataLayerSyncStatus.h` | 状态枚举、状态信息结构体、检测器类 |
| `Private/DataLayerSync/DataLayerSyncStatus.cpp` | 状态检测和提示生成实现 |

### 2.2 核心代码

**状态枚举**

```cpp
// DataLayerSyncStatus.h:14-25
UENUM(BlueprintType)
enum class EDataLayerSyncStatus : uint8
{
    NotImported,  // 未导入（蓝色）
    Synced,       // 已同步（绿色）
    OutOfSync     // 不同步（橙色）
};
```

**状态信息结构体**

```cpp
// DataLayerSyncStatus.h:32-77
USTRUCT(BlueprintType)
struct STAGEEDITOR_API FDataLayerSyncStatusInfo
{
    EDataLayerSyncStatus Status;
    TArray<FString> NewChildDataLayers;      // 新增的子 DataLayer
    TArray<FString> RemovedChildDataLayers;  // 删除的子 DataLayer
    int32 AddedActorCount = 0;               // 新增 Actor 数量
    int32 RemovedActorCount = 0;             // 移除 Actor 数量

    bool HasChanges() const;
    FString GetChangeSummary() const;
};
```

**状态检测逻辑**

```cpp
// DataLayerSyncStatus.cpp:15-78
FDataLayerSyncStatusInfo FDataLayerSyncStatusDetector::DetectStatus(const UDataLayerAsset* Asset)
{
    // 1. 获取元数据，无元数据则返回 NotImported
    // 2. 对比当前子 DataLayer 与快照，记录新增/删除
    // 3. 对比当前 Actor 与快照，统计变化数量
    // 4. 有变化则 OutOfSync，否则 Synced
}
```

**提示生成**

```cpp
// DataLayerSyncStatus.cpp:80-125
FText FDataLayerSyncStatusDetector::GenerateTooltip(const FDataLayerSyncStatusInfo& StatusInfo)
{
    // 根据状态和变化详情生成本地化提示文本
    // 例如："New child detected: DL_Act_Forest_Day"
}
```

**状态颜色**

```cpp
// DataLayerSyncStatus.cpp:127-143
FLinearColor FDataLayerSyncStatusDetector::GetStatusColor(EDataLayerSyncStatus Status)
{
    NotImported -> FLinearColor(0.3f, 0.5f, 0.8f)  // 蓝色
    Synced      -> FLinearColor(0.3f, 0.8f, 0.3f)  // 绿色
    OutOfSync   -> FLinearColor(0.9f, 0.6f, 0.2f)  // 橙色
}
```

### 2.3 辅助函数

| 函数 | 用途 |
|------|------|
| `GetCurrentChildDataLayerNames()` | 获取 DataLayer 当前的子层名称列表 |
| `GetCurrentActors()` | 获取 DataLayer 当前包含的 Actor 列表 |
| `GetStatusIconName()` | 获取状态对应的图标名称 |

### 2.4 编译验证

```
Result: Succeeded
Total execution time: 3.81 seconds
```

---

## 3. API 使用示例

```cpp
// 检测状态
FDataLayerSyncStatusInfo StatusInfo = FDataLayerSyncStatusDetector::DetectStatus(DataLayerAsset);

// 获取颜色用于 UI
FLinearColor Color = FDataLayerSyncStatusDetector::GetStatusColor(StatusInfo.Status);

// 生成提示文本
FText Tooltip = FDataLayerSyncStatusDetector::GenerateTooltip(StatusInfo);

// 判断是否需要同步
if (StatusInfo.Status == EDataLayerSyncStatus::OutOfSync)
{
    UE_LOG(LogTemp, Log, TEXT("Changes: %s"), *StatusInfo.GetChangeSummary());
}
```

---

*文档结束*
