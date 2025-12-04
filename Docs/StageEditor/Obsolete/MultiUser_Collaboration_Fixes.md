# StageEditor 多人协作架构改进方案

> 创建日期: 2025-11-22
> 状态: 待实施
> 优先级: 🔥 高（多人协作关键）

## 1. 问题诊断

### 1.1 Stage DataLayer查找使用字符串名称（冲突风险）

**当前实现：**
```cpp
// AStage.h
UPROPERTY(EditAnywhere, Category = "Stage|DataLayer")
FString StageDataLayerName;  // ⚠️ 字符串名称，不唯一

// StageEditorController.cpp - 通过名称字符串查找
FString StageAssetName = FString::Printf(TEXT("DL_Stage_%s"), *Stage->StageDataLayerName);
for (UDataLayerInstance* Instance : AllInstances)
{
    if (Instance->GetAsset()->GetName() == StageAssetName)  // ⚠️ 字符串匹配
    {
        // ...
    }
}
```

**问题场景：**
```
策划A创建: Stage "MainStage" → DataLayer "DL_Stage_MainStage"
策划B创建: Stage "MainStage" → DataLayer "DL_Stage_MainStage"
提交合并: DataLayer Asset名称冲突 ❌
```

---

### 1.2 ActorLabel用作UI标识（显示冲突）

**当前实现：**
```cpp
// StageEditorPanel.cpp:304
FString StageName = Stage->GetActorLabel();  // ⚠️ 用户可编辑，可能重复

// StageEditorPanel.cpp:333
FString PropName = PropActor->GetActorLabel();  // ⚠️ 同样问题
```

**问题场景：**
```
策划A: 创建Stage Actor，Label = "MainStage"
策划B: 创建Stage Actor，Label = "MainStage"
提交合并: UI中显示两个相同名称的Stage，无法区分 ❌
```

**注意：**
- Actor本身不冲突（External Actor机制，文件名是GUID）
- 但UI显示时无法区分

---

### 1.3 World Partition多人协作机制回顾

```
✅ Actor不会冲突:
MyLevel_ExternalActors/
    ├── 12AB34CD.uasset  → 策划A的Stage (GUID文件名)
    ├── 56EF78GH.uasset  → 策划B的Stage (不同GUID)
    └── 不会有文件名冲突

⚠️ 但逻辑查找/UI显示可能冲突:
- 如果两个Stage都绑定名为"MainStage"的DataLayer
- 如果两个Stage的ActorLabel都是"MainStage"
```

---

## 2. 解决方案

### 2.1 使用Asset引用而非字符串名称

| 当前 | 改进 |
|------|------|
| `FString StageDataLayerName` | `TObjectPtr<UDataLayerAsset> StageDataLayerAsset` |
| 字符串匹配查找 | 指针比较查找 |
| 不唯一，可能冲突 | 唯一，不会冲突 |

### 2.2 UI显示使用友好名称，但依赖唯一ID

| 元素 | 唯一标识 | 显示名称 |
|------|----------|----------|
| **Stage** | `StageID` (int32) + ActorGUID | `GetActorLabel()` (可重复，仅显示) |
| **Act** | `ActID` (int32) | `DisplayName` (可重复，仅显示) |
| **Prop** | `PropID` (int32) | `GetActorLabel()` (可重复，仅显示) |

**改进原则：**
- 内部逻辑全部使用唯一ID
- UI显示使用友好名称（ActorLabel）
- 如果名称重复，在UI中显示额外标识（例如GUID后缀）

---

## 3. 架构改进方案

### 3.1 Stage DataLayer引用改进

