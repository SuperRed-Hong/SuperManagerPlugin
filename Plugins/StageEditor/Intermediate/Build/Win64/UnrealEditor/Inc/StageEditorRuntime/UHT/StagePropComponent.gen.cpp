// Copyright Epic Games, Inc. All Rights Reserved.
/*===========================================================================
	Generated code exported from UnrealHeaderTool.
	DO NOT modify this manually! Edit the corresponding .h files instead!
===========================================================================*/

#include "UObject/GeneratedCppIncludes.h"
#include "Components/StagePropComponent.h"

PRAGMA_DISABLE_DEPRECATION_WARNINGS

void EmptyLinkFunctionForGeneratedCodeStagePropComponent() {}

// ********** Begin Cross Module References ********************************************************
ENGINE_API UClass* Z_Construct_UClass_UActorComponent();
STAGEEDITORRUNTIME_API UClass* Z_Construct_UClass_AStage_NoRegister();
STAGEEDITORRUNTIME_API UClass* Z_Construct_UClass_UStagePropComponent();
STAGEEDITORRUNTIME_API UClass* Z_Construct_UClass_UStagePropComponent_NoRegister();
STAGEEDITORRUNTIME_API UFunction* Z_Construct_UDelegateFunction_StageEditorRuntime_OnPropStateChanged__DelegateSignature();
UPackage* Z_Construct_UPackage__Script_StageEditorRuntime();
// ********** End Cross Module References **********************************************************

