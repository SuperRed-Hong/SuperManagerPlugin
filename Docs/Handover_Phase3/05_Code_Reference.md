# 05 - 代码参考和关键文件

## 📁 项目文件结构

```
ExtendEditor/
├── Plugins/
│   └── StageEditor/
│       ├── Source/
│       │   ├── StageEditor/                    # 编辑器模块
│       │   │   ├── Public/
│       │   │   │   ├── EditorLogic/
│       │   │   │   │   └── StageEditorController.h      # ⭐ 核心控制器
│       │   │   │   ├── EditorUI/
│       │   │   │   │   └── StageEditorPanel.h           # UI 面板
│       │   │   │   ├── Subsystems/
│       │   │   │   │   └── StageEditorSubsystem.h       # ⏳ 待创建
│       │   │   │   ├── StageEditorModule.h              # 模块定义
│       │   │   │   └── DebugHeader.h                    # 调试工具
│       │   │   ├── Private/
│       │   │   │   ├── EditorLogic/
│       │   │   │   │   └── StageEditorController.cpp    # ⭐ 核心逻辑
│       │   │   │   ├── EditorUI/
│       │   │   │   │   └── StageEditorPanel.cpp
│       │   │   │   ├── Subsystems/
│       │   │   │   │   └── StageEditorSubsystem.cpp     # ⏳ 待创建
│       │   │   │   └── StageEditorModule.cpp
│       │   │   └── StageEditor.Build.cs                 # 模块依赖
│       │   └── StageEditorRuntime/             # 运行时模块
│       │       ├── Public/
│       │       │   ├── Actors/
│       │       │   │   ├── Stage.h                      # ⭐ Stage Actor
│       │       │   │   └── Prop.h                       # Prop Actor
│       │       │   ├── Components/
│       │       │   │   └── StagePropComponent.h         # Prop 组件
│       │       │   ├── Core/
│       │       │   │   └── StageCoreTypes.h             # ⭐ 核心数据结构
│       │       │   └── StageEditorRuntimeModule.h
│       │       ├── Private/
│       │       │   ├── Actors/
│       │       │   │   ├── Stage.cpp                    # ⭐ Stage 实现
│       │       │   │   └── Prop.cpp
│       │       │   ├── Components/
│       │       │   │   └── StagePropComponent.cpp
│       │       │   └── StageEditorRuntimeModule.cpp
│       │       └── StageEditorRuntime.Build.cs
│       └── Content/                            # 蓝图和资产
└── Docs/
    ├── Artifacts/
    │   ├── detailed_specification.md           # ⭐ 核心规格文档
    │   └── MultiUser_And_DataLayer_Configuration/
    ├── 11.26/                                  # 最近的会话文档
    └── Handover_Phase3/                        # ⭐ 当前交接文档
```

---

## ⭐ 关键文件详解

### 1. 核心数据结构

#### `StageCoreTypes.h`
**路径**: `StageEditorRuntime/Public/Core/StageCoreTypes.h`

**关键结构**:

```cpp
// 层级 ID 结构
struct FStageHierarchicalId
{
    int32 StageID;  // 全局唯一
    int32 ActID;    // Stage 内唯一
    int32 PropID;   // Stage 内唯一
};

// Act 数据结构
struct FAct
{
    FStageHierarchicalId ActID;
    FString DisplayName;
    TMap<int32, int32> PropStateOverrides;
    TObjectPtr<UDataLayerAsset> AssociatedDataLayer;
};
```

**您需要了解**:
- `FStageHierarchicalId` 是整个系统的基础
- `FAct` 包含 Act 的所有数据
- `AssociatedDataLayer` 是 Phase 2 添加的

---

### 2. Stage Actor

#### `Stage.h`
**路径**: `StageEditorRuntime/Public/Actors/Stage.h`

**关键成员**:

```cpp
class AStage : public AActor
{
    // 核心数据
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stage")
    int32 StageID = 0;  // ⭐ 您需要确保这个被正确分配
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stage")
    TArray<FAct> Acts;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stage")
    TMap<int32, TSoftObjectPtr<AActor>> PropRegistry;
    
    // DataLayer 相关
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stage|DataLayer")
    TObjectPtr<UDataLayerAsset> StageDataLayerAsset;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stage|DataLayer")
    FString StageDataLayerName;
    
#if WITH_EDITOR
    // ⏳ 您需要添加这些
    virtual void PostActorCreated() override;
    virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
    
private:
    void RegisterWithSubsystem();
    bool bIsRegistered = false;
#endif
};
```

