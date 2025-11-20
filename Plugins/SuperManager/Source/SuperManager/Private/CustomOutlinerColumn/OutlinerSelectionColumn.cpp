// Fill out your copyright notice in the Description page of Project Settings.


#include "CustomOutlinerColumn/OutlinerSelectionColumn.h"
#include "CustomStyle/SuperManagerStyle.h"
#include "ActorTreeItem.h"
#include "EditorStyleSet.h"
#include "SuperManager.h"
FOutlinerSelectionLockColumn::FOutlinerSelectionLockColumn(ISceneOutliner& SceneOutliner)
{
}

SHeaderRow::FColumn::FArguments FOutlinerSelectionLockColumn::ConstructHeaderRowColumn()
{
	SHeaderRow::FColumn::FArguments ConstructHeaderRowColumn=
	SHeaderRow::Column(GetColumnID())
		.FixedWidth(24.f)
		.HAlignHeader(HAlign_Center)
		.VAlignHeader(VAlign_Center)
		.HAlignCell(HAlign_Center)
		.VAlignCell(VAlign_Center)
		.DefaultTooltip(FText::FromString(TEXT("Actor Selection Lock - Press icon to lock actor selection")))
		[
			SNew(SImage)
			.ColorAndOpacity(FSlateColor::UseForeground())
			.Image(FSuperManagerStyleSetRegistry::GetBrush(TEXT("SceneOutliner.LockActorSelection")))
		];
	return ConstructHeaderRowColumn;
}

const TSharedRef<SWidget> FOutlinerSelectionLockColumn::ConstructRowWidget(FSceneOutlinerTreeItemRef TreeItem,
                                                                           const STableRow<FSceneOutlinerTreeItemPtr>&
                                                                           Row)
{
	FActorTreeItem* ActorTreeItem =   TreeItem->CastTo<FActorTreeItem>();
	
	if (!ActorTreeItem || !ActorTreeItem->IsValid()) return SNullWidget::NullWidget;
	FSuperManagerModule& SuperManagerModule =  FModuleManager::LoadModuleChecked<FSuperManagerModule>("SuperManager");
	/*ECheckBoxState InitCheck;
	if (SuperManagerModule.CheckIsActorSelectionLocked(ActorTreeItem->Actor.Get()))
	{
		InitCheck = ECheckBoxState::Checked;
	}
	else
	{
		InitCheck = ECheckBoxState::Unchecked;
	}*/
	const bool bIsActorSelectionLocked = SuperManagerModule.CheckIsActorSelectionLocked(ActorTreeItem->Actor.Get());

	const FCheckBoxStyle& ToggleButtonStyle =
		FSuperManagerStyleSetRegistry::Get().GetWidgetStyle<FCheckBoxStyle>(FName("SceneOutliner.SelectionLock"));
	auto ConstructedRowCheckBox = 
	SNew(SCheckBox)
	.HAlign(HAlign_Center)
	.IsChecked(bIsActorSelectionLocked? ECheckBoxState::Checked : ECheckBoxState::Unchecked)
	.Visibility(EVisibility::Visible)
		.Type(ESlateCheckBoxType::ToggleButton)
		.Style(&ToggleButtonStyle)
		.OnCheckStateChanged(this, &FOutlinerSelectionLockColumn::OnRowWidgetCheckBoxStateChanged, ActorTreeItem->Actor);
	return ConstructedRowCheckBox;
}

void FOutlinerSelectionLockColumn::OnRowWidgetCheckBoxStateChanged(ECheckBoxState NewState,
	TWeakObjectPtr<AActor> CorrespondingActor)
{
	FSuperManagerModule& SuperManagerModule =  FModuleManager::LoadModuleChecked<FSuperManagerModule>("SuperManager");
	switch (NewState) {
	case ECheckBoxState::Unchecked:
		SuperManagerModule.ProcessLockingForOutliner(CorrespondingActor.Get(),false);
		break;
	case ECheckBoxState::Checked:
		SuperManagerModule.ProcessLockingForOutliner(CorrespondingActor.Get(),true);
		break;
	case ECheckBoxState::Undetermined:
		break;
	}
}
