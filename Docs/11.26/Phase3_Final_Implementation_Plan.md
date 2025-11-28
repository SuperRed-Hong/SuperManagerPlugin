# Phase 3 最终方案：StageEditorSubsystem + ID 注册系统

## 📋 方案确认

### 核心决策
1. ✅ 创建 `UStageEditorSubsystem` 管理 Stage 注册
2. ✅ StageID 从 1 开始
3. ✅ 暂不实现离线修复功能
4. ✅ 不使用 GUID 后备
5. ✅ 所有命名基于 StageID

---

## 🏗️ 架构设计

### 系统组件

```
UStageEditorSubsystem (Editor Subsystem)
├─ Stage 注册表 (TMap<int32, TWeakObjectPtr<AStage>>)
├─ ID 分配器 (NextStageID 计数器)
└─ API
   ├─ RegisterStage(AStage*) → int32 StageID
   ├─ UnregisterStage(int32 StageID)
   ├─ GetStage(int32 StageID) → AStage*
   └─ GetAllStages() → TArray<AStage*>
```

### 工作流程

```
1. 用户创建 Stage Actor
   ↓
2. AStage::PostEditChangeProperty() 检测到新创建
   ↓
3. 调用 Subsystem->RegisterStage(this)
   ↓
4. Subsystem 分配唯一 StageID
   ↓
5. 返回 StageID 并设置到 Stage->StageID
   ↓
6. 后续所有操作使用这个 StageID
```

---

## 📝 详细实现

### 1. 创建 StageEditorSubsystem

#### 1.1 头文件：`StageEditorSubsystem.h`

```cpp
#pragma once

#include "CoreMinimal.h"
#include "EditorSubsystem.h"
#include "StageEditorSubsystem.generated.h"

class AStage;

/**
 * @brief Editor Subsystem for managing Stage registration and ID allocation.
 * 
 * This subsystem maintains a central registry of all Stage actors in the editor
 * and provides unique ID allocation for each Stage.
 */
UCLASS()
class STAGEEDITOR_API UStageEditorSubsystem : public UEditorSubsystem
{
    GENERATED_BODY()

public:
    //----------------------------------------------------------------
    // Lifecycle
    //----------------------------------------------------------------
    
    /** Called when the subsystem is initialized. */
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    
    /** Called when the subsystem is deinitialized. */
    virtual void Deinitialize() override;

public:
    //----------------------------------------------------------------
    // Stage Registration
    //----------------------------------------------------------------
    
    /**
     * Register a Stage actor and allocate a unique StageID.
     * @param Stage - The Stage actor to register
     * @return The allocated StageID, or -1 if registration failed
     */
    UFUNCTION(BlueprintCallable, Category = "Stage Editor")
    int32 RegisterStage(AStage* Stage);
    
    /**
     * Unregister a Stage actor.
     * @param StageID - The ID of the Stage to unregister
     */
    UFUNCTION(BlueprintCallable, Category = "Stage Editor")
    void UnregisterStage(int32 StageID);
    
    /**
     * Get a Stage actor by its ID.
     * @param StageID - The ID of the Stage to retrieve
     * @return The Stage actor, or nullptr if not found
     */
    UFUNCTION(BlueprintCallable, Category = "Stage Editor")
    AStage* GetStage(int32 StageID) const;
    
    /**
     * Get all registered Stage actors.
     * @return Array of all registered Stages
     */
    UFUNCTION(BlueprintCallable, Category = "Stage Editor")
    TArray<AStage*> GetAllStages() const;
    
    /**
     * Check if a StageID is already registered.
     * @param StageID - The ID to check
     * @return True if the ID is registered
     */
    UFUNCTION(BlueprintCallable, Category = "Stage Editor")
    bool IsStageIDRegistered(int32 StageID) const;

private:
    //----------------------------------------------------------------
    // Internal State
    //----------------------------------------------------------------
    
    /** Registry of all Stages. Key: StageID, Value: Weak pointer to Stage actor */
    TMap<int32, TWeakObjectPtr<AStage>> StageRegistry;
    
    /** Next available StageID to allocate */
    int32 NextStageID = 1;
    
    /**
     * Allocate a new unique StageID.
     * @return A unique StageID
     */
    int32 AllocateStageID();
    
    /**
     * Clean up invalid entries in the registry (garbage collected actors).
     */
    void CleanupRegistry();
};
```

#### 1.2 实现文件：`StageEditorSubsystem.cpp`

