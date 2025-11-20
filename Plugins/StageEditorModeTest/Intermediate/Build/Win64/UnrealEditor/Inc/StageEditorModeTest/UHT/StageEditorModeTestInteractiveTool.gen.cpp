// Copyright Epic Games, Inc. All Rights Reserved.
/*===========================================================================
	Generated code exported from UnrealHeaderTool.
	DO NOT modify this manually! Edit the corresponding .h files instead!
===========================================================================*/

#include "UObject/GeneratedCppIncludes.h"
#include "Tools/StageEditorModeTestInteractiveTool.h"

PRAGMA_DISABLE_DEPRECATION_WARNINGS

void EmptyLinkFunctionForGeneratedCodeStageEditorModeTestInteractiveTool() {}

// ********** Begin Cross Module References ********************************************************
COREUOBJECT_API UScriptStruct* Z_Construct_UScriptStruct_FVector();
INTERACTIVETOOLSFRAMEWORK_API UClass* Z_Construct_UClass_UInteractiveTool();
INTERACTIVETOOLSFRAMEWORK_API UClass* Z_Construct_UClass_UInteractiveToolBuilder();
INTERACTIVETOOLSFRAMEWORK_API UClass* Z_Construct_UClass_UInteractiveToolPropertySet();
STAGEEDITORMODETEST_API UClass* Z_Construct_UClass_UStageEditorModeTestInteractiveTool();
STAGEEDITORMODETEST_API UClass* Z_Construct_UClass_UStageEditorModeTestInteractiveTool_NoRegister();
STAGEEDITORMODETEST_API UClass* Z_Construct_UClass_UStageEditorModeTestInteractiveToolBuilder();
STAGEEDITORMODETEST_API UClass* Z_Construct_UClass_UStageEditorModeTestInteractiveToolBuilder_NoRegister();
STAGEEDITORMODETEST_API UClass* Z_Construct_UClass_UStageEditorModeTestInteractiveToolProperties();
STAGEEDITORMODETEST_API UClass* Z_Construct_UClass_UStageEditorModeTestInteractiveToolProperties_NoRegister();
UPackage* Z_Construct_UPackage__Script_StageEditorModeTest();
// ********** End Cross Module References **********************************************************

