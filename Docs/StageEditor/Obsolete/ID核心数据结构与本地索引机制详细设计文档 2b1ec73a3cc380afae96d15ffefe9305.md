# ID核心数据结构与本地索引机制详细设计文档

---

# ID核心数据结构与本地索引机制详细设计文档

- **文档版本**: 1.1
- **日期**: 2025年9月23日
- **关联文档**:
    - 《“关卡编辑器”系统需求说明书 (PRD) V2.1》
    - 《“关卡编辑器”系统概要设计文档 V4.1》
    - 《中心化ID分配服务后端详细设计文档》
    - 《AStage Actor核心逻辑详细设计》

---

## 1. 引言与文档定位

### 1.1 本文档的核心使命

本文档是“关卡编辑器”系统**数据层**的基石。其核心使命是为整个系统定义一套**全局唯一、持久稳定、且查询高效**的身份标识（ID）与本地索引机制。它专注于回答以下两个核心问题：

1. **“是什么 (What)”**: 定义作为系统“身份证”的`FStageHierarchicalId`结构体，以及作为Stage“户籍系统”的`PropRegistry`道具注册表的精确数据结构。
2. **“为什么这样设计 (Why)”**: 深入阐述关键技术选型（尤其是`TSoftObjectPtr`）背后的设计原理，为其稳定性、鲁棒性和性能表现提供权威的技术论证。

### 1.2 与《AStage Actor核心逻辑详细设计》的职责分离

为确保架构的清晰性与高内聚，本设计与《AStage Actor核心逻辑详细设计》严格遵循“职责分离”原则：

- **本文档 (ID与索引设计)** 是**“定义者”**。它负责**奠定数据基础**。
- **《AStage Actor核心逻辑详细设计》是“使用者”**。它负责**实现运行时行为**，它会引用并消费本文档定义的数据结构来完成其业务逻辑。

因此，`PropRegistry`等核心数据结构在本设计中被**权威性地定义和论证**，而在`AStage`的设计文档中则被**引用和使用**。这种分离确保了数据模型的稳定与逻辑实现的灵活。

## 2. 核心ID数据结构：`FStageHierarchicalId`

### 2.1 设计目标

- **全局唯一性**: 结合由外部服务分配的`StageID`，确保每个ID在整个项目中是独一无二的。
- **层级清晰性**: 结构体本身能清晰地表达出 Stage -> Act -> Prop 的数据层级关系。
- **高效索引**: 必须能够作为`TMap`的Key进行高效的哈希查找和比较。
- **稳定性**: 结构体为纯数据（Plain Old Data），不含任何指针，确保序列化和网络传输的绝对稳定。

### 2.2 C++头文件阐述与引用 (Explanation & Reference)

`FStageHierarchicalId`结构体的**最终C++定义位于项目的核心类型文件 `StageCoreTypes.h` 中**。该文件是所有模块共享的基础类型定义中心，确立了其作为“单一真实数据源”的地位。本章节旨在阐述该结构的核心设计与实现要点。

其核心结构如下，完整实现请直接参阅 `StageCoreTypes.h` 文件：

```
// 在 StageCoreTypes.h 中定义

USTRUCT(BlueprintType)
struct FStageHierarchicalId
{
    GENERATED_BODY()

    // 全局唯一的Stage ID，由中心化服务分配。
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ID")
    int32 StageID = 0;

    // Stage内唯一的Act ID，由AStage Actor本地分配。
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ID")
    int32 ActID = 0;

    // Stage内唯一的Prop ID，由AStage Actor本地分配。
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ID")
    int32 PropID = 0;

    // 为了能作为TMap的Key，需要重载等于操作符。
    bool operator==(const FStageHierarchicalId& Other) const
    {
        return StageID == Other.StageID && ActID == Other.ActID && PropID == Other.PropID;
    }

    // 默认构造函数
    FStageHierarchicalId() = default;
};

// 为TMap提供哈希函数。
inline uint32 GetTypeHash(const FStageHierarchicalId& ID)
{
    return HashCombine(HashCombine(GetTypeHash(ID.StageID), GetTypeHash(ID.ActID)), GetTypeHash(ID.PropID));
}

```

### **具体的配置与索引例子：**

### 场景设定：一个新手村 - “溪谷镇”

假设我们正在构建一个名为“溪谷镇”的新手村场景。

1. **分配“区域代码” (`StageID`)**
    1. 关卡设计师在编辑器里创建了一个`AStage` Actor来管理整个溪谷镇。
    2. 他通过一个编辑器按钮（背后调用了Web API）为这个Stage申请了一个**全局唯一**的ID。服务器返回了 `101`。
    3. 所以，**溪谷镇 StageID = 101**。