// ********** Begin Delegate FOnPropStateChanged ***************************************************
struct Z_Construct_UDelegateFunction_StageEditorRuntime_OnPropStateChanged__DelegateSignature_Statics
{
	struct _Script_StageEditorRuntime_eventOnPropStateChanged_Parms
	{
		int32 NewState;
		int32 OldState;
	};
#if WITH_METADATA
	static constexpr UECodeGen_Private::FMetaDataPairParam Function_MetaDataParams[] = {
		{ "ModuleRelativePath", "Public/Components/StagePropComponent.h" },
	};
#endif // WITH_METADATA
	static const UECodeGen_Private::FIntPropertyParams NewProp_NewState;
	static const UECodeGen_Private::FIntPropertyParams NewProp_OldState;
	static const UECodeGen_Private::FPropertyParamsBase* const PropPointers[];
	static const UECodeGen_Private::FDelegateFunctionParams FuncParams;
};
const UECodeGen_Private::FIntPropertyParams Z_Construct_UDelegateFunction_StageEditorRuntime_OnPropStateChanged__DelegateSignature_Statics::NewProp_NewState = { "NewState", nullptr, (EPropertyFlags)0x0010000000000080, UECodeGen_Private::EPropertyGenFlags::Int, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(_Script_StageEditorRuntime_eventOnPropStateChanged_Parms, NewState), METADATA_PARAMS(0, nullptr) };
const UECodeGen_Private::FIntPropertyParams Z_Construct_UDelegateFunction_StageEditorRuntime_OnPropStateChanged__DelegateSignature_Statics::NewProp_OldState = { "OldState", nullptr, (EPropertyFlags)0x0010000000000080, UECodeGen_Private::EPropertyGenFlags::Int, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(_Script_StageEditorRuntime_eventOnPropStateChanged_Parms, OldState), METADATA_PARAMS(0, nullptr) };
const UECodeGen_Private::FPropertyParamsBase* const Z_Construct_UDelegateFunction_StageEditorRuntime_OnPropStateChanged__DelegateSignature_Statics::PropPointers[] = {
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UDelegateFunction_StageEditorRuntime_OnPropStateChanged__DelegateSignature_Statics::NewProp_NewState,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UDelegateFunction_StageEditorRuntime_OnPropStateChanged__DelegateSignature_Statics::NewProp_OldState,
};
static_assert(UE_ARRAY_COUNT(Z_Construct_UDelegateFunction_StageEditorRuntime_OnPropStateChanged__DelegateSignature_Statics::PropPointers) < 2048);
const UECodeGen_Private::FDelegateFunctionParams Z_Construct_UDelegateFunction_StageEditorRuntime_OnPropStateChanged__DelegateSignature_Statics::FuncParams = { { (UObject*(*)())Z_Construct_UPackage__Script_StageEditorRuntime, nullptr, "OnPropStateChanged__DelegateSignature", Z_Construct_UDelegateFunction_StageEditorRuntime_OnPropStateChanged__DelegateSignature_Statics::PropPointers, UE_ARRAY_COUNT(Z_Construct_UDelegateFunction_StageEditorRuntime_OnPropStateChanged__DelegateSignature_Statics::PropPointers), sizeof(Z_Construct_UDelegateFunction_StageEditorRuntime_OnPropStateChanged__DelegateSignature_Statics::_Script_StageEditorRuntime_eventOnPropStateChanged_Parms), RF_Public|RF_Transient|RF_MarkAsNative, (EFunctionFlags)0x00130000, 0, 0, METADATA_PARAMS(UE_ARRAY_COUNT(Z_Construct_UDelegateFunction_StageEditorRuntime_OnPropStateChanged__DelegateSignature_Statics::Function_MetaDataParams), Z_Construct_UDelegateFunction_StageEditorRuntime_OnPropStateChanged__DelegateSignature_Statics::Function_MetaDataParams)},  };
static_assert(sizeof(Z_Construct_UDelegateFunction_StageEditorRuntime_OnPropStateChanged__DelegateSignature_Statics::_Script_StageEditorRuntime_eventOnPropStateChanged_Parms) < MAX_uint16);
UFunction* Z_Construct_UDelegateFunction_StageEditorRuntime_OnPropStateChanged__DelegateSignature()
{
	static UFunction* ReturnFunction = nullptr;
	if (!ReturnFunction)
	{
		UECodeGen_Private::ConstructUDelegateFunction(&ReturnFunction, Z_Construct_UDelegateFunction_StageEditorRuntime_OnPropStateChanged__DelegateSignature_Statics::FuncParams);
	}
	return ReturnFunction;
}
void FOnPropStateChanged_DelegateWrapper(const FMulticastScriptDelegate& OnPropStateChanged, int32 NewState, int32 OldState)
{
	struct _Script_StageEditorRuntime_eventOnPropStateChanged_Parms
	{
		int32 NewState;
		int32 OldState;
	};
	_Script_StageEditorRuntime_eventOnPropStateChanged_Parms Parms;
	Parms.NewState=NewState;
	Parms.OldState=OldState;
	OnPropStateChanged.ProcessMulticastDelegate<UObject>(&Parms);
}
// ********** End Delegate FOnPropStateChanged *****************************************************

