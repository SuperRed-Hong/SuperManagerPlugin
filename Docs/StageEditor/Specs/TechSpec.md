# DataLayer 导入功能 - 技术规格文档

> 创建日期: 2025-11-29
> 状态: ✅ Phase 11 完成
> 关联文档: [DevLog/Overview.md](../DevLog/Overview.md)

---

## 架构变更说明

> ⚠️ **重要**: 本文档第 1-2 节的元数据存储方案已废弃，改用反向查找方案。
> 详见: [DevLog/Architecture_ReverseLookup.md](DevLog/Architecture_ReverseLookup.md)

---

## 1. ~~元数据存储设计~~ (已废弃)

<details>
<summary>展开查看废弃方案（仅供参考）</summary>

### 1.1 为什么使用 UAssetUserData

| 方案 | 时间复杂度 | 说明 |
|------|-----------|------|
| **UAssetUserData（选用）** | O(1) | 直接从 DataLayerAsset 读取附加数据 |
| 遍历 StageManagerSubsystem | O(N×M) | N=Stage数量，M=每个Stage的Acts数量 |

**废弃原因**: `UDataLayerAsset` 继承自 `UDataAsset -> UObject`，不支持 `IInterface_AssetUserData`。

### 1.2 类定义

```cpp
// ❌ 废弃 - UDataLayerAsset 不支持 AssetUserData
UCLASS()
class STAGEEDITOR_API UStageEditorDataLayerUserData : public UAssetUserData
{
    // ...
};
```

### 1.3 元数据操作 API

```cpp
// ❌ 废弃
namespace StageEditorDataLayerUtils
{
    void AttachUserData(...);
    UStageEditorDataLayerUserData* GetUserData(...);
    void UpdateSyncSnapshot(...);
    void RemoveUserData(...);
}
```

</details>

---

## 2. 反向查找方案（采纳）

### 2.1 核心思想

利用现有数据结构进行反向查找：

```cpp
// Stage.h - 已存在的关联关系
class AStage
{
    TObjectPtr<UDataLayerAsset> StageDataLayerAsset;  // Stage 的 DataLayer
    TArray<FAct> Acts;  // 每个 FAct 有 AssociatedDataLayer
};
```

### 2.2 新增 API（在 UStageManagerSubsystem 中）

```cpp
// StageManagerSubsystem.h

/**
 * 通过 DataLayer 反向查找关联的 Stage
 */
UFUNCTION(BlueprintCallable, Category = "StageEditor|DataLayerSync")
AStage* FindStageByDataLayer(UDataLayerAsset* DataLayerAsset) const;

/**
 * 检测 DataLayer 的同步状态
 */
UFUNCTION(BlueprintCallable, Category = "StageEditor|DataLayerSync")
FDataLayerSyncStatusInfo DetectSyncStatus(UDataLayerAsset* DataLayerAsset) const;

/**
 * 检查 DataLayer 是否已被导入
 */
UFUNCTION(BlueprintCallable, Category = "StageEditor|DataLayerSync")
bool IsDataLayerImported(UDataLayerAsset* DataLayerAsset) const;
```

### 2.3 实现逻辑

```cpp
AStage* UStageManagerSubsystem::FindStageByDataLayer(UDataLayerAsset* DataLayerAsset) const
{
    if (!DataLayerAsset) return nullptr;

    // 遍历 StageRegistry
    for (const auto& Pair : StageRegistry)
    {
        AStage* Stage = Pair.Value.Get();
        if (!Stage) continue;

        // 检查 Stage 级别 DataLayer
        if (Stage->StageDataLayerAsset == DataLayerAsset)
        {
            return Stage;
        }

        // 检查 Act 级别 DataLayer
        for (const FAct& Act : Stage->GetActs())
        {
            if (Act.AssociatedDataLayer == DataLayerAsset)
            {
                return Stage;
            }
        }
    }

    return nullptr;
}

bool UStageManagerSubsystem::IsDataLayerImported(UDataLayerAsset* DataLayerAsset) const
{
    return FindStageByDataLayer(DataLayerAsset) != nullptr;
}
```

