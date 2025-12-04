# 【编辑器】《FStageEditorController详细设计文档》

---

# **《FStageEditorController详细设计文档》**

| 文档版本 | 作者 | 日期 | 备注 |
| --- | --- | --- | --- |
| 1.0 | Gemini (UE Tools Programmer) | 2025-09-23 | 初始版本创建。 |
| **1.1** | Gemini (UE Tools Programmer) | 2025-09-23 | **采纳建议，将所有修改操作的API返回值从 `void` 改为 `bool`，以增强健壮性和错误处理能力。** |

**关联文档:**

- 《“关卡编辑器”系统需求说明书 (PRD) V2.1》
- 《“关卡编辑器”系统概要设计文档 V4.1》
- 《ID核心数据结构与本地索引机制详细设计文档 V1.1》
- 《“关卡编辑器”系统: AStage Actor数据结构与核心逻辑详细设计 V4.2》

## 1. 引言

本文档旨在为“动态舞台”系统的**控制器（Controller）**—— `FStageEditorController` 类提供详尽的技术设计。

根据《概要设计文档 V4.1》中定义的MVC（模型-视图-控制器）架构，`FStageEditorController` 扮演着连接**视图**（`SStageEditorPanel` 等UI界面）与**模型**（`AStage` Actor及其数据）的唯一桥梁。

它的核心职责是：

1. **封装业务逻辑**：提供一组清晰的公共API，供UI层调用以执行所有数据修改操作。
2. **保证数据安全**：通过持有对当前编辑的 `AStage` Actor 的弱引用来安全地访问模型数据。
3. **实现事务性操作**：利用Unreal Engine的事务系统，确保所有对模型的修改都是**可撤销/重做**的，这是工具健壮性的关键。
4. **解耦视图与模型**：在数据修改完成后，通过广播委托来通知UI层进行刷新，从而实现逻辑与表现的分离。

本设计将严格依赖《AStage Actor详细设计》和《ID核心数据结构详细设计》，直接操作其中定义的C++数据结构。

## 2. 类定义 (FStageEditorController.h)

`FStageEditorController` 是一个纯C++类，不继承自 `UObject`。它的生命周期将由 `TSharedPtr` 进行管理，以适应在Slate UI框架中的使用。

```
// FStageEditorController.h

#pragma once

#include "CoreMinimal.h"
#include "Delegates/Delegate.h"
// 包含ID定义，确保编译器知道FStageHierarchicalId是什么
#include "StageCoreTypes.h"

// 前向声明，避免不必要的头文件包含
class AStage;
class AActor;

/**
 * @enum EStageModelUpdateType
 * @brief 定义数据模型更新的粒度，用于通知UI进行精细化刷新。
 */
enum class EStageModelUpdateType
{
        /** 完全刷新，通常在切换编辑的Stage时使用 */
        FullRefresh,
        /** Act列表发生变化 (新增、删除) */
        ActList,
        /** Prop注册列表发生变化 (注册、注销) */
        PropList,
        /** 某个Act下的PropState值被修改 */
        PropState,
};

// 声明数据模型变更的委托
DECLARE_MULTICAST_DELEGATE_OneParam(FOnStageModelChanged, EStageModelUpdateType /*UpdateType*/);

/**
 * @class FStageEditorController
 * @brief "动态舞台"编辑器的核心逻辑控制器 (MVC中的C)
 * 负责处理所有来自UI的请求，并以事务性的方式修改AStage Actor (模型)。
 */
class FStageEditorController : public TSharedFromThis<FStageEditorController>
{
public:
        FStageEditorController();
        ~FStageEditorController();

        //----------------------------------------------------------------
        // 核心API
        //----------------------------------------------------------------

        /** 设置当前控制器操作的目标AStage Actor */
        void SetEditingStage(AStage* NewStage);

        /**
         * 在当前Stage中创建一个新的Act。
         * @return bool - 操作成功返回true，否则返回false。
         */
        bool CreateNewAct();

        /**
         * 从当前Stage中删除一个指定的Act。
         * @return bool - 操作成功返回true，否则返回false。
         */
        bool DeleteAct(const FStageHierarchicalId& ActID);

        /**
         * 将一组选中的Actor注册为当前Stage的Prop。
         * @return bool - 操作成功返回true，否则返回false。
         */
        bool RegisterProps(const TArray<AActor*>& ActorsToRegister);

        /**
         * 从当前Stage中注销一组指定的Prop。
         * @return bool - 操作成功返回true，否则返回false。
         */
        bool UnregisterProps(const TArray<FStageHierarchicalId>& PropIDs);

        /**
         * 为指定的Act中的指定Prop设置其目标PropState值。
         * @return bool - 操作成功返回true，否则返回false。
         */
        bool SetPropStateForAct(const FStageHierarchicalId& ActID, const FStageHierarchicalId& PropID, int32 NewState);
        //----------------------------------------------------------------
        // 委托
        //----------------------------------------------------------------

        /** 当数据模型被修改时广播此委托 */
        FOnStageModelChanged OnModelChanged;

private:
        /**
         * @brief 当前正在编辑的AStage Actor的弱引用。
         * 使用 TWeakObjectPtr 是为了防止悬垂指针。AStage是一个UObject，可能在任何时候被编辑器
         * (例如，用户删除、切换关卡)垃圾回收。弱指针允许我们在访问前安全地检查其有效性，
         * 从而避免崩溃，并确保了Controller的健壮性。
         */
        TWeakObjectPtr<AStage> EditingStage;
};

```

