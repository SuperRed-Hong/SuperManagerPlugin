#pragma region Imports
#include "StageEditorRuntimeModule.h"

#define LOCTEXT_NAMESPACE "FStageEditorRuntimeModule"
#pragma endregion Imports

#pragma region Module Interface
void FStageEditorRuntimeModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
}

void FStageEditorRuntimeModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}
#pragma endregion Module Interface

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FStageEditorRuntimeModule, StageEditorRuntime)
