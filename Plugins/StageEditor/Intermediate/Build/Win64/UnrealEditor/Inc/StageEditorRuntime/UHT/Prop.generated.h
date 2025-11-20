// Copyright Epic Games, Inc. All Rights Reserved.
/*===========================================================================
	Generated code exported from UnrealHeaderTool.
	DO NOT modify this manually! Edit the corresponding .h files instead!
===========================================================================*/

// IWYU pragma: private, include "Actors/Prop.h"

#ifdef STAGEEDITORRUNTIME_Prop_generated_h
#error "Prop.generated.h already included, missing '#pragma once' in Prop.h"
#endif
#define STAGEEDITORRUNTIME_Prop_generated_h

#include "UObject/ObjectMacros.h"
#include "UObject/ScriptMacros.h"

PRAGMA_DISABLE_DEPRECATION_WARNINGS

// ********** Begin Class AProp ********************************************************************
#define FID_ExtendEditor_Plugins_StageEditor_Source_StageEditorRuntime_Public_Actors_Prop_h_16_RPC_WRAPPERS_NO_PURE_DECLS \
	DECLARE_FUNCTION(execGetPropState); \
	DECLARE_FUNCTION(execSetPropState);


STAGEEDITORRUNTIME_API UClass* Z_Construct_UClass_AProp_NoRegister();

#define FID_ExtendEditor_Plugins_StageEditor_Source_StageEditorRuntime_Public_Actors_Prop_h_16_INCLASS_NO_PURE_DECLS \
private: \
	static void StaticRegisterNativesAProp(); \
	friend struct Z_Construct_UClass_AProp_Statics; \
	static UClass* GetPrivateStaticClass(); \
	friend STAGEEDITORRUNTIME_API UClass* Z_Construct_UClass_AProp_NoRegister(); \
public: \
	DECLARE_CLASS2(AProp, AActor, COMPILED_IN_FLAGS(CLASS_Abstract | CLASS_Config), CASTCLASS_None, TEXT("/Script/StageEditorRuntime"), Z_Construct_UClass_AProp_NoRegister) \
	DECLARE_SERIALIZER(AProp)


#define FID_ExtendEditor_Plugins_StageEditor_Source_StageEditorRuntime_Public_Actors_Prop_h_16_ENHANCED_CONSTRUCTORS \
	/** Deleted move- and copy-constructors, should never be used */ \
	AProp(AProp&&) = delete; \
	AProp(const AProp&) = delete; \
	DECLARE_VTABLE_PTR_HELPER_CTOR(NO_API, AProp); \
	DEFINE_VTABLE_PTR_HELPER_CTOR_CALLER(AProp); \
	DEFINE_ABSTRACT_DEFAULT_CONSTRUCTOR_CALL(AProp) \
	NO_API virtual ~AProp();


#define FID_ExtendEditor_Plugins_StageEditor_Source_StageEditorRuntime_Public_Actors_Prop_h_13_PROLOG
#define FID_ExtendEditor_Plugins_StageEditor_Source_StageEditorRuntime_Public_Actors_Prop_h_16_GENERATED_BODY \
PRAGMA_DISABLE_DEPRECATION_WARNINGS \
public: \
	FID_ExtendEditor_Plugins_StageEditor_Source_StageEditorRuntime_Public_Actors_Prop_h_16_RPC_WRAPPERS_NO_PURE_DECLS \
	FID_ExtendEditor_Plugins_StageEditor_Source_StageEditorRuntime_Public_Actors_Prop_h_16_INCLASS_NO_PURE_DECLS \
	FID_ExtendEditor_Plugins_StageEditor_Source_StageEditorRuntime_Public_Actors_Prop_h_16_ENHANCED_CONSTRUCTORS \
private: \
PRAGMA_ENABLE_DEPRECATION_WARNINGS


class AProp;

// ********** End Class AProp **********************************************************************

#undef CURRENT_FILE_ID
#define CURRENT_FILE_ID FID_ExtendEditor_Plugins_StageEditor_Source_StageEditorRuntime_Public_Actors_Prop_h

PRAGMA_ENABLE_DEPRECATION_WARNINGS