### 2.4 优势对比

| 方面 | 原方案（元数据存储） | 新方案（反向查找） |
|-----|-------------------|------------------|
| 存储 | 需要持久化映射表 | 无需额外存储 |
| 一致性 | 可能与实际数据不同步 | 始终反映真实状态 |
| 复杂度 | 3 个新类 | 仅扩展现有类 |
| 性能 | O(1) | O(N)，可接受 |

---

## 3. 同步状态检测

### 3.1 状态枚举

```cpp
UENUM(BlueprintType)
enum class EDataLayerSyncStatus : uint8
{
    NotImported,    // 未导入（蓝色）
    Synced,         // 已同步（绿色）
    OutOfSync       // 不同步（橙色）
};
```

### 3.2 状态信息结构

```cpp
USTRUCT(BlueprintType)
struct STAGEEDITOR_API FDataLayerSyncStatusInfo
{
    GENERATED_BODY()

    EDataLayerSyncStatus Status = EDataLayerSyncStatus::NotImported;

    /** 新增的子 DataLayer 名称 */
    TArray<FString> NewChildDataLayers;

    /** 被删除的子 DataLayer 名称 */
    TArray<FString> RemovedChildDataLayers;

    /** 新增 Actor 数量 */
    int32 AddedActorCount = 0;

    /** 移除 Actor 数量 */
    int32 RemovedActorCount = 0;

    bool HasChanges() const;
    FString GetChangeSummary() const;
};
```

### 3.3 状态检测逻辑（新方案）

```cpp
FDataLayerSyncStatusInfo UStageManagerSubsystem::DetectSyncStatus(UDataLayerAsset* DataLayerAsset) const
{
    FDataLayerSyncStatusInfo Info;

    // 1. 反向查找 Stage
    AStage* Stage = FindStageByDataLayer(DataLayerAsset);
    if (!Stage)
    {
        Info.Status = EDataLayerSyncStatus::NotImported;
        return Info;
    }

    // 2. 判断是 Stage 级别还是 Act 级别
    if (Stage->StageDataLayerAsset == DataLayerAsset)
    {
        // Stage 级别：对比子 DataLayer 与 Acts
        DetectStageLevelChanges(Stage, DataLayerAsset, Info);
    }
    else
    {
        // Act 级别：对比 Actor 成员与 PropRegistry
        int32 ActID = FindActIDByDataLayer(Stage, DataLayerAsset);
        DetectActLevelChanges(Stage, ActID, DataLayerAsset, Info);
    }

    Info.Status = Info.HasChanges()
        ? EDataLayerSyncStatus::OutOfSync
        : EDataLayerSyncStatus::Synced;

    return Info;
}
```

---

## 4. Hover 提示生成

### 4.1 本地化文本定义

```cpp
#define LOCTEXT_NAMESPACE "StageEditorDataLayerSync"

// 英文版（默认）
// "New Act detected: {0}. Click Sync to import."
// "Act '{0}' was removed. Click Sync to update Stage."
// "Props changed: {0} added, {1} removed. Click Sync to update."

// 中文版
// "检测到新 Act: {0}，点击同步以导入"
// "Act '{0}' 已被删除，点击同步以更新 Stage"
// "Props 变化：新增 {0} 个，移除 {1} 个，点击同步以更新"

#undef LOCTEXT_NAMESPACE
```

### 4.2 提示生成函数

```cpp
FText FDataLayerSyncStatusDetector::GenerateTooltip(const FDataLayerSyncStatusInfo& Info)
{
    if (Info.Status == EDataLayerSyncStatus::Synced)
    {
        return LOCTEXT("StatusSynced", "已同步");
    }

    if (Info.Status == EDataLayerSyncStatus::NotImported)
    {
        return LOCTEXT("StatusNotImported", "未导入 - 点击 Import 按钮导入");
    }

    // 不同步状态，生成详细原因
    TArray<FText> Lines;
    // ... 构建详细提示 ...
    return FText::Join(FText::FromString(TEXT("\n")), Lines);
}
```

