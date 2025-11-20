#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FStageEditorController;
class SDockTab;
class FSpawnTabArgs;

class FStageEditorModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

private:
	TSharedRef<SDockTab> OnSpawnStageEditorTab(const FSpawnTabArgs& Args);
	TSharedPtr<FStageEditorController> Controller;
};
