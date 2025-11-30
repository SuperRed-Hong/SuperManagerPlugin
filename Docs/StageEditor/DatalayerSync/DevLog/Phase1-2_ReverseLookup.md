# Phase 1-2: 反向查找与状态检测

> 日期: 2025-11-29
> 状态: ✅ 已完成
> 最后更新: 2025-11-29 15:43

---

## 1. 目标

将原本分离的 Phase 1（元数据基础设施）和 Phase 2（状态检测）合并，采用反向查找方案实现。

**核心改变**：
- 废弃独立的 `UStageDataLayerSyncSubsystem`
- 利用现有 `UStageManagerSubsystem` + `AStage` 数据结构
- 实时反向查找代替元数据存储

---

## 2. 实施清单

### 2.1 删除文件 ✅

| 文件 | 说明 | 状态 |
|------|------|------|
| `Public/DataLayerSync/StageEditorDataLayerUserData.h` | 废弃的元数据结构体 | ✅ 已删除 |
| `Public/DataLayerSync/StageDataLayerSyncSubsystem.h` | 废弃的 Subsystem | ✅ 已删除 |
| `Private/DataLayerSync/StageDataLayerSyncSubsystem.cpp` | | ✅ 已删除 |
| `Public/DataLayerSync/StageEditorDataLayerUtils.h` | 废弃的工具函数 | ✅ 已删除 |
| `Private/DataLayerSync/StageEditorDataLayerUtils.cpp` | | ✅ 已删除 |

### 2.2 修改 UStageManagerSubsystem ✅

新增方法（`StageEditorRuntime` 模块）：

```cpp
// StageManagerSubsystem.h - DataLayer Sync API

/**
 * 通过 DataLayer 反向查找关联的 Stage
 */
UFUNCTION(BlueprintCallable, Category = "Stage Manager|DataLayerSync")
AStage* FindStageByDataLayer(class UDataLayerAsset* DataLayerAsset) const;

/**
 * 检查 DataLayer 是否已被导入
 */
UFUNCTION(BlueprintCallable, Category = "Stage Manager|DataLayerSync")
bool IsDataLayerImported(class UDataLayerAsset* DataLayerAsset) const;

/**
 * 在 Stage 中查找关联此 DataLayer 的 ActID
 */
UFUNCTION(BlueprintCallable, Category = "Stage Manager|DataLayerSync")
int32 FindActIDByDataLayer(AStage* Stage, class UDataLayerAsset* DataLayerAsset) const;
```

### 2.3 更新 DataLayerSyncStatus ✅

保留：
- `EDataLayerSyncStatus` 枚举
- `FDataLayerSyncStatusInfo` 结构体

修改 `FDataLayerSyncStatusDetector::DetectStatus()`：
- 移除对废弃 Subsystem 的依赖
- 调用 `UStageManagerSubsystem::FindStageByDataLayer()` 实现反向查找
- 根据 Stage-level 或 Act-level 分别检测变化

---

## 3. 实施记录

### 3.1 删除文件 ✅ (2025-11-29 15:40)

```
[x] StageEditorDataLayerUserData.h
[x] StageDataLayerSyncSubsystem.h
[x] StageDataLayerSyncSubsystem.cpp
[x] StageEditorDataLayerUtils.h
[x] StageEditorDataLayerUtils.cpp
```

### 3.2 代码修改 ✅ (2025-11-29 15:35)

**StageManagerSubsystem.h** (行 216-252):
```cpp
#pragma region DataLayer Sync API
    AStage* FindStageByDataLayer(class UDataLayerAsset* DataLayerAsset) const;
    bool IsDataLayerImported(class UDataLayerAsset* DataLayerAsset) const;
    int32 FindActIDByDataLayer(AStage* Stage, class UDataLayerAsset* DataLayerAsset) const;
#pragma endregion DataLayer Sync API
```

**StageManagerSubsystem.cpp** (行 424-483):
- `FindStageByDataLayer()`: 遍历 `StageRegistry`，检查 `StageDataLayerAsset` 和 `Acts[].AssociatedDataLayer`
- `IsDataLayerImported()`: 调用 `FindStageByDataLayer() != nullptr`
- `FindActIDByDataLayer()`: 遍历 `Stage->Acts`，返回匹配的 `Act.SUID.ActID`

**DataLayerSyncStatus.cpp**:
- 移除对 `StageEditorDataLayerUtils` 和 `FDataLayerImportMetadata` 的依赖
- 添加 `GetStageManagerSubsystem()` 辅助函数
- 重写 `DetectStatus()` 使用反向查找
- 添加 `DetectStageLevelChanges()` 和 `DetectActLevelChanges()` 辅助函数

### 3.3 编译验证 ✅ (2025-11-29 15:42)

```
Result: Succeeded
Total execution time: 7.21 seconds
```

---

## 4. 关键文件

```
Plugins/StageEditor/Source/
├── StageEditorRuntime/
│   ├── Public/Subsystems/StageManagerSubsystem.h   # 新增反向查找 API
│   └── Private/Subsystems/StageManagerSubsystem.cpp
└── StageEditor/
    ├── Public/DataLayerSync/DataLayerSyncStatus.h   # 保留
    └── Private/DataLayerSync/DataLayerSyncStatus.cpp # 重写
```

---

*文档结束*
