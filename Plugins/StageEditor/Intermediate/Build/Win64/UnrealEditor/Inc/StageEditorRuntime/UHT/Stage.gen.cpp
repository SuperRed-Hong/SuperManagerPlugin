// Copyright Epic Games, Inc. All Rights Reserved.
/*===========================================================================
	Generated code exported from UnrealHeaderTool.
	DO NOT modify this manually! Edit the corresponding .h files instead!
===========================================================================*/

#include "UObject/GeneratedCppIncludes.h"
#include "Actors/Stage.h"
#include "Core/StageCoreTypes.h"

PRAGMA_DISABLE_DEPRECATION_WARNINGS

void EmptyLinkFunctionForGeneratedCodeStage() {}

// ********** Begin Cross Module References ********************************************************
ENGINE_API UClass* Z_Construct_UClass_AActor();
ENGINE_API UClass* Z_Construct_UClass_AActor_NoRegister();
STAGEEDITORRUNTIME_API UClass* Z_Construct_UClass_AStage();
STAGEEDITORRUNTIME_API UClass* Z_Construct_UClass_AStage_NoRegister();
STAGEEDITORRUNTIME_API UScriptStruct* Z_Construct_UScriptStruct_FAct();
UPackage* Z_Construct_UPackage__Script_StageEditorRuntime();
// ********** End Cross Module References **********************************************************

// ********** Begin Class AStage Function ActivateAct **********************************************
struct Z_Construct_UFunction_AStage_ActivateAct_Statics
{
	struct Stage_eventActivateAct_Parms
	{
		int32 ActID;
	};
#if WITH_METADATA
	static constexpr UECodeGen_Private::FMetaDataPairParam Function_MetaDataParams[] = {
		{ "Category", "Stage" },
#if !UE_BUILD_SHIPPING
		{ "Comment", "/**\n\x09 * @brief Activates a specific Act by its local ID.\n\x09 * Applies the PropState overrides defined in the Act to the registered Props.\n\x09 */" },
#endif
		{ "ModuleRelativePath", "Public/Actors/Stage.h" },
#if !UE_BUILD_SHIPPING
		{ "ToolTip", "@brief Activates a specific Act by its local ID.\nApplies the PropState overrides defined in the Act to the registered Props." },
#endif
	};
#endif // WITH_METADATA
	static const UECodeGen_Private::FIntPropertyParams NewProp_ActID;
	static const UECodeGen_Private::FPropertyParamsBase* const PropPointers[];
	static const UECodeGen_Private::FFunctionParams FuncParams;
};
const UECodeGen_Private::FIntPropertyParams Z_Construct_UFunction_AStage_ActivateAct_Statics::NewProp_ActID = { "ActID", nullptr, (EPropertyFlags)0x0010000000000080, UECodeGen_Private::EPropertyGenFlags::Int, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(Stage_eventActivateAct_Parms, ActID), METADATA_PARAMS(0, nullptr) };
const UECodeGen_Private::FPropertyParamsBase* const Z_Construct_UFunction_AStage_ActivateAct_Statics::PropPointers[] = {
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UFunction_AStage_ActivateAct_Statics::NewProp_ActID,
};
static_assert(UE_ARRAY_COUNT(Z_Construct_UFunction_AStage_ActivateAct_Statics::PropPointers) < 2048);
const UECodeGen_Private::FFunctionParams Z_Construct_UFunction_AStage_ActivateAct_Statics::FuncParams = { { (UObject*(*)())Z_Construct_UClass_AStage, nullptr, "ActivateAct", Z_Construct_UFunction_AStage_ActivateAct_Statics::PropPointers, UE_ARRAY_COUNT(Z_Construct_UFunction_AStage_ActivateAct_Statics::PropPointers), sizeof(Z_Construct_UFunction_AStage_ActivateAct_Statics::Stage_eventActivateAct_Parms), RF_Public|RF_Transient|RF_MarkAsNative, (EFunctionFlags)0x04020401, 0, 0, METADATA_PARAMS(UE_ARRAY_COUNT(Z_Construct_UFunction_AStage_ActivateAct_Statics::Function_MetaDataParams), Z_Construct_UFunction_AStage_ActivateAct_Statics::Function_MetaDataParams)},  };
static_assert(sizeof(Z_Construct_UFunction_AStage_ActivateAct_Statics::Stage_eventActivateAct_Parms) < MAX_uint16);
UFunction* Z_Construct_UFunction_AStage_ActivateAct()
{
	static UFunction* ReturnFunction = nullptr;
	if (!ReturnFunction)
	{
		UECodeGen_Private::ConstructUFunction(&ReturnFunction, Z_Construct_UFunction_AStage_ActivateAct_Statics::FuncParams);
	}
	return ReturnFunction;
}
DEFINE_FUNCTION(AStage::execActivateAct)
{
	P_GET_PROPERTY(FIntProperty,Z_Param_ActID);
	P_FINISH;
	P_NATIVE_BEGIN;
	P_THIS->ActivateAct(Z_Param_ActID);
	P_NATIVE_END;
}
// ********** End Class AStage Function ActivateAct ************************************************

