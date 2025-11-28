# 《AStage Details自定义详细设计文档》

---

# 《AStage Details自定义详细设计文档》

**文档版本:** 1.1

**作者:** (AI Assistant)

**日期:** 2025-09-23

---

## 1. 引言

### 1.1 目的

本文档旨在为 `AStage` Actor 提供一个自定义的Details面板UI，其核心任务是实现“无感ID申请”的交互流程。本文档将提供实现此功能所需的全部C++类定义、UI布局、状态机逻辑以及与网络层的交互细节。

此设计严格遵循《“关卡编辑器”系统概要设计文档 V4.1》中的架构决策，并直接利用《FIDManagerClient详细设计文档》中定义的C++客户端与中心化ID分配服务进行通信。

### 1.2 核心设计原则

- **数据驱动UI**: 控件的显示状态（如按钮是否可用、加载图标是否显示）完全由内部状态机驱动，确保UI表现与后台逻辑的严格一致。
- **异步非阻塞**: 所有网络请求均为异步操作，确保在等待服务器响应时，编辑器UI不会被冻结，提供流畅的用户体验。
- **健壮的生命周期管理**: 精心管理UI控件的生命周期，确保在控件被销毁时，能主动取消正在进行的网络请求并解绑回调，从根源上杜绝悬空指针导致的编辑器崩溃。
- **清晰的用户反馈**: 在异步操作的各个阶段（请求中、成功、失败）为用户提供明确、即时的视觉反馈。

---

## 2. Details自定义入口 (`FAStageDetailCustomization`)

这是将我们的自定义UI逻辑注入到Unreal Engine编辑器Details面板的标准入口。

### 2.1 头文件定义

```
// Editor/DetailCustomizations/FAStageDetailCustomization.h

#pragma once

#include "CoreMinimal.h"
#include "IDetailCustomization.h"

class IDetailLayoutBuilder;

/**
 * AStage Actor的Details面板自定义类。
 * 负责查找StageID属性，并用自定义的 SStageIDRequestWidget 替换其默认UI。
 */
class FAStageDetailCustomization : public IDetailCustomization
{
public:
    /** 创建此自定义类的实例的静态工厂方法 */
    static TSharedRef<IDetailCustomization> MakeInstance();

    /** ~Begin IDetailCustomization interface */
    // 当Details面板需要被构建时，引擎会调用此函数
    virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;
    /** ~End IDetailCustomization interface */
};

```

### 2.2 实现详解

`FAStageDetailCustomization` 继承自 `IDetailCustomization` 接口，是实现Details面板自定义的标准方式。

- **`MakeInstance()`**: 一个静态工厂方法，用于创建自身的实例。编辑器模块（`FEditorModule`）在启动时，会调用 `FPropertyEditorModule::RegisterCustomClassLayout` 并将这个工厂方法注册给 `AStage::StaticClass()`。
- **`CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)`**: 这是该类的核心。当用户在编辑器中选中一个 `AStage` Actor时，引擎会调用此函数。其实现逻辑如下：
    - **获取分类**: 从 `DetailBuilder` 中获取 `StageID` 属性所在的分类（例如 "Stage|ID System"）。
    - **获取属性句柄**: 调用 `DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(AStage, StageID))` 来获取 `StageID` 属性的句柄 (`TSharedPtr<IPropertyHandle>`)。这个句柄是后续与该属性数据交互的唯一桥梁。
    - **隐藏默认UI**: 调用属性句柄的 `MarkHiddenByCustomization()` 方法，隐藏引擎为 `int32` 类型生成的默认数字输入框。
    - **添加自定义行**: 在获取的分类下，调用 `AddCustomRow()` 方法来添加一个新的UI行，并为其提供一个搜索关键词（如 "Stage ID"）。
    - **构建并插入自定义控件**: 在自定义行中，构建核心UI控件 `SStageIDRequestWidget` 的一个实例。最关键的一步，是将第2步中获取的 `StageID` 属性句柄作为参数 (`.StageIDPropertyHandle(...)`) 传递给 `SStageIDRequestWidget`，从而建立UI与底层数据之间的双向连接。

