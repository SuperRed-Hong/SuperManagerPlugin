# Phase 3 重新设计：基于 ID 系统的 DataLayer 唯一性方案

## 📋 问题重新审视

### 当前问题
使用 **名称** 来标识 DataLayer 存在根本性缺陷：
- ❌ 多用户可能创建同名 Stage/Act
- ❌ 名称冲突需要复杂的后缀处理
- ❌ 用户重命名会导致混乱
- ❌ 不符合我们的 ID 系统设计理念

### 根本原因
**我们已经有了完善的 ID 系统，但 DataLayer 命名没有使用它！**

---

## 💡 新方案：基于 ID 的 DataLayer 命名

### 核心思路

**DataLayer 名称应该基于 ID，而不是用户可见的 DisplayName**

### 命名规则

#### Stage DataLayer
```
格式: DL_Stage_{StageID}
示例:
  - DL_Stage_1001
  - DL_Stage_1002
  - DL_Stage_-1  (离线临时 ID)
```

#### Act DataLayer
```
格式: DL_Act_{StageID}_{ActID}
示例:
  - DL_Act_1001_1
  - DL_Act_1001_2
  - DL_Act_-1_1  (离线临时 ID)
```

### 优势

1. ✅ **绝对唯一性**
   - StageID 由系统分配，全局唯一
   - ActID 在 Stage 内唯一
   - 组合后的 DataLayer 名称必然唯一

2. ✅ **符合现有架构**
   - 完全遵循 `detailed_specification.md` 的 ID 系统设计
   - 与 `FStageHierarchicalId` 结构一致

3. ✅ **简单可靠**
   - 不需要冲突检测
   - 不需要递增后缀
   - 不需要缓存

4. ✅ **支持离线工作流**
   - 离线时使用负数 ID: `DL_Stage_-1`
   - 联网后修复 ID: `DL_Stage_-1` → `DL_Stage_1001`
   - DataLayer 资产需要重命名，但这是一次性操作

5. ✅ **用户友好**
   - 用户看到的是 DisplayName（如 "主舞台"）
   - 底层使用 ID 确保唯一性
   - 两者分离，互不干扰

---

## 🔧 详细设计

### 1. 数据结构（无需修改）

当前的数据结构已经完美支持这个方案：

```cpp
// Stage.h
UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stage")
int32 StageID = 0;  // ✅ 已有

// StageCoreTypes.h
struct FAct
{
    FStageHierarchicalId ActID;  // ✅ 已有，包含 StageID 和 ActID
    FString DisplayName;         // ✅ 已有，用于 UI 显示
    TObjectPtr<UDataLayerAsset> AssociatedDataLayer;  // ✅ 已有
};
```

### 2. DataLayer 创建逻辑

#### 2.1 创建 Stage DataLayer

```cpp
bool FStageEditorController::CreateDataLayerForStage(AStage* Stage)
{
    if (!Stage) return false;
    
    // 使用 StageID 生成唯一名称
    FString AssetName = FString::Printf(TEXT("DL_Stage_%d"), Stage->StageID);
    
    // 创建 DataLayer 资产
    UDataLayerAsset* NewAsset = CreateDataLayerAsset(AssetName, DataLayerAssetFolderPath);
    if (!NewAsset) return false;
    
    // 关联到 Stage
    Stage->StageDataLayerAsset = NewAsset;
    
    // 更新显示名称（用于 UI）
    Stage->StageDataLayerName = AssetName;
    
    UE_LOG(LogTemp, Log, TEXT("✅ Created Stage DataLayer: %s for Stage '%s' (ID: %d)"),
        *AssetName, *Stage->GetName(), Stage->StageID);
    
    return true;
}
```

#### 2.2 创建 Act DataLayer

