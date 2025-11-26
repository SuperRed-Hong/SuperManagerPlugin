# Phase 3: DataLayer Creation Uniqueness - 实施方案讨论

## 📋 目标

防止 DataLayer 名称冲突，确保在多用户协作环境中每个 DataLayer 都有唯一的名称。

---

## 🤔 问题场景

### 场景 1: 多用户同时创建同名 Stage
```
用户 A: 创建 Stage "MainStage"
用户 B: 同时创建 Stage "MainStage"
```

**当前行为**:
- 第一个用户创建 `DL_Stage_MainStage`
- 第二个用户尝试创建同名 DataLayer
- 由于已存在，直接返回现有资产（第 714-718 行）

**问题**:
- ✅ 不会崩溃（已有检查）
- ❌ 两个不同的 Stage 共享同一个 DataLayer（错误！）
- ❌ 可能导致数据混乱

### 场景 2: 用户创建同名 Act
```
Stage "MainStage" 下:
  - Act 1: "CameraAct"
  - Act 2: "CameraAct" (同名)
```

**当前行为**:
- 第一个 Act 创建 `DL_Act_MainStage_1_CameraAct`
- 第二个 Act 尝试创建 `DL_Act_MainStage_2_CameraAct`
- 如果 ActID 不同，名称会不同 ✅
- 但如果用户手动重命名 DataLayer，可能冲突 ❌

---

## 💡 解决方案选项

### 方案 A: 自动递增后缀 ⭐ **推荐**

#### 核心思路
当检测到名称冲突时，自动添加递增的数字后缀。

#### 命名规则
```
原始名称: DL_Stage_MainStage
冲突时:
  - DL_Stage_MainStage_2
  - DL_Stage_MainStage_3
  - DL_Stage_MainStage_4
  ...
```

#### 优点
- ✅ 简单直观
- ✅ 用户容易理解
- ✅ 实现相对简单
- ✅ 符合 UE 的命名习惯（类似 Actor 重命名）

#### 缺点
- ⚠️ 名称可能变长
- ⚠️ 用户需要手动识别哪个是哪个

#### 实现伪代码
```cpp
UDataLayerAsset* CreateUniqueDataLayerAsset(const FString& BaseAssetName, const FString& FolderPath)
{
    FString AssetName = BaseAssetName;
    int32 Suffix = 2;
    
    // 检查是否存在
    while (DoesDataLayerAssetExist(AssetName, FolderPath))
    {
        AssetName = FString::Printf(TEXT("%s_%d"), *BaseAssetName, Suffix);
        Suffix++;
    }
    
    // 创建资产
    return CreateDataLayerAsset(AssetName, FolderPath);
}
```

---

### 方案 B: 使用 GUID 后缀

#### 核心思路
在名称后添加短 GUID 确保唯一性。

#### 命名规则
```
原始名称: DL_Stage_MainStage
冲突时:
  - DL_Stage_MainStage_A3F2
  - DL_Stage_MainStage_B7E9
  ...
```

#### 优点
- ✅ 绝对唯一
- ✅ 不需要检查现有资产
- ✅ 性能更好（不需要循环检查）

#### 缺点
- ❌ 名称不直观
- ❌ 用户难以理解
- ❌ 不符合 UE 习惯

#### 实现伪代码
```cpp
UDataLayerAsset* CreateUniqueDataLayerAsset(const FString& BaseAssetName, const FString& FolderPath)
{
    FString ShortGUID = FGuid::NewGuid().ToString().Left(4);
    FString AssetName = FString::Printf(TEXT("%s_%s"), *BaseAssetName, *ShortGUID);
    
    return CreateDataLayerAsset(AssetName, FolderPath);
}
```

---

### 方案 C: 混合方案（递增 + GUID）

#### 核心思路
优先使用递增后缀，如果超过一定次数（如 10 次）则使用 GUID。

#### 命名规则
```
第 1-10 次冲突: DL_Stage_MainStage_2 到 DL_Stage_MainStage_11
第 11+ 次冲突: DL_Stage_MainStage_A3F2
```

#### 优点
- ✅ 平衡可读性和唯一性
- ✅ 处理极端情况

#### 缺点
- ⚠️ 实现复杂
- ⚠️ 规则不一致

---

### 方案 D: 提示用户手动重命名

#### 核心思路
检测到冲突时，提示用户手动输入新名称。

#### 优点
- ✅ 用户完全控制
- ✅ 名称有意义

#### 缺点
- ❌ 打断工作流
- ❌ 不适合自动化
- ❌ 多用户协作时不友好

