// Fill out your copyright notice in the Description page of Project Settings.


#include "SlateWidgets/LockedActorsListWidget.h"
#include "SlateWidgets/LockedActorsListRow.h"

#include "Styling/AppStyle.h"
#include "Styling/SlateTypes.h"
#include "Templates/UnrealTemplate.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Input/SComboBox.h"
#include "Widgets/Input/SSearchBox.h"
#include "Widgets/Images/SThrobber.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SSpacer.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/SNullWidget.h"
#include "Widgets/Views/SHeaderRow.h"
#include "Widgets/Views/STableRow.h"
#include "Misc/MessageDialog.h"
#define LOCTEXT_NAMESPACE "SLockedActorsListTab"

namespace LockedActorsListStyle
{
	static const FSlateBrush* ResolveBrush(const FName BrushName)
	{
		const FSlateBrush* Brush = FSuperManagerStyleSetRegistry::GetBrush(BrushName);
		checkf(Brush, TEXT("Missing SuperManager style entry: %s"), *BrushName.ToString());
		return Brush;
	}

	static FTableRowStyle CreateRowStyle()
	{
		FTableRowStyle Style = FAppStyle::Get().GetWidgetStyle<FTableRowStyle>("TableView.DarkRow");
		const FSlateBrush* HighlightedBrush = ResolveBrush(FSuperManagerStyleNames::HighlightedRowBrushName);
		const FSlateBrush* InactiveBrush = ResolveBrush(FSuperManagerStyleNames::InactiveRowBrushName);
		Style.SetActiveBrush(*HighlightedBrush);
		Style.SetInactiveBrush(*InactiveBrush);
		Style.SetActiveHighlightedBrush(*HighlightedBrush);
		Style.SetInactiveHighlightedBrush(*HighlightedBrush);
		return Style;
	}
}



void SLockedActorsListTab::Construct(const FArguments& InArgs)
{
	bCanSupportFocus = true;
	SetActorLockDelegate = InArgs._OnSetActorLockState;
	UnlockAllActorsDelegate = InArgs._OnUnlockAllActors;
	RefreshDataDelegate = InArgs._OnRequestRefreshData;
	RowDoubleClickedDelegate = InArgs._OnRowDoubleClicked;

	SourceActors = InArgs._InitialItems;
	InitializeFilterOptions();
	StartDataRefresh();

	ChildSlot
	[
		BuildRootLayout()
	];
}


TSharedRef<SWidget> SLockedActorsListTab::BuildRootLayout()
{
	const FSlateBrush* CardBrush = LockedActorsListStyle::ResolveBrush(FSuperManagerStyleNames::PrimaryCardBrushName);
	return SNew(SBorder)
	    .BorderImage(CardBrush)
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
	const FSlateBrush* ToolbarBrush = LockedActorsListStyle::ResolveBrush(FSuperManagerStyleNames::PrimaryToolbarBrushName);
	const FSlateBrush* PanelBrush = LockedActorsListStyle::ResolveBrush(FSuperManagerStyleNames::PrimaryPanelBrushName);
	const FSuperManagerPalette& Palette = FSuperManagerStyleSetRegistry::GetPalette();
	return SNew(SBorder)
	    .BorderImage(ToolbarBrush)
        .Padding(FMargin(16.f, 12.f))
        [
            SNew(SHorizontalBox)
            + SHorizontalBox::Slot()
            .AutoWidth()
            .Padding(0.f, 0.f, 12.f, 0.f)
			[
				SAssignNew(SearchBoxWidget, SSearchBox)
				.MinDesiredWidth(250.f)
				.InitialText(SearchText)
				.OnTextChanged(this, &SLockedActorsListTab::OnSearchTextChanged)
			]
            + SHorizontalBox::Slot()
            .AutoWidth()
            .Padding(0.f, 0.f, 12.f, 0.f)
            [
                SNew(SBorder)
                .BorderImage(PanelBrush)
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
            .FillWidth(1.f)
            [
                SNew(SSpacer)
            ]
            + SHorizontalBox::Slot()
            .AutoWidth()
            .Padding(4.f, 0.f)
            [
                SNew(SButton)
                .ButtonColorAndOpacity(Palette.NeutralButtonColor)
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
                .ButtonColorAndOpacity(Palette.AccentColor)
                .ForegroundColor(FLinearColor::White)
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
	                .ColorAndOpacity(FLinearColor::White)
                ]
            ]
            + SHorizontalBox::Slot()
            .AutoWidth()
            .Padding(4.f, 0.f)
            [
                SNew(SThrobber)
                .Visibility_Lambda([this]()
                {
                    return bIsRefreshing ? EVisibility::Visible : EVisibility::Collapsed;
                })
            ]
        ];
}
TSharedRef<SWidget> SLockedActorsListTab::BuildListView()
{
	const FSlateBrush* PanelBrush = LockedActorsListStyle::ResolveBrush(FSuperManagerStyleNames::PrimaryPanelBrushName);
	return SNew(SBorder)
	    .BorderImage(PanelBrush)
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
                  .ManualWidth(110.f)
                  .HAlignCell(HAlign_Center)
                  .HAlignHeader(HAlign_Center)
                [
                    SAssignNew(HeaderLockCheckBox, SCheckBox)
                    .HAlign(HAlign_Center)
                    .IsChecked_Lambda([this]() { return HeaderLockCheckState; })
                    .OnCheckStateChanged(this, &SLockedActorsListTab::OnHeaderLockCheckStateChanged)
                ]
                + SHeaderRow::Column(LockedActorsListColumns::ActorColumn)
                  .DefaultLabel(LOCTEXT("HeaderActor", "Actor Label"))
                  .SortMode_Lambda([this]() { return ActorColumnSortMode; })
                  .OnSort(this, &SLockedActorsListTab::OnSortModeChanged)
                  .FillWidth(0.55f)
                + SHeaderRow::Column(LockedActorsListColumns::ClassColumn)
                  .DefaultLabel(LOCTEXT("HeaderClass", "Class Name"))
                  .SortMode_Lambda([this]() { return ClassColumnSortMode; })
                  .OnSort(this, &SLockedActorsListTab::OnSortModeChanged)
                  .FillWidth(0.45f)
            )
        ];
}