**您需要修改**:
1. 添加 `PostActorCreated()`
2. 更新 `PostEditChangeProperty()`
3. 添加 `RegisterWithSubsystem()`
4. 添加 `bIsRegistered` 标志

---

### 3. StageEditorController

#### `StageEditorController.h`
**路径**: `StageEditor/Public/EditorLogic/StageEditorController.h`

**关键方法**:

```cpp
class FStageEditorController
{
public:
    // Stage 管理
    AStage* CreateNewStage(const FString& StageName, UClass* StageClass);
    void DeleteStage(AStage* Stage);
    
    // Act 管理
    int32 CreateNewAct(const FString& ActName);
    void DeleteAct(int32 ActID);
    
    // DataLayer 创建
    bool CreateDataLayerForStage(AStage* Stage);  // ⭐ 需要修改
    bool CreateDataLayerForAct(int32 ActID);      // ⭐ 需要修改
    
private:
    // ⏳ 您需要添加
    UStageEditorSubsystem* GetSubsystem() const;
    
    // DataLayer 路径配置
    FString DataLayerAssetFolderPath;
};
```

**您需要修改**:
1. 添加 `GetSubsystem()`
2. 更新 `CreateDataLayerForStage()` 使用 StageID
3. 更新 `CreateDataLayerForAct()` 使用 StageID + ActID

---

### 4. StageEditorSubsystem (待创建)

#### `StageEditorSubsystem.h`
**路径**: `StageEditor/Public/Subsystems/StageEditorSubsystem.h` ⏳ **待创建**

**完整代码参考**: 见 `04_Implementation_Plan.md` 第 1.1 节

**关键点**:
- 继承自 `UEditorSubsystem`
- 使用 `TMap<int32, TWeakObjectPtr<AStage>>` 存储注册表
- 使用 `int32 NextStageID` 作为计数器
- 提供 `RegisterStage`, `UnregisterStage` 等 API

---

## 🔍 代码模式和示例

### 模式 1: 获取 Subsystem

```cpp
UStageEditorSubsystem* FStageEditorController::GetSubsystem() const
{
    if (!GEditor)
    {
        return nullptr;
    }
    
    return GEditor->GetEditorSubsystem<UStageEditorSubsystem>();
}
```

**何时使用**: 在 Controller 中需要访问 Subsystem 时

---

### 模式 2: 注册 Stage

```cpp
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
    
    if (StageID <= 0)
    {
        StageID = Subsystem->RegisterStage(this);
        
        if (StageID > 0)
        {
            bIsRegistered = true;
            UE_LOG(LogTemp, Log, TEXT("✅ Stage '%s' registered with ID: %d"), *GetName(), StageID);
            Modify();  // 标记为脏，确保保存
        }
    }
}
```

**何时使用**: 在 `PostActorCreated` 和 `PostEditChangeProperty` 中

---

### 模式 3: 创建 DataLayer (基于 ID)

```cpp
bool FStageEditorController::CreateDataLayerForStage(AStage* Stage)
{
    if (!Stage) return false;
    
    // 检查 StageID
    if (Stage->StageID <= 0)
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot create DataLayer for Stage with invalid ID"));
        return false;
    }
    
    // 使用 StageID 生成名称
    FString AssetName = FString::Printf(TEXT("DL_Stage_%d"), Stage->StageID);
    
    // 创建 DataLayer
    UDataLayerAsset* NewAsset = CreateDataLayerAsset(AssetName, DataLayerAssetFolderPath);
    if (!NewAsset) return false;
    
    // 关联
    Stage->StageDataLayerAsset = NewAsset;
    Stage->StageDataLayerName = AssetName;
    
    return true;
}
```

**何时使用**: 在创建 Stage/Act 的 DataLayer 时

---

### 模式 4: 使用 FScopedTransaction

```cpp
// 支持 Undo/Redo
const FScopedTransaction Transaction(LOCTEXT("CreateStageDataLayer", "Create Stage DataLayer"));
Stage->Modify();  // 标记对象为可撤销
Stage->StageDataLayerAsset = NewAsset;
```

**何时使用**: 任何修改 Actor 属性的操作

---

### 模式 5: 错误处理和日志