// ********** Begin Class UStagePropComponent Function GetResolvedStageID **************************
struct Z_Construct_UFunction_UStagePropComponent_GetResolvedStageID_Statics
{
	struct StagePropComponent_eventGetResolvedStageID_Parms
	{
		int32 ReturnValue;
	};
#if WITH_METADATA
	static constexpr UECodeGen_Private::FMetaDataPairParam Function_MetaDataParams[] = {
		{ "Category", "Stage Prop" },
#if !UE_BUILD_SHIPPING
		{ "Comment", "/**\n\x09 * @brief Get the full hierarchical ID by resolving the OwnerStage reference.\n\x09 * @return StageID from the OwnerStage, or -1 if invalid.\n\x09 */" },
#endif
		{ "ModuleRelativePath", "Public/Components/StagePropComponent.h" },
#if !UE_BUILD_SHIPPING
		{ "ToolTip", "@brief Get the full hierarchical ID by resolving the OwnerStage reference.\n@return StageID from the OwnerStage, or -1 if invalid." },
#endif
	};
#endif // WITH_METADATA
	static const UECodeGen_Private::FIntPropertyParams NewProp_ReturnValue;
	static const UECodeGen_Private::FPropertyParamsBase* const PropPointers[];
	static const UECodeGen_Private::FFunctionParams FuncParams;
};
const UECodeGen_Private::FIntPropertyParams Z_Construct_UFunction_UStagePropComponent_GetResolvedStageID_Statics::NewProp_ReturnValue = { "ReturnValue", nullptr, (EPropertyFlags)0x0010000000000580, UECodeGen_Private::EPropertyGenFlags::Int, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(StagePropComponent_eventGetResolvedStageID_Parms, ReturnValue), METADATA_PARAMS(0, nullptr) };
const UECodeGen_Private::FPropertyParamsBase* const Z_Construct_UFunction_UStagePropComponent_GetResolvedStageID_Statics::PropPointers[] = {
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UFunction_UStagePropComponent_GetResolvedStageID_Statics::NewProp_ReturnValue,
};
static_assert(UE_ARRAY_COUNT(Z_Construct_UFunction_UStagePropComponent_GetResolvedStageID_Statics::PropPointers) < 2048);
const UECodeGen_Private::FFunctionParams Z_Construct_UFunction_UStagePropComponent_GetResolvedStageID_Statics::FuncParams = { { (UObject*(*)())Z_Construct_UClass_UStagePropComponent, nullptr, "GetResolvedStageID", Z_Construct_UFunction_UStagePropComponent_GetResolvedStageID_Statics::PropPointers, UE_ARRAY_COUNT(Z_Construct_UFunction_UStagePropComponent_GetResolvedStageID_Statics::PropPointers), sizeof(Z_Construct_UFunction_UStagePropComponent_GetResolvedStageID_Statics::StagePropComponent_eventGetResolvedStageID_Parms), RF_Public|RF_Transient|RF_MarkAsNative, (EFunctionFlags)0x54020401, 0, 0, METADATA_PARAMS(UE_ARRAY_COUNT(Z_Construct_UFunction_UStagePropComponent_GetResolvedStageID_Statics::Function_MetaDataParams), Z_Construct_UFunction_UStagePropComponent_GetResolvedStageID_Statics::Function_MetaDataParams)},  };
static_assert(sizeof(Z_Construct_UFunction_UStagePropComponent_GetResolvedStageID_Statics::StagePropComponent_eventGetResolvedStageID_Parms) < MAX_uint16);
UFunction* Z_Construct_UFunction_UStagePropComponent_GetResolvedStageID()
{
	static UFunction* ReturnFunction = nullptr;
	if (!ReturnFunction)
	{
		UECodeGen_Private::ConstructUFunction(&ReturnFunction, Z_Construct_UFunction_UStagePropComponent_GetResolvedStageID_Statics::FuncParams);
	}
	return ReturnFunction;
}
DEFINE_FUNCTION(UStagePropComponent::execGetResolvedStageID)
{
	P_FINISH;
	P_NATIVE_BEGIN;
	*(int32*)Z_Param__Result=P_THIS->GetResolvedStageID();
	P_NATIVE_END;
}
// ********** End Class UStagePropComponent Function GetResolvedStageID ****************************

