# Phase 1: 元数据基础设施

> 日期: 2025-11-29 13:15-13:25
> 状态: ✅ 完成

---

## 1. 目标

实现 DataLayer 导入状态的存储机制，用于追踪哪些 DataLayer 已被导入、关联的 SUID、以及同步快照。

---

## 2. 技术问题与解决

### 2.1 原方案问题

原 TechSpec 设计使用 `UAssetUserData` 附加到 `UDataLayerAsset`：

```cpp
// 原方案 - 无法工作
Asset->AddAssetUserData(UserData);
Asset->GetAssetUserData<UStageEditorDataLayerUserData>();
```

**编译错误**：`UDataLayerAsset` 继承自 `UDataAsset -> UObject`，不支持 `IInterface_AssetUserData`。

### 2.2 解决方案

改用 **EditorSubsystem + 映射表** 方案：

```cpp
// 新方案
UStageDataLayerSyncSubsystem
├── TMap<FSoftObjectPath, FDataLayerImportMetadata> MetadataMap
├── SetMetadata() / GetMetadata() / RemoveMetadata()
└── SaveToConfig() / LoadFromConfig()  // TODO: 持久化
```

**优点**：
- 不依赖 UDataLayerAsset 的内部实现
- Subsystem 生命周期由引擎管理
- 可以灵活扩展持久化方式

---

## 3. 实施记录

### 3.1 创建的文件

| 文件 | 说明 |
|------|------|
| `Public/DataLayerSync/StageEditorDataLayerUserData.h` | `FDataLayerImportMetadata` 结构体 |
| `Public/DataLayerSync/StageDataLayerSyncSubsystem.h` | 同步管理 Subsystem |
| `Private/DataLayerSync/StageDataLayerSyncSubsystem.cpp` | Subsystem 实现 |

### 3.2 修改的文件

| 文件 | 修改内容 |
|------|---------|
| `Public/DataLayerSync/StageEditorDataLayerUtils.h` | 更新 API，使用新 Subsystem |
| `Private/DataLayerSync/StageEditorDataLayerUtils.cpp` | 实现调用 Subsystem |

### 3.3 核心代码

**FDataLayerImportMetadata 结构体**

```cpp
// StageEditorDataLayerUserData.h:17-76
USTRUCT(BlueprintType)
struct STAGEEDITOR_API FDataLayerImportMetadata
{
    GENERATED_BODY()

    FSUID LinkedSUID;                              // 关联的 Stage/Act SUID
    FDateTime LastSyncTime;                        // 最后同步时间
    TArray<FString> LastKnownChildDataLayerNames;  // 子 DataLayer 快照
    TArray<TSoftObjectPtr<AActor>> LastKnownActors; // Actor 快照

    bool IsImported() const { return LinkedSUID.IsValid(); }
    bool IsStageLevel() const { return LinkedSUID.IsStageLevel(); }
    bool IsActLevel() const { return LinkedSUID.IsActLevel(); }
};
```

**Subsystem 存储机制**

```cpp
// StageDataLayerSyncSubsystem.h
UPROPERTY()
TMap<FSoftObjectPath, FDataLayerImportMetadata> MetadataMap;

// 使用 FSoftObjectPath 作为 Key，确保资产重命名后仍能正确关联
void UStageDataLayerSyncSubsystem::SetMetadata(UDataLayerAsset* Asset, const FDataLayerImportMetadata& Metadata)
{
    FSoftObjectPath AssetPath(Asset);
    MetadataMap.Add(AssetPath, Metadata);
    MarkDirty();
}
```

### 3.4 编译验证

```
Result: Succeeded
Total execution time: 12.77 seconds
```

---

## 4. 待完成项

- [ ] `SaveToConfig()` / `LoadFromConfig()` 持久化实现
  - 可选方案：GConfig .ini 文件 或 项目目录 JSON 文件

---

*文档结束*
