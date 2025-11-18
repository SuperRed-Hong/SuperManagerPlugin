// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Input/SComboBox.h"
#include "Widgets/Views/SListView.h"
#include "Widgets/Views/SHeaderRow.h"
#include "Brushes/SlateRoundedBoxBrush.h"
#include "LockedActorsListRow.h"
class AActor;
class UEditorActorSubsystem;
class SSearchBox;
class SCheckBox;

/**
 * 表示锁定列表中的一行数据，包含目标 Actor 与锁定状态。
 */
struct FLockedActorListItem
{
	TWeakObjectPtr<AActor> Actor;
	bool bIsLocked = false;
};

/** 支持的筛选模式。 */
enum class ELockedActorsViewMode : uint8
{
	AllActors,
	LockedOnly
};

namespace LockedActorsListColumns
{
	inline const FName LockColumn(TEXT("Lock"));
	inline const FName ActorColumn(TEXT("Actor"));
	inline const FName ClassColumn(TEXT("Class"));
}

DECLARE_DELEGATE_TwoParams(FOnSetActorLockState, TWeakObjectPtr<AActor> /*Actor*/, bool /*bShouldLock*/);
DECLARE_DELEGATE(FOnUnlockAllActors);
DECLARE_DELEGATE_RetVal(TArray<TSharedPtr<FLockedActorListItem>>, FOnRequestLockedActorRows);
DECLARE_DELEGATE_OneParam(FOnLockedActorRowDoubleClicked, TWeakObjectPtr<AActor> /*Actor*/);

namespace LockedActorsListStyle
{
	inline const FLinearColor CardColor(0.07f, 0.09f, 0.15f, 0.92f);
	inline const FLinearColor ToolbarColor(0.12f, 0.16f, 0.26f, 0.9f);
	inline const FLinearColor ControlColor(0.16f, 0.2f, 0.34f, 0.85f);
	inline const FLinearColor AccentColor(0.4f, 0.7f, 1.0f, 1.f);
	inline const FLinearColor AccentHoverColor(0.36f, 0.6f, 0.98f, 1.f);
	inline const FLinearColor NeutralButtonColor(0.36f, 0.4f, 0.55f, 0.9f);

	inline const FSlateRoundedBoxBrush GlassCardBrush(CardColor, 18.f);
	inline const FSlateRoundedBoxBrush ToolbarBrush(ToolbarColor, 14.f);
	inline const FSlateRoundedBoxBrush ControlBrush(ControlColor, 10.f);
	inline const FSlateRoundedBoxBrush LockCellBrush(FLinearColor(0.16f, 0.2f, 0.35f, 0.7f), 10.f);
	inline const FSlateRoundedBoxBrush RowActiveBrush(FLinearColor(0.27f, 0.45f, 0.95f, 0.35f), 10.f);
	inline const FSlateRoundedBoxBrush RowInactiveBrush(FLinearColor(0.09f, 0.12f, 0.18f, 0.65f), 10.f);
}


/**
 * 锁定 Actor 面板主体，负责列表渲染、筛选、排序与批量操作。
 */
class SLockedActorsListTab : public SCompoundWidget
{
public:
	/**
	 * Slate 构造入口。
	 * @param InArgs Slate 参数集合。
	 */
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

	/** 构建控件层级并初始化委托。 */
	void Construct(const FArguments& InArgs);

	/**
	 * 高亮指定 Actor 所在的行。
	 * @param ActorPtr 需要同步 selection 的 Actor。
	 */
	void HighlightActorRow(TWeakObjectPtr<AActor> ActorPtr);

	/** 触发重新拉取锁定 Actor 数据。 */
	void RequestRefresh();

	/**
	 * 供行控件调用的锁定状态修改入口。
	 * @param NewState 新的复选框状态。
	 * @param Item 目标行数据。
	 */
	void RequestLockStateChange(ECheckBoxState NewState, TSharedPtr<FLockedActorListItem> Item);

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
	void StartDataRefresh();
	void RefreshDisplayedActors();
	void ApplyFiltersAndSorting();
	void ApplySorting();
	bool DoesItemMatchSearch(const TSharedPtr<FLockedActorListItem>& Item) const;
	void HandleLockStateChanged(ECheckBoxState NewState, TSharedPtr<FLockedActorListItem> Item);

	FReply HandleUnlockAllButtonClicked();
	FReply HandleRefreshButtonClicked();
	void HandleListSelectionChanged(TSharedPtr<FLockedActorListItem> Item, ESelectInfo::Type SelectInfo);
	void HandleRowDoubleClicked(TSharedPtr<FLockedActorListItem> Item);
	void OnSearchTextChanged(const FText& InText);
	void OnSortModeChanged(EColumnSortPriority::Type SortPriority, const FName& ColumnId, EColumnSortMode::Type NewSortMode);
	TSharedRef<ITableRow> OnGenerateRow(TSharedPtr<FLockedActorListItem> Item, const TSharedRef<STableViewBase>& OwnerTable);
	TSharedRef<SWidget> OnGenerateFilterWidget(TSharedPtr<FLockedActorsFilterOption> InOption) const;
	void OnFilterSelectionChanged(TSharedPtr<FLockedActorsFilterOption> NewSelection, ESelectInfo::Type SelectInfo);
	void OnHeaderLockCheckStateChanged(ECheckBoxState NewState);
	void UpdateHeaderLockCheckState();
	FText GetCurrentFilterText() const;
	void SyncSelectionToHighlightedActor();

	TArray<TSharedPtr<FLockedActorListItem>> SourceActors;
	TArray<TSharedPtr<FLockedActorListItem>> DisplayedActors;
	TSharedPtr<SListView<TSharedPtr<FLockedActorListItem>>> ActorsListView;

	TArray<TSharedPtr<FLockedActorsFilterOption>> FilterOptions;
	TSharedPtr<FLockedActorsFilterOption> CurrentFilterOption;
	TSharedPtr<SComboBox<TSharedPtr<FLockedActorsFilterOption>>> FilterComboBox;
	TSharedPtr<SSearchBox> SearchBoxWidget;
	TSharedPtr<SCheckBox> HeaderLockCheckBox;

	FOnSetActorLockState SetActorLockDelegate;
	FOnUnlockAllActors UnlockAllActorsDelegate;
	FOnRequestLockedActorRows RefreshDataDelegate;
	FOnLockedActorRowDoubleClicked RowDoubleClickedDelegate;

	TWeakObjectPtr<AActor> HighlightedActorPtr;
	FText SearchText;
	bool bIsRefreshing = false;
	EColumnSortMode::Type ActorColumnSortMode = EColumnSortMode::Ascending;
	EColumnSortMode::Type ClassColumnSortMode = EColumnSortMode::None;
	FName ActiveSortColumn = LockedActorsListColumns::ActorColumn;
	bool bSortAscending = true;
	ECheckBoxState HeaderLockCheckState = ECheckBoxState::Undetermined;
};