// ********** Begin Class UStagePropComponent Function SetPropState ********************************
struct Z_Construct_UFunction_UStagePropComponent_SetPropState_Statics
{
	struct StagePropComponent_eventSetPropState_Parms
	{
		int32 NewState;
		bool bForce;
	};
#if WITH_METADATA
	static constexpr UECodeGen_Private::FMetaDataPairParam Function_MetaDataParams[] = {
		{ "Category", "Stage Prop" },
#if !UE_BUILD_SHIPPING
		{ "Comment", "/**\n\x09 * @brief Sets the new state for this Prop.\n\x09 * @param NewState The target state value.\n\x09 * @param bForce If true, triggers update even if NewState == CurrentState.\n\x09 */" },
#endif
		{ "CPP_Default_bForce", "false" },
		{ "ModuleRelativePath", "Public/Components/StagePropComponent.h" },
#if !UE_BUILD_SHIPPING
		{ "ToolTip", "@brief Sets the new state for this Prop.\n@param NewState The target state value.\n@param bForce If true, triggers update even if NewState == CurrentState." },
#endif
	};
#endif // WITH_METADATA
	static const UECodeGen_Private::FIntPropertyParams NewProp_NewState;
	static void NewProp_bForce_SetBit(void* Obj);
	static const UECodeGen_Private::FBoolPropertyParams NewProp_bForce;
	static const UECodeGen_Private::FPropertyParamsBase* const PropPointers[];
	static const UECodeGen_Private::FFunctionParams FuncParams;
};
const UECodeGen_Private::FIntPropertyParams Z_Construct_UFunction_UStagePropComponent_SetPropState_Statics::NewProp_NewState = { "NewState", nullptr, (EPropertyFlags)0x0010000000000080, UECodeGen_Private::EPropertyGenFlags::Int, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(StagePropComponent_eventSetPropState_Parms, NewState), METADATA_PARAMS(0, nullptr) };
void Z_Construct_UFunction_UStagePropComponent_SetPropState_Statics::NewProp_bForce_SetBit(void* Obj)
{
	((StagePropComponent_eventSetPropState_Parms*)Obj)->bForce = 1;
}
const UECodeGen_Private::FBoolPropertyParams Z_Construct_UFunction_UStagePropComponent_SetPropState_Statics::NewProp_bForce = { "bForce", nullptr, (EPropertyFlags)0x0010000000000080, UECodeGen_Private::EPropertyGenFlags::Bool | UECodeGen_Private::EPropertyGenFlags::NativeBool, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, sizeof(bool), sizeof(StagePropComponent_eventSetPropState_Parms), &Z_Construct_UFunction_UStagePropComponent_SetPropState_Statics::NewProp_bForce_SetBit, METADATA_PARAMS(0, nullptr) };
const UECodeGen_Private::FPropertyParamsBase* const Z_Construct_UFunction_UStagePropComponent_SetPropState_Statics::PropPointers[] = {
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UFunction_UStagePropComponent_SetPropState_Statics::NewProp_NewState,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UFunction_UStagePropComponent_SetPropState_Statics::NewProp_bForce,
};
static_assert(UE_ARRAY_COUNT(Z_Construct_UFunction_UStagePropComponent_SetPropState_Statics::PropPointers) < 2048);
const UECodeGen_Private::FFunctionParams Z_Construct_UFunction_UStagePropComponent_SetPropState_Statics::FuncParams = { { (UObject*(*)())Z_Construct_UClass_UStagePropComponent, nullptr, "SetPropState", Z_Construct_UFunction_UStagePropComponent_SetPropState_Statics::PropPointers, UE_ARRAY_COUNT(Z_Construct_UFunction_UStagePropComponent_SetPropState_Statics::PropPointers), sizeof(Z_Construct_UFunction_UStagePropComponent_SetPropState_Statics::StagePropComponent_eventSetPropState_Parms), RF_Public|RF_Transient|RF_MarkAsNative, (EFunctionFlags)0x04020401, 0, 0, METADATA_PARAMS(UE_ARRAY_COUNT(Z_Construct_UFunction_UStagePropComponent_SetPropState_Statics::Function_MetaDataParams), Z_Construct_UFunction_UStagePropComponent_SetPropState_Statics::Function_MetaDataParams)},  };
static_assert(sizeof(Z_Construct_UFunction_UStagePropComponent_SetPropState_Statics::StagePropComponent_eventSetPropState_Parms) < MAX_uint16);
UFunction* Z_Construct_UFunction_UStagePropComponent_SetPropState()
{
	static UFunction* ReturnFunction = nullptr;
	if (!ReturnFunction)
	{
		UECodeGen_Private::ConstructUFunction(&ReturnFunction, Z_Construct_UFunction_UStagePropComponent_SetPropState_Statics::FuncParams);
	}
	return ReturnFunction;
}
DEFINE_FUNCTION(UStagePropComponent::execSetPropState)
{
	P_GET_PROPERTY(FIntProperty,Z_Param_NewState);
	P_GET_UBOOL(Z_Param_bForce);
	P_FINISH;
	P_NATIVE_BEGIN;
	P_THIS->SetPropState(Z_Param_NewState,Z_Param_bForce);
	P_NATIVE_END;
}
// ********** End Class UStagePropComponent Function SetPropState **********************************

