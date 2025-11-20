// Copyright Epic Games, Inc. All Rights Reserved.
/*===========================================================================
	Generated code exported from UnrealHeaderTool.
	DO NOT modify this manually! Edit the corresponding .h files instead!
===========================================================================*/

#include "UObject/GeneratedCppIncludes.h"
#include "Tools/StageEditorModeTestSimpleTool.h"

PRAGMA_DISABLE_DEPRECATION_WARNINGS

void EmptyLinkFunctionForGeneratedCodeStageEditorModeTestSimpleTool() {}

// ********** Begin Cross Module References ********************************************************
INTERACTIVETOOLSFRAMEWORK_API UClass* Z_Construct_UClass_UInteractiveToolBuilder();
INTERACTIVETOOLSFRAMEWORK_API UClass* Z_Construct_UClass_UInteractiveToolPropertySet();
INTERACTIVETOOLSFRAMEWORK_API UClass* Z_Construct_UClass_USingleClickTool();
STAGEEDITORMODETEST_API UClass* Z_Construct_UClass_UStageEditorModeTestSimpleTool();
STAGEEDITORMODETEST_API UClass* Z_Construct_UClass_UStageEditorModeTestSimpleTool_NoRegister();
STAGEEDITORMODETEST_API UClass* Z_Construct_UClass_UStageEditorModeTestSimpleToolBuilder();
STAGEEDITORMODETEST_API UClass* Z_Construct_UClass_UStageEditorModeTestSimpleToolBuilder_NoRegister();
STAGEEDITORMODETEST_API UClass* Z_Construct_UClass_UStageEditorModeTestSimpleToolProperties();
STAGEEDITORMODETEST_API UClass* Z_Construct_UClass_UStageEditorModeTestSimpleToolProperties_NoRegister();
UPackage* Z_Construct_UPackage__Script_StageEditorModeTest();
// ********** End Cross Module References **********************************************************

