# 【通用】“关卡编辑器”系统：AProp数据结构详细设计 (修订版)_V2.1

![](image1.png)

![](image2.png)

![](image3.png)

![](image4.png)

![](image5.png)

![](image6.png)

【通用】“关卡编辑器”系统：AProp数据结构

详细设计 (修订版)_V2.1

“关卡编辑器”系统：AProp数据结构详细设计 (修订版)

文档版本: 2.1

关联文档:

- 《“关卡编辑器”系统需求说明书 (PRD)》
- 
- 
- 《中心化ID分配与本地索引系统详细设计》

| 文档版本 | 作者 | 日期 | 备注 |
| --- | --- | --- | --- |
| 1.0 | 洪硕 | 2025-09-19 | 根据《AStage Actor核心逻辑详细设计》V1.0，移除了FAct 结构体中的

Priority 成员。状态冲突解决机制由显式优先级改为由AStage Actor运行时逻辑控制的“最新优先”模式。 |
| 2.0 | 洪硕 | 2025-09-22 | 本文档根据V4.0核心逻辑设计进行了重大修订。所有基于FName 的ID引用均已被替换为新的FStageHierarchicalId 复合结构体ID系统。核心数据结构被整合到新的StageCoreTypes.h 头文件中，以建立一个统一、稳定的类型定义中心。 |
| 2.1 | 洪硕 | 2025-09-22 | 此版本将FAct的设计内容迁移到了 AStageActor核心设计文档当中 |

1. 设计目标

本文档旨在为“关卡编辑器”系统的两个核心通用数据结构—— AProp 基类和FAct 结构体——提供

精确、完整且符合Unreal Engine最佳实践的C++头文件定义。此设计旨在确保：

- 类型安全与稳定性: 利用UObject反射系统提供健壮的C++与蓝图交互接口。
- 高内聚低耦合: 将通用数据结构分离到独立的头文件中，便于维护和扩展。
- 编辑器友好性: 通过恰当的UPROPERTY 宏，为策划和设计师提供清晰、易于配置的编辑器界面。

2. AProp Actor C++ 头文件 (AProp.h)

AProp 是场景中所有受“关卡编辑器”系统控制的Actor的基类。它封装了核心状态变量PropState 以及响应其变化的核心事件委托OnPropStateChanged 。

设计说明:

- PropState: PropState 是AProp 的核心状态。我们提供了一个SetPropState 函数来封装其修改逻辑。直接在蓝图中设置此变量是允许的，但推荐通过调用SetPropState 函数来修改，这能确保OnPropStateChanged 委托在值发生实际变化时才被广播，从而避免不必要的逻辑触

发。

- OnPropStateChanged: 这是一个多播委托，允许多个系统（包括AProp 自身的蓝图逻辑）订阅

状态变更事件。它传递了旧值和新值，为实现复杂的响应逻辑（例如，从状态2变为3时执行特定行

为）提供了必要的上下文。

- 职责单一: AProp 的设计保持了高度的内聚性。它本身不关心自己的ID是什么，只关心自己的状态（PropState ）。如何查找和识别它，是上层管理者AStage Actor 的责任。
- 解耦设计: 这种设计将ID管理和状态表现完全解耦。AStage Actor 在其PropRegistry ( TMap<FStageHierarchicalId, TSoftObjectPtr<AProp>> )中将

FStageHierarchicalId 与具体的AProp 实例关联起来。因此，AProp.h 无需包含 StageCoreTypes.h ，维持了其作为通用基类的简洁性。本次架构升级，AProp.h 的代码无

需改动，这恰恰证明了其设计的健壮性。

代码块

1 // AProp.h

2

3 #pragma once

4

5 #include"CoreMinimal.h"

6 #include"GameFramework/Actor.h"

7 #include"AProp.generated.h"

8

9 /**

10 * 状态变更事件的多播委托。

11 * 当PropState的值发生改变时广播。

12

* @param OldState 变化前的状态值13 * @param NewState 变化后的新状态值

14 */

15 DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPropStateChangedSignature, int32, OldState, int32, NewState);

16

17 /**

18 * “关卡编辑器”系统中所有可控“道具”单位的基类。

19 * 场景中的任何Actor（如门、NPC、灯光）只要希望被Stage控制，都必须继承自该类。20

*/ 21 UCLASS(Blueprintable)

22 classYOURPROJECT_API AProp : public AActor

23 {

24

GENERATED_BODY() 25

26 public:

27 AProp();

28

29

/** 30 * 当该Prop的PropState发生变化时广播。

31 * 策划可以在蓝图中绑定事件到此委托，以实现自定义的状态响应逻辑。

32 */

33

UPROPERTY(BlueprintAssignable, Category = "Dynamic Stage | Prop") 34 FOnPropStateChangedSignature OnPropStateChanged;

35

36 /**

37 * 设置Prop的新状态值。

38 * 会检查新旧值的差异，只有在值确实发生改变时才更新并广播OnPropStateChanged委托。39 * @param NewState 要设置的新状态值。

40 */

41 UFUNCTION(BlueprintCallable, Category = "Dynamic Stage | Prop")

42 voidSetPropState(int32 NewState);

43

44 /**

45 * 获取当前的PropState值。

46

* @return 当前的PropState值。47 */

48 UFUNCTION(BlueprintPure, Category = "Dynamic Stage | Prop")

49 int32 GetPropState() const { return PropState; }

50

51 protected:

52 /**

53 * 道具的核心状态变量。这是一个整数，其含义由具体道具的蓝图逻辑来定义。

54 * 例如，对于一扇门：0=关闭，1=打开，2=上锁，3=毁坏。

55 * AStage Actor将通过修改这个值来驱动道具的行为。

56 */

57 UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dynamic Stage |

Prop")

58 int32 PropState = 0;

59 };