---

## 🎯 我的推荐：方案 A（自动递增后缀）

### 理由

1. **符合 UE 习惯**
   - UE 在复制 Actor 时使用类似命名：`Actor_2`, `Actor_3`
   - 用户已经熟悉这种模式

2. **实现简单**
   - 代码逻辑清晰
   - 易于维护和调试

3. **用户友好**
   - 名称直观，容易理解
   - 可以通过数字后缀快速识别

4. **性能可接受**
   - 虽然需要循环检查，但 DataLayer 数量通常不多
   - 可以通过缓存优化

---

## 🔧 详细实现方案（方案 A）

### 1. 添加辅助函数

#### 1.1 检查 DataLayer 是否存在
```cpp
/**
 * 检查指定名称的 DataLayer 资产是否已存在
 * @param AssetName - 资产名称（不含路径）
 * @param FolderPath - 文件夹路径
 * @return 如果存在返回 true
 */
bool FStageEditorController::DoesDataLayerAssetExist(const FString& AssetName, const FString& FolderPath) const
{
    FString PackagePath = FolderPath / AssetName;
    
    // 方法 1: 尝试加载（简单但可能触发加载）
    UDataLayerAsset* ExistingAsset = LoadObject<UDataLayerAsset>(nullptr, *PackagePath, nullptr, LOAD_NoWarn | LOAD_Quiet);
    if (ExistingAsset)
    {
        return true;
    }
    
    // 方法 2: 查询 AssetRegistry（推荐，不会触发加载）
    FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
    IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();
    
    FAssetData AssetData = AssetRegistry.GetAssetByObjectPath(FSoftObjectPath(PackagePath));
    return AssetData.IsValid();
}
```

#### 1.2 生成唯一名称
```cpp
/**
 * 生成唯一的 DataLayer 资产名称
 * @param BaseAssetName - 基础名称
 * @param FolderPath - 文件夹路径
 * @return 唯一的资产名称
 */
FString FStageEditorController::GenerateUniqueDataLayerName(const FString& BaseAssetName, const FString& FolderPath) const
{
    FString AssetName = BaseAssetName;
    
    // 如果基础名称不存在，直接返回
    if (!DoesDataLayerAssetExist(AssetName, FolderPath))
    {
        return AssetName;
    }
    
    // 否则添加递增后缀
    int32 Suffix = 2;
    const int32 MaxAttempts = 1000; // 防止无限循环
    
    while (Suffix < MaxAttempts)
    {
        AssetName = FString::Printf(TEXT("%s_%d"), *BaseAssetName, Suffix);
        
        if (!DoesDataLayerAssetExist(AssetName, FolderPath))
        {
            UE_LOG(LogTemp, Log, TEXT("Generated unique DataLayer name: %s (original: %s)"), *AssetName, *BaseAssetName);
            return AssetName;
        }
        
        Suffix++;
    }
    
    // 极端情况：超过最大尝试次数，使用 GUID
    FString ShortGUID = FGuid::NewGuid().ToString().Left(8);
    AssetName = FString::Printf(TEXT("%s_%s"), *BaseAssetName, *ShortGUID);
    
    UE_LOG(LogTemp, Warning, TEXT("Exceeded max attempts, using GUID: %s"), *AssetName);
    return AssetName;
}
```

### 2. 修改现有函数

#### 2.1 更新 `CreateDataLayerAsset`
```cpp
UDataLayerAsset* FStageEditorController::CreateDataLayerAsset(const FString& AssetName, const FString& FolderPath)
{
    // 生成唯一名称
    FString UniqueAssetName = GenerateUniqueDataLayerName(AssetName, FolderPath);
    
    // 如果名称被修改，记录日志
    if (UniqueAssetName != AssetName)
    {
        UE_LOG(LogTemp, Warning, TEXT("DataLayer name conflict detected. Original: %s, Unique: %s"), *AssetName, *UniqueAssetName);
        DebugHeader::ShowNotifyInfo(FString::Printf(TEXT("DataLayer renamed to: %s"), *UniqueAssetName));
    }
    
    // 构建包路径
    FString PackagePath = FolderPath / UniqueAssetName;
    FString PackageName = FPackageName::ObjectPathToPackageName(PackagePath);
    
    // 创建包
    UPackage* Package = CreatePackage(*PackageName);
    // ... 其余创建逻辑保持不变 ...
}
```

