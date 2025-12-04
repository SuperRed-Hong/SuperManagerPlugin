# Phase 5: 导入逻辑与预览对话框

> 完成日期: 2025-11-29
> 状态: ✅ 完成

---

## 目标

实现从 DataLayer 导入 Stage-Act-Prop 架构的功能，包括预览对话框让用户确认导入内容。

---

## 实现内容

### 1. 导入逻辑类 (DataLayerImporter)

**文件：**
- `Public/DataLayerSync/DataLayerImporter.h`
- `Private/DataLayerSync/DataLayerImporter.cpp`

**核心结构：**

```cpp
// 导入预览项 - 使用扁平化设计（避免 UHT 递归数组限制）
USTRUCT(BlueprintType)
struct FDataLayerImportPreviewItem
{
    FString DisplayName;      // 显示名称
    FString SUIDDisplay;      // SUID 显示 (S:1, A:1.1 等)
    FString ItemType;         // 类型 (Stage, Act, Props)
    UDataLayerAsset* DataLayerAsset;
    int32 Depth = 0;          // 层级深度：0=Stage, 1=Act, 2=Props
    int32 ActorCount = 0;     // Actor 数量（仅 Props 类型）
};

// 导入预览结果
USTRUCT(BlueprintType)
struct FDataLayerImportPreview
{
    bool bIsValid = false;
    FText ErrorMessage;
    FString SourceDataLayerName;
    TArray<FDataLayerImportPreviewItem> Items;  // 扁平化列表
    int32 StageCount, ActCount, PropCount;
};

// 导入执行结果
USTRUCT(BlueprintType)
struct FDataLayerImportResult
{
    bool bSuccess = false;
    FText ErrorMessage;
    AStage* CreatedStage;
    int32 CreatedActCount = 0;
    int32 RegisteredPropCount = 0;
};
```

**API：**

```cpp
class FDataLayerImporter
{
public:
    // 生成导入预览
    static FDataLayerImportPreview GeneratePreview(UDataLayerAsset* StageDataLayerAsset, UWorld* World);

    // 执行导入
    static FDataLayerImportResult ExecuteImport(UDataLayerAsset* StageDataLayerAsset, UWorld* World);

    // 验证是否可导入
    static bool CanImport(UDataLayerAsset* DataLayerAsset, UWorld* World, FText& OutErrorMessage);
};
```

### 2. 导入预览对话框 (SDataLayerImportPreviewDialog)

**文件：**
- `Public/DataLayerSync/SDataLayerImportPreviewDialog.h`
- `Private/DataLayerSync/SDataLayerImportPreviewDialog.cpp`

**功能：**
- 模态对话框显示将要创建的层级结构
- ListView 多列显示：Item Label（带缩进）、SUID
- Import / Cancel 按钮
- 错误状态显示

**使用方式：**

```cpp
bool bImported = SDataLayerImportPreviewDialog::ShowDialog(DataLayerAsset);
```

### 3. 集成到 SStageDataLayerBrowser

**修改文件：**
- `Public/DataLayerSync/SStageDataLayerBrowser.h`
- `Private/DataLayerSync/SStageDataLayerBrowser.cpp`

**新增方法：**

```cpp
// Import Selected 按钮回调
FReply OnImportSelectedClicked();

// 检查是否可以导入
bool CanImportSelected() const;

// 获取可导入的 DataLayerAsset
UDataLayerAsset* GetSelectedImportableDataLayerAsset() const;
```

---

## 技术要点

### UHT 递归数组限制

**问题：** 原设计使用 `TArray<FDataLayerImportPreviewItem> Children` 导致 UHT 错误：
```
Struct recursion via arrays is unsupported
```

**解决方案：** 改用扁平化列表 + `Depth` 字段表示层级：
- Depth 0 = Stage
- Depth 1 = Act
- Depth 2 = Props

### SUID 分配

**问题：** `FAct` 使用 `SUID` 结构而非直接 `ActID` 字段。

**解决方案：**
```cpp
FAct NewAct;
NewAct.SUID = FSUID::MakeActID(Stage->SUID.StageID, ActIndex + 1);
```

### const_cast 使用

**问题：** `UDataLayerInstance::GetAsset()` 返回 `const UDataLayerAsset*`，但存储需要非 const。

**解决方案：** 在确定安全的上下文中使用 `const_cast`。

---

## 导入流程

```
1. 用户选择 DL_Stage_* 命名的 DataLayer
2. 点击 "Import Selected" 按钮
3. 显示预览对话框，展示将创建的层级结构
4. 用户确认后执行导入：
   a. 创建 Stage Actor
   b. 分配 SUID
   c. 关联 Stage DataLayer
   d. 遍历子 DataLayer，创建 Acts
   e. 遍历 Act 中的 Actors，注册为 Props
5. 刷新缓存
```

---

*完成日期: 2025-11-29 17:00*
