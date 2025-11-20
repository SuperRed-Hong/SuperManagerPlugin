// Copyright Epic Games, Inc. All Rights Reserved.

#include "StageEditorModeTestEditorMode.h"
#include "StageEditorModeTestEditorModeToolkit.h"
#include "EdModeInteractiveToolsContext.h"
#include "InteractiveToolManager.h"
#include "StageEditorModeTestEditorModeCommands.h"
#include "Modules/ModuleManager.h"


//////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////// 
// AddYourTool Step 1 - include the header file for your Tools here
//////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////// 
#include "Tools/StageEditorModeTestSimpleTool.h"
#include "Tools/StageEditorModeTestInteractiveTool.h"

// step 2: register a ToolBuilder in FStageEditorModeTestEditorMode::Enter() below


#define LOCTEXT_NAMESPACE "StageEditorModeTestEditorMode"

const FEditorModeID UStageEditorModeTestEditorMode::EM_StageEditorModeTestEditorModeId = TEXT("EM_StageEditorModeTestEditorMode");

FString UStageEditorModeTestEditorMode::SimpleToolName = TEXT("StageEditorModeTest_ActorInfoTool");
FString UStageEditorModeTestEditorMode::InteractiveToolName = TEXT("StageEditorModeTest_MeasureDistanceTool");


UStageEditorModeTestEditorMode::UStageEditorModeTestEditorMode()
{
	FModuleManager::Get().LoadModule("EditorStyle");

	// appearance and icon in the editing mode ribbon can be customized here
	Info = FEditorModeInfo(UStageEditorModeTestEditorMode::EM_StageEditorModeTestEditorModeId,
		LOCTEXT("ModeName", "StageEditorModeTest"),
		FSlateIcon(),
		true);
}


UStageEditorModeTestEditorMode::~UStageEditorModeTestEditorMode()
{
}


void UStageEditorModeTestEditorMode::ActorSelectionChangeNotify()
{
}

void UStageEditorModeTestEditorMode::Enter()
{
	UEdMode::Enter();

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	// AddYourTool Step 2 - register the ToolBuilders for your Tools here.
	// The string name you pass to the ToolManager is used to select/activate your ToolBuilder later.
	//////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////// 
	const FStageEditorModeTestEditorModeCommands& SampleToolCommands = FStageEditorModeTestEditorModeCommands::Get();

	RegisterTool(SampleToolCommands.SimpleTool, SimpleToolName, NewObject<UStageEditorModeTestSimpleToolBuilder>(this));
	RegisterTool(SampleToolCommands.InteractiveTool, InteractiveToolName, NewObject<UStageEditorModeTestInteractiveToolBuilder>(this));

	// active tool type is not relevant here, we just set to default
	GetToolManager()->SelectActiveToolType(EToolSide::Left, SimpleToolName);
}

void UStageEditorModeTestEditorMode::CreateToolkit()
{
	Toolkit = MakeShareable(new FStageEditorModeTestEditorModeToolkit);
}

TMap<FName, TArray<TSharedPtr<FUICommandInfo>>> UStageEditorModeTestEditorMode::GetModeCommands() const
{
	return FStageEditorModeTestEditorModeCommands::Get().GetCommands();
}

#undef LOCTEXT_NAMESPACE
