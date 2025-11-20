// Copyright Epic Games, Inc. All Rights Reserved.
/*===========================================================================
	Generated code exported from UnrealHeaderTool.
	DO NOT modify this manually! Edit the corresponding .h files instead!
===========================================================================*/

#include "UObject/GeneratedCppIncludes.h"
#include "StageEditorModeTestEditorMode.h"

PRAGMA_DISABLE_DEPRECATION_WARNINGS

void EmptyLinkFunctionForGeneratedCodeStageEditorModeTestEditorMode() {}

// ********** Begin Cross Module References ********************************************************
STAGEEDITORMODETEST_API UClass* Z_Construct_UClass_UStageEditorModeTestEditorMode();
STAGEEDITORMODETEST_API UClass* Z_Construct_UClass_UStageEditorModeTestEditorMode_NoRegister();
UNREALED_API UClass* Z_Construct_UClass_UEdMode();
UPackage* Z_Construct_UPackage__Script_StageEditorModeTest();
// ********** End Cross Module References **********************************************************

// ********** Begin Class UStageEditorModeTestEditorMode *******************************************
void UStageEditorModeTestEditorMode::StaticRegisterNativesUStageEditorModeTestEditorMode()
{
}
FClassRegistrationInfo Z_Registration_Info_UClass_UStageEditorModeTestEditorMode;
UClass* UStageEditorModeTestEditorMode::GetPrivateStaticClass()
{
	using TClass = UStageEditorModeTestEditorMode;
	if (!Z_Registration_Info_UClass_UStageEditorModeTestEditorMode.InnerSingleton)
	{
		GetPrivateStaticClassBody(
			StaticPackage(),
			TEXT("StageEditorModeTestEditorMode"),
			Z_Registration_Info_UClass_UStageEditorModeTestEditorMode.InnerSingleton,
			StaticRegisterNativesUStageEditorModeTestEditorMode,
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
	return Z_Registration_Info_UClass_UStageEditorModeTestEditorMode.InnerSingleton;
}
UClass* Z_Construct_UClass_UStageEditorModeTestEditorMode_NoRegister()
{
	return UStageEditorModeTestEditorMode::GetPrivateStaticClass();
}
struct Z_Construct_UClass_UStageEditorModeTestEditorMode_Statics
{
#if WITH_METADATA
	static constexpr UECodeGen_Private::FMetaDataPairParam Class_MetaDataParams[] = {
#if !UE_BUILD_SHIPPING
		{ "Comment", "/**\n * This class provides an example of how to extend a UEdMode to add some simple tools\n * using the InteractiveTools framework. The various UEdMode input event handlers (see UEdMode.h)\n * forward events to a UEdModeInteractiveToolsContext instance, which\n * has all the logic for interacting with the InputRouter, ToolManager, etc.\n * The functions provided here are the minimum to get started inserting some custom behavior.\n * Take a look at the UEdMode markup for more extensibility options.\n */" },
#endif
		{ "IncludePath", "StageEditorModeTestEditorMode.h" },
		{ "ModuleRelativePath", "Public/StageEditorModeTestEditorMode.h" },
#if !UE_BUILD_SHIPPING
		{ "ToolTip", "This class provides an example of how to extend a UEdMode to add some simple tools\nusing the InteractiveTools framework. The various UEdMode input event handlers (see UEdMode.h)\nforward events to a UEdModeInteractiveToolsContext instance, which\nhas all the logic for interacting with the InputRouter, ToolManager, etc.\nThe functions provided here are the minimum to get started inserting some custom behavior.\nTake a look at the UEdMode markup for more extensibility options." },
#endif
	};
#endif // WITH_METADATA
	static UObject* (*const DependentSingletons[])();
	static constexpr FCppClassTypeInfoStatic StaticCppClassTypeInfo = {
		TCppClassTypeTraits<UStageEditorModeTestEditorMode>::IsAbstract,
	};
	static const UECodeGen_Private::FClassParams ClassParams;
};
UObject* (*const Z_Construct_UClass_UStageEditorModeTestEditorMode_Statics::DependentSingletons[])() = {
	(UObject* (*)())Z_Construct_UClass_UEdMode,
	(UObject* (*)())Z_Construct_UPackage__Script_StageEditorModeTest,
};
static_assert(UE_ARRAY_COUNT(Z_Construct_UClass_UStageEditorModeTestEditorMode_Statics::DependentSingletons) < 16);
const UECodeGen_Private::FClassParams Z_Construct_UClass_UStageEditorModeTestEditorMode_Statics::ClassParams = {
	&UStageEditorModeTestEditorMode::StaticClass,
	nullptr,
	&StaticCppClassTypeInfo,
	DependentSingletons,
	nullptr,
	nullptr,
	nullptr,
	UE_ARRAY_COUNT(DependentSingletons),
	0,
	0,
	0,
	0x000000A0u,
	METADATA_PARAMS(UE_ARRAY_COUNT(Z_Construct_UClass_UStageEditorModeTestEditorMode_Statics::Class_MetaDataParams), Z_Construct_UClass_UStageEditorModeTestEditorMode_Statics::Class_MetaDataParams)
};
UClass* Z_Construct_UClass_UStageEditorModeTestEditorMode()
{
	if (!Z_Registration_Info_UClass_UStageEditorModeTestEditorMode.OuterSingleton)
	{
		UECodeGen_Private::ConstructUClass(Z_Registration_Info_UClass_UStageEditorModeTestEditorMode.OuterSingleton, Z_Construct_UClass_UStageEditorModeTestEditorMode_Statics::ClassParams);
	}
	return Z_Registration_Info_UClass_UStageEditorModeTestEditorMode.OuterSingleton;
}
DEFINE_VTABLE_PTR_HELPER_CTOR(UStageEditorModeTestEditorMode);
// ********** End Class UStageEditorModeTestEditorMode *********************************************

// ********** Begin Registration *******************************************************************
struct Z_CompiledInDeferFile_FID_ExtendEditor_Plugins_StageEditorModeTest_Source_StageEditorModeTest_Public_StageEditorModeTestEditorMode_h__Script_StageEditorModeTest_Statics
{
	static constexpr FClassRegisterCompiledInInfo ClassInfo[] = {
		{ Z_Construct_UClass_UStageEditorModeTestEditorMode, UStageEditorModeTestEditorMode::StaticClass, TEXT("UStageEditorModeTestEditorMode"), &Z_Registration_Info_UClass_UStageEditorModeTestEditorMode, CONSTRUCT_RELOAD_VERSION_INFO(FClassReloadVersionInfo, sizeof(UStageEditorModeTestEditorMode), 3613584303U) },
	};
};
static FRegisterCompiledInInfo Z_CompiledInDeferFile_FID_ExtendEditor_Plugins_StageEditorModeTest_Source_StageEditorModeTest_Public_StageEditorModeTestEditorMode_h__Script_StageEditorModeTest_2482929713(TEXT("/Script/StageEditorModeTest"),
	Z_CompiledInDeferFile_FID_ExtendEditor_Plugins_StageEditorModeTest_Source_StageEditorModeTest_Public_StageEditorModeTestEditorMode_h__Script_StageEditorModeTest_Statics::ClassInfo, UE_ARRAY_COUNT(Z_CompiledInDeferFile_FID_ExtendEditor_Plugins_StageEditorModeTest_Source_StageEditorModeTest_Public_StageEditorModeTestEditorMode_h__Script_StageEditorModeTest_Statics::ClassInfo),
	nullptr, 0,
	nullptr, 0);
// ********** End Registration *********************************************************************

PRAGMA_ENABLE_DEPRECATION_WARNINGS
