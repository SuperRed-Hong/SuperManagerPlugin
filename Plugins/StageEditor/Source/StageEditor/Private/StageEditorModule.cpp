#include "StageEditorModule.h"
#include "EditorUI/StageEditorPanel.h"
#include "EditorLogic/StageEditorController.h"
#include "WorkspaceMenuStructure.h"
#include "WorkspaceMenuStructureModule.h"
#include "Widgets/Docking/SDockTab.h"

#define LOCTEXT_NAMESPACE "FStageEditorModule"

static const FName StageEditorTabName("StageEditorTab");

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

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FStageEditorModule, StageEditor)