```cpp
#include "Subsystems/StageEditorSubsystem.h"
#include "Actors/Stage.h"
#include "DebugHeader.h"

void UStageEditorSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Log, TEXT("✅ StageEditorSubsystem initialized"));
    
    // 重置 ID 计数器
    NextStageID = 1;
    StageRegistry.Empty();
}

void UStageEditorSubsystem::Deinitialize()
{
    UE_LOG(LogTemp, Log, TEXT("StageEditorSubsystem deinitialized"));
    
    StageRegistry.Empty();
    
    Super::Deinitialize();
}

int32 UStageEditorSubsystem::RegisterStage(AStage* Stage)
{
    if (!Stage)
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot register null Stage"));
        return -1;
    }
    
    // 清理无效条目
    CleanupRegistry();
    
    // 检查是否已经注册
    if (Stage->StageID > 0 && IsStageIDRegistered(Stage->StageID))
    {
        // 已经注册，返回现有 ID
        UE_LOG(LogTemp, Warning, TEXT("Stage already registered with ID: %d"), Stage->StageID);
        return Stage->StageID;
    }
    
    // 分配新 ID
    int32 NewStageID = AllocateStageID();
    
    // 注册到表
    StageRegistry.Add(NewStageID, Stage);
    
    UE_LOG(LogTemp, Log, TEXT("✅ Registered Stage '%s' with ID: %d"), *Stage->GetName(), NewStageID);
    
    return NewStageID;
}

void UStageEditorSubsystem::UnregisterStage(int32 StageID)
{
    if (StageRegistry.Remove(StageID) > 0)
    {
        UE_LOG(LogTemp, Log, TEXT("Unregistered Stage ID: %d"), StageID);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Attempted to unregister non-existent Stage ID: %d"), StageID);
    }
}

AStage* UStageEditorSubsystem::GetStage(int32 StageID) const
{
    const TWeakObjectPtr<AStage>* StagePtr = StageRegistry.Find(StageID);
    if (StagePtr && StagePtr->IsValid())
    {
        return StagePtr->Get();
    }
    return nullptr;
}

TArray<AStage*> UStageEditorSubsystem::GetAllStages() const
{
    TArray<AStage*> Stages;
    
    for (const TPair<int32, TWeakObjectPtr<AStage>>& Pair : StageRegistry)
    {
        if (Pair.Value.IsValid())
        {
            Stages.Add(Pair.Value.Get());
        }
    }
    
    return Stages;
}

bool UStageEditorSubsystem::IsStageIDRegistered(int32 StageID) const
{
    const TWeakObjectPtr<AStage>* StagePtr = StageRegistry.Find(StageID);
    return StagePtr && StagePtr->IsValid();
}

int32 UStageEditorSubsystem::AllocateStageID()
{
    // 简单递增分配
    int32 AllocatedID = NextStageID;
    NextStageID++;
    
    UE_LOG(LogTemp, Verbose, TEXT("Allocated StageID: %d"), AllocatedID);
    
    return AllocatedID;
}

void UStageEditorSubsystem::CleanupRegistry()
{
    // 移除已被垃圾回收的 Stage
    TArray<int32> InvalidIDs;
    
    for (const TPair<int32, TWeakObjectPtr<AStage>>& Pair : StageRegistry)
    {
        if (!Pair.Value.IsValid())
        {
            InvalidIDs.Add(Pair.Key);
        }
    }
    
    for (int32 ID : InvalidIDs)
    {
        StageRegistry.Remove(ID);
        UE_LOG(LogTemp, Verbose, TEXT("Cleaned up invalid Stage ID: %d"), ID);
    }
}
```

---

### 2. 修改 Stage Actor

#### 2.1 在 `Stage.h` 中添加

```cpp
#if WITH_EDITOR
public:
    /**
     * Called when a property is changed in the editor.
     * Used to auto-register the Stage when first created.
     */
    virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
    
    /**
     * Called after the actor is constructed in the editor.
     * Used to auto-register the Stage.
     */
    virtual void PostActorCreated() override;

private:
    /** Flag to track if this Stage has been registered with the Subsystem */
    bool bIsRegistered = false;
#endif
```

#### 2.2 在 `Stage.cpp` 中实现

