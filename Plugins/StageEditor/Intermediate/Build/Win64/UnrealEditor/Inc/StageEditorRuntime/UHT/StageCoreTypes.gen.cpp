// Copyright Epic Games, Inc. All Rights Reserved.
/*===========================================================================
	Generated code exported from UnrealHeaderTool.
	DO NOT modify this manually! Edit the corresponding .h files instead!
===========================================================================*/

#include "UObject/GeneratedCppIncludes.h"
#include "Core/StageCoreTypes.h"

PRAGMA_DISABLE_DEPRECATION_WARNINGS

void EmptyLinkFunctionForGeneratedCodeStageCoreTypes() {}

// ********** Begin Cross Module References ********************************************************
STAGEEDITORRUNTIME_API UEnum* Z_Construct_UEnum_StageEditorRuntime_EStageRuntimeState();
STAGEEDITORRUNTIME_API UScriptStruct* Z_Construct_UScriptStruct_FAct();
STAGEEDITORRUNTIME_API UScriptStruct* Z_Construct_UScriptStruct_FStageHierarchicalId();
UPackage* Z_Construct_UPackage__Script_StageEditorRuntime();
// ********** End Cross Module References **********************************************************

// ********** Begin Enum EStageRuntimeState ********************************************************
static FEnumRegistrationInfo Z_Registration_Info_UEnum_EStageRuntimeState;
static UEnum* EStageRuntimeState_StaticEnum()
{
	if (!Z_Registration_Info_UEnum_EStageRuntimeState.OuterSingleton)
	{
		Z_Registration_Info_UEnum_EStageRuntimeState.OuterSingleton = GetStaticEnum(Z_Construct_UEnum_StageEditorRuntime_EStageRuntimeState, (UObject*)Z_Construct_UPackage__Script_StageEditorRuntime(), TEXT("EStageRuntimeState"));
	}
	return Z_Registration_Info_UEnum_EStageRuntimeState.OuterSingleton;
}
template<> STAGEEDITORRUNTIME_API UEnum* StaticEnum<EStageRuntimeState>()
{
	return EStageRuntimeState_StaticEnum();
}
struct Z_Construct_UEnum_StageEditorRuntime_EStageRuntimeState_Statics
{
#if WITH_METADATA
	static constexpr UECodeGen_Private::FMetaDataPairParam Enum_MetaDataParams[] = {
		{ "Active.Comment", "/** The stage is fully active and running logic. */" },
		{ "Active.DisplayName", "Active" },
		{ "Active.Name", "EStageRuntimeState::Active" },
		{ "Active.ToolTip", "The stage is fully active and running logic." },
		{ "BlueprintType", "true" },
#if !UE_BUILD_SHIPPING
		{ "Comment", "/**\n * @brief Represents the runtime state of a Stage.\n */" },
#endif
		{ "Loading.Comment", "/** The stage is loading its resources. */" },
		{ "Loading.DisplayName", "Loading" },
		{ "Loading.Name", "EStageRuntimeState::Loading" },
		{ "Loading.ToolTip", "The stage is loading its resources." },
		{ "ModuleRelativePath", "Public/Core/StageCoreTypes.h" },
#if !UE_BUILD_SHIPPING
		{ "ToolTip", "@brief Represents the runtime state of a Stage." },
#endif
		{ "Unloaded.Comment", "/** The stage is unloaded or inactive. */" },
		{ "Unloaded.DisplayName", "Unloaded" },
		{ "Unloaded.Name", "EStageRuntimeState::Unloaded" },
		{ "Unloaded.ToolTip", "The stage is unloaded or inactive." },
	};
#endif // WITH_METADATA
	static constexpr UECodeGen_Private::FEnumeratorParam Enumerators[] = {
		{ "EStageRuntimeState::Unloaded", (int64)EStageRuntimeState::Unloaded },
		{ "EStageRuntimeState::Loading", (int64)EStageRuntimeState::Loading },
		{ "EStageRuntimeState::Active", (int64)EStageRuntimeState::Active },
	};
	static const UECodeGen_Private::FEnumParams EnumParams;
};
const UECodeGen_Private::FEnumParams Z_Construct_UEnum_StageEditorRuntime_EStageRuntimeState_Statics::EnumParams = {
	(UObject*(*)())Z_Construct_UPackage__Script_StageEditorRuntime,
	nullptr,
	"EStageRuntimeState",
	"EStageRuntimeState",
	Z_Construct_UEnum_StageEditorRuntime_EStageRuntimeState_Statics::Enumerators,
	RF_Public|RF_Transient|RF_MarkAsNative,
	UE_ARRAY_COUNT(Z_Construct_UEnum_StageEditorRuntime_EStageRuntimeState_Statics::Enumerators),
	EEnumFlags::None,
	(uint8)UEnum::ECppForm::EnumClass,
	METADATA_PARAMS(UE_ARRAY_COUNT(Z_Construct_UEnum_StageEditorRuntime_EStageRuntimeState_Statics::Enum_MetaDataParams), Z_Construct_UEnum_StageEditorRuntime_EStageRuntimeState_Statics::Enum_MetaDataParams)
};
UEnum* Z_Construct_UEnum_StageEditorRuntime_EStageRuntimeState()
{
	if (!Z_Registration_Info_UEnum_EStageRuntimeState.InnerSingleton)
	{
		UECodeGen_Private::ConstructUEnum(Z_Registration_Info_UEnum_EStageRuntimeState.InnerSingleton, Z_Construct_UEnum_StageEditorRuntime_EStageRuntimeState_Statics::EnumParams);
	}
	return Z_Registration_Info_UEnum_EStageRuntimeState.InnerSingleton;
}
// ********** End Enum EStageRuntimeState **********************************************************