```cpp
if (!Subsystem)
{
    UE_LOG(LogTemp, Error, TEXT("❌ Failed to get Subsystem"));
    DebugHeader::ShowNotifyInfo(TEXT("Error: Subsystem not available"));
    return false;
}

UE_LOG(LogTemp, Log, TEXT("✅ Operation successful: %s"), *Result);
```

**何时使用**: 所有可能失败的操作

---

## 📝 重要的宏和定义

### LOCTEXT_NAMESPACE

```cpp
#define LOCTEXT_NAMESPACE "FStageEditorController"

// 使用
const FScopedTransaction Transaction(LOCTEXT("ActionName", "Display Name"));

#undef LOCTEXT_NAMESPACE
```

**位置**: 每个 .cpp 文件的开头和结尾

---

### WITH_EDITOR

```cpp
#if WITH_EDITOR
    // 仅在编辑器中编译的代码
    virtual void PostActorCreated() override;
#endif
```

**用途**: 区分编辑器代码和运行时代码

---

### UPROPERTY 元数据

```cpp
UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stage")
int32 StageID = 0;

UPROPERTY(VisibleAnywhere, Category = "Stage|DataLayer", meta = (DisplayName = "DataLayer Name"))
FString StageDataLayerName;
```

**常用标记**:
- `EditAnywhere` - 可在任何地方编辑
- `VisibleAnywhere` - 只读显示
- `BlueprintReadOnly` - 蓝图只读
- `Category` - 分类
- `meta = (DisplayName = "...")` - 显示名称

---

## 🔧 常用工具函数

### DebugHeader

```cpp
// 显示通知
DebugHeader::ShowNotifyInfo(TEXT("Message"));

// 显示进度
DebugHeader::ShowProgress(TEXT("Title"), TEXT("Message"), 0.5f);
```

**位置**: `StageEditor/Public/DebugHeader.h`

---

### FString::Printf

```cpp
FString AssetName = FString::Printf(TEXT("DL_Stage_%d"), StageID);
FString Message = FString::Printf(TEXT("Created %d items"), Count);
```

**用途**: 格式化字符串

---

### GEditor

```cpp
if (GEditor)
{
    UStageEditorSubsystem* Subsystem = GEditor->GetEditorSubsystem<UStageEditorSubsystem>();
}
```

**用途**: 访问编辑器全局对象

---

## 📊 数据流图

### Stage 创建流程

```
用户在编辑器中创建 Stage Actor
    ↓
AStage::PostActorCreated()
    ↓
AStage::RegisterWithSubsystem()
    ↓
UStageEditorSubsystem::RegisterStage(this)
    ↓
Subsystem 分配 StageID (从 1 开始)
    ↓
返回 StageID 并设置到 Stage->StageID
    ↓
Stage->Modify() 标记为脏
    ↓
FStageEditorController::CreateDataLayerForStage(Stage)
    ↓
使用 StageID 创建 DataLayer: DL_Stage_{StageID}
```

### Act 创建流程

```
用户点击 "Add Act" 按钮
    ↓
FStageEditorController::CreateNewAct(ActName)
    ↓
分配 ActID (在 Stage 内递增)
    ↓
创建 FAct 结构体
    ↓
添加到 Stage->Acts 数组
    ↓
FStageEditorController::CreateDataLayerForAct(ActID)
    ↓
使用 StageID + ActID 创建 DataLayer: DL_Act_{StageID}_{ActID}
```

---

## 🎯 关键文件修改点

### 需要创建的文件 (2 个)
1. `StageEditor/Public/Subsystems/StageEditorSubsystem.h`
2. `StageEditor/Private/Subsystems/StageEditorSubsystem.cpp`

### 需要修改的文件 (4 个)
1. `StageEditorRuntime/Public/Actors/Stage.h`
   - 添加 `PostActorCreated`, `RegisterWithSubsystem`, `bIsRegistered`

2. `StageEditorRuntime/Private/Actors/Stage.cpp`
   - 实现注册逻辑

3. `StageEditor/Public/EditorLogic/StageEditorController.h`
   - 添加 `GetSubsystem()`

4. `StageEditor/Private/EditorLogic/StageEditorController.cpp`
   - 更新 `CreateDataLayerForStage` 和 `CreateDataLayerForAct`

---

**有了这些代码参考，您可以开始实施了！** 💪