// ********** Begin Class UStageEditorModeTestInteractiveToolBuilder *******************************
void UStageEditorModeTestInteractiveToolBuilder::StaticRegisterNativesUStageEditorModeTestInteractiveToolBuilder()
{
}
FClassRegistrationInfo Z_Registration_Info_UClass_UStageEditorModeTestInteractiveToolBuilder;
UClass* UStageEditorModeTestInteractiveToolBuilder::GetPrivateStaticClass()
{
	using TClass = UStageEditorModeTestInteractiveToolBuilder;
	if (!Z_Registration_Info_UClass_UStageEditorModeTestInteractiveToolBuilder.InnerSingleton)
	{
		GetPrivateStaticClassBody(
			StaticPackage(),
			TEXT("StageEditorModeTestInteractiveToolBuilder"),
			Z_Registration_Info_UClass_UStageEditorModeTestInteractiveToolBuilder.InnerSingleton,
			StaticRegisterNativesUStageEditorModeTestInteractiveToolBuilder,
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
	return Z_Registration_Info_UClass_UStageEditorModeTestInteractiveToolBuilder.InnerSingleton;
}
UClass* Z_Construct_UClass_UStageEditorModeTestInteractiveToolBuilder_NoRegister()
{
	return UStageEditorModeTestInteractiveToolBuilder::GetPrivateStaticClass();
}
struct Z_Construct_UClass_UStageEditorModeTestInteractiveToolBuilder_Statics
{
#if WITH_METADATA
	static constexpr UECodeGen_Private::FMetaDataPairParam Class_MetaDataParams[] = {
#if !UE_BUILD_SHIPPING
		{ "Comment", "/**\n * Builder for UStageEditorModeTestInteractiveTool\n */" },
#endif
		{ "IncludePath", "Tools/StageEditorModeTestInteractiveTool.h" },
		{ "ModuleRelativePath", "Private/Tools/StageEditorModeTestInteractiveTool.h" },
#if !UE_BUILD_SHIPPING
		{ "ToolTip", "Builder for UStageEditorModeTestInteractiveTool" },
#endif
	};
#endif // WITH_METADATA
	static UObject* (*const DependentSingletons[])();
	static constexpr FCppClassTypeInfoStatic StaticCppClassTypeInfo = {
		TCppClassTypeTraits<UStageEditorModeTestInteractiveToolBuilder>::IsAbstract,
	};
	static const UECodeGen_Private::FClassParams ClassParams;
};
UObject* (*const Z_Construct_UClass_UStageEditorModeTestInteractiveToolBuilder_Statics::DependentSingletons[])() = {
	(UObject* (*)())Z_Construct_UClass_UInteractiveToolBuilder,
	(UObject* (*)())Z_Construct_UPackage__Script_StageEditorModeTest,
};
static_assert(UE_ARRAY_COUNT(Z_Construct_UClass_UStageEditorModeTestInteractiveToolBuilder_Statics::DependentSingletons) < 16);
const UECodeGen_Private::FClassParams Z_Construct_UClass_UStageEditorModeTestInteractiveToolBuilder_Statics::ClassParams = {
	&UStageEditorModeTestInteractiveToolBuilder::StaticClass,
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
	METADATA_PARAMS(UE_ARRAY_COUNT(Z_Construct_UClass_UStageEditorModeTestInteractiveToolBuilder_Statics::Class_MetaDataParams), Z_Construct_UClass_UStageEditorModeTestInteractiveToolBuilder_Statics::Class_MetaDataParams)
};
UClass* Z_Construct_UClass_UStageEditorModeTestInteractiveToolBuilder()
{
	if (!Z_Registration_Info_UClass_UStageEditorModeTestInteractiveToolBuilder.OuterSingleton)
	{
		UECodeGen_Private::ConstructUClass(Z_Registration_Info_UClass_UStageEditorModeTestInteractiveToolBuilder.OuterSingleton, Z_Construct_UClass_UStageEditorModeTestInteractiveToolBuilder_Statics::ClassParams);
	}
	return Z_Registration_Info_UClass_UStageEditorModeTestInteractiveToolBuilder.OuterSingleton;
}
UStageEditorModeTestInteractiveToolBuilder::UStageEditorModeTestInteractiveToolBuilder(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer) {}
DEFINE_VTABLE_PTR_HELPER_CTOR(UStageEditorModeTestInteractiveToolBuilder);
UStageEditorModeTestInteractiveToolBuilder::~UStageEditorModeTestInteractiveToolBuilder() {}
// ********** End Class UStageEditorModeTestInteractiveToolBuilder *********************************

// ********** Begin Class UStageEditorModeTestInteractiveToolProperties ****************************
void UStageEditorModeTestInteractiveToolProperties::StaticRegisterNativesUStageEditorModeTestInteractiveToolProperties()
{
}
FClassRegistrationInfo Z_Registration_Info_UClass_UStageEditorModeTestInteractiveToolProperties;
UClass* UStageEditorModeTestInteractiveToolProperties::GetPrivateStaticClass()
{
	using TClass = UStageEditorModeTestInteractiveToolProperties;
	if (!Z_Registration_Info_UClass_UStageEditorModeTestInteractiveToolProperties.InnerSingleton)
	{
		GetPrivateStaticClassBody(
			StaticPackage(),
			TEXT("StageEditorModeTestInteractiveToolProperties"),
			Z_Registration_Info_UClass_UStageEditorModeTestInteractiveToolProperties.InnerSingleton,
			StaticRegisterNativesUStageEditorModeTestInteractiveToolProperties,
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
	return Z_Registration_Info_UClass_UStageEditorModeTestInteractiveToolProperties.InnerSingleton;
}
UClass* Z_Construct_UClass_UStageEditorModeTestInteractiveToolProperties_NoRegister()
{
	return UStageEditorModeTestInteractiveToolProperties::GetPrivateStaticClass();
}
struct Z_Construct_UClass_UStageEditorModeTestInteractiveToolProperties_Statics
{
#if WITH_METADATA
	static constexpr UECodeGen_Private::FMetaDataPairParam Class_MetaDataParams[] = {
#if !UE_BUILD_SHIPPING
		{ "Comment", "/**\n * Property set for the UStageEditorModeTestInteractiveTool\n */" },
#endif
		{ "IncludePath", "Tools/StageEditorModeTestInteractiveTool.h" },
		{ "ModuleRelativePath", "Private/Tools/StageEditorModeTestInteractiveTool.h" },
#if !UE_BUILD_SHIPPING
		{ "ToolTip", "Property set for the UStageEditorModeTestInteractiveTool" },
#endif
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_StartPoint_MetaData[] = {
		{ "Category", "Options" },
#if !UE_BUILD_SHIPPING
		{ "Comment", "/** First point of measurement */" },
#endif
		{ "ModuleRelativePath", "Private/Tools/StageEditorModeTestInteractiveTool.h" },
#if !UE_BUILD_SHIPPING
		{ "ToolTip", "First point of measurement" },
#endif
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_EndPoint_MetaData[] = {
		{ "Category", "Options" },
#if !UE_BUILD_SHIPPING
		{ "Comment", "/** Second point of measurement */" },
#endif
		{ "ModuleRelativePath", "Private/Tools/StageEditorModeTestInteractiveTool.h" },
#if !UE_BUILD_SHIPPING
		{ "ToolTip", "Second point of measurement" },
#endif
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_Distance_MetaData[] = {
		{ "Category", "Options" },
#if !UE_BUILD_SHIPPING
		{ "Comment", "/** Current distance measurement */" },
#endif
		{ "ModuleRelativePath", "Private/Tools/StageEditorModeTestInteractiveTool.h" },
#if !UE_BUILD_SHIPPING
		{ "ToolTip", "Current distance measurement" },
#endif
	};
#endif // WITH_METADATA
	static const UECodeGen_Private::FStructPropertyParams NewProp_StartPoint;
	static const UECodeGen_Private::FStructPropertyParams NewProp_EndPoint;
	static const UECodeGen_Private::FDoublePropertyParams NewProp_Distance;
	static const UECodeGen_Private::FPropertyParamsBase* const PropPointers[];
	static UObject* (*const DependentSingletons[])();
	static constexpr FCppClassTypeInfoStatic StaticCppClassTypeInfo = {
		TCppClassTypeTraits<UStageEditorModeTestInteractiveToolProperties>::IsAbstract,
	};
	static const UECodeGen_Private::FClassParams ClassParams;
};
const UECodeGen_Private::FStructPropertyParams Z_Construct_UClass_UStageEditorModeTestInteractiveToolProperties_Statics::NewProp_StartPoint = { "StartPoint", nullptr, (EPropertyFlags)0x0010000000000001, UECodeGen_Private::EPropertyGenFlags::Struct, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(UStageEditorModeTestInteractiveToolProperties, StartPoint), Z_Construct_UScriptStruct_FVector, METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_StartPoint_MetaData), NewProp_StartPoint_MetaData) };
const UECodeGen_Private::FStructPropertyParams Z_Construct_UClass_UStageEditorModeTestInteractiveToolProperties_Statics::NewProp_EndPoint = { "EndPoint", nullptr, (EPropertyFlags)0x0010000000000001, UECodeGen_Private::EPropertyGenFlags::Struct, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(UStageEditorModeTestInteractiveToolProperties, EndPoint), Z_Construct_UScriptStruct_FVector, METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_EndPoint_MetaData), NewProp_EndPoint_MetaData) };
const UECodeGen_Private::FDoublePropertyParams Z_Construct_UClass_UStageEditorModeTestInteractiveToolProperties_Statics::NewProp_Distance = { "Distance", nullptr, (EPropertyFlags)0x0010000000000001, UECodeGen_Private::EPropertyGenFlags::Double, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(UStageEditorModeTestInteractiveToolProperties, Distance), METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_Distance_MetaData), NewProp_Distance_MetaData) };
const UECodeGen_Private::FPropertyParamsBase* const Z_Construct_UClass_UStageEditorModeTestInteractiveToolProperties_Statics::PropPointers[] = {
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_UStageEditorModeTestInteractiveToolProperties_Statics::NewProp_StartPoint,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_UStageEditorModeTestInteractiveToolProperties_Statics::NewProp_EndPoint,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_UStageEditorModeTestInteractiveToolProperties_Statics::NewProp_Distance,
};
static_assert(UE_ARRAY_COUNT(Z_Construct_UClass_UStageEditorModeTestInteractiveToolProperties_Statics::PropPointers) < 2048);
UObject* (*const Z_Construct_UClass_UStageEditorModeTestInteractiveToolProperties_Statics::DependentSingletons[])() = {
	(UObject* (*)())Z_Construct_UClass_UInteractiveToolPropertySet,
	(UObject* (*)())Z_Construct_UPackage__Script_StageEditorModeTest,
};
static_assert(UE_ARRAY_COUNT(Z_Construct_UClass_UStageEditorModeTestInteractiveToolProperties_Statics::DependentSingletons) < 16);
const UECodeGen_Private::FClassParams Z_Construct_UClass_UStageEditorModeTestInteractiveToolProperties_Statics::ClassParams = {
	&UStageEditorModeTestInteractiveToolProperties::StaticClass,
	nullptr,
	&StaticCppClassTypeInfo,
	DependentSingletons,
	nullptr,
	Z_Construct_UClass_UStageEditorModeTestInteractiveToolProperties_Statics::PropPointers,
	nullptr,
	UE_ARRAY_COUNT(DependentSingletons),
	0,
	UE_ARRAY_COUNT(Z_Construct_UClass_UStageEditorModeTestInteractiveToolProperties_Statics::PropPointers),
	0,
	0x001000A8u,
	METADATA_PARAMS(UE_ARRAY_COUNT(Z_Construct_UClass_UStageEditorModeTestInteractiveToolProperties_Statics::Class_MetaDataParams), Z_Construct_UClass_UStageEditorModeTestInteractiveToolProperties_Statics::Class_MetaDataParams)
};
UClass* Z_Construct_UClass_UStageEditorModeTestInteractiveToolProperties()
{
	if (!Z_Registration_Info_UClass_UStageEditorModeTestInteractiveToolProperties.OuterSingleton)
	{
		UECodeGen_Private::ConstructUClass(Z_Registration_Info_UClass_UStageEditorModeTestInteractiveToolProperties.OuterSingleton, Z_Construct_UClass_UStageEditorModeTestInteractiveToolProperties_Statics::ClassParams);
	}
	return Z_Registration_Info_UClass_UStageEditorModeTestInteractiveToolProperties.OuterSingleton;
}
DEFINE_VTABLE_PTR_HELPER_CTOR(UStageEditorModeTestInteractiveToolProperties);
UStageEditorModeTestInteractiveToolProperties::~UStageEditorModeTestInteractiveToolProperties() {}
// ********** End Class UStageEditorModeTestInteractiveToolProperties ******************************

// ********** Begin Class UStageEditorModeTestInteractiveTool **************************************
void UStageEditorModeTestInteractiveTool::StaticRegisterNativesUStageEditorModeTestInteractiveTool()
{
}
FClassRegistrationInfo Z_Registration_Info_UClass_UStageEditorModeTestInteractiveTool;
UClass* UStageEditorModeTestInteractiveTool::GetPrivateStaticClass()
{
	using TClass = UStageEditorModeTestInteractiveTool;
	if (!Z_Registration_Info_UClass_UStageEditorModeTestInteractiveTool.InnerSingleton)
	{
		GetPrivateStaticClassBody(
			StaticPackage(),
			TEXT("StageEditorModeTestInteractiveTool"),
			Z_Registration_Info_UClass_UStageEditorModeTestInteractiveTool.InnerSingleton,
			StaticRegisterNativesUStageEditorModeTestInteractiveTool,
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
	return Z_Registration_Info_UClass_UStageEditorModeTestInteractiveTool.InnerSingleton;
}
UClass* Z_Construct_UClass_UStageEditorModeTestInteractiveTool_NoRegister()
{
	return UStageEditorModeTestInteractiveTool::GetPrivateStaticClass();
}
struct Z_Construct_UClass_UStageEditorModeTestInteractiveTool_Statics
{
#if WITH_METADATA
	static constexpr UECodeGen_Private::FMetaDataPairParam Class_MetaDataParams[] = {
#if !UE_BUILD_SHIPPING
		{ "Comment", "/**\n * UStageEditorModeTestInteractiveTool is an example Tool that allows the user to measure the \n * distance between two points. The first point is set by click-dragging the mouse, and\n * the second point is set by shift-click-dragging the mouse.\n */" },
#endif
		{ "IncludePath", "Tools/StageEditorModeTestInteractiveTool.h" },
		{ "ModuleRelativePath", "Private/Tools/StageEditorModeTestInteractiveTool.h" },
#if !UE_BUILD_SHIPPING
		{ "ToolTip", "UStageEditorModeTestInteractiveTool is an example Tool that allows the user to measure the\ndistance between two points. The first point is set by click-dragging the mouse, and\nthe second point is set by shift-click-dragging the mouse." },
#endif
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_Properties_MetaData[] = {
#if !UE_BUILD_SHIPPING
		{ "Comment", "/** Properties of the tool are stored here */" },
#endif
		{ "ModuleRelativePath", "Private/Tools/StageEditorModeTestInteractiveTool.h" },
#if !UE_BUILD_SHIPPING
		{ "ToolTip", "Properties of the tool are stored here" },
#endif
	};
#endif // WITH_METADATA
	static const UECodeGen_Private::FObjectPropertyParams NewProp_Properties;
	static const UECodeGen_Private::FPropertyParamsBase* const PropPointers[];
	static UObject* (*const DependentSingletons[])();
	static constexpr FCppClassTypeInfoStatic StaticCppClassTypeInfo = {
		TCppClassTypeTraits<UStageEditorModeTestInteractiveTool>::IsAbstract,
	};
	static const UECodeGen_Private::FClassParams ClassParams;
};
const UECodeGen_Private::FObjectPropertyParams Z_Construct_UClass_UStageEditorModeTestInteractiveTool_Statics::NewProp_Properties = { "Properties", nullptr, (EPropertyFlags)0x0124080000000000, UECodeGen_Private::EPropertyGenFlags::Object | UECodeGen_Private::EPropertyGenFlags::ObjectPtr, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(UStageEditorModeTestInteractiveTool, Properties), Z_Construct_UClass_UStageEditorModeTestInteractiveToolProperties_NoRegister, METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_Properties_MetaData), NewProp_Properties_MetaData) };
const UECodeGen_Private::FPropertyParamsBase* const Z_Construct_UClass_UStageEditorModeTestInteractiveTool_Statics::PropPointers[] = {
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_UStageEditorModeTestInteractiveTool_Statics::NewProp_Properties,
};
static_assert(UE_ARRAY_COUNT(Z_Construct_UClass_UStageEditorModeTestInteractiveTool_Statics::PropPointers) < 2048);
UObject* (*const Z_Construct_UClass_UStageEditorModeTestInteractiveTool_Statics::DependentSingletons[])() = {
	(UObject* (*)())Z_Construct_UClass_UInteractiveTool,
	(UObject* (*)())Z_Construct_UPackage__Script_StageEditorModeTest,
};
static_assert(UE_ARRAY_COUNT(Z_Construct_UClass_UStageEditorModeTestInteractiveTool_Statics::DependentSingletons) < 16);
const UECodeGen_Private::FClassParams Z_Construct_UClass_UStageEditorModeTestInteractiveTool_Statics::ClassParams = {
	&UStageEditorModeTestInteractiveTool::StaticClass,
	nullptr,
	&StaticCppClassTypeInfo,
	DependentSingletons,
	nullptr,
	Z_Construct_UClass_UStageEditorModeTestInteractiveTool_Statics::PropPointers,
	nullptr,
	UE_ARRAY_COUNT(DependentSingletons),
	0,
	UE_ARRAY_COUNT(Z_Construct_UClass_UStageEditorModeTestInteractiveTool_Statics::PropPointers),
	0,
	0x001000A8u,
	METADATA_PARAMS(UE_ARRAY_COUNT(Z_Construct_UClass_UStageEditorModeTestInteractiveTool_Statics::Class_MetaDataParams), Z_Construct_UClass_UStageEditorModeTestInteractiveTool_Statics::Class_MetaDataParams)
};
UClass* Z_Construct_UClass_UStageEditorModeTestInteractiveTool()
{
	if (!Z_Registration_Info_UClass_UStageEditorModeTestInteractiveTool.OuterSingleton)
	{
		UECodeGen_Private::ConstructUClass(Z_Registration_Info_UClass_UStageEditorModeTestInteractiveTool.OuterSingleton, Z_Construct_UClass_UStageEditorModeTestInteractiveTool_Statics::ClassParams);
	}
	return Z_Registration_Info_UClass_UStageEditorModeTestInteractiveTool.OuterSingleton;
}
UStageEditorModeTestInteractiveTool::UStageEditorModeTestInteractiveTool() {}
DEFINE_VTABLE_PTR_HELPER_CTOR(UStageEditorModeTestInteractiveTool);
UStageEditorModeTestInteractiveTool::~UStageEditorModeTestInteractiveTool() {}
// ********** End Class UStageEditorModeTestInteractiveTool ****************************************

// ********** Begin Registration *******************************************************************
struct Z_CompiledInDeferFile_FID_ExtendEditor_Plugins_StageEditorModeTest_Source_StageEditorModeTest_Private_Tools_StageEditorModeTestInteractiveTool_h__Script_StageEditorModeTest_Statics
{
	static constexpr FClassRegisterCompiledInInfo ClassInfo[] = {
		{ Z_Construct_UClass_UStageEditorModeTestInteractiveToolBuilder, UStageEditorModeTestInteractiveToolBuilder::StaticClass, TEXT("UStageEditorModeTestInteractiveToolBuilder"), &Z_Registration_Info_UClass_UStageEditorModeTestInteractiveToolBuilder, CONSTRUCT_RELOAD_VERSION_INFO(FClassReloadVersionInfo, sizeof(UStageEditorModeTestInteractiveToolBuilder), 2666576888U) },
		{ Z_Construct_UClass_UStageEditorModeTestInteractiveToolProperties, UStageEditorModeTestInteractiveToolProperties::StaticClass, TEXT("UStageEditorModeTestInteractiveToolProperties"), &Z_Registration_Info_UClass_UStageEditorModeTestInteractiveToolProperties, CONSTRUCT_RELOAD_VERSION_INFO(FClassReloadVersionInfo, sizeof(UStageEditorModeTestInteractiveToolProperties), 1408386281U) },
		{ Z_Construct_UClass_UStageEditorModeTestInteractiveTool, UStageEditorModeTestInteractiveTool::StaticClass, TEXT("UStageEditorModeTestInteractiveTool"), &Z_Registration_Info_UClass_UStageEditorModeTestInteractiveTool, CONSTRUCT_RELOAD_VERSION_INFO(FClassReloadVersionInfo, sizeof(UStageEditorModeTestInteractiveTool), 139771224U) },
	};
};
static FRegisterCompiledInInfo Z_CompiledInDeferFile_FID_ExtendEditor_Plugins_StageEditorModeTest_Source_StageEditorModeTest_Private_Tools_StageEditorModeTestInteractiveTool_h__Script_StageEditorModeTest_2761540060(TEXT("/Script/StageEditorModeTest"),
	Z_CompiledInDeferFile_FID_ExtendEditor_Plugins_StageEditorModeTest_Source_StageEditorModeTest_Private_Tools_StageEditorModeTestInteractiveTool_h__Script_StageEditorModeTest_Statics::ClassInfo, UE_ARRAY_COUNT(Z_CompiledInDeferFile_FID_ExtendEditor_Plugins_StageEditorModeTest_Source_StageEditorModeTest_Private_Tools_StageEditorModeTestInteractiveTool_h__Script_StageEditorModeTest_Statics::ClassInfo),
	nullptr, 0,
	nullptr, 0);
// ********** End Registration *********************************************************************

PRAGMA_ENABLE_DEPRECATION_WARNINGS
