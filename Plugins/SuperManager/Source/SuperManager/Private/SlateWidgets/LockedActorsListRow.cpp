#include "SlateWidgets/LockedActorsListRow.h"

#include "CustomStyle/SuperManagerStyle.h"
#include "SlateWidgets/LockedActorsListWidget.h"
#include "Brushes/SlateRoundedBoxBrush.h"
#include "Styling/AppStyle.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Text/STextBlock.h"
//1
void SLockedActorsListRow::Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& OwnerTableView)
{
    Item = InArgs._Item;
    OwnerWidget = InArgs._OwnerWidget;

    static const FTableRowStyle RowStyle = []()
    {
        FTableRowStyle Style = FAppStyle::Get().GetWidgetStyle<FTableRowStyle>("TableView.DarkRow");
        Style.SetActiveBrush(FSuperManagerStyle::HighlightedRowBrush);
        Style.SetInactiveBrush(FSuperManagerStyle::InactiveRowBrush);
        Style.SetActiveHighlightedBrush(FSuperManagerStyle::HighlightedRowBrush);
        Style.SetInactiveHighlightedBrush(FSuperManagerStyle::HighlightedRowBrush);
        return Style;
    }();

    SMultiColumnTableRow<TSharedPtr<FLockedActorListItem>>::Construct(
        SMultiColumnTableRow<TSharedPtr<FLockedActorListItem>>::FArguments()
        .Style(&RowStyle),
        OwnerTableView);
}

TSharedRef<SWidget> SLockedActorsListRow::GenerateWidgetForColumn(const FName& ColumnName)
{
    if (ColumnName == LockedActorsListColumns::LockColumn)
    {
        return SNew(SBorder)
            .Padding(FMargin(4.f))
            .BorderImage(&FSuperManagerStyle::EmphasisCellBrush)
            [
                SNew(SCheckBox)
                .ToolTipText(NSLOCTEXT("LockedActors", "ToggleActorLock", "切换该 Actor 的锁定状态"))
                .IsChecked_Lambda([WeakItem = Item]()
                {
                    return WeakItem.IsValid() && WeakItem->bIsLocked ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
                })
                .OnCheckStateChanged_Lambda([WeakOwner = OwnerWidget, WeakItem = Item](ECheckBoxState NewState)
                {
                    if (TSharedPtr<SLockedActorsListTab> Owner = WeakOwner.Pin())
                    {
                        Owner->RequestLockStateChange(NewState, WeakItem);
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
                    : NSLOCTEXT("LockedActors", "MissingActor", "<Missing Actor>");
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
                    return NSLOCTEXT("LockedActors", "UnknownClass", "Unknown Class");
                }
                return FText::FromString(ActorPtr->GetClass()->GetName());
            })
            .Font(FAppStyle::GetFontStyle("ItalicFont"))
            .ColorAndOpacity(FSlateColor(FSuperManagerStyle::AccentColor));
    }

    return SNullWidget::NullWidget;
}
