// Fill out your copyright notice in the Description page of Project Settings.


#include "SlateWidgets/LockedActorsListWidget.h"

#include "Brushes/SlateRoundedBoxBrush.h"
#include "Styling/AppStyle.h"
#include "Styling/SlateTypes.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Input/SComboBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/SNullWidget.h"
#include "Widgets/Views/SHeaderRow.h"
#include "Widgets/Views/STableRow.h"
#define LOCTEXT_NAMESPACE "SLockedActorsListTab"

namespace LockedActorsListColumns
{
	static const FName LockColumn(TEXT("Lock"));
	static const FName ActorColumn(TEXT("Actor"));
	static const FName ClassColumn(TEXT("Class"));
}

namespace LockedActorsListStyle
{
	static const FLinearColor CardColor = FLinearColor(0.07f, 0.09f, 0.15f, 0.92f);
	static const FLinearColor ToolbarColor = FLinearColor(0.12f, 0.16f, 0.26f, 0.9f);
	static const FLinearColor ControlColor = FLinearColor(0.16f, 0.2f, 0.34f, 0.85f);
	static const FLinearColor AccentColor = FLinearColor(0.32f, 0.55f, 0.95f, 1.f);
	static const FLinearColor AccentHoverColor = FLinearColor(0.36f, 0.6f, 0.98f, 1.f);
	static const FLinearColor NeutralButtonColor = FLinearColor(0.36f, 0.4f, 0.55f, 0.9f);

	static const FSlateRoundedBoxBrush GlassCardBrush(CardColor, 18.f);
	static const FSlateRoundedBoxBrush ToolbarBrush(ToolbarColor, 14.f);
	static const FSlateRoundedBoxBrush ControlBrush(ControlColor, 10.f);
	static const FSlateRoundedBoxBrush LockCellBrush(FLinearColor(0.16f, 0.2f, 0.35f, 0.7f), 10.f);
	static const FSlateRoundedBoxBrush RowActiveBrush(FLinearColor(0.27f, 0.45f, 0.95f, 0.35f), 10.f);
	static const FSlateRoundedBoxBrush RowInactiveBrush(FLinearColor(0.09f, 0.12f, 0.18f, 0.65f), 10.f);

	static FTableRowStyle CreateRowStyle()
	{
		FTableRowStyle Style = FAppStyle::Get().GetWidgetStyle<FTableRowStyle>("TableView.DarkRow");
		Style.SetActiveBrush(RowActiveBrush);
		Style.SetInactiveBrush(RowInactiveBrush);
		Style.SetActiveHighlightedBrush(RowActiveBrush);
		Style.SetInactiveHighlightedBrush(RowActiveBrush);
		return Style;
	}
}