// ********** Begin Class UStagePropComponent ******************************************************
void UStagePropComponent::StaticRegisterNativesUStagePropComponent()
{
	UClass* Class = UStagePropComponent::StaticClass();
	static const FNameNativePtrPair Funcs[] = {
		{ "GetResolvedStageID", &UStagePropComponent::execGetResolvedStageID },
		{ "SetPropState", &UStagePropComponent::execSetPropState },
	};
	FNativeFunctionRegistrar::RegisterFunctions(Class, Funcs, UE_ARRAY_COUNT(Funcs));
}
FClassRegistrationInfo Z_Registration_Info_UClass_UStagePropComponent;
UClass* UStagePropComponent::GetPrivateStaticClass()
{
	using TClass = UStagePropComponent;
	if (!Z_Registration_Info_UClass_UStagePropComponent.InnerSingleton)
	{
		GetPrivateStaticClassBody(
			StaticPackage(),
			TEXT("StagePropComponent"),
			Z_Registration_Info_UClass_UStagePropComponent.InnerSingleton,
			StaticRegisterNativesUStagePropComponent,
			sizeof(TClass),
			alignof(TClass),
			TClass::StaticClassFlags,
			TClass::StaticClassCastFlags(),
			TClass::StaticConfigName(),
			(UClass::ClassConstructorType)InternalConstructor<TClass>,
			(UClass::ClassVTableHelperCtorCallerType)InternalVTableHelperCtorCaller<TClass>,
			UOBJECT_CPPCLASS_STATICFUNCTIONS_FORCLASS(TClass),
			&TClass::Super::StaticClass,
			&TClass::WithinClass::StaticClass
		);
	}
	return Z_Registration_Info_UClass_UStagePropComponent.InnerSingleton;
}
UClass* Z_Construct_UClass_UStagePropComponent_NoRegister()
{
	return UStagePropComponent::GetPrivateStaticClass();
}
struct Z_Construct_UClass_UStagePropComponent_Statics
{
#if WITH_METADATA
	static constexpr UECodeGen_Private::FMetaDataPairParam Class_MetaDataParams[] = {
		{ "BlueprintSpawnableComponent", "" },
		{ "ClassGroupNames", "StageEditor" },
#if !UE_BUILD_SHIPPING
		{ "Comment", "/**\n * @brief Core component that makes any Actor a controllable Prop in the Stage system.\n * Can be added to any Actor to make it respond to Stage state changes.\n */" },
#endif
		{ "IncludePath", "Components/StagePropComponent.h" },
		{ "ModuleRelativePath", "Public/Components/StagePropComponent.h" },
#if !UE_BUILD_SHIPPING
		{ "ToolTip", "@brief Core component that makes any Actor a controllable Prop in the Stage system.\nCan be added to any Actor to make it respond to Stage state changes." },
#endif
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_PropID_MetaData[] = {
		{ "Category", "Stage Prop" },
#if !UE_BUILD_SHIPPING
		{ "Comment", "/** The unique ID of this Prop within its owning Stage. */" },
#endif
		{ "ModuleRelativePath", "Public/Components/StagePropComponent.h" },
#if !UE_BUILD_SHIPPING
		{ "ToolTip", "The unique ID of this Prop within its owning Stage." },
#endif
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_OwnerStage_MetaData[] = {
		{ "Category", "Stage Prop" },
#if !UE_BUILD_SHIPPING
		{ "Comment", "/** \n\x09 * Soft reference to the owning Stage Actor.\n\x09 * Used to dynamically resolve StageID without storing it directly.\n\x09 */" },
#endif
		{ "ModuleRelativePath", "Public/Components/StagePropComponent.h" },
#if !UE_BUILD_SHIPPING
		{ "ToolTip", "Soft reference to the owning Stage Actor.\nUsed to dynamically resolve StageID without storing it directly." },
#endif
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_PropState_MetaData[] = {
		{ "Category", "Stage Prop" },
#if !UE_BUILD_SHIPPING
		{ "Comment", "/** \n\x09 * The current state of this Prop. \n\x09 * Modified by the Stage Manager via SetPropState().\n\x09 */" },
#endif
		{ "ModuleRelativePath", "Public/Components/StagePropComponent.h" },
#if !UE_BUILD_SHIPPING
		{ "ToolTip", "The current state of this Prop.\nModified by the Stage Manager via SetPropState()." },
#endif
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_OnPropStateChanged_MetaData[] = {
		{ "Category", "Stage Prop" },
#if !UE_BUILD_SHIPPING
		{ "Comment", "/** Event fired when PropState changes. Implement logic here in Blueprints. */" },
#endif
		{ "ModuleRelativePath", "Public/Components/StagePropComponent.h" },
#if !UE_BUILD_SHIPPING
		{ "ToolTip", "Event fired when PropState changes. Implement logic here in Blueprints." },
#endif
	};
#endif // WITH_METADATA
	static const UECodeGen_Private::FIntPropertyParams NewProp_PropID;
	static const UECodeGen_Private::FSoftObjectPropertyParams NewProp_OwnerStage;
	static const UECodeGen_Private::FIntPropertyParams NewProp_PropState;
	static const UECodeGen_Private::FMulticastDelegatePropertyParams NewProp_OnPropStateChanged;
	static const UECodeGen_Private::FPropertyParamsBase* const PropPointers[];
	static UObject* (*const DependentSingletons[])();
	static constexpr FClassFunctionLinkInfo FuncInfo[] = {
		{ &Z_Construct_UFunction_UStagePropComponent_GetResolvedStageID, "GetResolvedStageID" }, // 2946484814
		{ &Z_Construct_UFunction_UStagePropComponent_SetPropState, "SetPropState" }, // 919126078
	};
	static_assert(UE_ARRAY_COUNT(FuncInfo) < 2048);
	static constexpr FCppClassTypeInfoStatic StaticCppClassTypeInfo = {
		TCppClassTypeTraits<UStagePropComponent>::IsAbstract,
	};
	static const UECodeGen_Private::FClassParams ClassParams;
};
const UECodeGen_Private::FIntPropertyParams Z_Construct_UClass_UStagePropComponent_Statics::NewProp_PropID = { "PropID", nullptr, (EPropertyFlags)0x0010000000000015, UECodeGen_Private::EPropertyGenFlags::Int, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(UStagePropComponent, PropID), METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_PropID_MetaData), NewProp_PropID_MetaData) };
const UECodeGen_Private::FSoftObjectPropertyParams Z_Construct_UClass_UStagePropComponent_Statics::NewProp_OwnerStage = { "OwnerStage", nullptr, (EPropertyFlags)0x0014000000000015, UECodeGen_Private::EPropertyGenFlags::SoftObject, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(UStagePropComponent, OwnerStage), Z_Construct_UClass_AStage_NoRegister, METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_OwnerStage_MetaData), NewProp_OwnerStage_MetaData) };
const UECodeGen_Private::FIntPropertyParams Z_Construct_UClass_UStagePropComponent_Statics::NewProp_PropState = { "PropState", nullptr, (EPropertyFlags)0x0010000000020015, UECodeGen_Private::EPropertyGenFlags::Int, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(UStagePropComponent, PropState), METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_PropState_MetaData), NewProp_PropState_MetaData) };
const UECodeGen_Private::FMulticastDelegatePropertyParams Z_Construct_UClass_UStagePropComponent_Statics::NewProp_OnPropStateChanged = { "OnPropStateChanged", nullptr, (EPropertyFlags)0x0010000010080000, UECodeGen_Private::EPropertyGenFlags::InlineMulticastDelegate, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(UStagePropComponent, OnPropStateChanged), Z_Construct_UDelegateFunction_StageEditorRuntime_OnPropStateChanged__DelegateSignature, METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_OnPropStateChanged_MetaData), NewProp_OnPropStateChanged_MetaData) }; // 1639479447
const UECodeGen_Private::FPropertyParamsBase* const Z_Construct_UClass_UStagePropComponent_Statics::PropPointers[] = {
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_UStagePropComponent_Statics::NewProp_PropID,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_UStagePropComponent_Statics::NewProp_OwnerStage,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_UStagePropComponent_Statics::NewProp_PropState,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_UStagePropComponent_Statics::NewProp_OnPropStateChanged,
};
static_assert(UE_ARRAY_COUNT(Z_Construct_UClass_UStagePropComponent_Statics::PropPointers) < 2048);
UObject* (*const Z_Construct_UClass_UStagePropComponent_Statics::DependentSingletons[])() = {
	(UObject* (*)())Z_Construct_UClass_UActorComponent,
	(UObject* (*)())Z_Construct_UPackage__Script_StageEditorRuntime,
};
static_assert(UE_ARRAY_COUNT(Z_Construct_UClass_UStagePropComponent_Statics::DependentSingletons) < 16);
const UECodeGen_Private::FClassParams Z_Construct_UClass_UStagePropComponent_Statics::ClassParams = {
	&UStagePropComponent::StaticClass,
	"Engine",
	&StaticCppClassTypeInfo,
	DependentSingletons,
	FuncInfo,
	Z_Construct_UClass_UStagePropComponent_Statics::PropPointers,
	nullptr,
	UE_ARRAY_COUNT(DependentSingletons),
	UE_ARRAY_COUNT(FuncInfo),
	UE_ARRAY_COUNT(Z_Construct_UClass_UStagePropComponent_Statics::PropPointers),
	0,
	0x00B000A4u,
	METADATA_PARAMS(UE_ARRAY_COUNT(Z_Construct_UClass_UStagePropComponent_Statics::Class_MetaDataParams), Z_Construct_UClass_UStagePropComponent_Statics::Class_MetaDataParams)
};
UClass* Z_Construct_UClass_UStagePropComponent()
{
	if (!Z_Registration_Info_UClass_UStagePropComponent.OuterSingleton)
	{
		UECodeGen_Private::ConstructUClass(Z_Registration_Info_UClass_UStagePropComponent.OuterSingleton, Z_Construct_UClass_UStagePropComponent_Statics::ClassParams);
	}
	return Z_Registration_Info_UClass_UStagePropComponent.OuterSingleton;
}
DEFINE_VTABLE_PTR_HELPER_CTOR(UStagePropComponent);
UStagePropComponent::~UStagePropComponent() {}
// ********** End Class UStagePropComponent ********************************************************