void SLockedActorsListTab::StartDataRefresh()
{
    if (bIsRefreshing)
    {
        return;
    }

    TGuardValue<bool> RefreshGuard(bIsRefreshing, true);
    RefreshDisplayedActors();
}

void SLockedActorsListTab::ApplyFiltersAndSorting()
{
    DisplayedActors.Reset();
    for (const TSharedPtr<FLockedActorListItem>& Item : SourceActors)
    {
        if (!Item.IsValid())
        {
            continue;
        }

        const bool bShowAll = !CurrentFilterOption.IsValid() || CurrentFilterOption->Mode == ELockedActorsViewMode::AllActors;
        const bool bShowLockedOnly = CurrentFilterOption.IsValid() && CurrentFilterOption->Mode == ELockedActorsViewMode::LockedOnly && Item->bIsLocked;

        if ((bShowAll || bShowLockedOnly) && DoesItemMatchSearch(Item))
        {
            DisplayedActors.Add(Item);
        }
    }

    ApplySorting();
    UpdateHeaderLockCheckState();

    if (ActorsListView.IsValid())
    {
        ActorsListView->RequestListRefresh();
    }

    SyncSelectionToHighlightedActor();
}

void SLockedActorsListTab::ApplySorting()
{
    if (ActiveSortColumn == LockedActorsListColumns::ActorColumn)
    {
        DisplayedActors.Sort([this](const TSharedPtr<FLockedActorListItem>& LHS, const TSharedPtr<FLockedActorListItem>& RHS)
        {
            const FString LeftLabel = LHS.IsValid() && LHS->Actor.IsValid() ? LHS->Actor->GetActorLabel() : TEXT("");
            const FString RightLabel = RHS.IsValid() && RHS->Actor.IsValid() ? RHS->Actor->GetActorLabel() : TEXT("");
            return bSortAscending ? LeftLabel < RightLabel : LeftLabel > RightLabel;
        });
        ActorColumnSortMode = bSortAscending ? EColumnSortMode::Ascending : EColumnSortMode::Descending;
        ClassColumnSortMode = EColumnSortMode::None;
    }
    else if (ActiveSortColumn == LockedActorsListColumns::ClassColumn)
    {
        DisplayedActors.Sort([this](const TSharedPtr<FLockedActorListItem>& LHS, const TSharedPtr<FLockedActorListItem>& RHS)
        {
            const FString LeftClass = (LHS.IsValid() && LHS->Actor.IsValid() && LHS->Actor->GetClass()) ? LHS->Actor->GetClass()->GetName() : TEXT("");
            const FString RightClass = (RHS.IsValid() && RHS->Actor.IsValid() && RHS->Actor->GetClass()) ? RHS->Actor->GetClass()->GetName() : TEXT("");
            return bSortAscending ? LeftClass < RightClass : LeftClass > RightClass;
        });
        ClassColumnSortMode = bSortAscending ? EColumnSortMode::Ascending : EColumnSortMode::Descending;
        ActorColumnSortMode = EColumnSortMode::None;
    }
}