class SLockedActorsListRow : public SMultiColumnTableRow<TSharedPtr<FLockedActorListItem>>
{
public:
	SLATE_BEGIN_ARGS(SLockedActorsListRow) {}
		SLATE_ARGUMENT(TSharedPtr<FLockedActorListItem>, Item)
		SLATE_ARGUMENT(TWeakPtr<SLockedActorsListTab>, OwnerWidget)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& OwnerTableView)
	{
		Item = InArgs._Item;
		OwnerWidget = InArgs._OwnerWidget;

		static const FTableRowStyle RowStyle = LockedActorsListStyle::CreateRowStyle();

		SMultiColumnTableRow<TSharedPtr<FLockedActorListItem>>::Construct(
			SMultiColumnTableRow<TSharedPtr<FLockedActorListItem>>::FArguments()
			.Style(&RowStyle),
			OwnerTableView);
	}

	virtual TSharedRef<SWidget> GenerateWidgetForColumn(const FName& ColumnName) override
	{
		if (ColumnName == LockedActorsListColumns::LockColumn)
		{
			return SNew(SBorder)
				.Padding(FMargin(4.f))
				.BorderImage(&LockedActorsListStyle::LockCellBrush)
				[
					SNew(SCheckBox)
					.ToolTipText(LOCTEXT("ToggleActorLock", "切换该 Actor 的锁定状态"))
					.IsChecked_Lambda([WeakItem = Item]()
					{
						return WeakItem.IsValid() && WeakItem->bIsLocked ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
					})
					.OnCheckStateChanged_Lambda([WeakOwner = OwnerWidget, WeakItem = Item](ECheckBoxState NewState)
					{
						if (TSharedPtr<SLockedActorsListTab> Owner = WeakOwner.Pin())
						{
							Owner->HandleLockStateChanged(NewState, WeakItem);
						}
					})
				];
		}

		if (ColumnName == LockedActorsListColumns::ActorColumn)
		{
			return SNew(STextBlock)
				.Text_Lambda([WeakItem = Item]()
				{
					const TWeakObjectPtr<AActor> ActorPtr = WeakItem.IsValid() ? WeakItem->Actor : nullptr;
					return ActorPtr.IsValid()
						? FText::FromString(ActorPtr->GetActorLabel())
						: LOCTEXT("ActorInvalid", "<Missing Actor>");
				})
				.Font(FAppStyle::GetFontStyle("BoldFont"))
				.ColorAndOpacity(FSlateColor(FLinearColor(0.95f, 0.97f, 1.f)))
				.ShadowOffset(FVector2D(0.f, 1.f))
				.ShadowColorAndOpacity(FLinearColor(0.f, 0.f, 0.f, 0.5f));
		}

		if (ColumnName == LockedActorsListColumns::ClassColumn)
		{
			return SNew(STextBlock)
				.Text_Lambda([WeakItem = Item]()
				{
					const TWeakObjectPtr<AActor> ActorPtr = WeakItem.IsValid() ? WeakItem->Actor : nullptr;
					if (!ActorPtr.IsValid() || ActorPtr->GetClass() == nullptr)
					{
						return LOCTEXT("ClassInvalid", "Unknown Class");
					}
					return FText::FromString(ActorPtr->GetClass()->GetName());
				})
				.Font(FAppStyle::GetFontStyle("ItalicFont"))
				.ColorAndOpacity(FSlateColor(LockedActorsListStyle::AccentColor));
		}

		return SNullWidget::NullWidget;
	}

private:
	TSharedPtr<FLockedActorListItem> Item;
	TWeakPtr<SLockedActorsListTab> OwnerWidget;
};

void SLockedActorsListTab::Construct(const FArguments& InArgs)
{
	bCanSupportFocus = true;
	SetActorLockDelegate = InArgs._OnSetActorLockState;
	UnlockAllActorsDelegate = InArgs._OnUnlockAllActors;
	RefreshDataDelegate = InArgs._OnRequestRefreshData;
	RowDoubleClickedDelegate = InArgs._OnRowDoubleClicked;

	SourceActors = InArgs._InitialItems;
	InitializeFilterOptions();
	RefreshDisplayedActors();

	ChildSlot
	[
		BuildRootLayout(LockedActorsListStyle::GlassCardBrush)
	];
}

TSharedRef<SWidget> SLockedActorsListTab::BuildRootLayout(const FSlateRoundedBoxBrush& CardBrush)
{
	return SNew(SBorder)
		.BorderImage(&CardBrush)
		.Padding(FMargin(18.f, 16.f))
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.f, 0.f, 0.f, 12.f)
			[
				BuildToolbar()
			]
			+ SVerticalBox::Slot()
			.FillHeight(1.f)
			[
				BuildListView()
			]
		];
}