2. **编写“剧本” (`ActID`)**
    1. 设计师为溪谷镇定义了三种不同的状态（Acts）：
        - **和平时期 (Act_Peaceful)**: 本地分配 `ActID = 1`
        - **被兽人攻击 (Act_UnderAttack)**: 本地分配 `ActID = 2`
        - **战后废墟 (Act_Ruined)**: 本地分配 `ActID = 3`
    2. 这些ID只在`StageID = 101`的范围内有意义。
3. **布置“道具” (`PropID`)**
    1. 设计师在村里放置了几个关键的、需要被控制的物体（Props）：
        - **村庄大门 (BP_VillageGate)**: 本地分配 `PropID = 1`
        - **中央水井 (BP_Well)**: 本地分配 `PropID = 2`
        - **英雄雕像 (BP_Statue)**: 本地分配 `PropID = 3`
    2. 这些ID同样只在`StageID = 101`的范围内有意义。

### 示例1：为村庄大门（Prop)分配PropID (注册到PropRegistry)

这是最基础的用途：在“户籍系统”(PropRegistry)中，为每个道具创建一个唯一的、永久的身份标识。

- **用途**: 在`AStage`的`PropRegistry`这个`TMap`中作为 **Key**。
- **规则**: 注册道具“身份”时，`ActID`部分约定为`0`，因为它代表道具本身，与任何具体状态（Act）无关。

这个ID的构成是：

- `StageID`: `101` (我们正在溪谷镇)
- `ActID`: `0` (规则：代表这是道具的“身份证号”)
- `PropID`: `1` (这是村庄大门)

所以，“村庄大门”的最终“身份证号”是 `{101, 0, 1}`。

在C++代码中，它像这样被添加到注册表里：

codeC++

```
// FStageHierarchicalId Key = {101, 0, 1};// TSoftObjectPtr<AProp> Value = 指向场景中BP_VillageGate实例的软指针;
PropRegistry.Add(Key, Value);

```

**中央水井**的“身份证号”就是 `{101, 0, 2}`，**英雄雕像**的“身份证号”是 `{101, 0, 3}`。

### 示例2：定义一个“场景状态蓝图”（FAct）

现在，我们要详细定义**“被兽人攻击 (Act_UnderAttack)”**这个剧本。在这个剧本里：

- 村庄大门应该被**关闭并加上路障 (PropState = 2)**。
- 英雄雕像应该**倒塌 (PropState = 5)**。
- 水井保持不变，所以我们不需要定义它。

这个`FAct`数据结构内部的`TargetPropStates`这个`TMap`会包含以下条目：

- **条目一**:
    - **Key**: `{101, 0, 1}` (我们引用的是“村庄大门”的**身份证号**)
    - **Value**: `2` (它应该进入的状态)
- **条目二**:
    - **Key**: `{101, 0, 3}` (我们引用的是“英雄雕像”的**身份证号**)
    - **Value**: `5` (它应该进入的状态)

**（后续FAct的SHID 作为 AStage Actor中切换 Act函数的入参，遍历并切换对应的 prop state)**

### 示例3：标识一个Act本身

“被兽人攻击”这个Act本身也需要一个ID来标识自己。

- **用途**: 在`FAct`结构体自己的`ActID`成员变量中存储。
- **规则**: 标识Act时，`PropID`部分约定为`0`，因为它代表整个场景状态，与任何单个道具无关。

这个ID的构成是：

- `StageID`: `101` (我们正在溪谷镇)
- `ActID`: `2` (这是“被兽人攻击”这个Act)
- `PropID`: `0` (规则：代表这是Act的ID)

所以，“被兽人攻击”这个Act的ID就是 `{101, 2, 0}`。

### 总结表格

表格 还在加载中，请等待加载完成后再尝试复制

