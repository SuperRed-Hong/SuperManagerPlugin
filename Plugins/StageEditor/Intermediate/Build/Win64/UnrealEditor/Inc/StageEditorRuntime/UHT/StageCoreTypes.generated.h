// Copyright Epic Games, Inc. All Rights Reserved.
/*===========================================================================
	Generated code exported from UnrealHeaderTool.
	DO NOT modify this manually! Edit the corresponding .h files instead!
===========================================================================*/

// IWYU pragma: private, include "Core/StageCoreTypes.h"

#ifdef STAGEEDITORRUNTIME_StageCoreTypes_generated_h
#error "StageCoreTypes.generated.h already included, missing '#pragma once' in StageCoreTypes.h"
#endif
#define STAGEEDITORRUNTIME_StageCoreTypes_generated_h

#include "UObject/ObjectMacros.h"
#include "UObject/ScriptMacros.h"

PRAGMA_DISABLE_DEPRECATION_WARNINGS

// ********** Begin ScriptStruct FStageHierarchicalId **********************************************
#define FID_ExtendEditor_Plugins_StageEditor_Source_StageEditorRuntime_Public_Core_StageCoreTypes_h_29_GENERATED_BODY \
	friend struct Z_Construct_UScriptStruct_FStageHierarchicalId_Statics; \
	static class UScriptStruct* StaticStruct();


struct FStageHierarchicalId;
// ********** End ScriptStruct FStageHierarchicalId ************************************************

// ********** Begin ScriptStruct FAct **************************************************************
#define FID_ExtendEditor_Plugins_StageEditor_Source_StageEditorRuntime_Public_Core_StageCoreTypes_h_70_GENERATED_BODY \
	friend struct Z_Construct_UScriptStruct_FAct_Statics; \
	static class UScriptStruct* StaticStruct();


struct FAct;
// ********** End ScriptStruct FAct ****************************************************************

#undef CURRENT_FILE_ID
#define CURRENT_FILE_ID FID_ExtendEditor_Plugins_StageEditor_Source_StageEditorRuntime_Public_Core_StageCoreTypes_h

// ********** Begin Enum EStageRuntimeState ********************************************************
#define FOREACH_ENUM_ESTAGERUNTIMESTATE(op) \
	op(EStageRuntimeState::Unloaded) \
	op(EStageRuntimeState::Loading) \
	op(EStageRuntimeState::Active) 

enum class EStageRuntimeState : uint8;
template<> struct TIsUEnumClass<EStageRuntimeState> { enum { Value = true }; };
template<> STAGEEDITORRUNTIME_API UEnum* StaticEnum<EStageRuntimeState>();
// ********** End Enum EStageRuntimeState **********************************************************

PRAGMA_ENABLE_DEPRECATION_WARNINGS
