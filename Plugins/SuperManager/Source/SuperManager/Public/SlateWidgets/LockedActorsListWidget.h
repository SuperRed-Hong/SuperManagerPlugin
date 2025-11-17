// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Input/SComboBox.h"
#include "Widgets/Views/SListView.h"
#include "Brushes/SlateRoundedBoxBrush.h"

class AActor;
class UEditorActorSubsystem;

struct FLockedActorListItem
{
	TWeakObjectPtr<AActor> Actor;
	bool bIsLocked = false;
};

enum class ELockedActorsViewMode : uint8
{
	AllActors,
	LockedOnly
};

DECLARE_DELEGATE_TwoParams(FOnSetActorLockState, TWeakObjectPtr<AActor> /*Actor*/, bool /*bShouldLock*/);
DECLARE_DELEGATE(FOnUnlockAllActors);
DECLARE_DELEGATE_RetVal(TArray<TSharedPtr<FLockedActorListItem>>, FOnRequestLockedActorRows);
DECLARE_DELEGATE_OneParam(FOnLockedActorRowDoubleClicked, TWeakObjectPtr<AActor> /*Actor*/);

class SLockedActorsListTab : public SCompoundWidget
{
public:
	friend class SLockedActorsListRow;

	SLATE_BEGIN_ARGS(SLockedActorsListTab)
		: _InitialItems()
		{
		}

		SLATE_ARGUMENT(TArray<TSharedPtr<FLockedActorListItem>>, InitialItems)
		SLATE_EVENT(FOnSetActorLockState, OnSetActorLockState)
		SLATE_EVENT(FOnUnlockAllActors, OnUnlockAllActors)
		SLATE_EVENT(FOnRequestLockedActorRows, OnRequestRefreshData)
		SLATE_EVENT(FOnLockedActorRowDoubleClicked, OnRowDoubleClicked)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
	void HighlightActorRow(TWeakObjectPtr<AActor> ActorPtr);
	void RequestRefresh();

private:
	struct FLockedActorsFilterOption
	{
		FLockedActorsFilterOption(ELockedActorsViewMode InMode, const FText& InLabel)
			: Mode(InMode)
			, Label(InLabel)
		{
		}

		ELockedActorsViewMode Mode;
		FText Label;
	};

	TSharedRef<SWidget> BuildRootLayout(const FSlateRoundedBoxBrush& CardBrush);
	TSharedRef<SWidget> BuildToolbar();
	TSharedRef<SWidget> BuildListView();
	void InitializeFilterOptions();
	void RefreshDisplayedActors();
	void HandleLockStateChanged(ECheckBoxState NewState, TSharedPtr<FLockedActorListItem> Item);
	FReply HandleUnlockAllButtonClicked();
	FReply HandleRefreshButtonClicked();
	void HandleListSelectionChanged(TSharedPtr<FLockedActorListItem> Item, ESelectInfo::Type SelectInfo);
	void HandleRowDoubleClicked(TSharedPtr<FLockedActorListItem> Item);
	TSharedRef<ITableRow> OnGenerateRow(TSharedPtr<FLockedActorListItem> Item, const TSharedRef<STableViewBase>& OwnerTable);
	TSharedRef<SWidget> OnGenerateFilterWidget(TSharedPtr<FLockedActorsFilterOption> InOption) const;
	void OnFilterSelectionChanged(TSharedPtr<FLockedActorsFilterOption> NewSelection, ESelectInfo::Type SelectInfo);
	FText GetCurrentFilterText() const;
	void SyncSelectionToHighlightedActor();

private:
	TArray<TSharedPtr<FLockedActorListItem>> SourceActors;
	TArray<TSharedPtr<FLockedActorListItem>> DisplayedActors;
	TSharedPtr<SListView<TSharedPtr<FLockedActorListItem>>> ActorsListView;

	TArray<TSharedPtr<FLockedActorsFilterOption>> FilterOptions;
	TSharedPtr<FLockedActorsFilterOption> CurrentFilterOption;
	TSharedPtr<SComboBox<TSharedPtr<FLockedActorsFilterOption>>> FilterComboBox;

	FOnSetActorLockState SetActorLockDelegate;
	FOnUnlockAllActors UnlockAllActorsDelegate;
	FOnRequestLockedActorRows RefreshDataDelegate;
	FOnLockedActorRowDoubleClicked RowDoubleClickedDelegate;

	TWeakObjectPtr<AActor> HighlightedActorPtr;
};