## 3. 核心API详细设计

### 3.1 `SetEditingStage`

```
void SetEditingStage(AStage* NewStage);

```

- **参数:**
    - `AStage* NewStage`: 用户在编辑器中新选中的、要进行编辑的 `AStage` Actor 实例，可以为 `nullptr` (表示取消选择)。
- **返回值:** `void`
- **核心职责:** 切换控制器当前操作的目标 `AStage`。这是所有其他编辑操作的前提。执行此操作后，会广播一次 `FullRefresh` 类型的 `OnModelChanged` 委托，通知UI完全重建视图。

### 3.2 `CreateNewAct`

```
bool CreateNewAct();

```

- **参数:** 无
- **返回值: `bool` - 如果操作成功执行则返回 `true`，如果因前置条件不满足（如 `EditingStage` 无效）而中止则返回 `false`。**
- **核心职责:** 在当前 `EditingStage` 的 `Acts` 数组中添加一个新的 `FAct` 实例。控制器负责调用 `AStage` 的内部逻辑来分配一个唯一的 `ActID` (通过递增 `NextAvailableActId`)，并初始化新 `FAct` 的数据。

### 3.3 `DeleteAct`

```
bool DeleteAct(const FStageHierarchicalId& ActID);

```

- **参数:**
    - `const FStageHierarchicalId& ActID`: 需要被删除的 `Act` 的唯一ID。
- **返回值:** `bool` - 如果操作成功执行则返回 `true`，否则返回 `false`。
- **核心职责:** 从当前 `EditingStage` 的 `Acts` 数组中查找并移除与 `ActID` 匹配的 `FAct`。

### 3.4 `RegisterProps`

```
bool RegisterProps(const TArray<AActor*>& ActorsToRegister);

```

- **参数:**
    - `const TArray<AActor*>& ActorsToRegister`: 用户在世界大纲视图中选中的、希望注册为 `Prop` 的 `AActor` 数组。
- **返回值:** `bool` - 如果操作成功执行则返回 `true`，否则返回 `false`。
- **核心职责:** 遍历传入的 `Actor` 数组，为每一个 `Actor`（必须是 `AProp` 的子类）在 `EditingStage` 的 `PropRegistry` 中创建一个条目。控制器将调用 `AStage` 的逻辑来分配唯一的 `PropID`，并创建一个从ID到 `Actor` 实例的稳定映射（`TSoftObjectPtr`）。

### 3.5 `UnregisterProps`

```
void UnregisterProps(const TArray<FStageHierarchicalId>& PropIDs);

```

- **参数:**
    - `const TArray<FStageHierarchicalId>& PropIDs`: 需要被注销的 `Prop` 的ID数组。
- **返回值:** `void`
- **核心职责:** 根据传入的 `PropID` 数组，从 `EditingStage` 的 `PropRegistry` 中移除对应的条目。

### 3.6 `SetPropStateForAct`

```
void SetPropStateForAct(const FStageHierarchicalId& ActID, const FStageHierarchicalId& PropID, int32 NewState);

```

- **参数:**
    - `const FStageHierarchicalId& ActID`: 目标 `Act` 的ID。
    - `const FStageHierarchicalId& PropID`: 目标 `Prop` 的ID。
    - `int32 NewState`: 要为该 `Prop` 在该 `Act` 下设置的新的 `PropState` 整数值。
