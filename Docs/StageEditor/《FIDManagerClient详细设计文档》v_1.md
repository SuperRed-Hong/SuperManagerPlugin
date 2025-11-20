# 《FIDManagerClient详细设计文档》v_1.1

## **FIDManagerClient 详细设计文档**

**文档版本:** 1.1

**日期:** 2025年9月22日

### 修订历史

- **V1.1 (2025-09-23):** 新增 `CancelActiveRequest()` 公共方法，以支持上层UI控件在销毁时安全地取消正在进行的HTTP请求，提高编辑器健壮性。
- **V1.0 (2025-09-22):** 初始版本。

### 1. 引言

### 1.1. 目的

本文档旨在详细阐述 `FIDManagerClient` C++ 辅助类的设计与实现方案。该类的核心职责是封装与“中心化ID分配服务”的所有HTTP通信细节，为上层编辑器模块（如 `AStage` 的Details自定义UI）提供一个简单、稳定且非阻塞的接口，用于异步请求全局唯一的 `StageID`。

### 1.2. 范围

本设计涵盖 `FIDManagerClient` 类的完整定义、核心函数的实现逻辑、异步回调机制和全面的错误处理策略。它专注于作为网络通信的“信使”，不涉及ID的本地存储、管理或业务逻辑。

### 1.3. 关联文档

本设计严格遵循以下文档中定义的需求和规范：