---

## 5. 命名解析

### 5.1 解析结果结构

```cpp
USTRUCT(BlueprintType)
struct STAGEEDITOR_API FDataLayerNameParseResult
{
    GENERATED_BODY()

    bool bIsValid = false;
    bool bIsStageLayer = false;
    FString StageName;
    FString ActName;
};
```

### 5.2 解析函数

```cpp
FDataLayerNameParseResult FStageDataLayerNameParser::Parse(const FString& DataLayerName)
{
    // Stage Pattern: ^DL_Stage_(.+)$
    // Act Pattern: ^DL_Act_([^_]+)_(.+)$
}
```

---

## 6. 导入执行逻辑

### 6.1 导入流程

```cpp
bool ImportDataLayerAsStage(UDataLayerAsset* StageDataLayer)
{
    // 1. 验证命名规范
    FDataLayerNameParseResult ParseResult = FStageDataLayerNameParser::Parse(StageDataLayer->GetName());
    if (!ParseResult.bIsValid || !ParseResult.bIsStageLayer)
    {
        return false;
    }

    // 2. 检查是否已导入（使用反向查找）
    if (UStageManagerSubsystem* Subsystem = GetWorld()->GetSubsystem<UStageManagerSubsystem>())
    if (Subsystem->IsDataLayerImported(StageDataLayer))
    {
        return false;
    }

    // 3. 创建 Stage
    FScopedTransaction Transaction(LOCTEXT("ImportStage", "Import DataLayer as Stage"));
    AStage* NewStage = CreateStageFromDataLayer(StageDataLayer, ParseResult.StageName);
    if (!NewStage) return false;

    // 4. 关联 Stage DataLayer
    NewStage->StageDataLayerAsset = StageDataLayer;

    // 5. 扫描并创建 Acts
    // ...

    return true;
}
```

---

## 7. 同步执行逻辑

### 7.1 同步流程

```cpp
bool SyncDataLayer(UDataLayerAsset* DataLayer, UWorld* World)
{
    UStageManagerSubsystem* Subsystem = World->GetSubsystem<UStageManagerSubsystem>();
    if (!Subsystem) return false;

    FDataLayerSyncStatusInfo Info = Subsystem->DetectSyncStatus(DataLayer);

    if (Info.Status != EDataLayerSyncStatus::OutOfSync)
    {
        return false;
    }

    AStage* Stage = Subsystem->FindStageByDataLayer(DataLayer);
    if (!Stage) return false;

    FScopedTransaction Transaction(LOCTEXT("SyncDataLayer", "Sync DataLayer"));

    // 处理变化...

    return true;
}
```

---

## 8. 辅助函数

### 8.1 获取子 DataLayer 名称列表

```cpp
TArray<FString> GetChildDataLayerNames(UDataLayerAsset* ParentAsset)
{
    TArray<FString> Result;

    UDataLayerManager* Manager = UDataLayerManager::GetDataLayerManager(GWorld);
    if (!Manager) return Result;

    for (const UDataLayerInstance* Instance : Manager->GetDataLayerInstances())
    {
        if (Instance->GetParent() && Instance->GetParent()->GetAsset() == ParentAsset)
        {
            Result.Add(Instance->GetDataLayerShortName());
        }
    }

    return Result;
}
```

### 8.2 获取 DataLayer 中的 Actor 列表

```cpp
TArray<TSoftObjectPtr<AActor>> GetActorsInDataLayer(UDataLayerAsset* Asset)
{
    // 使用 TActorIterator 遍历世界中的 Actor
    // 检查每个 Actor 的 DataLayerAssets 是否包含目标 Asset
}
```

---

## 9. 缓存系统（Phase 11）

### 9.1 设计原则