// ********** Begin ScriptStruct FStageHierarchicalId **********************************************
static FStructRegistrationInfo Z_Registration_Info_UScriptStruct_FStageHierarchicalId;
class UScriptStruct* FStageHierarchicalId::StaticStruct()
{
	if (!Z_Registration_Info_UScriptStruct_FStageHierarchicalId.OuterSingleton)
	{
		Z_Registration_Info_UScriptStruct_FStageHierarchicalId.OuterSingleton = GetStaticStruct(Z_Construct_UScriptStruct_FStageHierarchicalId, (UObject*)Z_Construct_UPackage__Script_StageEditorRuntime(), TEXT("StageHierarchicalId"));
	}
	return Z_Registration_Info_UScriptStruct_FStageHierarchicalId.OuterSingleton;
}
struct Z_Construct_UScriptStruct_FStageHierarchicalId_Statics
{
#if WITH_METADATA
	static constexpr UECodeGen_Private::FMetaDataPairParam Struct_MetaDataParams[] = {
		{ "BlueprintType", "true" },
#if !UE_BUILD_SHIPPING
		{ "Comment", "/**\n * @brief A hierarchical ID structure to uniquely identify a Prop within a Stage and Act context.\n * Structure: StageID -> ActID -> PropID\n */" },
#endif
		{ "ModuleRelativePath", "Public/Core/StageCoreTypes.h" },
#if !UE_BUILD_SHIPPING
		{ "ToolTip", "@brief A hierarchical ID structure to uniquely identify a Prop within a Stage and Act context.\nStructure: StageID -> ActID -> PropID" },
#endif
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_StageID_MetaData[] = {
		{ "Category", "Stage ID" },
#if !UE_BUILD_SHIPPING
		{ "Comment", "/** The globally unique ID of the Stage (assigned by central server). */" },
#endif
		{ "ModuleRelativePath", "Public/Core/StageCoreTypes.h" },
#if !UE_BUILD_SHIPPING
		{ "ToolTip", "The globally unique ID of the Stage (assigned by central server)." },
#endif
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_ActID_MetaData[] = {
		{ "Category", "Stage ID" },
#if !UE_BUILD_SHIPPING
		{ "Comment", "/** The local ID of the Act within the Stage. */" },
#endif
		{ "ModuleRelativePath", "Public/Core/StageCoreTypes.h" },
#if !UE_BUILD_SHIPPING
		{ "ToolTip", "The local ID of the Act within the Stage." },
#endif
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_PropID_MetaData[] = {
		{ "Category", "Stage ID" },
#if !UE_BUILD_SHIPPING
		{ "Comment", "/** The local ID of the Prop within the Stage. */" },
#endif
		{ "ModuleRelativePath", "Public/Core/StageCoreTypes.h" },
#if !UE_BUILD_SHIPPING
		{ "ToolTip", "The local ID of the Prop within the Stage." },
#endif
	};
#endif // WITH_METADATA
	static const UECodeGen_Private::FIntPropertyParams NewProp_StageID;
	static const UECodeGen_Private::FIntPropertyParams NewProp_ActID;
	static const UECodeGen_Private::FIntPropertyParams NewProp_PropID;
	static const UECodeGen_Private::FPropertyParamsBase* const PropPointers[];
	static void* NewStructOps()
	{
		return (UScriptStruct::ICppStructOps*)new UScriptStruct::TCppStructOps<FStageHierarchicalId>();
	}
	static const UECodeGen_Private::FStructParams StructParams;
};
const UECodeGen_Private::FIntPropertyParams Z_Construct_UScriptStruct_FStageHierarchicalId_Statics::NewProp_StageID = { "StageID", nullptr, (EPropertyFlags)0x0010000000000005, UECodeGen_Private::EPropertyGenFlags::Int, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(FStageHierarchicalId, StageID), METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_StageID_MetaData), NewProp_StageID_MetaData) };
const UECodeGen_Private::FIntPropertyParams Z_Construct_UScriptStruct_FStageHierarchicalId_Statics::NewProp_ActID = { "ActID", nullptr, (EPropertyFlags)0x0010000000000005, UECodeGen_Private::EPropertyGenFlags::Int, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(FStageHierarchicalId, ActID), METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_ActID_MetaData), NewProp_ActID_MetaData) };
const UECodeGen_Private::FIntPropertyParams Z_Construct_UScriptStruct_FStageHierarchicalId_Statics::NewProp_PropID = { "PropID", nullptr, (EPropertyFlags)0x0010000000000005, UECodeGen_Private::EPropertyGenFlags::Int, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(FStageHierarchicalId, PropID), METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_PropID_MetaData), NewProp_PropID_MetaData) };
const UECodeGen_Private::FPropertyParamsBase* const Z_Construct_UScriptStruct_FStageHierarchicalId_Statics::PropPointers[] = {
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UScriptStruct_FStageHierarchicalId_Statics::NewProp_StageID,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UScriptStruct_FStageHierarchicalId_Statics::NewProp_ActID,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UScriptStruct_FStageHierarchicalId_Statics::NewProp_PropID,
};
static_assert(UE_ARRAY_COUNT(Z_Construct_UScriptStruct_FStageHierarchicalId_Statics::PropPointers) < 2048);
const UECodeGen_Private::FStructParams Z_Construct_UScriptStruct_FStageHierarchicalId_Statics::StructParams = {
	(UObject* (*)())Z_Construct_UPackage__Script_StageEditorRuntime,
	nullptr,
	&NewStructOps,
	"StageHierarchicalId",
	Z_Construct_UScriptStruct_FStageHierarchicalId_Statics::PropPointers,
	UE_ARRAY_COUNT(Z_Construct_UScriptStruct_FStageHierarchicalId_Statics::PropPointers),
	sizeof(FStageHierarchicalId),
	alignof(FStageHierarchicalId),
	RF_Public|RF_Transient|RF_MarkAsNative,
	EStructFlags(0x00000201),
	METADATA_PARAMS(UE_ARRAY_COUNT(Z_Construct_UScriptStruct_FStageHierarchicalId_Statics::Struct_MetaDataParams), Z_Construct_UScriptStruct_FStageHierarchicalId_Statics::Struct_MetaDataParams)
};
UScriptStruct* Z_Construct_UScriptStruct_FStageHierarchicalId()
{
	if (!Z_Registration_Info_UScriptStruct_FStageHierarchicalId.InnerSingleton)
	{
		UECodeGen_Private::ConstructUScriptStruct(Z_Registration_Info_UScriptStruct_FStageHierarchicalId.InnerSingleton, Z_Construct_UScriptStruct_FStageHierarchicalId_Statics::StructParams);
	}
	return Z_Registration_Info_UScriptStruct_FStageHierarchicalId.InnerSingleton;
}
// ********** End ScriptStruct FStageHierarchicalId ************************************************