- **产品需求:** 《“关卡编辑器”系统需求说明书 (PRD) V2.1》[“动态舞台”系统需求说明书 (PRD)](https://o082e0vcfd2.feishu.cn/wiki/J4JEw9gjsi6emokdakAchvm7nQf)
- **系统架构:** 《“关卡编辑器”系统概要设计文档 V4.1》[“动态舞台”系统概要设计文档 (High-Level Design)_V4.1](https://o082e0vcfd2.feishu.cn/wiki/FjM7wwfOeiHmU7kKaYiciJn8n0N)
- **外部依赖:** 《中心化ID分配服务后端详细设计文档》[中心化ID分配服务后端详细设计](https://o082e0vcfd2.feishu.cn/wiki/UJZhwHeyKiZvP7k8U8TcQSjGnec)

### 2. 核心设计原则

- **独立性与可复用性:** 设计为一个非 `UObject` 的标准C++类，不依赖于特定的Actor或UI框架，使其可以在任何需要获取 `StageID` 的编辑器模块中轻松实例化和使用。
- **异步非阻塞:** 所有网络请求都将是异步的，通过UE的 `FHttpModule` 实现。这可以确保在等待网络响应时，编辑器UI不会被冻结，提供流畅的用户体验。
- **接口驱动:** 客户端的实现严格遵守 **【外部依赖】** 文档中定义的API契约，包括端点、请求方法、请求头、请求体以及所有成功和失败的响应格式。
- **健壮的错误处理:** 对网络故障和后端返回的各类HTTP错误码进行精细化处理，并通过回调机制向上层调用者提供清晰、具体的错误信息。
- **可配置性:** API服务的基地址是可配置的，支持在不同开发环境（开发、测试、生产）之间轻松切换，而无需修改代码。

### 3. 类设计

### 3.1. 头文件定义 (`FIDManagerClient.h`)

```
#pragma once

#include "CoreMinimal.h"
#include "Interfaces/IHttpRequest.h"

// 前置声明
class IHttpRouter;

/**
 * @brief 请求StageID完成时的回调委托。
 * @param bWasSuccessful 请求是否成功。网络错误或服务器返回错误状态码均视为失败。
 * @param StageID 成功时返回的全局唯一StageID。失败时为-1。
 * @param ErrorMessage 失败时返回的错误信息，用于日志记录或UI提示。
 */
DECLARE_MULTICAST_DELEGATE_ThreeParams(FOnRequestStageIDComplete, bool /*bWasSuccessful*/, int32 /*StageID*/, FString /*ErrorMessage*/);

/**
 * @class FIDManagerClient
 * @brief 一个用于与中心化ID分配服务通信的C++辅助类。
 * @note 这是一个独立的、非UObject的类，用于封装获取StageID的HTTP请求。
 */
class FIDManagerClient
{
public:
    /**
     * @brief 构造函数。
     * @param InApiBaseUrl 可选参数，用于指定API的基地址。
     */
    explicit FIDManagerClient(const FString& InApiBaseUrl = TEXT(""));
    ~FIDManagerClient();

    /**
     * @brief 异步发起一个获取新StageID的请求。
     * @return 如果成功发起请求则返回true，如果当前已有请求正在处理中则返回false。
     */
    bool RequestNewStageID();

    /**
     * @brief [新增 V1.1] 取消当前正在进行中的HTTP请求。
     * @note 这是一个安全操作。如果没有正在进行的请求，则此函数不执行任何操作。
     *       主要用于UI控件销毁时，防止悬空的回调导致崩溃。
     */
    void CancelActiveRequest();

    /**
     * @brief 获取用于绑定回调的委托。
     * @return 返回委托的引用。
     */
    FOnRequestStageIDComplete& OnRequestStageIDComplete() { return OnRequestStageIDCompleteDelegate; }

private:
    /**
     * @brief 当HTTP请求处理完成时被调用的内部回调函数。
     */
    void OnRequestCompleted(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);

    /**
     * @brief 辅助函数，用于解析服务器响应并广播最终结果。
     */
    void HandleResponse(FHttpResponsePtr Response);

private:
    /** API服务的基地址 */
    FString ApiBaseUrl;

    /** 核心回调委托实例 */
    FOnRequestStageIDComplete OnRequestStageIDCompleteDelegate;

    /** 持有当前正在进行的HTTP请求的智能指针，同时也作为请求是否在进行中的状态锁 */
    TSharedPtr<IHttpRequest, ESPMode::ThreadSafe> ActiveRequest;
};

```

### 4. 核心函数实现逻辑

### 4.1. `RequestNewStageID()` 实现描述

此函数是与外部交互的主要入口。

1. **前置条件检查**: 检查 `ActiveRequest` 智能指针是否有效。如果有效，意味着上一个请求尚未完成，函数将立即返回 `false`。
2. **获取Http模块**: 通过 `FHttpModule::Get()` 获取HTTP模块单例。
3. **创建请求对象**: 调用 `FHttpModule::CreateRequest()` 创建一个新的 `IHttpRequest` 实例，并保存在 `ActiveRequest` 成员中。
4. **绑定回调**: 将 `OnRequestCompleted` 绑定到请求对象的 `OnProcessRequestComplete()` 委托上。
5. **配置请求URL和Verb**:
    1. `SetURL()`: 设置为后端定义的端点。
    2. `SetVerb("POST")`。
6. **设置请求头 (Headers)**:
    1. `Content-Type` 和 `Accept` 设为 `application/json`。
    2. **关键**: 生成一个唯一的 `X-Request-ID` 头，用于后端幂等性处理。
7. **设置请求体 (Body)**: 发送一个空的JSON对象 `"{}"`。
8. **发送请求**: 调用 `ProcessRequest()` 异步发送。函数返回 `true`。

### 4.2. `OnRequestCompleted()` 实现描述

此函数是引擎HTTP线程在请求完成后调用的回调。

1. **清理状态**: 立即调用 `ActiveRequest.Reset()` 来释放对请求对象的引用，允许发起新的请求。
2. **基础网络检查**: 检查 `bWasSuccessful` 和 `Response.IsValid()`。如果任一为 `false`，广播网络错误。
3. **处理有效响应**: 否则，调用 `HandleResponse(Response)`。

### 4.3. `HandleResponse()` 错误处理与JSON解析逻辑

此辅助函数负责解析HTTP响应，是错误处理的核心。

1. **获取状态码**: 从 `Response` 对象中获取HTTP状态码。
2. **解析JSON**: 使用 `FJsonSerializer::Deserialize` 解析响应体。如果失败，广播JSON格式错误。
3. **处理不同状态码**: 使用 `switch` 语句处理所有预期的状态码：
    1. **200/201 (成功)**: 从JSON中提取 `stage_id` 并广播成功委托。如果字段缺失，广播格式错误。
    2. **4xx/5xx (失败)**: 从JSON中提取 `error` 字段并广播失败委托。如果字段缺失，广播通用错误。
    3. **default (未知状态码)**: 广播包含未知状态码的失败委托。

### 4.4. `CancelActiveRequest()` 实现描述 [新增 V1.1]

此函数为上层调用者提供了一个安全地中断正在进行的网络操作的机制。

1. **检查有效性**: 函数首先检查 `ActiveRequest` 智能指针是否有效 (`ActiveRequest.IsValid()`)。
2. **执行取消**: 如果 `ActiveRequest` 有效，说明有一个请求正在进行中，此时调用其成员函数 `ActiveRequest->CancelRequest()`。这是UE HTTP模块提供的标准方法，用于终止请求。
3. **安全无操作**: 如果 `ActiveRequest` 无效（即当前没有请求在进行），此函数不执行任何操作，直接返回。

### 5. 使用示例

以下代码片段展示了在一个编辑器模块（例如一个自定义的Slate控件）中如何使用 `FIDManagerClient`。

```
// MyEditorWidget.h
#include "FIDManagerClient.h" // 包含设计好的头文件
#include "Templates/SharedPointer.h"

class SMyEditorWidget : public SCompoundWidget
{
    // ...
private:
    void OnGetIDButtonClicked();
    void OnStageIDRequestComplete(bool bWasSuccessful, int32 StageID, FString ErrorMessage);

    TSharedPtr<FIDManagerClient> IDClient;
};

// MyEditorWidget.cpp
void SMyEditorWidget::Construct(const FArguments& InArgs)
{
    // 1. 创建客户端实例
    IDClient = MakeShared<FIDManagerClient>();

    // 2. 绑定回调函数
    IDClient->OnRequestStageIDComplete().AddRaw(this, &SMyEditorWidget::OnStageIDRequestComplete);

    ChildSlot
    [
        SNew(SButton)
        .Text(FText::FromString("Request New Stage ID"))
        .OnClicked(this, &SMyEditorWidget::OnGetIDButtonClicked)
    ];
}

FReply SMyEditorWidget::OnGetIDButtonClicked()
{
    // 3. 发起异步请求
    if (!IDClient->RequestNewStageID())
    {
        // 可选：提示用户请求正在进行中
        UE_LOG(LogTemp, Warning, TEXT("A request for a new StageID is already in progress."));
    }
    return FReply::Handled();
}

// 4. 实现回调函数以处理结果
void SMyEditorWidget::OnStageIDRequestComplete(bool bWasSuccessful, int32 StageID, FString ErrorMessage)
{
    if (bWasSuccessful)
    {
        // 成功：使用获取到的StageID更新UI或数据模型
        UE_LOG(LogTemp, Log, TEXT("Successfully acquired new StageID: %d"), StageID);
        // e.g., MyTextField->SetText(FText::AsNumber(StageID));
    }
    else
    {
        // 失败：在UI上显示错误信息或输出到日志
        UE_LOG(LogTemp, Error, TEXT("Failed to acquire new StageID. Reason: %s"), *ErrorMessage);
        // e.g., MyErrorLabel->SetText(FText::FromString(ErrorMessage));
    }
}```

```