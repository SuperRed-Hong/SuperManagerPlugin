// Copyright Epic Games, Inc. All Rights Reserved.
/*===========================================================================
	Generated code exported from UnrealHeaderTool.
	DO NOT modify this manually! Edit the corresponding .h files instead!
===========================================================================*/

// IWYU pragma: private, include "Components/StagePropComponent.h"

#ifdef STAGEEDITORRUNTIME_StagePropComponent_generated_h
#error "StagePropComponent.generated.h already included, missing '#pragma once' in StagePropComponent.h"
#endif
#define STAGEEDITORRUNTIME_StagePropComponent_generated_h

#include "UObject/ObjectMacros.h"
#include "UObject/ScriptMacros.h"

PRAGMA_DISABLE_DEPRECATION_WARNINGS

// ********** Begin Delegate FOnPropStateChanged ***************************************************
#define FID_ExtendEditor_Plugins_StageEditor_Source_StageEditorRuntime_Public_Components_StagePropComponent_h_10_DELEGATE \
STAGEEDITORRUNTIME_API void FOnPropStateChanged_DelegateWrapper(const FMulticastScriptDelegate& OnPropStateChanged, int32 NewState, int32 OldState);


// ********** End Delegate FOnPropStateChanged *****************************************************

// ********** Begin Class UStagePropComponent ******************************************************
#define FID_ExtendEditor_Plugins_StageEditor_Source_StageEditorRuntime_Public_Components_StagePropComponent_h_19_RPC_WRAPPERS_NO_PURE_DECLS \
	DECLARE_FUNCTION(execGetResolvedStageID); \
	DECLARE_FUNCTION(execSetPropState);


STAGEEDITORRUNTIME_API UClass* Z_Construct_UClass_UStagePropComponent_NoRegister();

#define FID_ExtendEditor_Plugins_StageEditor_Source_StageEditorRuntime_Public_Components_StagePropComponent_h_19_INCLASS_NO_PURE_DECLS \
private: \
	static void StaticRegisterNativesUStagePropComponent(); \
	friend struct Z_Construct_UClass_UStagePropComponent_Statics; \
	static UClass* GetPrivateStaticClass(); \
	friend STAGEEDITORRUNTIME_API UClass* Z_Construct_UClass_UStagePropComponent_NoRegister(); \
public: \
	DECLARE_CLASS2(UStagePropComponent, UActorComponent, COMPILED_IN_FLAGS(0 | CLASS_Config), CASTCLASS_None, TEXT("/Script/StageEditorRuntime"), Z_Construct_UClass_UStagePropComponent_NoRegister) \
	DECLARE_SERIALIZER(UStagePropComponent)


#define FID_ExtendEditor_Plugins_StageEditor_Source_StageEditorRuntime_Public_Components_StagePropComponent_h_19_ENHANCED_CONSTRUCTORS \
	/** Deleted move- and copy-constructors, should never be used */ \
	UStagePropComponent(UStagePropComponent&&) = delete; \
	UStagePropComponent(const UStagePropComponent&) = delete; \
	DECLARE_VTABLE_PTR_HELPER_CTOR(NO_API, UStagePropComponent); \
	DEFINE_VTABLE_PTR_HELPER_CTOR_CALLER(UStagePropComponent); \
	DEFINE_DEFAULT_CONSTRUCTOR_CALL(UStagePropComponent) \
	NO_API virtual ~UStagePropComponent();


#define FID_ExtendEditor_Plugins_StageEditor_Source_StageEditorRuntime_Public_Components_StagePropComponent_h_16_PROLOG
#define FID_ExtendEditor_Plugins_StageEditor_Source_StageEditorRuntime_Public_Components_StagePropComponent_h_19_GENERATED_BODY \
PRAGMA_DISABLE_DEPRECATION_WARNINGS \
public: \
	FID_ExtendEditor_Plugins_StageEditor_Source_StageEditorRuntime_Public_Components_StagePropComponent_h_19_RPC_WRAPPERS_NO_PURE_DECLS \
	FID_ExtendEditor_Plugins_StageEditor_Source_StageEditorRuntime_Public_Components_StagePropComponent_h_19_INCLASS_NO_PURE_DECLS \
	FID_ExtendEditor_Plugins_StageEditor_Source_StageEditorRuntime_Public_Components_StagePropComponent_h_19_ENHANCED_CONSTRUCTORS \
private: \
PRAGMA_ENABLE_DEPRECATION_WARNINGS


class UStagePropComponent;

// ********** End Class UStagePropComponent ********************************************************

#undef CURRENT_FILE_ID
#define CURRENT_FILE_ID FID_ExtendEditor_Plugins_StageEditor_Source_StageEditorRuntime_Public_Components_StagePropComponent_h

PRAGMA_ENABLE_DEPRECATION_WARNINGS