[](https://ccn7ua54lvww.feishu.cn/space/api/box/stream/download/asynccode/?code=MDlkZDMyOTI3NzQ5OWY5ZDE3MzgxZjg4OTdkOTUxMTdfbGRNOUNMbEM5YlBHaFh4Z1NwVnBnNGVuQ0xhdEVIMGNfVG9rZW46UnBMamJwb0wzb2JQbWl4VjNONGNBYVYzbmpjXzE3NjM2MDc5MjQ6MTc2MzYxMTUyNF9WNA)

---

## 3. AStage 道具注册表 (Prop Registry)

道具注册表是`AStage` Actor的核心成员，它维护了一个从道具ID到场景中具体道具实例的映射关系。

### 3.1 设计目标

- **持久化映射**: 提供一个从`FStageHierarchicalId`到`AProp` Actor实例的、可被序列化并持久保存的稳定映射。
- **鲁棒性**: 必须能正确处理编辑器重启、关卡保存与加载的场景，确保引用不会丢失。
- **性能友好**: 必须支持动态的、按需的关卡流式加载（Level Streaming），不能因为引用关系而阻止非活动关卡的卸载。

### 3.2 C++成员最终定义

`PropRegistry`是`AStage`类的核心成员变量。经过技术评审，其**最终的、权威的C++定义**如下，该定义融合了各方案优点，达到了最优设计。

```
// 最终权威定义，将在 AStage.h 的 protected 部分实现

/**
 * @brief 道具注册表 (The Prop Registry)
 * 核心数据结构，存储了本Stage管理的所有Prop的ID与其Actor实例之间的稳定映射。
 */
UPROPERTY(VisibleAnywhere, Category = "Stage|ID System", meta=(DisplayName="Prop Registry", AllowPrivateAccess = "true"))
TMap<FStageHierarchicalId, TSoftObjectPtr<AProp>> PropRegistry;

```

### 3.2.1 最终定义决策理由 (The "Why")

这个最终版本在编辑器可用性、代码健robust性和未来可扩展性三个方面都达到了最优：

- **`Category = "Stage|ID System"`**:
    - **决策**: 采纳自《AStage Actor核心逻辑详细设计》。
    - **理由**: 此分类将`PropRegistry`与`StageID`, `NextAvailableActId`, `NextAvailablePropId`等所有ID管理相关的属性组织在一起。这为使用者在编辑器Details面板中提供了极佳的上下文，所有相关属性一目了然，便于理解和调试。
- **`meta = (AllowPrivateAccess = "true")`**:
    - **决策**: 采纳自本文档的早期版本。
    - **理由**: 这是本次决策中最重要的功能性考量。此元标记允许`AStage`的蓝图子类安全地访问这个`protected`成员，从而进行只读操作（例如，在蓝图中遍历注册表获取信息）。这极大地增强了系统的灵活性和策划的可扩展空间，同时又维持了C++层面的封装（外部代码无法写入）。
- **`VisibleAnywhere` & `meta=(DisplayName="Prop Registry")`**:
    - **决策**: 两者共同采纳。
    - **理由**: `VisibleAnywhere`确保了该属性在编辑器中可见以供调试。`DisplayName`则是一种精细的视觉优化，确保了其显示名称的规范性。

### 3.3 `TSoftObjectPtr` 选型深度解析

在`PropRegistry`中，使用 `TSoftObjectPtr<AProp>` 而非原生指针 (`AProp*`) 或硬指针 (`TObjectPtr<AProp>`) 是一个经过深思熟虑的关键架构决策。这是唯一能够同时满足系统持久化、鲁棒性和性能需求的技术方案。

- **为什么不使用原生指针 (**`AProp*`**)?**
    - **瞬态性**: 原生指针存储的是对象在内存中的地址。当编辑器关闭或游戏结束时，内存被回收，地址变得无效。下次启动时，即使同一个Actor被重新加载，其内存地址也几乎一定会改变。依赖原生指针进行持久化存储是不可行的，会导致悬垂指针和必然的程序崩溃。
- **为什么不使用硬指针 (**`TObjectPtr<AProp>`**)?**
    - **强引用**: `TObjectPtr` (以及旧版的`UPROPERTY`指针) 会创建一个到对象的强引用。这意味着，只要`AStage` Actor存在于内存中，它就会强制其`PropRegistry`中引用的所有`AProp` Actor也必须保持在内存中。
    - **破坏流式加载**: 这种强引用关系会彻底破坏“按需加载”的核心目标。如果一个`AProp`位于一个当前应被卸载的流式关卡（Sub-Level）中，`AStage`对它的强引用会阻止这个关卡被正常卸载，从而导致不必要的内存占用和性能开销。
- **为什么**`TSoftObjectPtr<AProp>`**是唯一正确的选择?**
    - **持久化**: `TSoftObjectPtr` 本质上存储的不是内存地址，而是一个对象的**字符串路径**（例如：`/Game/Maps/MyLevel.MyLevel:PersistentLevel.BP_Door_42`）。这个路径在编辑器会话之间、在打包的游戏中都是稳定且唯一的。因此，它可以被安全地序列化到磁盘并被正确地加载回来。
    - **解耦引用**: 它是一种**弱引用**，不会阻止垃圾回收器回收对象，也不会阻止其所在的流式关卡被卸载。它仅仅记录了“在哪里可以找到这个对象”的信息，而不强制它必须存在于内存中。
    - **按需解析**: 在运行时，我们可以通过`TSoftObjectPtr`的`.Get()`方法尝试将其解析为一个原生指针。如果对象恰好在内存中（因为其关卡已被加载），该方法会立即返回有效的指针。如果不在，则返回`nullptr`。这种“用时查找”的特性，与我们“先加载Data Layer，后应用状态”的工作流完美契合，确保了系统的稳定性和高效性。

---

## 4.核心工作流 (详细扩展版)

### 4.1 Prop注册流程 (编辑器时)

此流程详细描述了当策划在编辑器中将一个`AProp` Actor注册到一个`AStage`中时，系统内部发生的精确、可撤销的操作序列。

1. **触发点 (User Action)**
    1. 在`SStageEditorPanel`编辑器界面中，用户在世界大纲视图(World Outliner)中选中一个或多个继承自`AProp`的Actor，然后点击“注册选中项”按钮。
2. **控制器层 (Controller Logic)**
    1. 按钮的`OnClicked`委托调用`FStageEditorController`中的一个处理函数，例如 `void FStageEditorController::HandleRegisterSelectedActors()`。
    2. **开启事务**: 为了支持完整的撤销/重做(Undo/Redo)功能，操作的第一步就是创建一个事务。
    3. codeC++
    
    ```
    const FScopedTransaction Transaction(FText::FromString("Register Props to Stage"));
    
    ```
    
    1. **获取上下文**: 控制器获取当前正在编辑的`AStage` Actor的引用，并使用`GEditor->GetSelectedActors()`获取用户选中的Actor列表。
    2. **修改前置准备**: 在修改`AStage`之前，控制器需要通知引擎该对象即将被修改，这是确保事务正确记录的关键。
    3. codeC++
    
    ```
    CurrentStage->Modify();
    
    ```
    
3. **循环处理与调用模型层 (Loop & Model Call)**
    1. 控制器遍历所有选中的Actor。
    2. 对于每一个Actor，进行类型安全检查，确保它是一个`AProp`。
    3. codeC++
    
    ```
    for (AActor* SelectedActor : SelectedActors)
    {
        if (AProp* PropToRegister = Cast<AProp>(SelectedActor))
        {
            // 调用AStage的模型层接口
            CurrentStage->RegisterProp(PropToRegister);
        }
    }
    
    ```
    
4. **模型层 (AStage Actor Logic)**
    1. 在`AStage::RegisterProp(AProp* PropToRegister)`函数内部，执行以下核心逻辑：
    2. **a. 前置条件检查 (Pre-condition Check)**: 检查此`AProp`实例是否已经被注册，防止重复添加。
    3. codeC++
    
    ```
    for (const auto& Kvp : PropRegistry)
    {
        if (Kvp.Value.Get() == PropToRegister)
        {
            UE_LOG(LogDynamicStage, Warning, TEXT("Prop %s is already registered."), *PropToRegister->GetName());
            return; // Abort for this prop
        }
    }
    
    ```
    
    1. **b. 生成唯一ID (ID Generation)**: 从`AStage`的本地计数器获取一个新的`PropID`并递增计数器。
    2. codeC++
    
    ```
    const int32 NewPropID = NextAvailablePropId++;
    
    ```
    
    1. **c. 组合层级键 (Key Composition)**: 使用`StageID`和新生成的`PropID`组合成注册表的Key。根据设计约定，`ActID`在此处始终为`0`，代表这是对Prop本身的“身份”注册。
    2. codeC++
    
    ```
    const FStageHierarchicalId RegistryKey(this->StageID, 0, NewPropID);
    
    ```
    
    1. **d. 数据存储 (Data Storage)**: 将`AProp*`转换为`TSoftObjectPtr<AProp>`，并添加到`TMap`中。
    2. codeC++
    
    ```
    TSoftObjectPtr<AProp> PropSoftPtr(PropToRegister);
    PropRegistry.Add(RegistryKey, PropSoftPtr);
    
    ```
    
5. **收尾 (Finalization)**
    1. 当控制器中的循环结束后，`FScopedTransaction`对象在其作用域结束时自动完成事务的提交。
    2. **标记为脏**: 控制器最后需要标记`AStage`所在的包（Package）为已修改状态，这样编辑器才会提示用户保存更改。
    3. codeC++
    
    ```
    CurrentStage->MarkPackageDirty();
    
    ```
    
    1. **结果**: `PropRegistry`中新增了一个或多个条目，将稳定的ID映射到了场景中的`AProp`实例上，并且整个操作可以被用户通过`Ctrl+Z`撤销。

### 4.2 Prop索引与状态应用流程 (运行时)

此流程详细描述了在游戏运行时，`AStage`如何根据一个激活的`FAct`中的数据，安全、高效地查找到对应的`AProp`实例并应用其目标状态。

1. **触发点 (Triggering Context)**
    1. 此工作流在`AStage`的运行时逻辑中被触发，最典型的场景是当蓝图逻辑调用`SetRecentActID(FStageHierarchicalId ActID)`时。
    2. 在该函数内部，`AStage`会首先找到`ActID`对应的`FAct`数据，然后调用一个核心内部函数，例如 `void AStage::ApplyPropStatesFromAct(const FAct& ActToApply)`。
2. **状态应用准备 (Preparation)**
    1. 在`ApplyPropStatesFromAct`函数内部，首先检查`ActToApply`是否有效。
    2. **前置条件检查**: 确认`AStage`当前处于`EStageRuntimeState::Active`状态。这是一个关键的健壮性检查，确保只在资源加载完毕后才应用逻辑状态。
    3. codeC++
    
    ```
    if (CurrentState != EStageRuntimeState::Active)
    {
        UE_LOG(LogDynamicStage, Warning, TEXT("Attempted to apply Act states while Stage was not Active. Aborting."));
        return;
    }
    
    ```
    
3. **遍历Act数据并索引 (Iteration and Indexing)**
    1. 函数遍历`ActToApply`中的核心数据成员 `TargetPropStates`。这是一个`TMap<FStageHierarchicalId, int32>`。
    2. codeC++
    
    ```
    for (const TPair<FStageHierarchicalId, int32>& PropStatePair : ActToApply.TargetPropStates)
    {
        const FStageHierarchicalId& PropIdFromAct = PropStatePair.Key;
        const int32 TargetState = PropStatePair.Value;
    
        // ... a, b, c steps here ...
    }
    
    ```
    
    1. **a. 查找注册表 (Registry Lookup)**: 使用从Act中获取的`PropIdFromAct`作为Key，在`PropRegistry`中进行查找。
    2. codeC++
    
    ```
    const TSoftObjectPtr<AProp>* FoundSoftPtr = PropRegistry.Find(PropIdFromAct);
    
    ```
    
    1. **b. 解析与验证 (Pointer Resolution & Validation)**: 这是流程中最关键的安全检查部分。
        - **第一层检查：ID是否存在？** 如果`Find`返回`nullptr`，说明`FAct`的数据引用了一个从未在本`AStage`注册过的`PropID`。这是一个严重的数据错误。
        - codeC++
        
        ```
        if (!FoundSoftPtr)
        {
            UE_LOG(LogDynamicStage, Error, TEXT("Data Error: Act '%s' references an unregistered PropID '%s'."), *ActToApply.ActID.ToString(), *PropIdFromAct.ToString());
            continue; // Skip to the next Prop in the loop
        }
        
        ```
        
        - **第二层检查：Actor是否已加载？** 如果ID存在，则尝试将软指针解析为硬指针。
        - codeC++
        
        ```
        AProp* PropInstance = FoundSoftPtr->Get();
        if (!IsValid(PropInstance))
        {
            UE_LOG(LogDynamicStage, Error, TEXT("State Apply Error: PropID '%s' is registered but its Actor is not loaded. Check DataLayer configuration for Act '%s'."), *PropIdFromAct.ToString(), *ActToApply.ActID.ToString());
            continue; // Skip to the next Prop in the loop
        }
        
        ```
        
    2. **c. 应用状态 (State Application)**: 只有通过全部验证后，才能安全地与`AProp`实例交互。
    3. codeC++
    
    ```
    PropInstance->SetPropState(TargetState);
    
    ```
    
4. **结果 (Outcome)**
    1. 当循环结束后，`ActToApply`中定义的所有、且当前已加载的`AProp`，其`PropState`变量都已被更新为目标值。
    2. `AProp`内部的`OnRep_PropState`或`OnPropStateChanged`事件将被触发，驱动其各自的蓝图逻辑（如改变模型、播放特效等）。
    3. 流程中通过详细的日志记录了所有可能的数据或逻辑错误，便于快速调试。