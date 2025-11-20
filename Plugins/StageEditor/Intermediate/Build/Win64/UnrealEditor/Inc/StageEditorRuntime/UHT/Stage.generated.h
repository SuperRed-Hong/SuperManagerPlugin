// Copyright Epic Games, Inc. All Rights Reserved.
/*===========================================================================
	Generated code exported from UnrealHeaderTool.
	DO NOT modify this manually! Edit the corresponding .h files instead!
===========================================================================*/

// IWYU pragma: private, include "Actors/Stage.h"

#ifdef STAGEEDITORRUNTIME_Stage_generated_h
#error "Stage.generated.h already included, missing '#pragma once' in Stage.h"
#endif
#define STAGEEDITORRUNTIME_Stage_generated_h

#include "UObject/ObjectMacros.h"
#include "UObject/ScriptMacros.h"

PRAGMA_DISABLE_DEPRECATION_WARNINGS

// ********** Begin Class AStage *******************************************************************
#define FID_ExtendEditor_Plugins_StageEditor_Source_StageEditorRuntime_Public_Actors_Stage_h_17_RPC_WRAPPERS_NO_PURE_DECLS \
	DECLARE_FUNCTION(execDeactivateAct); \
	DECLARE_FUNCTION(execActivateAct);


STAGEEDITORRUNTIME_API UClass* Z_Construct_UClass_AStage_NoRegister();

#define FID_ExtendEditor_Plugins_StageEditor_Source_StageEditorRuntime_Public_Actors_Stage_h_17_INCLASS_NO_PURE_DECLS \
private: \
	static void StaticRegisterNativesAStage(); \
	friend struct Z_Construct_UClass_AStage_Statics; \
	static UClass* GetPrivateStaticClass(); \
	friend STAGEEDITORRUNTIME_API UClass* Z_Construct_UClass_AStage_NoRegister(); \
public: \
	DECLARE_CLASS2(AStage, AActor, COMPILED_IN_FLAGS(0 | CLASS_Config), CASTCLASS_None, TEXT("/Script/StageEditorRuntime"), Z_Construct_UClass_AStage_NoRegister) \
	DECLARE_SERIALIZER(AStage)


#define FID_ExtendEditor_Plugins_StageEditor_Source_StageEditorRuntime_Public_Actors_Stage_h_17_ENHANCED_CONSTRUCTORS \
	/** Deleted move- and copy-constructors, should never be used */ \
	AStage(AStage&&) = delete; \
	AStage(const AStage&) = delete; \
	DECLARE_VTABLE_PTR_HELPER_CTOR(NO_API, AStage); \
	DEFINE_VTABLE_PTR_HELPER_CTOR_CALLER(AStage); \
	DEFINE_DEFAULT_CONSTRUCTOR_CALL(AStage) \
	NO_API virtual ~AStage();


#define FID_ExtendEditor_Plugins_StageEditor_Source_StageEditorRuntime_Public_Actors_Stage_h_14_PROLOG
#define FID_ExtendEditor_Plugins_StageEditor_Source_StageEditorRuntime_Public_Actors_Stage_h_17_GENERATED_BODY \
PRAGMA_DISABLE_DEPRECATION_WARNINGS \
public: \
	FID_ExtendEditor_Plugins_StageEditor_Source_StageEditorRuntime_Public_Actors_Stage_h_17_RPC_WRAPPERS_NO_PURE_DECLS \
	FID_ExtendEditor_Plugins_StageEditor_Source_StageEditorRuntime_Public_Actors_Stage_h_17_INCLASS_NO_PURE_DECLS \
	FID_ExtendEditor_Plugins_StageEditor_Source_StageEditorRuntime_Public_Actors_Stage_h_17_ENHANCED_CONSTRUCTORS \
private: \
PRAGMA_ENABLE_DEPRECATION_WARNINGS


class AStage;

// ********** End Class AStage *********************************************************************

#undef CURRENT_FILE_ID
#define CURRENT_FILE_ID FID_ExtendEditor_Plugins_StageEditor_Source_StageEditorRuntime_Public_Actors_Stage_h

PRAGMA_ENABLE_DEPRECATION_WARNINGS