// ********** Begin Class AStage Function DeactivateAct ********************************************
struct Z_Construct_UFunction_AStage_DeactivateAct_Statics
{
#if WITH_METADATA
	static constexpr UECodeGen_Private::FMetaDataPairParam Function_MetaDataParams[] = {
		{ "Category", "Stage" },
#if !UE_BUILD_SHIPPING
		{ "Comment", "/**\n\x09 * @brief Deactivates the current Act (optional logic, e.g., reset props).\n\x09 */" },
#endif
		{ "ModuleRelativePath", "Public/Actors/Stage.h" },
#if !UE_BUILD_SHIPPING
		{ "ToolTip", "@brief Deactivates the current Act (optional logic, e.g., reset props)." },
#endif
	};
#endif // WITH_METADATA
	static const UECodeGen_Private::FFunctionParams FuncParams;
};
const UECodeGen_Private::FFunctionParams Z_Construct_UFunction_AStage_DeactivateAct_Statics::FuncParams = { { (UObject*(*)())Z_Construct_UClass_AStage, nullptr, "DeactivateAct", nullptr, 0, 0, RF_Public|RF_Transient|RF_MarkAsNative, (EFunctionFlags)0x04020401, 0, 0, METADATA_PARAMS(UE_ARRAY_COUNT(Z_Construct_UFunction_AStage_DeactivateAct_Statics::Function_MetaDataParams), Z_Construct_UFunction_AStage_DeactivateAct_Statics::Function_MetaDataParams)},  };
UFunction* Z_Construct_UFunction_AStage_DeactivateAct()
{
	static UFunction* ReturnFunction = nullptr;
	if (!ReturnFunction)
	{
		UECodeGen_Private::ConstructUFunction(&ReturnFunction, Z_Construct_UFunction_AStage_DeactivateAct_Statics::FuncParams);
	}
	return ReturnFunction;
}
DEFINE_FUNCTION(AStage::execDeactivateAct)
{
	P_FINISH;
	P_NATIVE_BEGIN;
	P_THIS->DeactivateAct();
	P_NATIVE_END;
}
// ********** End Class AStage Function DeactivateAct **********************************************