- **返回值:** `void`
- **核心职责:** 找到 `ActID` 对应的 `FAct`，然后在其内部的 `TargetPropStates` TMap中，更新或添加 `PropID` 到 `NewState` 的映射。

## 4. Undo/Redo事务管理实现方案

这是本设计的核心，保证了编辑器工具的可用性和健壮性。**所有对 **`AStage`** 数据进行修改的公共API（`SetEditingStage`除外）都必须严格遵循以下模式**。

**核心原则:** 使用 `FScopedTransaction` RAII类来自动管理事务的开始和结束。

**实现模式 (以 `CreateNewAct` 为例):**

```
#include "ScopedTransaction.h"

// 定义本地化的文本，用于显示在编辑器的Undo/Redo历史记录中
#define LOCTEXT_NAMESPACE "FStageEditorController"

bool FStageEditorController::CreateNewAct()
{
        // 1. 安全性检查：确保我们有一个有效的操作对象
        if (!EditingStage.IsValid())
        {
                UE_LOG(LogTemp, Warning, TEXT("CreateNewAct: EditingStage is invalid. Operation aborted."));
                return false; // <-- 返回false，明确告知调用者操作失败
        }

        // 2. 开启事务：FScopedTransaction构造时开启事务，析构时提交。
        //    为事务提供一个对用户友好的描述性名称。
        const FScopedTransaction Transaction(LOCTEXT("Transaction_CreateAct", "Create New Act"));

        // 3. 记录修改前状态：在实际修改任何数据之前，必须调用Modify()。
        //    这会使UE的事务系统保存该对象的副本，以便能够回滚。
        EditingStage->Modify();

        // 4. 执行核心业务逻辑：在这里安全地修改AStage的数据。
        //    (伪代码)
        //    const int32 NewActId = EditingStage->GetNextAvailableActId();
        //    FAct NewAct;
        //    NewAct.ActID.StageID = EditingStage->GetStageID();
        //    NewAct.ActID.ActID = NewActId;
        //    EditingStage->Acts.Add(NewAct);

        // 5. 广播通知：在数据修改后，通知UI刷新。
        OnModelChanged.Broadcast(EStageModelUpdateType::ActList);

        // 6. 返回成功
        return true; // <-- 返回true，明确告知调用者操作成功
}

#undef LOCTEXT_NAMESPACE

```

**所有其他修改API (`DeleteAct`, `RegisterProps`, `UnregisterProps`, `SetPropStateForAct`) 都将遵循完全相同的结构：**

1. **有效性检查** (`EditingStage.IsValid()`) 并返回 `false` 如果无效。
2. **创建 `FScopedTransaction`** 并提供一个清晰的 `LOCTEXT` 描述。
3. **调用 `EditingStage->Modify()`**。
4. **执行具体的增、删、改逻辑**。
5. **广播相应的 `OnModelChanged` 委托**。
6. **返回 `true`** 表示成功。

## 5. 数据变更通知委托设计

为了将UI（View）与数据修改逻辑（Controller）解耦，控制器在每次成功修改模型后，都会广播一个多播委托。

**委托定义:**

```
// 在FStageEditorController.h中定义
enum class EStageModelUpdateType { ... }; // 如上文所示
DECLARE_MULTICAST_DELEGATE_OneParam(FOnStageModelChanged, EStageModelUpdateType);

```

**委托实例:**

```
// 在FStageEditorController.h的public部分
FOnStageModelChanged OnModelChanged;

```

**广播时机与类型:**

`SStageEditorPanel` (View) 将会订阅此委托。当委托被广播时，`View` 会检查传入的 `EStageModelUpdateType` 参数，以决定执行何种强度的UI刷新，从而提高编辑器的响应性能。

| API 函数 | 广播的 `EStageModelUpdateType` | 目的 |
| --- | --- | --- |
| `SetEditingStage` | `FullRefresh` | 通知UI目标已更换，需要完全重建显示内容。 |
| `CreateNewAct` | `ActList` | 通知UI Act列表已增加，只需刷新Act列表部分。 |
| `DeleteAct` | `ActList` | 通知UI Act列表已减少，只需刷新Act列表部分。 |
| `RegisterProps` | `PropList` | 通知UI Prop注册表已更新，刷新Prop列表。 |
| `UnregisterProps` | `PropList` | 通知UI Prop注册表已更新，刷新Prop列表。 |
| `SetPropStateForAct` | `PropState` | 通知UI某个具体的PropState值已改变，可能只需要刷新一个输入框或单元格。 |