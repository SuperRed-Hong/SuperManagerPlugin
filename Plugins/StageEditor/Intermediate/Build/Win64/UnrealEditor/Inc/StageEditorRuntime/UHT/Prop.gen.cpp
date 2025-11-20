// Copyright Epic Games, Inc. All Rights Reserved.
/*===========================================================================
	Generated code exported from UnrealHeaderTool.
	DO NOT modify this manually! Edit the corresponding .h files instead!
===========================================================================*/

#include "UObject/GeneratedCppIncludes.h"
#include "Actors/Prop.h"

PRAGMA_DISABLE_DEPRECATION_WARNINGS

void EmptyLinkFunctionForGeneratedCodeProp() {}

// ********** Begin Cross Module References ********************************************************
ENGINE_API UClass* Z_Construct_UClass_AActor();
STAGEEDITORRUNTIME_API UClass* Z_Construct_UClass_AProp();
STAGEEDITORRUNTIME_API UClass* Z_Construct_UClass_AProp_NoRegister();
STAGEEDITORRUNTIME_API UClass* Z_Construct_UClass_UStagePropComponent_NoRegister();
UPackage* Z_Construct_UPackage__Script_StageEditorRuntime();
// ********** End Cross Module References **********************************************************

// ********** Begin Class AProp Function GetPropState **********************************************
struct Z_Construct_UFunction_AProp_GetPropState_Statics
{
	struct Prop_eventGetPropState_Parms
	{
		int32 ReturnValue;
	};
#if WITH_METADATA
	static constexpr UECodeGen_Private::FMetaDataPairParam Function_MetaDataParams[] = {
		{ "Category", "Stage Prop" },
#if !UE_BUILD_SHIPPING
		{ "Comment", "/**\n\x09 * @brief Get the current PropState.\n\x09 */" },
#endif
		{ "ModuleRelativePath", "Public/Actors/Prop.h" },
#if !UE_BUILD_SHIPPING
		{ "ToolTip", "@brief Get the current PropState." },
#endif
	};
#endif // WITH_METADATA
	static const UECodeGen_Private::FIntPropertyParams NewProp_ReturnValue;
	static const UECodeGen_Private::FPropertyParamsBase* const PropPointers[];
	static const UECodeGen_Private::FFunctionParams FuncParams;
};
const UECodeGen_Private::FIntPropertyParams Z_Construct_UFunction_AProp_GetPropState_Statics::NewProp_ReturnValue = { "ReturnValue", nullptr, (EPropertyFlags)0x0010000000000580, UECodeGen_Private::EPropertyGenFlags::Int, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(Prop_eventGetPropState_Parms, ReturnValue), METADATA_PARAMS(0, nullptr) };
const UECodeGen_Private::FPropertyParamsBase* const Z_Construct_UFunction_AProp_GetPropState_Statics::PropPointers[] = {
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UFunction_AProp_GetPropState_Statics::NewProp_ReturnValue,
};
static_assert(UE_ARRAY_COUNT(Z_Construct_UFunction_AProp_GetPropState_Statics::PropPointers) < 2048);
const UECodeGen_Private::FFunctionParams Z_Construct_UFunction_AProp_GetPropState_Statics::FuncParams = { { (UObject*(*)())Z_Construct_UClass_AProp, nullptr, "GetPropState", Z_Construct_UFunction_AProp_GetPropState_Statics::PropPointers, UE_ARRAY_COUNT(Z_Construct_UFunction_AProp_GetPropState_Statics::PropPointers), sizeof(Z_Construct_UFunction_AProp_GetPropState_Statics::Prop_eventGetPropState_Parms), RF_Public|RF_Transient|RF_MarkAsNative, (EFunctionFlags)0x54020401, 0, 0, METADATA_PARAMS(UE_ARRAY_COUNT(Z_Construct_UFunction_AProp_GetPropState_Statics::Function_MetaDataParams), Z_Construct_UFunction_AProp_GetPropState_Statics::Function_MetaDataParams)},  };
static_assert(sizeof(Z_Construct_UFunction_AProp_GetPropState_Statics::Prop_eventGetPropState_Parms) < MAX_uint16);
UFunction* Z_Construct_UFunction_AProp_GetPropState()
{
	static UFunction* ReturnFunction = nullptr;
	if (!ReturnFunction)
	{
		UECodeGen_Private::ConstructUFunction(&ReturnFunction, Z_Construct_UFunction_AProp_GetPropState_Statics::FuncParams);
	}
	return ReturnFunction;
}
DEFINE_FUNCTION(AProp::execGetPropState)
{
	P_FINISH;
	P_NATIVE_BEGIN;
	*(int32*)Z_Param__Result=P_THIS->GetPropState();
	P_NATIVE_END;
}
// ********** End Class AProp Function GetPropState ************************************************

