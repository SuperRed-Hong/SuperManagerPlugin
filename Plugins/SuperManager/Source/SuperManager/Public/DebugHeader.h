#pragma once

#include "Widgets/Notifications/SNotificationList.h"
#include "Framework/Notifications/NotificationManager.h"
#include "Misc/ScopedSlowTask.h"
#include "Templates/UniquePtr.h"


namespace DebugHeader
{
	static void Print(const FString& Message, const FColor& Color = FColor::MakeRandomColor())
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 8.f, Color, Message);
		}
	}

	static void PrintLog(const FString& Message)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s"), *Message);
	}

	static EAppReturnType::Type ShowMsgDialog(EAppMsgType::Type MsgType, const FString& Msg,
	                                          bool bShowMsgAsWarning = true)
	{
		EAppReturnType::Type UserResponse;
		if (bShowMsgAsWarning)
		{
			FText MsgTitle = FText::FromString(TEXT("Warning"));
			UserResponse = FMessageDialog::Open(
				MsgType,
				FText::FromString(Msg),
				MsgTitle
			);
		}
		else
		{
			UserResponse = FMessageDialog::Open(
				MsgType,
				FText::FromString(Msg)
			);
		}
		return UserResponse;
	}

	static void ShowNotifyInfo(const FString& Message)
	{
		FNotificationInfo NotifyInfo(FText::FromString(Message));
		NotifyInfo.bUseLargeFont = true;
		NotifyInfo.FadeOutDuration = 7.f;

		FSlateNotificationManager::Get().AddNotification(NotifyInfo);
	}

	static TUniquePtr<FScopedSlowTask> CreateProgressTask(float TotalWork, const FText& TaskTitle, bool bShowCancelButton = true)
	{
		TUniquePtr<FScopedSlowTask> SlowTask = MakeUnique<FScopedSlowTask>(TotalWork, TaskTitle);
		SlowTask->MakeDialog(bShowCancelButton);
		return SlowTask;
	}
}