#### 2.2 更新 `CreateDataLayerForStage`
```cpp
bool FStageEditorController::CreateDataLayerForStage(AStage* Stage)
{
    if (!Stage) return false;
    
    // 生成基础名称
    FString BaseAssetName = FString::Printf(TEXT("DL_Stage_%s"), *Stage->GetName());
    
    // CreateDataLayerAsset 内部会处理唯一性
    UDataLayerAsset* NewAsset = CreateDataLayerAsset(BaseAssetName, DataLayerAssetFolderPath);
    
    // ... 其余逻辑保持不变 ...
}
```

#### 2.3 更新 `CreateDataLayerForAct`
```cpp
bool FStageEditorController::CreateDataLayerForAct(int32 ActID)
{
    // ... 查找 Act 的逻辑 ...
    
    // 生成基础名称
    FString BaseAssetName = FString::Printf(
        TEXT("DL_Act_%s_%d_%s"),
        *Stage->GetName(),
        ActID,
        *TargetAct->DisplayName
    );
    
    // CreateDataLayerAsset 内部会处理唯一性
    UDataLayerAsset* NewAsset = CreateDataLayerAsset(BaseAssetName, DataLayerAssetFolderPath);
    
    // ... 其余逻辑保持不变 ...
}
```

---

## 🤔 需要讨论的问题

### 问题 1: 命名冲突的通知方式

**选项 A**: 静默处理，只在日志中记录
- ✅ 不打断工作流
- ❌ 用户可能不知道发生了什么

**选项 B**: 显示通知
- ✅ 用户知道发生了什么
- ⚠️ 可能有点烦人

**选项 C**: 仅在首次冲突时显示通知
- ✅ 平衡通知和干扰

**您的偏好？** 我推荐选项 C。

---

### 问题 2: 最大尝试次数

**当前设置**: 1000 次

**考虑**:
- DataLayer 数量通常不会超过 100 个
- 1000 次足够安全
- 超过后使用 GUID 作为后备

**您觉得合理吗？** 或者需要调整？

---

### 问题 3: 检查方法的选择

**方法 1**: `LoadObject` (第 714 行当前使用)
- ✅ 简单直接
- ❌ 会触发资产加载（性能影响）

**方法 2**: `AssetRegistry.GetAssetByObjectPath`
- ✅ 不触发加载
- ✅ 性能更好
- ⚠️ 需要 AssetRegistry 已扫描

**您的偏好？** 我推荐方法 2（AssetRegistry）。

---

### 问题 4: 是否需要缓存

**考虑添加缓存**:
```cpp
// 在 FStageEditorController 中添加
TSet<FString> ExistingDataLayerNames; // 缓存已存在的名称

void RefreshDataLayerCache()
{
    // 扫描并缓存所有 DataLayer 名称
}
```

**优点**:
- ✅ 避免重复查询
- ✅ 性能更好

**缺点**:
- ⚠️ 需要维护缓存一致性
- ⚠️ 多用户环境下可能不准确

**您觉得需要吗？** 我觉得暂时可以不加，除非性能成为问题。

---

### 问题 5: 用户手动重命名的处理

**场景**: 用户在 Content Browser 中手动重命名了 DataLayer 资产

**问题**: Stage/Act 中的引用会失效吗？

**答案**: 不会！因为我们使用 `TObjectPtr<UDataLayerAsset>` 引用，UE 会自动处理重定向。

**但是**: 如果用户重命名后，新名称与我们的命名规则冲突，可能会混乱。

**建议**: 在文档中说明不建议手动重命名 DataLayer 资产。

---

## 📝 实施步骤

### 步骤 1: 添加辅助函数
1. 在 `StageEditorController.h` 中声明
2. 在 `StageEditorController.cpp` 中实现
3. 编译测试

### 步骤 2: 修改创建函数
1. 更新 `CreateDataLayerAsset`
2. 测试单独调用

### 步骤 3: 集成到现有流程
1. 验证 `CreateDataLayerForStage` 正常工作
2. 验证 `CreateDataLayerForAct` 正常工作

### 步骤 4: 测试
1. 单用户测试：创建同名 Stage/Act
2. 模拟多用户：手动创建冲突的 DataLayer
3. 边界测试：大量同名资产

---

## 🎯 您的意见？

请告诉我：

1. **方案选择**: 您同意方案 A（自动递增后缀）吗？还是更倾向其他方案？

2. **通知方式**: 选项 A/B/C 哪个更好？

3. **检查方法**: LoadObject 还是 AssetRegistry？

4. **是否需要缓存**: 现在加还是等性能问题出现再加？

5. **其他考虑**: 还有什么我没想到的问题吗？

我会根据您的反馈调整实施方案，然后开始编码！💪