---

## 3. 核心交互UI (`SStageIDRequestWidget`)

这是承载所有UI元素、状态管理和交互逻辑的核心Slate复合控件。

### 3.1 头文件定义

```
// Editor/Widgets/SStageIDRequestWidget.h

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "FIDManagerClient.h" // 依赖网络客户端

class IPropertyHandle;
class SButton;
class STextBlock;

/**
 * 一个复合Slate控件，用于显示StageID并提供一个按钮来异步请求新的ID。
 * 它封装了请求过程中的所有UI状态（空闲、请求中、成功、失败）。
 */
class SStageIDRequestWidget : public SCompoundWidget
{
public:
    // UI状态机枚举
    enum class EWidgetState : uint8
    {
        Idle,       // 空闲
        InProgress, // 请求处理中
        Success,    // 请求成功
        Error       // 请求失败
    };

    SLATE_BEGIN_ARGS(SStageIDRequestWidget) {}
        /** [必须] 传入要操作的StageID属性的句柄 */
        SLATE_ARGUMENT(TSharedPtr<IPropertyHandle>, StageIDPropertyHandle)
    SLATE_END_ARGS()

    /**
     * 构建此控件
     * @param InArgs - 声明式语法参数
     */
    void Construct(const FArguments& InArgs);

    /** 控件析构时，确保取消任何正在进行的网络请求，防止崩溃 */
    virtual ~SStageIDRequestWidget();

private:
    // -- UI绑定函数 (Attributes) -- //

    /** 获取当前StageID并格式化为文本，用于绑定到只读文本框 */
    FText GetCurrentStageIDText() const;

    /** 根据当前状态和ID值，决定“请求”按钮是否可点击 */
    bool IsRequestButtonEnabled() const;

    /** 根据当前状态决定“加载中”图标是否可见 */
    EVisibility GetThrobberVisibility() const;

    // -- 事件处理函数 (Delegates) -- //

    /** “请求新ID”按钮的点击事件回调 */
    FReply OnRequestButtonClicked();

    /** FIDManagerClient完成网络请求后的异步回调 */
    void OnStageIDRequestComplete(bool bWasSuccessful, int32 NewStageID, FString ErrorMessage);

    // -- 内部辅助函数 -- //

    /** 用于切换UI状态并更新相关UI元素的辅助函数 */
    void SetWidgetState(EWidgetState NewState);

private:
    /** 指向AStage::StageID属性的句柄，用于读/写数据 */
    TSharedPtr<IPropertyHandle> StageIDPropertyHandle;

    /** 指向UI元素的指针，用于直接控制其属性（如文本内容、颜色） */
    TSharedPtr<STextBlock> StatusTextBlock;

    /** 当前UI状态机的状态，驱动所有UI表现 */
    EWidgetState CurrentState = EWidgetState::Idle;

    /** 用于与后端通信的网络客户端实例 */
    TSharedPtr<FIDManagerClient> IDClient;
};

```

### 3.2 实现详解

### 3.2.1 构造与布局 (`Construct`)

此函数负责使用Slate声明式语法来构建控件的视觉布局，并完成初始化工作。

- **布局**: 使用 `SHorizontalBox` 水平排列以下元素：
    - 一个 `STextBlock` 作为标签，显示 "Stage ID"。
    - 一个只读的 `SEditableTextBox`，其 `Text` 属性绑定到 `GetCurrentStageIDText()` 函数。
    - 一个 `SButton`，其 `OnClicked` 事件绑定到 `OnRequestButtonClicked()`，`IsEnabled` 属性绑定到 `IsRequestButtonEnabled()`。
    - 一个 `SThrobber` (加载动画图标)，其 `Visibility` 属性绑定到 `GetThrobberVisibility()`。
    - 一个 `STextBlock` (`StatusTextBlock`) 用于显示状态和错误信息，默认隐藏。