// ********** Begin Class AStage *******************************************************************
void AStage::StaticRegisterNativesAStage()
{
	UClass* Class = AStage::StaticClass();
	static const FNameNativePtrPair Funcs[] = {
		{ "ActivateAct", &AStage::execActivateAct },
		{ "DeactivateAct", &AStage::execDeactivateAct },
	};
	FNativeFunctionRegistrar::RegisterFunctions(Class, Funcs, UE_ARRAY_COUNT(Funcs));
}
FClassRegistrationInfo Z_Registration_Info_UClass_AStage;
UClass* AStage::GetPrivateStaticClass()
{
	using TClass = AStage;
	if (!Z_Registration_Info_UClass_AStage.InnerSingleton)
	{
		GetPrivateStaticClassBody(
			StaticPackage(),
			TEXT("Stage"),
			Z_Registration_Info_UClass_AStage.InnerSingleton,
			StaticRegisterNativesAStage,
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
	return Z_Registration_Info_UClass_AStage.InnerSingleton;
}
UClass* Z_Construct_UClass_AStage_NoRegister()
{
	return AStage::GetPrivateStaticClass();
}
struct Z_Construct_UClass_AStage_Statics
{
#if WITH_METADATA
	static constexpr UECodeGen_Private::FMetaDataPairParam Class_MetaDataParams[] = {
#if !UE_BUILD_SHIPPING
		{ "Comment", "/**\n * @brief The \"Director\" of the stage.\n * Manages a list of Acts and a registry of Props.\n * Responsible for applying Act states to Props.\n */" },
#endif
		{ "IncludePath", "Actors/Stage.h" },
		{ "ModuleRelativePath", "Public/Actors/Stage.h" },
#if !UE_BUILD_SHIPPING
		{ "ToolTip", "@brief The \"Director\" of the stage.\nManages a list of Acts and a registry of Props.\nResponsible for applying Act states to Props." },
#endif
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_StageID_MetaData[] = {
		{ "Category", "Stage" },
#if !UE_BUILD_SHIPPING
		{ "Comment", "/** Globally unique ID for this Stage. */" },
#endif
		{ "ModuleRelativePath", "Public/Actors/Stage.h" },
#if !UE_BUILD_SHIPPING
		{ "ToolTip", "Globally unique ID for this Stage." },
#endif
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_Acts_MetaData[] = {
		{ "Category", "Stage" },
#if !UE_BUILD_SHIPPING
		{ "Comment", "/** List of all defined Acts for this Stage. */" },
#endif
		{ "ModuleRelativePath", "Public/Actors/Stage.h" },
#if !UE_BUILD_SHIPPING
		{ "ToolTip", "List of all defined Acts for this Stage." },
#endif
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_PropRegistry_MetaData[] = {
		{ "Category", "Stage" },
#if !UE_BUILD_SHIPPING
		{ "Comment", "/** \n\x09 * Registry of Props managed by this Stage.\n\x09 * Key: PropID, Value: Soft reference to the Prop Actor.\n\x09 */" },
#endif
		{ "ModuleRelativePath", "Public/Actors/Stage.h" },
#if !UE_BUILD_SHIPPING
		{ "ToolTip", "Registry of Props managed by this Stage.\nKey: PropID, Value: Soft reference to the Prop Actor." },
#endif
	};
#endif // WITH_METADATA
	static const UECodeGen_Private::FIntPropertyParams NewProp_StageID;
	static const UECodeGen_Private::FStructPropertyParams NewProp_Acts_Inner;
	static const UECodeGen_Private::FArrayPropertyParams NewProp_Acts;
	static const UECodeGen_Private::FSoftObjectPropertyParams NewProp_PropRegistry_ValueProp;
	static const UECodeGen_Private::FIntPropertyParams NewProp_PropRegistry_Key_KeyProp;
	static const UECodeGen_Private::FMapPropertyParams NewProp_PropRegistry;
	static const UECodeGen_Private::FPropertyParamsBase* const PropPointers[];
	static UObject* (*const DependentSingletons[])();
	static constexpr FClassFunctionLinkInfo FuncInfo[] = {
		{ &Z_Construct_UFunction_AStage_ActivateAct, "ActivateAct" }, // 2233688465
		{ &Z_Construct_UFunction_AStage_DeactivateAct, "DeactivateAct" }, // 730265823
	};
	static_assert(UE_ARRAY_COUNT(FuncInfo) < 2048);
	static constexpr FCppClassTypeInfoStatic StaticCppClassTypeInfo = {
		TCppClassTypeTraits<AStage>::IsAbstract,
	};
	static const UECodeGen_Private::FClassParams ClassParams;
};
const UECodeGen_Private::FIntPropertyParams Z_Construct_UClass_AStage_Statics::NewProp_StageID = { "StageID", nullptr, (EPropertyFlags)0x0010000000000015, UECodeGen_Private::EPropertyGenFlags::Int, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(AStage, StageID), METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_StageID_MetaData), NewProp_StageID_MetaData) };
const UECodeGen_Private::FStructPropertyParams Z_Construct_UClass_AStage_Statics::NewProp_Acts_Inner = { "Acts", nullptr, (EPropertyFlags)0x0000000000000000, UECodeGen_Private::EPropertyGenFlags::Struct, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, 0, Z_Construct_UScriptStruct_FAct, METADATA_PARAMS(0, nullptr) }; // 697850678
const UECodeGen_Private::FArrayPropertyParams Z_Construct_UClass_AStage_Statics::NewProp_Acts = { "Acts", nullptr, (EPropertyFlags)0x0010000000000005, UECodeGen_Private::EPropertyGenFlags::Array, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(AStage, Acts), EArrayPropertyFlags::None, METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_Acts_MetaData), NewProp_Acts_MetaData) }; // 697850678
const UECodeGen_Private::FSoftObjectPropertyParams Z_Construct_UClass_AStage_Statics::NewProp_PropRegistry_ValueProp = { "PropRegistry", nullptr, (EPropertyFlags)0x0004000000000001, UECodeGen_Private::EPropertyGenFlags::SoftObject, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, 1, Z_Construct_UClass_AActor_NoRegister, METADATA_PARAMS(0, nullptr) };
const UECodeGen_Private::FIntPropertyParams Z_Construct_UClass_AStage_Statics::NewProp_PropRegistry_Key_KeyProp = { "PropRegistry_Key", nullptr, (EPropertyFlags)0x0000000000000001, UECodeGen_Private::EPropertyGenFlags::Int, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, 0, METADATA_PARAMS(0, nullptr) };
const UECodeGen_Private::FMapPropertyParams Z_Construct_UClass_AStage_Statics::NewProp_PropRegistry = { "PropRegistry", nullptr, (EPropertyFlags)0x0014000000000015, UECodeGen_Private::EPropertyGenFlags::Map, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(AStage, PropRegistry), EMapPropertyFlags::None, METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_PropRegistry_MetaData), NewProp_PropRegistry_MetaData) };
const UECodeGen_Private::FPropertyParamsBase* const Z_Construct_UClass_AStage_Statics::PropPointers[] = {
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_AStage_Statics::NewProp_StageID,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_AStage_Statics::NewProp_Acts_Inner,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_AStage_Statics::NewProp_Acts,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_AStage_Statics::NewProp_PropRegistry_ValueProp,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_AStage_Statics::NewProp_PropRegistry_Key_KeyProp,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_AStage_Statics::NewProp_PropRegistry,
};
static_assert(UE_ARRAY_COUNT(Z_Construct_UClass_AStage_Statics::PropPointers) < 2048);
UObject* (*const Z_Construct_UClass_AStage_Statics::DependentSingletons[])() = {
	(UObject* (*)())Z_Construct_UClass_AActor,
	(UObject* (*)())Z_Construct_UPackage__Script_StageEditorRuntime,
};
static_assert(UE_ARRAY_COUNT(Z_Construct_UClass_AStage_Statics::DependentSingletons) < 16);
const UECodeGen_Private::FClassParams Z_Construct_UClass_AStage_Statics::ClassParams = {
	&AStage::StaticClass,
	"Engine",
	&StaticCppClassTypeInfo,
	DependentSingletons,
	FuncInfo,
	Z_Construct_UClass_AStage_Statics::PropPointers,
	nullptr,
	UE_ARRAY_COUNT(DependentSingletons),
	UE_ARRAY_COUNT(FuncInfo),
	UE_ARRAY_COUNT(Z_Construct_UClass_AStage_Statics::PropPointers),
	0,
	0x009001A4u,
	METADATA_PARAMS(UE_ARRAY_COUNT(Z_Construct_UClass_AStage_Statics::Class_MetaDataParams), Z_Construct_UClass_AStage_Statics::Class_MetaDataParams)
};
UClass* Z_Construct_UClass_AStage()
{
	if (!Z_Registration_Info_UClass_AStage.OuterSingleton)
	{
		UECodeGen_Private::ConstructUClass(Z_Registration_Info_UClass_AStage.OuterSingleton, Z_Construct_UClass_AStage_Statics::ClassParams);
	}
	return Z_Registration_Info_UClass_AStage.OuterSingleton;
}
DEFINE_VTABLE_PTR_HELPER_CTOR(AStage);
AStage::~AStage() {}
// ********** End Class AStage *********************************************************************

