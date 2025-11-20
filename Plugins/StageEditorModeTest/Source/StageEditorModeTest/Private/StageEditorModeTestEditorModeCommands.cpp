// Copyright Epic Games, Inc. All Rights Reserved.

#include "StageEditorModeTestEditorModeCommands.h"
#include "StageEditorModeTestEditorMode.h"
#include "EditorStyleSet.h"

#define LOCTEXT_NAMESPACE "StageEditorModeTestEditorModeCommands"

FStageEditorModeTestEditorModeCommands::FStageEditorModeTestEditorModeCommands()
	: TCommands<FStageEditorModeTestEditorModeCommands>("StageEditorModeTestEditorMode",
		NSLOCTEXT("StageEditorModeTestEditorMode", "StageEditorModeTestEditorModeCommands", "StageEditorModeTest Editor Mode"),
		NAME_None,
		FAppStyle::GetAppStyleSetName())
{
}

void FStageEditorModeTestEditorModeCommands::RegisterCommands()
{
	TArray <TSharedPtr<FUICommandInfo>>& ToolCommands = Commands.FindOrAdd(NAME_Default);

	UI_COMMAND(SimpleTool, "Show Actor Info", "Opens message box with info about a clicked actor", EUserInterfaceActionType::Button, FInputChord());
	ToolCommands.Add(SimpleTool);

	UI_COMMAND(InteractiveTool, "Measure Distance", "Measures distance between 2 points (click to set origin, shift-click to set end point)", EUserInterfaceActionType::ToggleButton, FInputChord());
	ToolCommands.Add(InteractiveTool);
}

TMap<FName, TArray<TSharedPtr<FUICommandInfo>>> FStageEditorModeTestEditorModeCommands::GetCommands()
{
	return FStageEditorModeTestEditorModeCommands::Get().Commands;
}

#undef LOCTEXT_NAMESPACE