bool SLockedActorsListTab::DoesItemMatchSearch(const TSharedPtr<FLockedActorListItem>& Item) const
{
    if (SearchText.IsEmpty())
    {
        return true;
    }

    if (!Item.IsValid() || !Item->Actor.IsValid())
    {
        return false;
    }

    const FString Query = SearchText.ToString();
    const FString Label = Item->Actor->GetActorLabel();
    const FString ClassName = Item->Actor->GetClass() ? Item->Actor->GetClass()->GetName() : TEXT("");

    return Label.Contains(Query, ESearchCase::IgnoreCase, ESearchDir::FromStart)
        || ClassName.Contains(Query, ESearchCase::IgnoreCase, ESearchDir::FromStart);
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

	ApplyFiltersAndSorting();
}

void SLockedActorsListTab::HandleLockStateChanged(ECheckBoxState NewState, TSharedPtr<FLockedActorListItem> Item)
{
	if (!Item.IsValid())
	{
		return;
	}

	Item->bIsLocked = (NewState == ECheckBoxState::Checked);

	if (SetActorLockDelegate.IsBound())
	{
		SetActorLockDelegate.Execute(Item->Actor, Item->bIsLocked);
	}

	StartDataRefresh();
}

FReply SLockedActorsListTab::HandleUnlockAllButtonClicked()
{
	const EAppReturnType::Type UserChoice = FMessageDialog::Open(
		EAppMsgType::YesNo,
		LOCTEXT("UnlockAllActorsConfirm", "是否解锁列表中的所有 Actor？"),
		LOCTEXT("UnlockAllActorsConfirmTitle", "解锁全部确认"));

	if (UserChoice != EAppReturnType::Yes)
	{
		return FReply::Handled();
	}

	if (UnlockAllActorsDelegate.IsBound())
	{
		UnlockAllActorsDelegate.Execute();
	}

	StartDataRefresh();
	return FReply::Handled();
}

FReply SLockedActorsListTab::HandleRefreshButtonClicked()
{
	StartDataRefresh();
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


void SLockedActorsListTab::OnFilterSelectionChanged(TSharedPtr<FLockedActorsFilterOption> NewSelection, ESelectInfo::Type InSelectInfo)
{
    CurrentFilterOption = NewSelection;
    ApplyFiltersAndSorting();
}

void SLockedActorsListTab::OnSearchTextChanged(const FText& InText)
{
    SearchText = InText;
    ApplyFiltersAndSorting();
}

void SLockedActorsListTab::OnSortModeChanged(EColumnSortPriority::Type /*SortPriority*/, const FName& ColumnId, EColumnSortMode::Type NewSortMode)
{
	ActiveSortColumn = ColumnId;
	bSortAscending = (NewSortMode != EColumnSortMode::Descending);

	if (NewSortMode == EColumnSortMode::None)
	{
		// Slate 会在第三次点击时返回 None，保持上一次排序方向，默认为升序。
		bSortAscending = true;
	}

	ApplyFiltersAndSorting();
}

void SLockedActorsListTab::OnHeaderLockCheckStateChanged(ECheckBoxState NewState)
{
    HeaderLockCheckState = NewState;

    if (!SetActorLockDelegate.IsBound())
    {
        return;
    }

    const bool bShouldLock = (NewState == ECheckBoxState::Checked);
    for (const TSharedPtr<FLockedActorListItem>& Item : DisplayedActors)
    {
        if (Item.IsValid())
        {
            Item->bIsLocked = bShouldLock;
            SetActorLockDelegate.Execute(Item->Actor, bShouldLock);
        }
    }

    StartDataRefresh();
}

void SLockedActorsListTab::UpdateHeaderLockCheckState()
{
	if (!HeaderLockCheckBox.IsValid())
	{
		return;
	}

    int32 LockedCount = 0;
    for (const TSharedPtr<FLockedActorListItem>& Item : DisplayedActors)
    {
        if (Item.IsValid() && Item->bIsLocked)
        {
            ++LockedCount;
        }
    }

    if (LockedCount == 0)
    {
        HeaderLockCheckState = ECheckBoxState::Unchecked;
    }
    else if (LockedCount == DisplayedActors.Num())
    {
        HeaderLockCheckState = ECheckBoxState::Checked;
    }
    else
    {
        HeaderLockCheckState = ECheckBoxState::Undetermined;
    }

	if (HeaderLockCheckBox->GetCheckedState() != HeaderLockCheckState)
	{
		HeaderLockCheckBox->SetIsChecked(HeaderLockCheckState);
	}
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
	StartDataRefresh();
}
void SLockedActorsListTab::RequestLockStateChange(ECheckBoxState NewState, TSharedPtr<FLockedActorListItem> Item)
{
	HandleLockStateChanged(NewState, Item);
}