TSharedRef<SWidget> SLockedActorsListTab::BuildToolbar()
{
	return SNew(SBorder)
		.BorderImage(&LockedActorsListStyle::ToolbarBrush)
		.Padding(FMargin(16.f, 12.f))
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.FillWidth(1.f)
			.Padding(0.f, 0.f, 12.f, 0.f)
			[
				SNew(SBorder)
				.BorderImage(&LockedActorsListStyle::ControlBrush)
				.Padding(FMargin(12.f, 6.f))
				[
					SAssignNew(FilterComboBox, SComboBox<TSharedPtr<FLockedActorsFilterOption>>)
					.OptionsSource(&FilterOptions)
					.InitiallySelectedItem(CurrentFilterOption)
					.OnSelectionChanged(this, &SLockedActorsListTab::OnFilterSelectionChanged)
					.OnGenerateWidget(this, &SLockedActorsListTab::OnGenerateFilterWidget)
					[
						SNew(STextBlock)
						.Text(this, &SLockedActorsListTab::GetCurrentFilterText)
						.ColorAndOpacity(FSlateColor(FLinearColor(0.85f, 0.9f, 1.f)))
					]
				]
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(4.f, 0.f)
			[
				SNew(SButton)
				.ButtonColorAndOpacity(LockedActorsListStyle::NeutralButtonColor)
				.ForegroundColor(FLinearColor::White)
				.ContentPadding(FMargin(16.f, 8.f))
				.OnClicked(this, &SLockedActorsListTab::HandleRefreshButtonClicked)
				[
					SNew(STextBlock)
					.Text(LOCTEXT("RefreshButton", "刷新列表"))
					.Font(FAppStyle::GetFontStyle("BoldFont"))
				]
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(4.f, 0.f)
			[
				SNew(SButton)
				.ButtonColorAndOpacity(LockedActorsListStyle::AccentColor)
				.ForegroundColor(FLinearColor::Black)
				.ContentPadding(FMargin(16.f, 8.f))
				.OnClicked(this, &SLockedActorsListTab::HandleUnlockAllButtonClicked)
				.IsEnabled_Lambda([this]()
				{
					return SourceActors.ContainsByPredicate([](const TSharedPtr<FLockedActorListItem>& Item)
					{
						return Item.IsValid() && Item->bIsLocked;
					});
				})
				[
					SNew(STextBlock)
					.Text(LOCTEXT("UnlockAllButton", "解锁全部"))
					.Font(FAppStyle::GetFontStyle("BoldFont"))
				]
			]
		];
}

TSharedRef<SWidget> SLockedActorsListTab::BuildListView()
{
	return SNew(SBorder)
		.BorderImage(&LockedActorsListStyle::ControlBrush)
		.Padding(FMargin(6.f))
		[
			SAssignNew(ActorsListView, SListView<TSharedPtr<FLockedActorListItem>>)
			.ListItemsSource(&DisplayedActors)
			.SelectionMode(ESelectionMode::Single)
			.OnGenerateRow(this, &SLockedActorsListTab::OnGenerateRow)
			.OnSelectionChanged(this, &SLockedActorsListTab::HandleListSelectionChanged)
			.OnMouseButtonDoubleClick(this, &SLockedActorsListTab::HandleRowDoubleClicked)
			.HeaderRow
			(
				SNew(SHeaderRow)
				.Style(&FAppStyle::Get().GetWidgetStyle<FHeaderRowStyle>("TableView.Header"))
				+ SHeaderRow::Column(LockedActorsListColumns::LockColumn)
				  .DefaultLabel(LOCTEXT("HeaderLocked", "锁定"))
				  .FixedWidth(100.f)
				  .HAlignCell(HAlign_Center)
				  .HAlignHeader(HAlign_Center)
				+ SHeaderRow::Column(LockedActorsListColumns::ActorColumn)
				  .DefaultLabel(LOCTEXT("HeaderActor", "Actor Label"))
				  .FillWidth(0.55f)
				+ SHeaderRow::Column(LockedActorsListColumns::ClassColumn)
				  .DefaultLabel(LOCTEXT("HeaderClass", "Class Name"))
				  .FillWidth(0.45f)
			)
		];
}

void SLockedActorsListTab::InitializeFilterOptions()
{
	FilterOptions.Reset();
	FilterOptions.Add(MakeShared<FLockedActorsFilterOption>(ELockedActorsViewMode::AllActors, LOCTEXT("AllActors", "显示全部 Actor")));
	FilterOptions.Add(MakeShared<FLockedActorsFilterOption>(ELockedActorsViewMode::LockedOnly, LOCTEXT("LockedOnly", "仅显示已锁定")));

	if (FilterOptions.Num() > 0)
	{
		CurrentFilterOption = FilterOptions[0];
	}
}

void SLockedActorsListTab::RefreshDisplayedActors()
{
	if (RefreshDataDelegate.IsBound())
	{
		SourceActors = RefreshDataDelegate.Execute();
	}

	DisplayedActors.Reset();
	for (const TSharedPtr<FLockedActorListItem>& Item : SourceActors)
	{
		if (!Item.IsValid())
		{
			continue;
		}

		const bool bShowAll = !CurrentFilterOption.IsValid() || CurrentFilterOption->Mode == ELockedActorsViewMode::AllActors;
		const bool bShowLockedOnly = CurrentFilterOption.IsValid() && CurrentFilterOption->Mode == ELockedActorsViewMode::LockedOnly && Item->bIsLocked;

		if (bShowAll || bShowLockedOnly)
		{
			DisplayedActors.Add(Item);
		}
	}

	if (ActorsListView.IsValid())
	{
		ActorsListView->RequestListRefresh();
	}

	SyncSelectionToHighlightedActor();
}

void SLockedActorsListTab::HandleLockStateChanged(ECheckBoxState NewState, TSharedPtr<FLockedActorListItem> Item)
{
	//a
	if (!Item.IsValid())
	{
		return;
	}

	Item->bIsLocked = (NewState == ECheckBoxState::Checked);

	if (SetActorLockDelegate.IsBound())
	{
		SetActorLockDelegate.Execute(Item->Actor, Item->bIsLocked);
	}

	RefreshDisplayedActors();
}

FReply SLockedActorsListTab::HandleUnlockAllButtonClicked()
{
	if (UnlockAllActorsDelegate.IsBound())
	{
		UnlockAllActorsDelegate.Execute();
	}

	RefreshDisplayedActors();
	return FReply::Handled();
}

FReply SLockedActorsListTab::HandleRefreshButtonClicked()
{
	RefreshDisplayedActors();
	return FReply::Handled();
}

void SLockedActorsListTab::HandleListSelectionChanged(TSharedPtr<FLockedActorListItem> Item, ESelectInfo::Type SelectInfo)
{
	if (Item.IsValid())
	{
		HighlightedActorPtr = Item->Actor;
	}
	else
	{
		HighlightedActorPtr.Reset();
	}
}

void SLockedActorsListTab::HandleRowDoubleClicked(TSharedPtr<FLockedActorListItem> Item)
{
	if (!Item.IsValid())
	{
		return;
	}

	HighlightActorRow(Item->Actor);

	if (RowDoubleClickedDelegate.IsBound())
	{
		RowDoubleClickedDelegate.Execute(Item->Actor);
	}
}

TSharedRef<ITableRow> SLockedActorsListTab::OnGenerateRow(TSharedPtr<FLockedActorListItem> Item, const TSharedRef<STableViewBase>& OwnerTable)
{
	return SNew(SLockedActorsListRow, OwnerTable)
		.Item(Item)
		.OwnerWidget(SharedThis(this));
}

TSharedRef<SWidget> SLockedActorsListTab::OnGenerateFilterWidget(TSharedPtr<FLockedActorsFilterOption> InOption) const
{
	return SNew(STextBlock)
		.Text(InOption.IsValid() ? InOption->Label : FText::GetEmpty());
}

void SLockedActorsListTab::OnFilterSelectionChanged(TSharedPtr<FLockedActorsFilterOption> NewSelection, ESelectInfo::Type SelectInfo)
{
	CurrentFilterOption = NewSelection;
	RefreshDisplayedActors();
}

FText SLockedActorsListTab::GetCurrentFilterText() const
{
	return CurrentFilterOption.IsValid() ? CurrentFilterOption->Label : LOCTEXT("FilterDefault", "筛选模式");
}

void SLockedActorsListTab::SyncSelectionToHighlightedActor()
{
	if (!ActorsListView.IsValid())
	{
		return;
	}

	TSharedPtr<FLockedActorListItem> MatchingItem;
	for (const TSharedPtr<FLockedActorListItem>& Item : DisplayedActors)
	{
		if (Item.IsValid() && Item->Actor == HighlightedActorPtr)
		{
			MatchingItem = Item;
			break;
		}
	}

	if (MatchingItem.IsValid())
	{
		ActorsListView->SetSelection(MatchingItem);
		ActorsListView->RequestScrollIntoView(MatchingItem);
	}
	else
	{
		ActorsListView->ClearSelection();
	}
}

#undef LOCTEXT_NAMESPACE

void SLockedActorsListTab::HighlightActorRow(TWeakObjectPtr<AActor> ActorPtr)
{
	HighlightedActorPtr = ActorPtr;
	SyncSelectionToHighlightedActor();
}

void SLockedActorsListTab::RequestRefresh()
{
	RefreshDisplayedActors();
}