- **读取优先**: UI 读取直接返回缓存，无需计算
- **延迟刷新**: 后台 Ticker 刷新，不阻塞 UI
- **事件驱动**: 监听变化事件，精准失效（无自动过期）

### 9.2 缓存管理器

```cpp
class STAGEEDITOR_API FDataLayerSyncStatusCache
{
public:
    static FDataLayerSyncStatusCache& Get();  // 单例

    // 缓存读取 - O(1)
    FDataLayerSyncStatusInfo GetCachedStatus(const UDataLayerAsset* Asset);
    bool HasValidCache(const UDataLayerAsset* Asset) const;

    // 缓存管理
    void InvalidateCache(const UDataLayerAsset* Asset);
    void InvalidateAll();
    void RequestRefresh(const UDataLayerAsset* Asset = nullptr);
    FDataLayerSyncStatusInfo ForceRefresh(const UDataLayerAsset* Asset);

    // 生命周期
    void Initialize();
    void Shutdown();
};
```

### 9.3 缓存失效触发点

| 事件 | 触发方式 | 失效范围 |
|------|----------|----------|
| DataLayer 变化 | `OnDataLayerChanged` 委托 | 变化的 DataLayer + Parent |
| Actor 添加/删除 | `OnLevelActorAdded/Deleted` | Actor 关联的 DataLayers |
| Stage 注册 | `OnStageRegistered` 委托 | Stage + 所有 Acts |
| Stage 注销 | `OnStageUnregistered` 委托 | Stage + 所有 Acts |
| Controller 操作 | 手动调用 `InvalidateCache()` | 相关 DataLayers |
| Sync All 按钮 | 用户手动 | 全部缓存 |

### 9.4 Stage 注册/注销委托

```cpp
// StageManagerSubsystem.h
DECLARE_MULTICAST_DELEGATE_OneParam(FOnStageRegistered, AStage*);
FOnStageRegistered OnStageRegistered;

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnStageUnregistered, AStage*, int32);
FOnStageUnregistered OnStageUnregistered;
```

---

## 10. Stage 注册机制（Phase 10.8）

### 10.1 注册流程总览

| 场景 | 处理方式 |
|------|----------|
| 新创建 Stage | `OnLevelActorAdded` → Controller 注册 |
| 关卡加载 Stage | `ScanWorldForExistingStages` → Subsystem 扫描注册 |
| **WP streaming 加载** | `PostLoad()` → Stage 自注册 |
| PIE / 游戏模式 | 不注册（避免污染编辑器数据） |

### 10.2 PostLoad 自注册

```cpp
void AStage::PostLoad()
{
    Super::PostLoad();
#if WITH_EDITOR
    UWorld* World = GetWorld();
    if (!World) return;

    // 跳过 PIE 和游戏模式
    if (World->IsPlayInEditor() || World->IsGameWorld()) return;

    EnsureRegisteredWithSubsystem();
#endif
}
```

---

## 11. 命名规范（Phase 10.9）

### 11.1 当前规范

| 类型 | 格式 | 示例 |
|------|------|------|
| Stage DataLayer | `DL_Stage_<StageName>` | `DL_Stage_MainStage` |
| Act DataLayer | `DL_Act_<StageName>_<ActName>` | `DL_Act_MainStage_Combat` |

### 11.2 解析函数

```cpp
FDataLayerNameParseResult FStageDataLayerNameParser::Parse(const FString& DataLayerName)
{
    // Stage Pattern: ^DL_Stage_(.+)$
    // Act Pattern: ^DL_Act_([^_]+)_(.+)$
}
```

---

> **实施计划已移至**: [DevLog/Overview.md](../DevLog/Overview.md)

---

**文档版本：v4.0**
**最后更新：2025-12-01 01:30**
**变更记录：**
- v4.0: 添加缓存系统(Phase 11)、Stage 注册机制(Phase 10.8)、命名规范(Phase 10.9)
- v3.1: 移除实施计划章节，集中到 DevLog/Overview.md