- **初始化**:
    - 从 `InArgs` 中保存传入的 `StageIDPropertyHandle`。
    - 实例化 `FIDManagerClient`：`IDClient = MakeShared<FIDManagerClient>();`。
    - **绑定回调**: 将 `OnStageIDRequestComplete` 成员函数绑定到 `IDClient` 的 `OnRequestStageIDComplete` 委托上：`IDClient->OnRequestStageIDComplete().AddRaw(this, &SStageIDRequestWidget::OnStageIDRequestComplete);`。

### 3.2.2 UI状态机与逻辑

控件的核心行为由 `EWidgetState` 状态机驱动，通过 `SetWidgetState` 函数进行切换。

- **`Idle`** (空闲状态)**:
    - **进入条件**: 控件初始化；请求成功或失败后。
    - **UI表现**:
        - `IsRequestButtonEnabled()`: 如果 `StageID` 为0或无效，返回 `true`；否则返回 `false`。
        - `GetThrobberVisibility()`: 返回 `EVisibility::Collapsed` (隐藏)。
        - `StatusTextBlock`: 隐藏。
- **`InProgress`** (请求中状态)**:
    - **进入条件**: 用户点击“申请新ID”按钮。
    - **UI表现**:
        - `IsRequestButtonEnabled()`: 返回 `false`，防止重复点击。
        - `GetThrobberVisibility()`: 返回 `EVisibility::Visible` (可见)。
        - `StatusTextBlock`: 显示 "Requesting..." 并设为中性色。
- **`Success`** (成功状态)**:
    - **进入条件**: `OnStageIDRequestComplete` 回调被触发且 `bWasSuccessful` 为 `true`。
    - **UI表现**:
        - `IsRequestButtonEnabled()`: 返回 `false` (因为已成功获取ID)。
        - `GetThrobberVisibility()`: 返回 `EVisibility::Collapsed`。
        - `StatusTextBlock`: 显示绿色成功消息 (如 "Success!")。
- **`Error`** (失败状态)**:
    - **进入条件**: `OnRequestButtonClicked` 调用失败或 `OnStageIDRequestComplete` 回调返回失败。
    - **UI表现**:
        - `IsRequestButtonEnabled()`: 返回 `true`，允许用户重试。
        - `GetThrobberVisibility()`: 返回 `EVisibility::Collapsed`。
        - `StatusTextBlock`: 显示红色错误信息。

### 3.2.3 事件与回调处理

- **`OnRequestButtonClicked()`**:
    - 检查 `CurrentState` 是否为 `Idle`。如果不是，直接返回，防止重复请求。
    - 调用 `SetWidgetState(EWidgetState::InProgress)` 立即更新UI。
    - 调用 `IDClient->RequestNewStageID()` 发起异步网络请求。
    - 如果 `RequestNewStageID()` 返回 `false`（表示已有请求在进行中），立即调用 `SetWidgetState(EWidgetState::Error)` 并显示相应错误。
- **`OnStageIDRequestComplete(bool bWasSuccessful, ...)`**:
    - **如果 `bWasSuccessful`** 为 `true**`:
        1. a. 调用 `SetWidgetState(EWidgetState::Success)`。
        2. b. **核心数据写入**: 使用 `StageIDPropertyHandle` 将 `NewStageID` 写回 `AStage` Actor。此操作必须被 `NotifyPreChange()` 和 `NotifyPostChange()` 包裹，以确保正确的事务处理，从而支持编辑器的Undo/Redo功能。
        3. c. `StageIDPropertyHandle->SetValue(NewStageID);`
    - **如果 `bWasSuccessful`** 为 `false**`:
        1. a. 调用 `SetWidgetState(EWidgetState::Error)`。
        2. b. 将 `ErrorMessage` 显示在 `StatusTextBlock` 中。

### 3.2.4 生命周期管理 (`~SStageIDRequestWidget`)

这是确保编辑器健壮性的关键。析构函数 `~SStageIDRequestWidget` 必须执行以下清理操作：

1. **解绑委托**: 调用 `IDClient->OnRequestStageIDComplete().RemoveAll(this)`。这可以确保即使网络请求无法被取消，其回调也不会尝试访问一个已经被销毁的控件对象。
2. **取消请求**: 调用 `IDClient->CancelActiveRequest()`。这会主动通知HTTP模块终止正在进行的请求，释放网络资源。