```cpp
// Stage.h
class STAGEEDITORRUNTIME_API AStage : public AActor
{
    GENERATED_BODY()

public:
    // ✅ 主要引用：直接指向DataLayerAsset
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stage|DataLayer",
        meta = (DisplayName = "Stage DataLayer Asset"))
    TObjectPtr<UDataLayerAsset> StageDataLayerAsset;

    // ⚠️ 辅助字段：仅用于显示/向后兼容（自动同步）
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stage|DataLayer",
        meta = (DisplayName = "DataLayer Name (Display Only)"))
    FString StageDataLayerName;

#if WITH_EDITOR
    // 自动从Asset同步名称
    virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override
    {
        Super::PostEditChangeProperty(PropertyChangedEvent);

        if (PropertyChangedEvent.GetPropertyName() == GET_MEMBER_NAME_CHECKED(AStage, StageDataLayerAsset))
        {
            if (StageDataLayerAsset)
            {
                StageDataLayerName = StageDataLayerAsset->GetName();
            }
        }
    }
#endif
};
```

### 3.2 Controller查找逻辑改进

```cpp
// StageEditorController.cpp

// ❌ 旧实现（字符串查找）
UDataLayerInstance* FindStageDataLayerInstance(AStage* Stage)
{
    FString StageAssetName = FString::Printf(TEXT("DL_Stage_%s"), *Stage->StageDataLayerName);
    for (UDataLayerInstance* Instance : AllInstances)
    {
        if (Instance->GetAsset()->GetName() == StageAssetName)  // 不可靠
            return Instance;
    }
}

// ✅ 新实现（Asset引用）
UDataLayerInstance* FindStageDataLayerInstance(AStage* Stage)
{
    if (!Stage->StageDataLayerAsset) return nullptr;

    for (UDataLayerInstance* Instance : AllInstances)
    {
        if (Instance->GetAsset() == Stage->StageDataLayerAsset)  // 唯一且可靠
            return Instance;
    }
    return nullptr;
}
```

### 3.3 DataLayer Asset创建时确保唯一性

```cpp
// StageEditorController.cpp - CreateDataLayerAsset()
UDataLayerAsset* FStageEditorController::CreateDataLayerAsset(const FString& BaseName, const FString& FolderPath)
{
    FString UniqueName = BaseName;
    int32 Suffix = 1;

    // 检查Asset是否已存在，避免覆盖
    FString PackagePath = FolderPath / UniqueName;
    while (LoadObject<UDataLayerAsset>(nullptr, *PackagePath))
    {
        UniqueName = FString::Printf(TEXT("%s_%d"), *BaseName, Suffix++);
        PackagePath = FolderPath / UniqueName;
    }

    // 使用唯一名称创建Asset
    UPackage* Package = CreatePackage(*PackagePath);
    UDataLayerAsset* NewAsset = NewObject<UDataLayerAsset>(Package, *UniqueName, RF_Public | RF_Standalone);
    // ... 保存逻辑

    return NewAsset;
}
```

### 3.4 UI显示改进（处理重名）

```cpp
// StageEditorPanel.cpp - RefreshUI()

// 检测Stage名称是否重复
TMap<FString, int32> LabelCounts;
for (const TWeakObjectPtr<AStage>& StagePtr : Controller->GetFoundStages())
{
    if (AStage* Stage = StagePtr.Get())
    {
        FString Label = Stage->GetActorLabel();
        LabelCounts.FindOrAdd(Label)++;
    }
}

// 生成显示名称
for (const TWeakObjectPtr<AStage>& StagePtr : Controller->GetFoundStages())
{
    if (AStage* Stage = StagePtr.Get())
    {
        FString BaseLabel = Stage->GetActorLabel();
        FString DisplayName = BaseLabel;

        // 如果名称重复，添加GUID后缀
        if (LabelCounts[BaseLabel] > 1)
        {
            FString ShortGuid = Stage->GetActorGuid().ToString(EGuidFormats::Short);
            DisplayName = FString::Printf(TEXT("%s [%s]"), *BaseLabel, *ShortGuid.Left(8));
        }

        TSharedPtr<FStageTreeItem> StageItem = MakeShared<FStageTreeItem>(
            EStageTreeItemType::Stage,
            DisplayName,  // ✅ 显示名称可能带GUID
            Stage->StageID,
            nullptr,
            Stage
        );
        // ...
    }
}
```

---

## 4. 实施计划