```cpp
#if WITH_EDITOR
#include "Subsystems/StageEditorSubsystem.h"
#include "Editor.h"

void AStage::PostActorCreated()
{
    Super::PostActorCreated();
    
    // 在编辑器中创建时自动注册
    if (GEditor && !bIsRegistered)
    {
        RegisterWithSubsystem();
    }
}

void AStage::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
    Super::PostEditChangeProperty(PropertyChangedEvent);
    
    // 确保已注册
    if (!bIsRegistered)
    {
        RegisterWithSubsystem();
    }
    
    // 如果 StageDataLayerAsset 改变，同步名称
    if (PropertyChangedEvent.GetPropertyName() == GET_MEMBER_NAME_CHECKED(AStage, StageDataLayerAsset))
    {
        if (StageDataLayerAsset)
        {
            StageDataLayerName = StageDataLayerAsset->GetName();
        }
    }
}

void AStage::RegisterWithSubsystem()
{
    if (bIsRegistered)
    {
        return;
    }
    
    UStageEditorSubsystem* Subsystem = GEditor->GetEditorSubsystem<UStageEditorSubsystem>();
    if (!Subsystem)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to get StageEditorSubsystem"));
        return;
    }
    
    // 如果还没有 StageID，请求分配
    if (StageID <= 0)
    {
        StageID = Subsystem->RegisterStage(this);
        
        if (StageID > 0)
        {
            bIsRegistered = true;
            UE_LOG(LogTemp, Log, TEXT("✅ Stage '%s' registered with ID: %d"), *GetName(), StageID);
            
            // 标记为脏，确保保存
            Modify();
        }
    }
    else
    {
        // 已有 ID，只需注册
        Subsystem->RegisterStage(this);
        bIsRegistered = true;
    }
}

void AStage::BeginDestroy()
{
    // 取消注册
    if (bIsRegistered && GEditor)
    {
        UStageEditorSubsystem* Subsystem = GEditor->GetEditorSubsystem<UStageEditorSubsystem>();
        if (Subsystem)
        {
            Subsystem->UnregisterStage(StageID);
        }
    }
    
    Super::BeginDestroy();
}
#endif
```

---

### 3. 修改 DataLayer 创建逻辑

#### 3.1 更新 `CreateDataLayerForStage`

```cpp
bool FStageEditorController::CreateDataLayerForStage(AStage* Stage)
{
    if (!Stage) return false;
    
    // 确保 Stage 已注册并有有效 ID
    if (Stage->StageID <= 0)
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot create DataLayer for Stage with invalid ID: %d"), Stage->StageID);
        DebugHeader::ShowNotifyInfo(TEXT("Error: Stage must be registered first"));
        return false;
    }
    
    // 使用 StageID 生成唯一名称
    FString AssetName = FString::Printf(TEXT("DL_Stage_%d"), Stage->StageID);
    
    // 创建 DataLayer 资产
    UDataLayerAsset* NewAsset = CreateDataLayerAsset(AssetName, DataLayerAssetFolderPath);
    if (!NewAsset) return false;
    
    // 关联到 Stage
    const FScopedTransaction Transaction(LOCTEXT("CreateStageDataLayer", "Create Stage DataLayer"));
    Stage->Modify();
    Stage->StageDataLayerAsset = NewAsset;
    Stage->StageDataLayerName = AssetName;
    
    UE_LOG(LogTemp, Log, TEXT("✅ Created Stage DataLayer: %s for Stage ID: %d"), *AssetName, Stage->StageID);
    DebugHeader::ShowNotifyInfo(FString::Printf(TEXT("Created DataLayer: %s"), *AssetName));
    
    return true;
}
```

#### 3.2 更新 `CreateDataLayerForAct`

```cpp
bool FStageEditorController::CreateDataLayerForAct(int32 ActID)
{
    AStage* Stage = GetActiveStage();
    if (!Stage) return false;
    
    // 确保 Stage 有有效 ID
    if (Stage->StageID <= 0)
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot create DataLayer for Act: Stage has invalid ID"));
        return false;
    }
    
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
    
    // 设置父 DataLayer
    UDataLayerInstance* ParentInstance = FindStageDataLayerInstance(Stage);
    if (ParentInstance)
    {
        UDataLayerInstance* NewInstance = GetOrCreateInstanceForAsset(NewAsset);
        if (NewInstance)
        {
            NewInstance->SetParent(ParentInstance);
        }
    }
    
    // 关联到 Act
    const FScopedTransaction Transaction(LOCTEXT("CreateActDataLayer", "Create Act DataLayer"));
    Stage->Modify();
    TargetAct->AssociatedDataLayer = NewAsset;
    
    UE_LOG(LogTemp, Log, TEXT("✅ Created Act DataLayer: %s for Act ID: %d"), *AssetName, ActID);
    
    return true;
}
```

---

### 4. 修改 Controller 使用 Subsystem

#### 4.1 在 `StageEditorController.h` 中添加

```cpp
private:
    /**
     * Get the StageEditorSubsystem instance.
     * @return The subsystem, or nullptr if not available
     */
    UStageEditorSubsystem* GetSubsystem() const;
```

#### 4.2 在 `StageEditorController.cpp` 中实现

