#pragma once

#include "Widgets/Views/STableRow.h"

class SLockedActorsListTab;
struct FLockedActorListItem;

/**
 * Row widget for displaying a single actor entry in the locked actors list.
 */
class SLockedActorsListRow : public SMultiColumnTableRow<TSharedPtr<FLockedActorListItem>>
{
public:
    SLATE_BEGIN_ARGS(SLockedActorsListRow) {}
        SLATE_ARGUMENT(TSharedPtr<FLockedActorListItem>, Item)
        SLATE_ARGUMENT(TWeakPtr<SLockedActorsListTab>, OwnerWidget)
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& OwnerTableView);

    // Begin SMultiColumnTableRow
    virtual TSharedRef<SWidget> GenerateWidgetForColumn(const FName& ColumnName) override;
    // End SMultiColumnTableRow

private:
    TSharedPtr<FLockedActorListItem> Item;
    TWeakPtr<SLockedActorsListTab> OwnerWidget;
};
