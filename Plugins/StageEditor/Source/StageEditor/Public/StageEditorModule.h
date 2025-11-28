#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FStageEditorController;
class SDockTab;
class FSpawnTabArgs;

/**
 * StageEditor module - Editor tools for dynamic stage management.
 *
 * Responsibilities:
 * - Register/unregister Stage Editor tab spawner
 * - Register toolbar button for quick access
 * - Manage custom Slate styles and icons
 * - Maintain Controller lifecycle
 */
class FStageEditorModule : public IModuleInterface
{
public:
	/** Tab identifier for the Stage Editor panel. */
	static const FName StageEditorTabName;

	#pragma region Module Interface
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	#pragma endregion Module Interface

private:
	#pragma region Initialization
	/** Initialize custom Slate style set (icons, brushes). */
	void InitializeStyleSet();

	/** Shutdown and cleanup style set. */
	void ShutdownStyleSet();
	#pragma endregion Initialization

	#pragma region Tab Registration
	/** Register the Stage Editor tab spawner in Window menu. */
	void RegisterTabSpawner();

	/** Unregister the Stage Editor tab spawner. */
	void UnregisterTabSpawner();

	/** Callback when Stage Editor tab is spawned. */
	TSharedRef<SDockTab> OnSpawnStageEditorTab(const FSpawnTabArgs& Args);
	#pragma endregion Tab Registration

	#pragma region Toolbar Extension
	/** Register toolbar button in LevelEditor.LevelEditorToolBar.User. */
	void RegisterToolbarButton();

	/** Callback to open the Stage Editor tab. */
	void OnToolbarButtonClicked();
	#pragma endregion Toolbar Extension

	#pragma region Members
	/** Controller for Stage Editor logic (shared across tab instances). */
	TSharedPtr<FStageEditorController> Controller;
	#pragma endregion Members
};