```cpp
bool FStageEditorController::CreateDataLayerForAct(int32 ActID)
{
    AStage* Stage = GetActiveStage();
    if (!Stage) return false;
    
    // 查找 Act
    FAct* TargetAct = nullptr;
    for (FAct& Act : Stage->Acts)
    {
        if (Act.ActID.ActID == ActID)
        {
            TargetAct = &Act;
            break;
        }
    }
    
    if (!TargetAct) return false;
    
    // 使用 StageID + ActID 生成唯一名称
    FString AssetName = FString::Printf(
        TEXT("DL_Act_%d_%d"),
        Stage->StageID,
        ActID
    );
    
    // 创建 DataLayer 资产
    UDataLayerAsset* NewAsset = CreateDataLayerAsset(AssetName, DataLayerAssetFolderPath);
    if (!NewAsset) return false;
    
    // 关联到 Act
    TargetAct->AssociatedDataLayer = NewAsset;
    
    UE_LOG(LogTemp, Log, TEXT("✅ Created Act DataLayer: %s for Act '%s' (ID: %d)"),
        *AssetName, *TargetAct->DisplayName, ActID);
    
    return true;
}
```

### 3. ID 分配策略

#### 3.1 StageID 分配

**Phase 1 (当前 - 原型阶段)**:
```cpp
// 简单的本地计数器
int32 FStageEditorController::AllocateStageID()
{
    static int32 NextStageID = 1;
    return NextStageID++;
}
```

**Phase 2 (未来 - 服务器集成)**:
```cpp
// 从服务器获取
int32 FStageEditorController::AllocateStageID()
{
    // TODO: 调用服务器 API
    // return ServerAPI->RequestStageID();
    
    // 离线时使用负数
    static int32 OfflineID = -1;
    return OfflineID--;
}
```

#### 3.2 ActID 分配

```cpp
int32 FStageEditorController::AllocateActID(AStage* Stage)
{
    if (!Stage) return -1;
    
    // 在 Stage 内部分配唯一的 ActID
    int32 MaxActID = 0;
    for (const FAct& Act : Stage->Acts)
    {
        if (Act.ActID.ActID > MaxActID)
        {
            MaxActID = Act.ActID.ActID;
        }
    }
    
    return MaxActID + 1;
}
```

### 4. UI 显示逻辑

#### 4.1 显示友好名称

在 UI 中，用户看到的是 `DisplayName`，而不是 DataLayer 的内部名称：

```cpp
// StageEditorPanel.cpp
void SStageEditorPanel::RefreshUI()
{
    // 显示 Stage
    TreeView->AddItem(
        Stage->GetName(),  // 显示 "主舞台"
        Stage->StageID     // 内部使用 ID: 1001
    );
    
    // 显示 Acts
    for (const FAct& Act : Stage->Acts)
    {
        TreeView->AddItem(
            Act.DisplayName,   // 显示 "开门场景"
            Act.ActID.ActID    // 内部使用 ID: 1
        );
    }
}
```

#### 4.2 DataLayer Outliner 中的显示

DataLayer Outliner 会显示资产名称（基于 ID），但这是技术细节，用户通常不需要关心：

```
DataLayer Outliner:
├─ DL_Stage_1001          (技术名称)
│  ├─ DL_Act_1001_1
│  └─ DL_Act_1001_2
└─ DL_Stage_1002
   └─ DL_Act_1002_1
```

---

## 🔄 离线到在线的 ID 修复流程

### 场景
用户离线创建了 Stage，使用临时 ID `-1`，然后联网修复。

### 步骤

#### 1. 离线创建
```cpp
// 用户创建 Stage
Stage->StageID = -1;  // 临时 ID

// 创建 DataLayer
CreateDataLayerForStage(Stage);
// 结果: DL_Stage_-1
```

