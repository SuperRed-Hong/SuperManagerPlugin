#pragma once

#pragma region Imports
#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
#pragma endregion Imports

#pragma region Forward Declarations
class FStageEditorController;
class SDockTab;
class FSpawnTabArgs;
#pragma endregion Forward Declarations

class FStageEditorModule : public IModuleInterface
{
public:

	#pragma region Module Interface
	/**
	 * @brief Called right after the module DLL has been loaded and the module object has been created.
	 */
	virtual void StartupModule() override;

	/**
	 * @brief Called before the module is unloaded, right before the module object is destroyed.
	 */
	virtual void ShutdownModule() override;
	#pragma endregion Module Interface

private:
	#pragma region Tab Spawner
	/**
	 * @brief Spawns the Stage Editor tab.
	 * 
	 * @param Args Arguments for spawning the tab.
	 * @return A shared reference to the spawned dock tab.
	 */
	TSharedRef<SDockTab> OnSpawnStageEditorTab(const FSpawnTabArgs& Args);

	/** Controller for the Stage Editor logic. */
	TSharedPtr<FStageEditorController> Controller;
	#pragma endregion Tab Spawner
};
