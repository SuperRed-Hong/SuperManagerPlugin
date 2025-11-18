// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "ISceneOutliner.h"
#include "ISceneOutlinerColumn.h"
class FOutlinerSelectionLockColumn : public ISceneOutlinerColumn
{
public:
	FOutlinerSelectionLockColumn(ISceneOutliner& SceneOutliner);

};