#### 2. 联网修复
```cpp
void FStageEditorController::FixStageID(AStage* Stage)
{
    if (Stage->StageID >= 0)
    {
        // 已经是正式 ID，无需修复
        return;
    }
    
    // 1. 从服务器获取正式 ID
    int32 NewStageID = RequestStageIDFromServer();  // 假设返回 1001
    
    // 2. 保存旧的 DataLayer 资产引用
    UDataLayerAsset* OldDataLayer = Stage->StageDataLayerAsset;
    
    // 3. 更新 Stage ID
    int32 OldStageID = Stage->StageID;
    Stage->StageID = NewStageID;
    
    // 4. 重命名 DataLayer 资产
    if (OldDataLayer)
    {
        FString OldName = FString::Printf(TEXT("DL_Stage_%d"), OldStageID);
        FString NewName = FString::Printf(TEXT("DL_Stage_%d"), NewStageID);
        
        RenameDataLayerAsset(OldDataLayer, NewName);
    }
    
    // 5. 修复所有 Act 的 DataLayer
    for (FAct& Act : Stage->Acts)
    {
        // 更新 ActID 中的 StageID
        Act.ActID.StageID = NewStageID;
        
        // 重命名 Act DataLayer
        if (Act.AssociatedDataLayer)
        {
            FString OldActName = FString::Printf(TEXT("DL_Act_%d_%d"), OldStageID, Act.ActID.ActID);
            FString NewActName = FString::Printf(TEXT("DL_Act_%d_%d"), NewStageID, Act.ActID.ActID);
            
            RenameDataLayerAsset(Act.AssociatedDataLayer, NewActName);
        }
    }
    
    // 6. 保存 Stage
    Stage->MarkPackageDirty();
    
    UE_LOG(LogTemp, Log, TEXT("✅ Fixed Stage ID: %d → %d"), OldStageID, NewStageID);
}
```

#### 3. 重命名 DataLayer 资产的辅助函数

```cpp
bool FStageEditorController::RenameDataLayerAsset(UDataLayerAsset* Asset, const FString& NewName)
{
    if (!Asset) return false;
    
    // 获取资产的包
    UPackage* Package = Asset->GetPackage();
    if (!Package) return false;
    
    // 构建新的包路径
    FString OldPackageName = Package->GetName();
    FString NewPackageName = FPaths::GetPath(OldPackageName) / NewName;
    
    // 使用 UE 的资产重命名 API
    FAssetToolsModule& AssetToolsModule = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools");
    IAssetTools& AssetTools = AssetToolsModule.Get();
    
    TArray<FAssetRenameData> RenameData;
    RenameData.Add(FAssetRenameData(Asset, NewPackageName, NewName));
    
    bool bSuccess = AssetTools.RenameAssets(RenameData);
    
    if (bSuccess)
    {
        UE_LOG(LogTemp, Log, TEXT("✅ Renamed DataLayer: %s → %s"), *Asset->GetName(), *NewName);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("❌ Failed to rename DataLayer: %s"), *Asset->GetName());
    }
    
    return bSuccess;
}
```

---

## 📊 对比：名称方案 vs ID 方案

| 特性 | 名称方案 | ID 方案 ⭐ |
|------|---------|-----------|
| **唯一性保证** | ❌ 需要冲突检测 | ✅ 系统保证 |
| **实现复杂度** | ⚠️ 中等（需要递增后缀） | ✅ 简单 |
| **性能** | ⚠️ 需要循环检查 | ✅ 无需检查 |
| **用户重命名** | ❌ 可能导致冲突 | ✅ 不影响 |
| **多用户协作** | ❌ 可能冲突 | ✅ 完美支持 |
| **符合架构** | ❌ 不符合 ID 系统 | ✅ 完全符合 |
| **离线工作流** | ⚠️ 复杂 | ✅ 原生支持 |
| **代码量** | ~150 行 | ~50 行 |

---

## 🎯 实施计划

### Phase 3A: 基础 ID 系统 (本次)

#### 任务清单
1. [ ] 添加 `AllocateStageID()` 函数
2. [ ] 添加 `AllocateActID()` 函数
3. [ ] 修改 `CreateDataLayerForStage()` 使用 StageID
4. [ ] 修改 `CreateDataLayerForAct()` 使用 StageID + ActID
5. [ ] 更新 `CreateNewStage()` 分配 StageID
6. [ ] 更新 `CreateNewAct()` 分配 ActID
7. [ ] 测试验证