### Phase 1: Stage DataLayer Asset引用 🔥
- [ ] `AStage`添加`StageDataLayerAsset`字段（TObjectPtr）
- [ ] `StageDataLayerName`降级为VisibleAnywhere（自动同步）
- [ ] 添加`PostEditChangeProperty`自动同步逻辑
- [ ] 测试编译

### Phase 2: Controller查找逻辑迁移
- [ ] 重写`FindStageDataLayerInstance()`使用Asset引用
- [ ] 更新`AssignPropToStageDataLayer()`使用Asset查找
- [ ] 更新`RemovePropFromStageDataLayer()`使用Asset查找
- [ ] 搜索所有使用`StageDataLayerName`的地方并迁移

### Phase 3: DataLayer创建唯一性
- [ ] 修改`CreateDataLayerAsset()`添加冲突检测
- [ ] 重复时自动添加数字后缀
- [ ] 更新`CreateDataLayerForStage()`使用新逻辑
- [ ] 更新`CreateDataLayerForAct()`使用新逻辑

### Phase 4: UI显示重名检测
- [ ] `RefreshUI()`中检测Stage重名
- [ ] 重名时添加GUID后缀显示
- [ ] Prop重名也添加类似逻辑
- [ ] 添加Tooltip显示完整信息

### Phase 5: 向后兼容与迁移
- [ ] 为旧数据添加迁移逻辑（从Name查找并设置Asset）
- [ ] 添加编辑器警告提示用户手动修复
- [ ] 文档更新

### Phase 6: 测试与验证
- [ ] 单人测试：创建/删除/关联DataLayer
- [ ] 多人模拟测试：两个人创建同名Stage/DataLayer
- [ ] 验证External Actor机制正常工作
- [ ] 验证UI显示正确

---

## 5. 关键代码位置

| 文件 | 需要修改的内容 |
|------|----------------|
| `Stage.h` | 添加`StageDataLayerAsset`字段 |
| `StageEditorController.cpp:965-991` | `AssignPropToStageDataLayer()` - 改用Asset查找 |
| `StageEditorController.cpp:994-1034` | `RemovePropFromStageDataLayer()` - 改用Asset查找 |
| `StageEditorController.cpp:675-724` | `CreateDataLayerAsset()` - 添加唯一性检测 |
| `StageEditorController.cpp:767-795` | `CreateDataLayerForStage()` - 设置Asset引用 |
| `StageEditorPanel.cpp:304` | Stage显示名称 - 添加重名检测 |
| `StageEditorPanel.cpp:333` | Prop显示名称 - 添加重名检测 |

---

## 6. 风险评估

| 风险 | 影响 | 缓解措施 |
|------|------|----------|
| **向后兼容** | 旧关卡无`StageDataLayerAsset` | 添加迁移代码自动修复 |
| **UI混乱** | 多个同名Stage显示 | GUID后缀 + Tooltip |
| **查找失败** | Asset引用丢失 | 添加Fallback到Name查找 |
| **测试不足** | 多人场景难模拟 | 使用P4/Git本地多分支测试 |

---

## 7. 预期效果

### 改进前
```
策划A提交: Stage "MainStage" → DataLayer "DL_Stage_MainStage"
策划B提交: Stage "MainStage" → DataLayer "DL_Stage_MainStage"
合并: ❌ DataLayer Asset冲突或查找错误
```

### 改进后
```
策划A提交: Stage "MainStage" → DataLayer Asset at /StageEditor/DataLayers/DL_Stage_MainStage
策划B提交: Stage "MainStage" → DataLayer Asset at /StageEditor/DataLayers/DL_Stage_MainStage_1
合并: ✅ 两个不同的Asset，不冲突
UI显示:
  - MainStage [12AB34CD]  (策划A的)
  - MainStage [56EF78GH]  (策划B的)
```

---

## 8. 相关文档

- `Docs/StageEditor/HighLevelDesign.md` - 总体架构
- `Docs/StageEditor/DataLayer_Integration_Design.md` - DataLayer集成方案
- `Docs/StageEditor/DataLayer_ReverseSync_Design.md` - 反向同步方案

---

**准备实施？建议按Phase顺序逐步推进。**