// ********** Begin Class UStageEditorModeTestSimpleToolBuilder ************************************
void UStageEditorModeTestSimpleToolBuilder::StaticRegisterNativesUStageEditorModeTestSimpleToolBuilder()
{
}
FClassRegistrationInfo Z_Registration_Info_UClass_UStageEditorModeTestSimpleToolBuilder;
UClass* UStageEditorModeTestSimpleToolBuilder::GetPrivateStaticClass()
{
	using TClass = UStageEditorModeTestSimpleToolBuilder;
	if (!Z_Registration_Info_UClass_UStageEditorModeTestSimpleToolBuilder.InnerSingleton)
	{
		GetPrivateStaticClassBody(
			StaticPackage(),
			TEXT("StageEditorModeTestSimpleToolBuilder"),
			Z_Registration_Info_UClass_UStageEditorModeTestSimpleToolBuilder.InnerSingleton,
			StaticRegisterNativesUStageEditorModeTestSimpleToolBuilder,
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
	return Z_Registration_Info_UClass_UStageEditorModeTestSimpleToolBuilder.InnerSingleton;
}
UClass* Z_Construct_UClass_UStageEditorModeTestSimpleToolBuilder_NoRegister()
{
	return UStageEditorModeTestSimpleToolBuilder::GetPrivateStaticClass();
}
struct Z_Construct_UClass_UStageEditorModeTestSimpleToolBuilder_Statics
{
#if WITH_METADATA
	static constexpr UECodeGen_Private::FMetaDataPairParam Class_MetaDataParams[] = {
#if !UE_BUILD_SHIPPING
		{ "Comment", "/**\n * Builder for UStageEditorModeTestSimpleTool\n */" },
#endif
		{ "IncludePath", "Tools/StageEditorModeTestSimpleTool.h" },
		{ "ModuleRelativePath", "Private/Tools/StageEditorModeTestSimpleTool.h" },
#if !UE_BUILD_SHIPPING
		{ "ToolTip", "Builder for UStageEditorModeTestSimpleTool" },
#endif
	};
#endif // WITH_METADATA
	static UObject* (*const DependentSingletons[])();
	static constexpr FCppClassTypeInfoStatic StaticCppClassTypeInfo = {
		TCppClassTypeTraits<UStageEditorModeTestSimpleToolBuilder>::IsAbstract,
	};
	static const UECodeGen_Private::FClassParams ClassParams;
};
UObject* (*const Z_Construct_UClass_UStageEditorModeTestSimpleToolBuilder_Statics::DependentSingletons[])() = {
	(UObject* (*)())Z_Construct_UClass_UInteractiveToolBuilder,
	(UObject* (*)())Z_Construct_UPackage__Script_StageEditorModeTest,
};
static_assert(UE_ARRAY_COUNT(Z_Construct_UClass_UStageEditorModeTestSimpleToolBuilder_Statics::DependentSingletons) < 16);
const UECodeGen_Private::FClassParams Z_Construct_UClass_UStageEditorModeTestSimpleToolBuilder_Statics::ClassParams = {
	&UStageEditorModeTestSimpleToolBuilder::StaticClass,
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
	0x001000A8u,
	METADATA_PARAMS(UE_ARRAY_COUNT(Z_Construct_UClass_UStageEditorModeTestSimpleToolBuilder_Statics::Class_MetaDataParams), Z_Construct_UClass_UStageEditorModeTestSimpleToolBuilder_Statics::Class_MetaDataParams)
};
UClass* Z_Construct_UClass_UStageEditorModeTestSimpleToolBuilder()
{
	if (!Z_Registration_Info_UClass_UStageEditorModeTestSimpleToolBuilder.OuterSingleton)
	{
		UECodeGen_Private::ConstructUClass(Z_Registration_Info_UClass_UStageEditorModeTestSimpleToolBuilder.OuterSingleton, Z_Construct_UClass_UStageEditorModeTestSimpleToolBuilder_Statics::ClassParams);
	}
	return Z_Registration_Info_UClass_UStageEditorModeTestSimpleToolBuilder.OuterSingleton;
}
UStageEditorModeTestSimpleToolBuilder::UStageEditorModeTestSimpleToolBuilder(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer) {}
DEFINE_VTABLE_PTR_HELPER_CTOR(UStageEditorModeTestSimpleToolBuilder);
UStageEditorModeTestSimpleToolBuilder::~UStageEditorModeTestSimpleToolBuilder() {}
// ********** End Class UStageEditorModeTestSimpleToolBuilder **************************************

// ********** Begin Class UStageEditorModeTestSimpleToolProperties *********************************
void UStageEditorModeTestSimpleToolProperties::StaticRegisterNativesUStageEditorModeTestSimpleToolProperties()
{
}
FClassRegistrationInfo Z_Registration_Info_UClass_UStageEditorModeTestSimpleToolProperties;
UClass* UStageEditorModeTestSimpleToolProperties::GetPrivateStaticClass()
{
	using TClass = UStageEditorModeTestSimpleToolProperties;
	if (!Z_Registration_Info_UClass_UStageEditorModeTestSimpleToolProperties.InnerSingleton)
	{
		GetPrivateStaticClassBody(
			StaticPackage(),
			TEXT("StageEditorModeTestSimpleToolProperties"),
			Z_Registration_Info_UClass_UStageEditorModeTestSimpleToolProperties.InnerSingleton,
			StaticRegisterNativesUStageEditorModeTestSimpleToolProperties,
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
	return Z_Registration_Info_UClass_UStageEditorModeTestSimpleToolProperties.InnerSingleton;
}
UClass* Z_Construct_UClass_UStageEditorModeTestSimpleToolProperties_NoRegister()
{
	return UStageEditorModeTestSimpleToolProperties::GetPrivateStaticClass();
}
struct Z_Construct_UClass_UStageEditorModeTestSimpleToolProperties_Statics
{
#if WITH_METADATA
	static constexpr UECodeGen_Private::FMetaDataPairParam Class_MetaDataParams[] = {
#if !UE_BUILD_SHIPPING
		{ "Comment", "/**\n * Settings UObject for UStageEditorModeTestSimpleTool. This UClass inherits from UInteractiveToolPropertySet,\n * which provides an OnModified delegate that the Tool will listen to for changes in property values.\n */" },
#endif
		{ "IncludePath", "Tools/StageEditorModeTestSimpleTool.h" },
		{ "ModuleRelativePath", "Private/Tools/StageEditorModeTestSimpleTool.h" },
#if !UE_BUILD_SHIPPING
		{ "ToolTip", "Settings UObject for UStageEditorModeTestSimpleTool. This UClass inherits from UInteractiveToolPropertySet,\nwhich provides an OnModified delegate that the Tool will listen to for changes in property values." },
#endif
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_ShowExtendedInfo_MetaData[] = {
		{ "Category", "Options" },
#if !UE_BUILD_SHIPPING
		{ "Comment", "/** If enabled, dialog should display extended information about the actor clicked on. Otherwise, only basic info will be shown. */" },
#endif
		{ "DisplayName", "Show Extended Info" },
		{ "ModuleRelativePath", "Private/Tools/StageEditorModeTestSimpleTool.h" },
#if !UE_BUILD_SHIPPING
		{ "ToolTip", "If enabled, dialog should display extended information about the actor clicked on. Otherwise, only basic info will be shown." },
#endif
	};
#endif // WITH_METADATA
	static void NewProp_ShowExtendedInfo_SetBit(void* Obj);
	static const UECodeGen_Private::FBoolPropertyParams NewProp_ShowExtendedInfo;
	static const UECodeGen_Private::FPropertyParamsBase* const PropPointers[];
	static UObject* (*const DependentSingletons[])();
	static constexpr FCppClassTypeInfoStatic StaticCppClassTypeInfo = {
		TCppClassTypeTraits<UStageEditorModeTestSimpleToolProperties>::IsAbstract,
	};
	static const UECodeGen_Private::FClassParams ClassParams;
};
void Z_Construct_UClass_UStageEditorModeTestSimpleToolProperties_Statics::NewProp_ShowExtendedInfo_SetBit(void* Obj)
{
	((UStageEditorModeTestSimpleToolProperties*)Obj)->ShowExtendedInfo = 1;
}
const UECodeGen_Private::FBoolPropertyParams Z_Construct_UClass_UStageEditorModeTestSimpleToolProperties_Statics::NewProp_ShowExtendedInfo = { "ShowExtendedInfo", nullptr, (EPropertyFlags)0x0010000000000001, UECodeGen_Private::EPropertyGenFlags::Bool | UECodeGen_Private::EPropertyGenFlags::NativeBool, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, sizeof(bool), sizeof(UStageEditorModeTestSimpleToolProperties), &Z_Construct_UClass_UStageEditorModeTestSimpleToolProperties_Statics::NewProp_ShowExtendedInfo_SetBit, METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_ShowExtendedInfo_MetaData), NewProp_ShowExtendedInfo_MetaData) };
const UECodeGen_Private::FPropertyParamsBase* const Z_Construct_UClass_UStageEditorModeTestSimpleToolProperties_Statics::PropPointers[] = {
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_UStageEditorModeTestSimpleToolProperties_Statics::NewProp_ShowExtendedInfo,
};
static_assert(UE_ARRAY_COUNT(Z_Construct_UClass_UStageEditorModeTestSimpleToolProperties_Statics::PropPointers) < 2048);
UObject* (*const Z_Construct_UClass_UStageEditorModeTestSimpleToolProperties_Statics::DependentSingletons[])() = {
	(UObject* (*)())Z_Construct_UClass_UInteractiveToolPropertySet,
	(UObject* (*)())Z_Construct_UPackage__Script_StageEditorModeTest,
};
static_assert(UE_ARRAY_COUNT(Z_Construct_UClass_UStageEditorModeTestSimpleToolProperties_Statics::DependentSingletons) < 16);
const UECodeGen_Private::FClassParams Z_Construct_UClass_UStageEditorModeTestSimpleToolProperties_Statics::ClassParams = {
	&UStageEditorModeTestSimpleToolProperties::StaticClass,
	nullptr,
	&StaticCppClassTypeInfo,
	DependentSingletons,
	nullptr,
	Z_Construct_UClass_UStageEditorModeTestSimpleToolProperties_Statics::PropPointers,
	nullptr,
	UE_ARRAY_COUNT(DependentSingletons),
	0,
	UE_ARRAY_COUNT(Z_Construct_UClass_UStageEditorModeTestSimpleToolProperties_Statics::PropPointers),
	0,
	0x001000A8u,
	METADATA_PARAMS(UE_ARRAY_COUNT(Z_Construct_UClass_UStageEditorModeTestSimpleToolProperties_Statics::Class_MetaDataParams), Z_Construct_UClass_UStageEditorModeTestSimpleToolProperties_Statics::Class_MetaDataParams)
};
UClass* Z_Construct_UClass_UStageEditorModeTestSimpleToolProperties()
{
	if (!Z_Registration_Info_UClass_UStageEditorModeTestSimpleToolProperties.OuterSingleton)
	{
		UECodeGen_Private::ConstructUClass(Z_Registration_Info_UClass_UStageEditorModeTestSimpleToolProperties.OuterSingleton, Z_Construct_UClass_UStageEditorModeTestSimpleToolProperties_Statics::ClassParams);
	}
	return Z_Registration_Info_UClass_UStageEditorModeTestSimpleToolProperties.OuterSingleton;
}
DEFINE_VTABLE_PTR_HELPER_CTOR(UStageEditorModeTestSimpleToolProperties);
UStageEditorModeTestSimpleToolProperties::~UStageEditorModeTestSimpleToolProperties() {}
// ********** End Class UStageEditorModeTestSimpleToolProperties ***********************************

// ********** Begin Class UStageEditorModeTestSimpleTool *******************************************
void UStageEditorModeTestSimpleTool::StaticRegisterNativesUStageEditorModeTestSimpleTool()
{
}
FClassRegistrationInfo Z_Registration_Info_UClass_UStageEditorModeTestSimpleTool;
UClass* UStageEditorModeTestSimpleTool::GetPrivateStaticClass()
{
	using TClass = UStageEditorModeTestSimpleTool;
	if (!Z_Registration_Info_UClass_UStageEditorModeTestSimpleTool.InnerSingleton)
	{
		GetPrivateStaticClassBody(
			StaticPackage(),
			TEXT("StageEditorModeTestSimpleTool"),
			Z_Registration_Info_UClass_UStageEditorModeTestSimpleTool.InnerSingleton,
			StaticRegisterNativesUStageEditorModeTestSimpleTool,
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
	return Z_Registration_Info_UClass_UStageEditorModeTestSimpleTool.InnerSingleton;
}
UClass* Z_Construct_UClass_UStageEditorModeTestSimpleTool_NoRegister()
{
	return UStageEditorModeTestSimpleTool::GetPrivateStaticClass();
}
struct Z_Construct_UClass_UStageEditorModeTestSimpleTool_Statics
{
#if WITH_METADATA
	static constexpr UECodeGen_Private::FMetaDataPairParam Class_MetaDataParams[] = {
#if !UE_BUILD_SHIPPING
		{ "Comment", "/**\n * UStageEditorModeTestSimpleTool is an example Tool that opens a message box displaying info about an actor that the user\n * clicks left mouse button. All the action is in the ::OnClicked handler.\n */" },
#endif
		{ "IncludePath", "Tools/StageEditorModeTestSimpleTool.h" },
		{ "ModuleRelativePath", "Private/Tools/StageEditorModeTestSimpleTool.h" },
#if !UE_BUILD_SHIPPING
		{ "ToolTip", "UStageEditorModeTestSimpleTool is an example Tool that opens a message box displaying info about an actor that the user\nclicks left mouse button. All the action is in the ::OnClicked handler." },
#endif
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_Properties_MetaData[] = {
		{ "ModuleRelativePath", "Private/Tools/StageEditorModeTestSimpleTool.h" },
	};
#endif // WITH_METADATA
	static const UECodeGen_Private::FObjectPropertyParams NewProp_Properties;
	static const UECodeGen_Private::FPropertyParamsBase* const PropPointers[];
	static UObject* (*const DependentSingletons[])();
	static constexpr FCppClassTypeInfoStatic StaticCppClassTypeInfo = {
		TCppClassTypeTraits<UStageEditorModeTestSimpleTool>::IsAbstract,
	};
	static const UECodeGen_Private::FClassParams ClassParams;
};
const UECodeGen_Private::FObjectPropertyParams Z_Construct_UClass_UStageEditorModeTestSimpleTool_Statics::NewProp_Properties = { "Properties", nullptr, (EPropertyFlags)0x0124080000000000, UECodeGen_Private::EPropertyGenFlags::Object | UECodeGen_Private::EPropertyGenFlags::ObjectPtr, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(UStageEditorModeTestSimpleTool, Properties), Z_Construct_UClass_UStageEditorModeTestSimpleToolProperties_NoRegister, METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_Properties_MetaData), NewProp_Properties_MetaData) };
const UECodeGen_Private::FPropertyParamsBase* const Z_Construct_UClass_UStageEditorModeTestSimpleTool_Statics::PropPointers[] = {
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_UStageEditorModeTestSimpleTool_Statics::NewProp_Properties,
};
static_assert(UE_ARRAY_COUNT(Z_Construct_UClass_UStageEditorModeTestSimpleTool_Statics::PropPointers) < 2048);
UObject* (*const Z_Construct_UClass_UStageEditorModeTestSimpleTool_Statics::DependentSingletons[])() = {
	(UObject* (*)())Z_Construct_UClass_USingleClickTool,
	(UObject* (*)())Z_Construct_UPackage__Script_StageEditorModeTest,
};
static_assert(UE_ARRAY_COUNT(Z_Construct_UClass_UStageEditorModeTestSimpleTool_Statics::DependentSingletons) < 16);
const UECodeGen_Private::FClassParams Z_Construct_UClass_UStageEditorModeTestSimpleTool_Statics::ClassParams = {
	&UStageEditorModeTestSimpleTool::StaticClass,
	nullptr,
	&StaticCppClassTypeInfo,
	DependentSingletons,
	nullptr,
	Z_Construct_UClass_UStageEditorModeTestSimpleTool_Statics::PropPointers,
	nullptr,
	UE_ARRAY_COUNT(DependentSingletons),
	0,
	UE_ARRAY_COUNT(Z_Construct_UClass_UStageEditorModeTestSimpleTool_Statics::PropPointers),
	0,
	0x001000A8u,
	METADATA_PARAMS(UE_ARRAY_COUNT(Z_Construct_UClass_UStageEditorModeTestSimpleTool_Statics::Class_MetaDataParams), Z_Construct_UClass_UStageEditorModeTestSimpleTool_Statics::Class_MetaDataParams)
};
UClass* Z_Construct_UClass_UStageEditorModeTestSimpleTool()
{
	if (!Z_Registration_Info_UClass_UStageEditorModeTestSimpleTool.OuterSingleton)
	{
		UECodeGen_Private::ConstructUClass(Z_Registration_Info_UClass_UStageEditorModeTestSimpleTool.OuterSingleton, Z_Construct_UClass_UStageEditorModeTestSimpleTool_Statics::ClassParams);
	}
	return Z_Registration_Info_UClass_UStageEditorModeTestSimpleTool.OuterSingleton;
}
DEFINE_VTABLE_PTR_HELPER_CTOR(UStageEditorModeTestSimpleTool);
UStageEditorModeTestSimpleTool::~UStageEditorModeTestSimpleTool() {}
// ********** End Class UStageEditorModeTestSimpleTool *********************************************

// ********** Begin Registration *******************************************************************
struct Z_CompiledInDeferFile_FID_ExtendEditor_Plugins_StageEditorModeTest_Source_StageEditorModeTest_Private_Tools_StageEditorModeTestSimpleTool_h__Script_StageEditorModeTest_Statics
{
	static constexpr FClassRegisterCompiledInInfo ClassInfo[] = {
		{ Z_Construct_UClass_UStageEditorModeTestSimpleToolBuilder, UStageEditorModeTestSimpleToolBuilder::StaticClass, TEXT("UStageEditorModeTestSimpleToolBuilder"), &Z_Registration_Info_UClass_UStageEditorModeTestSimpleToolBuilder, CONSTRUCT_RELOAD_VERSION_INFO(FClassReloadVersionInfo, sizeof(UStageEditorModeTestSimpleToolBuilder), 2709987119U) },
		{ Z_Construct_UClass_UStageEditorModeTestSimpleToolProperties, UStageEditorModeTestSimpleToolProperties::StaticClass, TEXT("UStageEditorModeTestSimpleToolProperties"), &Z_Registration_Info_UClass_UStageEditorModeTestSimpleToolProperties, CONSTRUCT_RELOAD_VERSION_INFO(FClassReloadVersionInfo, sizeof(UStageEditorModeTestSimpleToolProperties), 390384148U) },
		{ Z_Construct_UClass_UStageEditorModeTestSimpleTool, UStageEditorModeTestSimpleTool::StaticClass, TEXT("UStageEditorModeTestSimpleTool"), &Z_Registration_Info_UClass_UStageEditorModeTestSimpleTool, CONSTRUCT_RELOAD_VERSION_INFO(FClassReloadVersionInfo, sizeof(UStageEditorModeTestSimpleTool), 1705301137U) },
	};
};
static FRegisterCompiledInInfo Z_CompiledInDeferFile_FID_ExtendEditor_Plugins_StageEditorModeTest_Source_StageEditorModeTest_Private_Tools_StageEditorModeTestSimpleTool_h__Script_StageEditorModeTest_727945302(TEXT("/Script/StageEditorModeTest"),
	Z_CompiledInDeferFile_FID_ExtendEditor_Plugins_StageEditorModeTest_Source_StageEditorModeTest_Private_Tools_StageEditorModeTestSimpleTool_h__Script_StageEditorModeTest_Statics::ClassInfo, UE_ARRAY_COUNT(Z_CompiledInDeferFile_FID_ExtendEditor_Plugins_StageEditorModeTest_Source_StageEditorModeTest_Private_Tools_StageEditorModeTestSimpleTool_h__Script_StageEditorModeTest_Statics::ClassInfo),
	nullptr, 0,
	nullptr, 0);
// ********** End Registration *********************************************************************

PRAGMA_ENABLE_DEPRECATION_WARNINGS
