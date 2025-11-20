// Copyright Epic Games, Inc. All Rights Reserved.

#include "StageEditorModeTestEditorModeToolkit.h"
#include "StageEditorModeTestEditorMode.h"
#include "Engine/Selection.h"

#include "Modules/ModuleManager.h"
#include "PropertyEditorModule.h"
#include "IDetailsView.h"
#include "EditorModeManager.h"

#define LOCTEXT_NAMESPACE "StageEditorModeTestEditorModeToolkit"

FStageEditorModeTestEditorModeToolkit::FStageEditorModeTestEditorModeToolkit()
{
}

void FStageEditorModeTestEditorModeToolkit::Init(const TSharedPtr<IToolkitHost>& InitToolkitHost, TWeakObjectPtr<UEdMode> InOwningMode)
{
	FModeToolkit::Init(InitToolkitHost, InOwningMode);
}

void FStageEditorModeTestEditorModeToolkit::GetToolPaletteNames(TArray<FName>& PaletteNames) const
{
	PaletteNames.Add(NAME_Default);
}


FName FStageEditorModeTestEditorModeToolkit::GetToolkitFName() const
{
	return FName("StageEditorModeTestEditorMode");
}

FText FStageEditorModeTestEditorModeToolkit::GetBaseToolkitName() const
{
	return LOCTEXT("DisplayName", "StageEditorModeTestEditorMode Toolkit");
}

#undef LOCTEXT_NAMESPACE