// ********** Begin Class AProp Function SetPropState **********************************************
struct Z_Construct_UFunction_AProp_SetPropState_Statics
{
	struct Prop_eventSetPropState_Parms
	{
		int32 NewState;
		bool bForce;
	};
#if WITH_METADATA
	static constexpr UECodeGen_Private::FMetaDataPairParam Function_MetaDataParams[] = {
		{ "Category", "Stage Prop" },
#if !UE_BUILD_SHIPPING
		{ "Comment", "/**\n\x09 * @brief Sets the new state for this Prop.\n\x09 * Convenience wrapper that calls the component's SetPropState.\n\x09 */" },
#endif
		{ "CPP_Default_bForce", "false" },
		{ "ModuleRelativePath", "Public/Actors/Prop.h" },
#if !UE_BUILD_SHIPPING
		{ "ToolTip", "@brief Sets the new state for this Prop.\nConvenience wrapper that calls the component's SetPropState." },
#endif
	};
#endif // WITH_METADATA
	static const UECodeGen_Private::FIntPropertyParams NewProp_NewState;
	static void NewProp_bForce_SetBit(void* Obj);
	static const UECodeGen_Private::FBoolPropertyParams NewProp_bForce;
	static const UECodeGen_Private::FPropertyParamsBase* const PropPointers[];
	static const UECodeGen_Private::FFunctionParams FuncParams;
};
const UECodeGen_Private::FIntPropertyParams Z_Construct_UFunction_AProp_SetPropState_Statics::NewProp_NewState = { "NewState", nullptr, (EPropertyFlags)0x0010000000000080, UECodeGen_Private::EPropertyGenFlags::Int, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(Prop_eventSetPropState_Parms, NewState), METADATA_PARAMS(0, nullptr) };
void Z_Construct_UFunction_AProp_SetPropState_Statics::NewProp_bForce_SetBit(void* Obj)
{
	((Prop_eventSetPropState_Parms*)Obj)->bForce = 1;
}
const UECodeGen_Private::FBoolPropertyParams Z_Construct_UFunction_AProp_SetPropState_Statics::NewProp_bForce = { "bForce", nullptr, (EPropertyFlags)0x0010000000000080, UECodeGen_Private::EPropertyGenFlags::Bool | UECodeGen_Private::EPropertyGenFlags::NativeBool, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, sizeof(bool), sizeof(Prop_eventSetPropState_Parms), &Z_Construct_UFunction_AProp_SetPropState_Statics::NewProp_bForce_SetBit, METADATA_PARAMS(0, nullptr) };
const UECodeGen_Private::FPropertyParamsBase* const Z_Construct_UFunction_AProp_SetPropState_Statics::PropPointers[] = {
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UFunction_AProp_SetPropState_Statics::NewProp_NewState,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UFunction_AProp_SetPropState_Statics::NewProp_bForce,
};
static_assert(UE_ARRAY_COUNT(Z_Construct_UFunction_AProp_SetPropState_Statics::PropPointers) < 2048);
const UECodeGen_Private::FFunctionParams Z_Construct_UFunction_AProp_SetPropState_Statics::FuncParams = { { (UObject*(*)())Z_Construct_UClass_AProp, nullptr, "SetPropState", Z_Construct_UFunction_AProp_SetPropState_Statics::PropPointers, UE_ARRAY_COUNT(Z_Construct_UFunction_AProp_SetPropState_Statics::PropPointers), sizeof(Z_Construct_UFunction_AProp_SetPropState_Statics::Prop_eventSetPropState_Parms), RF_Public|RF_Transient|RF_MarkAsNative, (EFunctionFlags)0x04020401, 0, 0, METADATA_PARAMS(UE_ARRAY_COUNT(Z_Construct_UFunction_AProp_SetPropState_Statics::Function_MetaDataParams), Z_Construct_UFunction_AProp_SetPropState_Statics::Function_MetaDataParams)},  };
static_assert(sizeof(Z_Construct_UFunction_AProp_SetPropState_Statics::Prop_eventSetPropState_Parms) < MAX_uint16);
UFunction* Z_Construct_UFunction_AProp_SetPropState()
{
	static UFunction* ReturnFunction = nullptr;
	if (!ReturnFunction)
	{
		UECodeGen_Private::ConstructUFunction(&ReturnFunction, Z_Construct_UFunction_AProp_SetPropState_Statics::FuncParams);
	}
	return ReturnFunction;
}
DEFINE_FUNCTION(AProp::execSetPropState)
{
	P_GET_PROPERTY(FIntProperty,Z_Param_NewState);
	P_GET_UBOOL(Z_Param_bForce);
	P_FINISH;
	P_NATIVE_BEGIN;
	P_THIS->SetPropState(Z_Param_NewState,Z_Param_bForce);
	P_NATIVE_END;
}
// ********** End Class AProp Function SetPropState ************************************************

