#include "SlateWidgets/LockedActorsListRow.h"

#include "CustomStyle/SuperManagerStyle.h"
#include "SlateWidgets/LockedActorsListWidget.h"
#include "Styling/AppStyle.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Text/STextBlock.h"

namespace LockedActorsRowStyle
{
	static const FSlateBrush* ResolveBrush(const FName BrushName)
	{
		const FSlateBrush* Brush = FSuperManagerStyleSetRegistry::GetBrush(BrushName);
		checkf(Brush, TEXT("Missing SuperManager brush entry: %s"), *BrushName.ToString());
		return Brush;
	}

	static const FTableRowStyle& GetRowStyle()
	{
		static const FTableRowStyle RowStyle = []()
		{
			FTableRowStyle Style = FAppStyle::Get().GetWidgetStyle<FTableRowStyle>("TableView.DarkRow");
			const FSlateBrush* HighlightedBrush = ResolveBrush(FSuperManagerStyleNames::HighlightedRowBrushName);
			const FSlateBrush* InactiveBrush = ResolveBrush(FSuperManagerStyleNames::InactiveRowBrushName);
			Style.SetActiveBrush(*HighlightedBrush);
			Style.SetInactiveBrush(*InactiveBrush);
			Style.SetActiveHighlightedBrush(*HighlightedBrush);
			Style.SetInactiveHighlightedBrush(*HighlightedBrush);
			return Style;
		}();
		return RowStyle;
	}
}

//1
void SLockedActorsListRow::Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& OwnerTableView)
{
    Item = InArgs._Item;
    OwnerWidget = InArgs._OwnerWidget;

    SMultiColumnTableRow<TSharedPtr<FLockedActorListItem>>::Construct(
        SMultiColumnTableRow<TSharedPtr<FLockedActorListItem>>::FArguments()
        .Style(&LockedActorsRowStyle::GetRowStyle()),
        OwnerTableView);
}

TSharedRef<SWidget> SLockedActorsListRow::GenerateWidgetForColumn(const FName& ColumnName)
{
    if (ColumnName == LockedActorsListColumns::LockColumn)
    {
        const FSlateBrush* CellBrush = LockedActorsRowStyle::ResolveBrush(FSuperManagerStyleNames::EmphasisCellBrushName);
        return SNew(SBorder)
            .Padding(FMargin(4.f))
            .BorderImage(CellBrush)
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
            .ColorAndOpacity(FSlateColor(FSuperManagerStyleSetRegistry::GetPalette().AccentColor));
    }

    return SNullWidget::NullWidget;
}