// ********** Begin Registration *******************************************************************
struct Z_CompiledInDeferFile_FID_ExtendEditor_Plugins_StageEditor_Source_StageEditorRuntime_Public_Actors_Stage_h__Script_StageEditorRuntime_Statics
{
	static constexpr FClassRegisterCompiledInInfo ClassInfo[] = {
		{ Z_Construct_UClass_AStage, AStage::StaticClass, TEXT("AStage"), &Z_Registration_Info_UClass_AStage, CONSTRUCT_RELOAD_VERSION_INFO(FClassReloadVersionInfo, sizeof(AStage), 759409182U) },
	};
};
static FRegisterCompiledInInfo Z_CompiledInDeferFile_FID_ExtendEditor_Plugins_StageEditor_Source_StageEditorRuntime_Public_Actors_Stage_h__Script_StageEditorRuntime_2144941575(TEXT("/Script/StageEditorRuntime"),
	Z_CompiledInDeferFile_FID_ExtendEditor_Plugins_StageEditor_Source_StageEditorRuntime_Public_Actors_Stage_h__Script_StageEditorRuntime_Statics::ClassInfo, UE_ARRAY_COUNT(Z_CompiledInDeferFile_FID_ExtendEditor_Plugins_StageEditor_Source_StageEditorRuntime_Public_Actors_Stage_h__Script_StageEditorRuntime_Statics::ClassInfo),
	nullptr, 0,
	nullptr, 0);
// ********** End Registration *********************************************************************

PRAGMA_ENABLE_DEPRECATION_WARNINGS