```cpp
#include "Subsystems/StageEditorSubsystem.h"

UStageEditorSubsystem* FStageEditorController::GetSubsystem() const
{
    if (!GEditor)
    {
        return nullptr;
    }
    
    return GEditor->GetEditorSubsystem<UStageEditorSubsystem>();
}
```

#### 4.3 更新 `CreateNewStage`

```cpp
AStage* FStageEditorController::CreateNewStage(const FString& StageName, UClass* StageClass)
{
    // ... 现有的创建逻辑 ...
    
    // Stage 会在 PostActorCreated 中自动注册到 Subsystem
    // 我们只需要验证注册成功
    
    if (NewStage->StageID <= 0)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to register Stage with Subsystem"));
        DebugHeader::ShowNotifyInfo(TEXT("Error: Failed to register Stage"));
        return nullptr;
    }
    
    UE_LOG(LogTemp, Log, TEXT("✅ Created Stage '%s' with ID: %d"), *StageName, NewStage->StageID);
    
    // 创建 Stage DataLayer
    if (IsWorldPartitionActive())
    {
        CreateDataLayerForStage(NewStage);
    }
    
    return NewStage;
}
```

---

## 📁 文件清单

### 新增文件

1. **StageEditorSubsystem.h**
   - 路径: `Plugins/StageEditor/Source/StageEditor/Public/Subsystems/`
   - 内容: Subsystem 头文件

2. **StageEditorSubsystem.cpp**
   - 路径: `Plugins/StageEditor/Source/StageEditor/Private/Subsystems/`
   - 内容: Subsystem 实现

### 修改文件

1. **Stage.h**
   - 添加: `PostActorCreated`, `PostEditChangeProperty`, `RegisterWithSubsystem`

2. **Stage.cpp**
   - 实现: 自动注册逻辑

3. **StageEditorController.h**
   - 添加: `GetSubsystem()`

4. **StageEditorController.cpp**
   - 修改: `CreateDataLayerForStage`, `CreateDataLayerForAct`, `CreateNewStage`

5. **StageEditor.Build.cs**
   - 无需修改（Subsystem 在同一模块内）

---

## 🧪 测试计划

### 测试用例 1: Stage 注册
```
1. 创建新 Stage Actor
2. 验证 StageID > 0
3. 验证 Subsystem 中已注册
4. 验证 DataLayer 名称为 DL_Stage_{StageID}
```

### 测试用例 2: 多个 Stage
```
1. 创建 Stage A (应该得到 ID 1)
2. 创建 Stage B (应该得到 ID 2)
3. 创建 Stage C (应该得到 ID 3)
4. 验证所有 ID 唯一
5. 验证所有 DataLayer 名称唯一
```

### 测试用例 3: Act 创建
```
1. 在 Stage (ID=1) 中创建 Act
2. 验证 Act DataLayer 名称为 DL_Act_1_{ActID}
3. 验证父子关系正确
```

### 测试用例 4: Stage 删除
```
1. 创建 Stage (ID=1)
2. 删除 Stage
3. 验证 Subsystem 中已取消注册
4. 创建新 Stage
5. 验证新 Stage 得到 ID 2（不重用）
```

---

## 📊 实施步骤

### 步骤 1: 创建 Subsystem (30 分钟)
1. [ ] 创建 `StageEditorSubsystem.h`
2. [ ] 创建 `StageEditorSubsystem.cpp`
3. [ ] 编译测试

### 步骤 2: 修改 Stage Actor (30 分钟)
1. [ ] 添加 `PostActorCreated` 和 `PostEditChangeProperty`
2. [ ] 实现 `RegisterWithSubsystem`
3. [ ] 编译测试

### 步骤 3: 更新 Controller (30 分钟)
1. [ ] 添加 `GetSubsystem()`
2. [ ] 更新 `CreateDataLayerForStage`
3. [ ] 更新 `CreateDataLayerForAct`
4. [ ] 更新 `CreateNewStage`
5. [ ] 编译测试

### 步骤 4: 全面测试 (30 分钟)
1. [ ] 测试用例 1-4
2. [ ] 修复发现的问题

**总预计时间**: 2 小时

---

## ✅ 验收标准

- [ ] Subsystem 正确初始化
- [ ] Stage 创建时自动注册
- [ ] StageID 从 1 开始递增
- [ ] DataLayer 名称基于 StageID
- [ ] 多个 Stage 的 ID 和 DataLayer 名称都唯一
- [ ] Stage 删除时正确取消注册
- [ ] 编译无错误无警告
- [ ] 所有测试用例通过

---

**准备好开始实施了吗？** 🚀