// ********** Begin Class AProp ********************************************************************
void AProp::StaticRegisterNativesAProp()
{
	UClass* Class = AProp::StaticClass();
	static const FNameNativePtrPair Funcs[] = {
		{ "GetPropState", &AProp::execGetPropState },
		{ "SetPropState", &AProp::execSetPropState },
	};
	FNativeFunctionRegistrar::RegisterFunctions(Class, Funcs, UE_ARRAY_COUNT(Funcs));
}
FClassRegistrationInfo Z_Registration_Info_UClass_AProp;
UClass* AProp::GetPrivateStaticClass()
{
	using TClass = AProp;
	if (!Z_Registration_Info_UClass_AProp.InnerSingleton)
	{
		GetPrivateStaticClassBody(
			StaticPackage(),
			TEXT("Prop"),
			Z_Registration_Info_UClass_AProp.InnerSingleton,
			StaticRegisterNativesAProp,
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
	return Z_Registration_Info_UClass_AProp.InnerSingleton;
}
UClass* Z_Construct_UClass_AProp_NoRegister()
{
	return AProp::GetPrivateStaticClass();
}
struct Z_Construct_UClass_AProp_Statics
{
#if WITH_METADATA
	static constexpr UECodeGen_Private::FMetaDataPairParam Class_MetaDataParams[] = {
		{ "BlueprintType", "true" },
#if !UE_BUILD_SHIPPING
		{ "Comment", "/**\n * @brief Convenience base class for Prop Actors.\n * Automatically includes UStagePropComponent for quick setup.\n * All core logic is in the component - this is just a wrapper.\n */" },
#endif
		{ "IncludePath", "Actors/Prop.h" },
		{ "IsBlueprintBase", "true" },
		{ "ModuleRelativePath", "Public/Actors/Prop.h" },
#if !UE_BUILD_SHIPPING
		{ "ToolTip", "@brief Convenience base class for Prop Actors.\nAutomatically includes UStagePropComponent for quick setup.\nAll core logic is in the component - this is just a wrapper." },
#endif
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_PropComponent_MetaData[] = {
		{ "Category", "Stage Prop" },
#if !UE_BUILD_SHIPPING
		{ "Comment", "/** The core Prop component that holds all logic. */" },
#endif
		{ "EditInline", "true" },
		{ "ModuleRelativePath", "Public/Actors/Prop.h" },
#if !UE_BUILD_SHIPPING
		{ "ToolTip", "The core Prop component that holds all logic." },
#endif
	};
#endif // WITH_METADATA
	static const UECodeGen_Private::FObjectPropertyParams NewProp_PropComponent;
	static const UECodeGen_Private::FPropertyParamsBase* const PropPointers[];
	static UObject* (*const DependentSingletons[])();
	static constexpr FClassFunctionLinkInfo FuncInfo[] = {
		{ &Z_Construct_UFunction_AProp_GetPropState, "GetPropState" }, // 1017024482
		{ &Z_Construct_UFunction_AProp_SetPropState, "SetPropState" }, // 4268523349
	};
	static_assert(UE_ARRAY_COUNT(FuncInfo) < 2048);
	static constexpr FCppClassTypeInfoStatic StaticCppClassTypeInfo = {
		TCppClassTypeTraits<AProp>::IsAbstract,
	};
	static const UECodeGen_Private::FClassParams ClassParams;
};
const UECodeGen_Private::FObjectPropertyParams Z_Construct_UClass_AProp_Statics::NewProp_PropComponent = { "PropComponent", nullptr, (EPropertyFlags)0x01140000000a001d, UECodeGen_Private::EPropertyGenFlags::Object | UECodeGen_Private::EPropertyGenFlags::ObjectPtr, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(AProp, PropComponent), Z_Construct_UClass_UStagePropComponent_NoRegister, METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_PropComponent_MetaData), NewProp_PropComponent_MetaData) };
const UECodeGen_Private::FPropertyParamsBase* const Z_Construct_UClass_AProp_Statics::PropPointers[] = {
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_AProp_Statics::NewProp_PropComponent,
};
static_assert(UE_ARRAY_COUNT(Z_Construct_UClass_AProp_Statics::PropPointers) < 2048);
UObject* (*const Z_Construct_UClass_AProp_Statics::DependentSingletons[])() = {
	(UObject* (*)())Z_Construct_UClass_AActor,
	(UObject* (*)())Z_Construct_UPackage__Script_StageEditorRuntime,
};
static_assert(UE_ARRAY_COUNT(Z_Construct_UClass_AProp_Statics::DependentSingletons) < 16);
const UECodeGen_Private::FClassParams Z_Construct_UClass_AProp_Statics::ClassParams = {
	&AProp::StaticClass,
	"Engine",
	&StaticCppClassTypeInfo,
	DependentSingletons,
	FuncInfo,
	Z_Construct_UClass_AProp_Statics::PropPointers,
	nullptr,
	UE_ARRAY_COUNT(DependentSingletons),
	UE_ARRAY_COUNT(FuncInfo),
	UE_ARRAY_COUNT(Z_Construct_UClass_AProp_Statics::PropPointers),
	0,
	0x009001A5u,
	METADATA_PARAMS(UE_ARRAY_COUNT(Z_Construct_UClass_AProp_Statics::Class_MetaDataParams), Z_Construct_UClass_AProp_Statics::Class_MetaDataParams)
};
UClass* Z_Construct_UClass_AProp()
{
	if (!Z_Registration_Info_UClass_AProp.OuterSingleton)
	{
		UECodeGen_Private::ConstructUClass(Z_Registration_Info_UClass_AProp.OuterSingleton, Z_Construct_UClass_AProp_Statics::ClassParams);
	}
	return Z_Registration_Info_UClass_AProp.OuterSingleton;
}
DEFINE_VTABLE_PTR_HELPER_CTOR(AProp);
AProp::~AProp() {}
// ********** End Class AProp **********************************************************************

// ********** Begin Registration *******************************************************************
struct Z_CompiledInDeferFile_FID_ExtendEditor_Plugins_StageEditor_Source_StageEditorRuntime_Public_Actors_Prop_h__Script_StageEditorRuntime_Statics
{
	static constexpr FClassRegisterCompiledInInfo ClassInfo[] = {
		{ Z_Construct_UClass_AProp, AProp::StaticClass, TEXT("AProp"), &Z_Registration_Info_UClass_AProp, CONSTRUCT_RELOAD_VERSION_INFO(FClassReloadVersionInfo, sizeof(AProp), 3073948384U) },
	};
};
static FRegisterCompiledInInfo Z_CompiledInDeferFile_FID_ExtendEditor_Plugins_StageEditor_Source_StageEditorRuntime_Public_Actors_Prop_h__Script_StageEditorRuntime_1568108260(TEXT("/Script/StageEditorRuntime"),
	Z_CompiledInDeferFile_FID_ExtendEditor_Plugins_StageEditor_Source_StageEditorRuntime_Public_Actors_Prop_h__Script_StageEditorRuntime_Statics::ClassInfo, UE_ARRAY_COUNT(Z_CompiledInDeferFile_FID_ExtendEditor_Plugins_StageEditor_Source_StageEditorRuntime_Public_Actors_Prop_h__Script_StageEditorRuntime_Statics::ClassInfo),
	nullptr, 0,
	nullptr, 0);
// ********** End Registration *********************************************************************

PRAGMA_ENABLE_DEPRECATION_WARNINGS
