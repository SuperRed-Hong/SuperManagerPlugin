#pragma region Imports
#include "StageEditorModule.h"
#include "EditorUI/StageEditorPanel.h"
#include "EditorLogic/StageEditorController.h"
#include "WorkspaceMenuStructure.h"
#include "WorkspaceMenuStructureModule.h"
#include "Widgets/Docking/SDockTab.h"
#pragma endregion Imports

#define LOCTEXT_NAMESPACE "FStageEditorModule"

#pragma region Constants
static const FName StageEditorTabName("StageEditorTab");
#pragma endregion Constants

#pragma region Module Interface
void FStageEditorModule::StartupModule()
{
	// Register tab spawner in Window menu
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(StageEditorTabName, FOnSpawnTab::CreateRaw(this, &FStageEditorModule::OnSpawnStageEditorTab))
		.SetDisplayName(LOCTEXT("StageEditorTabTitle", "Stage Editor "))
		.SetMenuType(ETabSpawnerMenuType::Enabled)
		.SetGroup(WorkspaceMenu::GetMenuStructure().GetToolsCategory())
		.SetAutoGenerateMenuEntry(true);
	
}

void FStageEditorModule::ShutdownModule()
{
	// Unregister tab spawner
	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(StageEditorTabName);
}
#pragma endregion Module Interface

#pragma region Tab Spawner
TSharedRef<SDockTab> FStageEditorModule::OnSpawnStageEditorTab(const FSpawnTabArgs& Args)
{
	// Create the controller (singleton-ish for now)
	if (!Controller.IsValid())
	{
		Controller = MakeShared<FStageEditorController>();
	}

	return SNew(SDockTab)
		.TabRole(ETabRole::NomadTab)
		[
			SNew(SStageEditorPanel, Controller)
		];
}
#pragma endregion Tab Spawner

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FStageEditorModule, StageEditor)