// ********** Begin Registration *******************************************************************
struct Z_CompiledInDeferFile_FID_ExtendEditor_Plugins_StageEditor_Source_StageEditorRuntime_Public_Components_StagePropComponent_h__Script_StageEditorRuntime_Statics
{
	static constexpr FClassRegisterCompiledInInfo ClassInfo[] = {
		{ Z_Construct_UClass_UStagePropComponent, UStagePropComponent::StaticClass, TEXT("UStagePropComponent"), &Z_Registration_Info_UClass_UStagePropComponent, CONSTRUCT_RELOAD_VERSION_INFO(FClassReloadVersionInfo, sizeof(UStagePropComponent), 3739326817U) },
	};
};
static FRegisterCompiledInInfo Z_CompiledInDeferFile_FID_ExtendEditor_Plugins_StageEditor_Source_StageEditorRuntime_Public_Components_StagePropComponent_h__Script_StageEditorRuntime_3981536004(TEXT("/Script/StageEditorRuntime"),
	Z_CompiledInDeferFile_FID_ExtendEditor_Plugins_StageEditor_Source_StageEditorRuntime_Public_Components_StagePropComponent_h__Script_StageEditorRuntime_Statics::ClassInfo, UE_ARRAY_COUNT(Z_CompiledInDeferFile_FID_ExtendEditor_Plugins_StageEditor_Source_StageEditorRuntime_Public_Components_StagePropComponent_h__Script_StageEditorRuntime_Statics::ClassInfo),
	nullptr, 0,
	nullptr, 0);
// ********** End Registration *********************************************************************

PRAGMA_ENABLE_DEPRECATION_WARNINGS
