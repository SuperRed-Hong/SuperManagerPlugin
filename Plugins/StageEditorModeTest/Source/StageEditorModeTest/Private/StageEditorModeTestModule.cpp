// Copyright Epic Games, Inc. All Rights Reserved.

#include "StageEditorModeTestModule.h"
#include "StageEditorModeTestEditorModeCommands.h"

#define LOCTEXT_NAMESPACE "StageEditorModeTestModule"

void FStageEditorModeTestModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module

	FStageEditorModeTestEditorModeCommands::Register();
}

void FStageEditorModeTestModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.

	FStageEditorModeTestEditorModeCommands::Unregister();
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FStageEditorModeTestModule, StageEditorModeTest)