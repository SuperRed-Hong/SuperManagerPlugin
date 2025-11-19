// Fill out your copyright notice in the Description page of Project Settings.

#pragma once


#include "ISceneOutlinerColumn.h"
class FOutlinerSelectionLockColumn : public ISceneOutlinerColumn
{
public:
	FOutlinerSelectionLockColumn(ISceneOutliner& SceneOutliner);
	virtual FName GetColumnID() override {return FName("SelectionLockColumn");}

	static FName GetID() {return FName("SelectionLockColumn");}
	virtual SHeaderRow::FColumn::FArguments ConstructHeaderRowColumn() override;

	virtual const TSharedRef< SWidget> ConstructRowWidget(FSceneOutlinerTreeItemRef TreeItem, const STableRow<FSceneOutlinerTreeItemPtr>& Row) override;


private:
	void OnRowWidgetCheckBoxStateChanged(ECheckBoxState NewState, TWeakObjectPtr<AActor> CorrespondingActor );

};