# DataLayerSync 架构设计：反向查找方案

> 创建日期: 2025-11-29
> 状态: 采纳

---

## 1. 背景

### 1.1 原方案的问题

原方案试图使用 `UAssetUserData` 附加元数据到 `UDataLayerAsset`，但发现：

```
UDataLayerAsset -> UDataAsset -> UObject
                   ↑
                   不实现 IInterface_AssetUserData
```

为绕过此限制，创建了独立的 `UStageDataLayerSyncSubsystem` 存储映射：

```cpp
// 废弃方案
TMap<FSoftObjectPath, FDataLayerImportMetadata> MetadataMap;
```

**问题**：
- 增加了冗余的存储层
- 需要持久化（SaveToConfig/LoadFromConfig）
- 与 `UStageManagerSubsystem` 的 Stage 注册表功能重复

### 1.2 关键洞察

现有数据结构已包含 DataLayer 关联信息：

```cpp
// Stage.h
class AStage
{
    TObjectPtr<UDataLayerAsset> StageDataLayerAsset;  // Stage 的 DataLayer
    TArray<FAct> Acts;  // 每个 FAct 有 AssociatedDataLayer
};

// StageCoreTypes.h
struct FAct
{
    TObjectPtr<UDataLayerAsset> AssociatedDataLayer;  // Act 的 DataLayer
};
```

**结论**：无需存储元数据，只需反向查找。

---

## 2. 新架构

### 2.1 核心思想

```
DataLayer → Stage 反向查找
    ↓
遍历 StageRegistry，检查 StageDataLayerAsset 和 Acts[].AssociatedDataLayer
    ↓
找到关联的 Stage/Act
    ↓
实时对比检测同步状态
```

### 2.2 类职责

| 类 | 职责 |
|---|------|
| `UStageManagerSubsystem` | 新增 `FindStageByDataLayer()` 和 `DetectSyncStatus()` 方法 |
| `FDataLayerSyncStatusDetector` | 保留，但改为调用 Subsystem 方法 |
| `FStageDataLayerNameParser` | 保留，命名解析不变 |

### 2.3 删除的类

| 类 | 原职责 | 删除原因 |
|---|-------|---------|
| `UStageDataLayerSyncSubsystem` | 元数据存储 | 冗余，使用反向查找代替 |
| `FDataLayerImportMetadata` | 元数据结构体 | 不再需要持久化 |
| `StageEditorDataLayerUtils` | 工具函数 | 整合到 Subsystem |

---

## 3. API 设计

### 3.1 UStageManagerSubsystem 新增方法

```cpp
// StageManagerSubsystem.h

/**
 * 通过 DataLayer 反向查找关联的 Stage
 * @param DataLayerAsset 目标 DataLayerAsset
 * @return 关联的 Stage，未找到返回 nullptr
 */
UFUNCTION(BlueprintCallable, Category = "StageEditor|DataLayerSync")
AStage* FindStageByDataLayer(UDataLayerAsset* DataLayerAsset) const;

/**
 * 检测 DataLayer 的同步状态
 * @param DataLayerAsset 目标 DataLayerAsset
 * @return 同步状态信息
 */
UFUNCTION(BlueprintCallable, Category = "StageEditor|DataLayerSync")
FDataLayerSyncStatusInfo DetectSyncStatus(UDataLayerAsset* DataLayerAsset) const;

/**
 * 检查 DataLayer 是否已被导入（关联到某个 Stage/Act）
 * @param DataLayerAsset 目标 DataLayerAsset
 * @return 是否已导入
 */
UFUNCTION(BlueprintCallable, Category = "StageEditor|DataLayerSync")
bool IsDataLayerImported(UDataLayerAsset* DataLayerAsset) const;
```

### 3.2 同步状态检测逻辑

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
        // Stage 级别：检查子 DataLayer 是否与 Acts 一致
        DetectStageLevelChanges(Stage, Info);
    }
    else
    {
        // Act 级别：检查 Actor 成员是否与 PropRegistry 一致
        int32 ActID = FindActIDByDataLayer(Stage, DataLayerAsset);
        DetectActLevelChanges(Stage, ActID, DataLayerAsset, Info);
    }

    // 3. 判断最终状态
    Info.Status = Info.HasChanges()
        ? EDataLayerSyncStatus::OutOfSync
        : EDataLayerSyncStatus::Synced;

    return Info;
}
```

---

## 4. 优势对比

| 方面 | 原方案（元数据存储） | 新方案（反向查找） |
|-----|-------------------|------------------|
| 存储 | 需要持久化映射表 | 无需额外存储 |
| 一致性 | 可能与实际数据不同步 | 始终反映真实状态 |
| 复杂度 | 3 个新类 | 仅扩展现有类 |
| 查询性能 | O(1) | O(N)，N=Stage 数量 |
| 内存占用 | 需要维护映射表 | 无额外内存 |

**性能说明**：项目中 Stage 数量通常 < 100，O(N) 遍历开销可接受。

---

## 5. 实施步骤

1. 删除冗余文件（已废弃的类）
2. 在 `UStageManagerSubsystem` 中添加反向查找方法
3. 更新 `FDataLayerSyncStatusDetector` 调用新方法
4. 更新 UI 层调用

---

*文档版本: v1.0*
*最后更新: 2025-11-29 14:52*