// ********** Begin ScriptStruct FAct **************************************************************
static FStructRegistrationInfo Z_Registration_Info_UScriptStruct_FAct;
class UScriptStruct* FAct::StaticStruct()
{
	if (!Z_Registration_Info_UScriptStruct_FAct.OuterSingleton)
	{
		Z_Registration_Info_UScriptStruct_FAct.OuterSingleton = GetStaticStruct(Z_Construct_UScriptStruct_FAct, (UObject*)Z_Construct_UPackage__Script_StageEditorRuntime(), TEXT("Act"));
	}
	return Z_Registration_Info_UScriptStruct_FAct.OuterSingleton;
}
struct Z_Construct_UScriptStruct_FAct_Statics
{
#if WITH_METADATA
	static constexpr UECodeGen_Private::FMetaDataPairParam Struct_MetaDataParams[] = {
		{ "BlueprintType", "true" },
#if !UE_BUILD_SHIPPING
		{ "Comment", "/**\n * @brief Defines a \"Scene\" or \"State\" of the Stage.\n * Contains the target state for a set of Props.\n */" },
#endif
		{ "ModuleRelativePath", "Public/Core/StageCoreTypes.h" },
#if !UE_BUILD_SHIPPING
		{ "ToolTip", "@brief Defines a \"Scene\" or \"State\" of the Stage.\nContains the target state for a set of Props." },
#endif
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_ActID_MetaData[] = {
		{ "Category", "Act" },
#if !UE_BUILD_SHIPPING
		{ "Comment", "/** The unique ID of this Act. */" },
#endif
		{ "ModuleRelativePath", "Public/Core/StageCoreTypes.h" },
#if !UE_BUILD_SHIPPING
		{ "ToolTip", "The unique ID of this Act." },
#endif
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_DisplayName_MetaData[] = {
		{ "Category", "Act" },
#if !UE_BUILD_SHIPPING
		{ "Comment", "/** Display name for the Editor. */" },
#endif
		{ "ModuleRelativePath", "Public/Core/StageCoreTypes.h" },
#if !UE_BUILD_SHIPPING
		{ "ToolTip", "Display name for the Editor." },
#endif
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_PropStateOverrides_MetaData[] = {
		{ "Category", "Act" },
#if !UE_BUILD_SHIPPING
		{ "Comment", "/** \n\x09 * Map of PropID -> Target PropState Value.\n\x09 * Defines what state each Prop should be in when this Act is active.\n\x09 */" },
#endif
		{ "ModuleRelativePath", "Public/Core/StageCoreTypes.h" },
#if !UE_BUILD_SHIPPING
		{ "ToolTip", "Map of PropID -> Target PropState Value.\nDefines what state each Prop should be in when this Act is active." },
#endif
	};
#endif // WITH_METADATA
	static const UECodeGen_Private::FStructPropertyParams NewProp_ActID;
	static const UECodeGen_Private::FStrPropertyParams NewProp_DisplayName;
	static const UECodeGen_Private::FIntPropertyParams NewProp_PropStateOverrides_ValueProp;
	static const UECodeGen_Private::FIntPropertyParams NewProp_PropStateOverrides_Key_KeyProp;
	static const UECodeGen_Private::FMapPropertyParams NewProp_PropStateOverrides;
	static const UECodeGen_Private::FPropertyParamsBase* const PropPointers[];
	static void* NewStructOps()
	{
		return (UScriptStruct::ICppStructOps*)new UScriptStruct::TCppStructOps<FAct>();
	}
	static const UECodeGen_Private::FStructParams StructParams;
};
const UECodeGen_Private::FStructPropertyParams Z_Construct_UScriptStruct_FAct_Statics::NewProp_ActID = { "ActID", nullptr, (EPropertyFlags)0x0010000000000015, UECodeGen_Private::EPropertyGenFlags::Struct, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(FAct, ActID), Z_Construct_UScriptStruct_FStageHierarchicalId, METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_ActID_MetaData), NewProp_ActID_MetaData) }; // 4040836395
const UECodeGen_Private::FStrPropertyParams Z_Construct_UScriptStruct_FAct_Statics::NewProp_DisplayName = { "DisplayName", nullptr, (EPropertyFlags)0x0010000000000005, UECodeGen_Private::EPropertyGenFlags::Str, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(FAct, DisplayName), METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_DisplayName_MetaData), NewProp_DisplayName_MetaData) };
const UECodeGen_Private::FIntPropertyParams Z_Construct_UScriptStruct_FAct_Statics::NewProp_PropStateOverrides_ValueProp = { "PropStateOverrides", nullptr, (EPropertyFlags)0x0000000000000001, UECodeGen_Private::EPropertyGenFlags::Int, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, 1, METADATA_PARAMS(0, nullptr) };
const UECodeGen_Private::FIntPropertyParams Z_Construct_UScriptStruct_FAct_Statics::NewProp_PropStateOverrides_Key_KeyProp = { "PropStateOverrides_Key", nullptr, (EPropertyFlags)0x0000000000000001, UECodeGen_Private::EPropertyGenFlags::Int, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, 0, METADATA_PARAMS(0, nullptr) };
const UECodeGen_Private::FMapPropertyParams Z_Construct_UScriptStruct_FAct_Statics::NewProp_PropStateOverrides = { "PropStateOverrides", nullptr, (EPropertyFlags)0x0010000000000005, UECodeGen_Private::EPropertyGenFlags::Map, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(FAct, PropStateOverrides), EMapPropertyFlags::None, METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_PropStateOverrides_MetaData), NewProp_PropStateOverrides_MetaData) };
const UECodeGen_Private::FPropertyParamsBase* const Z_Construct_UScriptStruct_FAct_Statics::PropPointers[] = {
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UScriptStruct_FAct_Statics::NewProp_ActID,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UScriptStruct_FAct_Statics::NewProp_DisplayName,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UScriptStruct_FAct_Statics::NewProp_PropStateOverrides_ValueProp,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UScriptStruct_FAct_Statics::NewProp_PropStateOverrides_Key_KeyProp,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UScriptStruct_FAct_Statics::NewProp_PropStateOverrides,
};
static_assert(UE_ARRAY_COUNT(Z_Construct_UScriptStruct_FAct_Statics::PropPointers) < 2048);
const UECodeGen_Private::FStructParams Z_Construct_UScriptStruct_FAct_Statics::StructParams = {
	(UObject* (*)())Z_Construct_UPackage__Script_StageEditorRuntime,
	nullptr,
	&NewStructOps,
	"Act",
	Z_Construct_UScriptStruct_FAct_Statics::PropPointers,
	UE_ARRAY_COUNT(Z_Construct_UScriptStruct_FAct_Statics::PropPointers),
	sizeof(FAct),
	alignof(FAct),
	RF_Public|RF_Transient|RF_MarkAsNative,
	EStructFlags(0x00000201),
	METADATA_PARAMS(UE_ARRAY_COUNT(Z_Construct_UScriptStruct_FAct_Statics::Struct_MetaDataParams), Z_Construct_UScriptStruct_FAct_Statics::Struct_MetaDataParams)
};
UScriptStruct* Z_Construct_UScriptStruct_FAct()
{
	if (!Z_Registration_Info_UScriptStruct_FAct.InnerSingleton)
	{
		UECodeGen_Private::ConstructUScriptStruct(Z_Registration_Info_UScriptStruct_FAct.InnerSingleton, Z_Construct_UScriptStruct_FAct_Statics::StructParams);
	}
	return Z_Registration_Info_UScriptStruct_FAct.InnerSingleton;
}
// ********** End ScriptStruct FAct ****************************************************************