**预计时间**: 1-2 小时
**代码量**: ~100 行

### Phase 3B: ID 修复功能 (可选，Phase 2 再做)

#### 任务清单
1. [ ] 添加 `FixStageID()` 函数
2. [ ] 添加 `RenameDataLayerAsset()` 辅助函数
3. [ ] 在 UI 中添加 "Fix ID" 按钮
4. [ ] 测试离线到在线的转换

**预计时间**: 2-3 小时
**代码量**: ~150 行

---

## 🤔 需要讨论的问题

### 问题 1: StageID 初始值

**选项 A**: 从 1 开始
```cpp
static int32 NextStageID = 1;
```

**选项 B**: 从 1000 开始（预留空间）
```cpp
static int32 NextStageID = 1000;
```

**您的偏好？** 我推荐选项 A（简单直接）。

---

### 问题 2: 离线 ID 的范围

**选项 A**: 使用负数（-1, -2, -3...）
- ✅ 容易识别是临时 ID
- ✅ 不会与正式 ID 冲突

**选项 B**: 使用大数字（999999, 999998...）
- ⚠️ 可能与未来的正式 ID 冲突

**您的偏好？** 我推荐选项 A（负数）。

---

### 问题 3: ID 修复的时机

**选项 A**: 手动触发（Phase 2 实现）
- 用户点击 "Fix ID" 按钮
- 适合原型阶段

**选项 B**: 自动检测（Phase 2 实现）
- 检测到联网时自动修复
- 更智能但更复杂

**您的偏好？** 我推荐 Phase 1 先不实现，Phase 2 再做。

---

### 问题 4: 是否需要 GUID 作为后备？

**场景**: 如果 ID 分配失败怎么办？

**选项 A**: 使用 GUID
```cpp
int32 StageID = FGuid::NewGuid().A;  // 使用 GUID 的一部分
```

**选项 B**: 使用时间戳
```cpp
int32 StageID = FDateTime::Now().ToUnixTimestamp();
```

**选项 C**: 不需要后备，失败就失败
- 简单，但不够健壮

**您的偏好？** 我推荐选项 C（Phase 1 保持简单）。

---

## 💡 我的推荐

### 立即实施（Phase 3A）

```
✅ 使用 StageID 和 ActID 命名 DataLayer
✅ StageID 从 1 开始
✅ 离线 ID 使用负数
✅ 暂不实现 ID 修复功能
✅ 暂不实现 GUID 后备
```

### 配置总结

| 配置项 | 推荐值 |
|--------|--------|
| StageID 起始值 | 1 |
| ActID 起始值 | 1 |
| 离线 ID 范围 | 负数 (-1, -2...) |
| ID 修复 | Phase 2 实现 |
| GUID 后备 | 不实现 |

---

## 📝 实施步骤

### 步骤 1: 添加 ID 分配函数
1. 在 `StageEditorController.h` 中声明
2. 在 `StageEditorController.cpp` 中实现
3. 测试

### 步骤 2: 修改 DataLayer 创建
1. 更新 `CreateDataLayerForStage`
2. 更新 `CreateDataLayerForAct`
3. 测试

### 步骤 3: 集成到创建流程
1. 更新 `CreateNewStage` 分配 StageID
2. 更新 `CreateNewAct` 分配 ActID
3. 测试

### 步骤 4: 全面测试
1. 创建多个 Stage
2. 创建多个 Act
3. 验证 DataLayer 名称唯一性
4. 验证 ID 分配正确性

---

## 🎉 总结

**这个方案完美解决了所有问题**：

1. ✅ **唯一性**: ID 系统保证
2. ✅ **简单**: 无需冲突检测
3. ✅ **符合架构**: 完全遵循设计文档
4. ✅ **可扩展**: 支持未来的服务器集成
5. ✅ **用户友好**: DisplayName 和 ID 分离

**您觉得这个方案如何？** 如果同意，我可以立即开始实施！💪