// ********** Begin Registration *******************************************************************
struct Z_CompiledInDeferFile_FID_ExtendEditor_Plugins_StageEditor_Source_StageEditorRuntime_Public_Core_StageCoreTypes_h__Script_StageEditorRuntime_Statics
{
	static constexpr FEnumRegisterCompiledInInfo EnumInfo[] = {
		{ EStageRuntimeState_StaticEnum, TEXT("EStageRuntimeState"), &Z_Registration_Info_UEnum_EStageRuntimeState, CONSTRUCT_RELOAD_VERSION_INFO(FEnumReloadVersionInfo, 2875093427U) },
	};
	static constexpr FStructRegisterCompiledInInfo ScriptStructInfo[] = {
		{ FStageHierarchicalId::StaticStruct, Z_Construct_UScriptStruct_FStageHierarchicalId_Statics::NewStructOps, TEXT("StageHierarchicalId"), &Z_Registration_Info_UScriptStruct_FStageHierarchicalId, CONSTRUCT_RELOAD_VERSION_INFO(FStructReloadVersionInfo, sizeof(FStageHierarchicalId), 4040836395U) },
		{ FAct::StaticStruct, Z_Construct_UScriptStruct_FAct_Statics::NewStructOps, TEXT("Act"), &Z_Registration_Info_UScriptStruct_FAct, CONSTRUCT_RELOAD_VERSION_INFO(FStructReloadVersionInfo, sizeof(FAct), 697850678U) },
	};
};
static FRegisterCompiledInInfo Z_CompiledInDeferFile_FID_ExtendEditor_Plugins_StageEditor_Source_StageEditorRuntime_Public_Core_StageCoreTypes_h__Script_StageEditorRuntime_3319531455(TEXT("/Script/StageEditorRuntime"),
	nullptr, 0,
	Z_CompiledInDeferFile_FID_ExtendEditor_Plugins_StageEditor_Source_StageEditorRuntime_Public_Core_StageCoreTypes_h__Script_StageEditorRuntime_Statics::ScriptStructInfo, UE_ARRAY_COUNT(Z_CompiledInDeferFile_FID_ExtendEditor_Plugins_StageEditor_Source_StageEditorRuntime_Public_Core_StageCoreTypes_h__Script_StageEditorRuntime_Statics::ScriptStructInfo),
	Z_CompiledInDeferFile_FID_ExtendEditor_Plugins_StageEditor_Source_StageEditorRuntime_Public_Core_StageCoreTypes_h__Script_StageEditorRuntime_Statics::EnumInfo, UE_ARRAY_COUNT(Z_CompiledInDeferFile_FID_ExtendEditor_Plugins_StageEditor_Source_StageEditorRuntime_Public_Core_StageCoreTypes_h__Script_StageEditorRuntime_Statics::EnumInfo));
// ********** End Registration *********************************************************************

PRAGMA_ENABLE_DEPRECATION_WARNINGS
