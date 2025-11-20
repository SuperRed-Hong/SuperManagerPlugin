// Copyright Epic Games, Inc. All Rights Reserved.
/*===========================================================================
	Generated code exported from UnrealHeaderTool.
	DO NOT modify this manually! Edit the corresponding .h files instead!
===========================================================================*/

#include "UObject/GeneratedCppIncludes.h"
#include "EditorUI/StageEditorPanel.h"

PRAGMA_DISABLE_DEPRECATION_WARNINGS

void EmptyLinkFunctionForGeneratedCodeStageEditorPanel() {}

// ********** Begin Cross Module References ********************************************************
COREUOBJECT_API UScriptStruct* Z_Construct_UScriptStruct_FDirectoryPath();
STAGEEDITOR_API UScriptStruct* Z_Construct_UScriptStruct_FAssetCreationSettings();
UPackage* Z_Construct_UPackage__Script_StageEditor();
// ********** End Cross Module References **********************************************************

// ********** Begin ScriptStruct FAssetCreationSettings ********************************************
static FStructRegistrationInfo Z_Registration_Info_UScriptStruct_FAssetCreationSettings;
class UScriptStruct* FAssetCreationSettings::StaticStruct()
{
	if (!Z_Registration_Info_UScriptStruct_FAssetCreationSettings.OuterSingleton)
	{
		Z_Registration_Info_UScriptStruct_FAssetCreationSettings.OuterSingleton = GetStaticStruct(Z_Construct_UScriptStruct_FAssetCreationSettings, (UObject*)Z_Construct_UPackage__Script_StageEditor(), TEXT("AssetCreationSettings"));
	}
	return Z_Registration_Info_UScriptStruct_FAssetCreationSettings.OuterSingleton;
}
struct Z_Construct_UScriptStruct_FAssetCreationSettings_Statics
{
#if WITH_METADATA
	static constexpr UECodeGen_Private::FMetaDataPairParam Struct_MetaDataParams[] = {
#if !UE_BUILD_SHIPPING
		{ "Comment", "/**\n * Settings for asset creation paths\n */" },
#endif
		{ "ModuleRelativePath", "Public/EditorUI/StageEditorPanel.h" },
#if !UE_BUILD_SHIPPING
		{ "ToolTip", "Settings for asset creation paths" },
#endif
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_bIsCustomStageAssetFolderPath_MetaData[] = {
		{ "Category", "Asset Creation" },
#if !UE_BUILD_SHIPPING
		{ "Comment", "/** If true, use custom path for Stage Blueprints. Otherwise use default plugin path. */" },
#endif
		{ "ModuleRelativePath", "Public/EditorUI/StageEditorPanel.h" },
#if !UE_BUILD_SHIPPING
		{ "ToolTip", "If true, use custom path for Stage Blueprints. Otherwise use default plugin path." },
#endif
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_StageAssetFolderPath_MetaData[] = {
		{ "Category", "Asset Creation" },
#if !UE_BUILD_SHIPPING
		{ "Comment", "/** Custom folder path for Stage Blueprint creation */" },
#endif
		{ "ContentDir", "" },
		{ "EditCondition", "bIsCustomStageAssetFolderPath" },
		{ "ModuleRelativePath", "Public/EditorUI/StageEditorPanel.h" },
		{ "RelativeToGameContentDir", "" },
#if !UE_BUILD_SHIPPING
		{ "ToolTip", "Custom folder path for Stage Blueprint creation" },
#endif
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_bIsCustomPropAssetPath_MetaData[] = {
		{ "Category", "Asset Creation" },
#if !UE_BUILD_SHIPPING
		{ "Comment", "/** If true, use custom path for Prop Blueprints. Otherwise use default plugin path. */" },
#endif
		{ "ModuleRelativePath", "Public/EditorUI/StageEditorPanel.h" },
#if !UE_BUILD_SHIPPING
		{ "ToolTip", "If true, use custom path for Prop Blueprints. Otherwise use default plugin path." },
#endif
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_PropAssetFolderPath_MetaData[] = {
		{ "Category", "Asset Creation" },
#if !UE_BUILD_SHIPPING
		{ "Comment", "/** Custom folder path for Prop Blueprint creation */" },
#endif
		{ "ContentDir", "" },
		{ "EditCondition", "bIsCustomPropAssetPath" },
		{ "ModuleRelativePath", "Public/EditorUI/StageEditorPanel.h" },
		{ "RelativeToGameContentDir", "" },
#if !UE_BUILD_SHIPPING
		{ "ToolTip", "Custom folder path for Prop Blueprint creation" },
#endif
	};
#endif // WITH_METADATA
	static void NewProp_bIsCustomStageAssetFolderPath_SetBit(void* Obj);
	static const UECodeGen_Private::FBoolPropertyParams NewProp_bIsCustomStageAssetFolderPath;
	static const UECodeGen_Private::FStructPropertyParams NewProp_StageAssetFolderPath;
	static void NewProp_bIsCustomPropAssetPath_SetBit(void* Obj);
	static const UECodeGen_Private::FBoolPropertyParams NewProp_bIsCustomPropAssetPath;
	static const UECodeGen_Private::FStructPropertyParams NewProp_PropAssetFolderPath;
	static const UECodeGen_Private::FPropertyParamsBase* const PropPointers[];
	static void* NewStructOps()
	{
		return (UScriptStruct::ICppStructOps*)new UScriptStruct::TCppStructOps<FAssetCreationSettings>();
	}
	static const UECodeGen_Private::FStructParams StructParams;
};
void Z_Construct_UScriptStruct_FAssetCreationSettings_Statics::NewProp_bIsCustomStageAssetFolderPath_SetBit(void* Obj)
{
	((FAssetCreationSettings*)Obj)->bIsCustomStageAssetFolderPath = 1;
}
const UECodeGen_Private::FBoolPropertyParams Z_Construct_UScriptStruct_FAssetCreationSettings_Statics::NewProp_bIsCustomStageAssetFolderPath = { "bIsCustomStageAssetFolderPath", nullptr, (EPropertyFlags)0x0010000000000001, UECodeGen_Private::EPropertyGenFlags::Bool | UECodeGen_Private::EPropertyGenFlags::NativeBool, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, sizeof(bool), sizeof(FAssetCreationSettings), &Z_Construct_UScriptStruct_FAssetCreationSettings_Statics::NewProp_bIsCustomStageAssetFolderPath_SetBit, METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_bIsCustomStageAssetFolderPath_MetaData), NewProp_bIsCustomStageAssetFolderPath_MetaData) };
const UECodeGen_Private::FStructPropertyParams Z_Construct_UScriptStruct_FAssetCreationSettings_Statics::NewProp_StageAssetFolderPath = { "StageAssetFolderPath", nullptr, (EPropertyFlags)0x0010000000000001, UECodeGen_Private::EPropertyGenFlags::Struct, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(FAssetCreationSettings, StageAssetFolderPath), Z_Construct_UScriptStruct_FDirectoryPath, METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_StageAssetFolderPath_MetaData), NewProp_StageAssetFolderPath_MetaData) };
void Z_Construct_UScriptStruct_FAssetCreationSettings_Statics::NewProp_bIsCustomPropAssetPath_SetBit(void* Obj)
{
	((FAssetCreationSettings*)Obj)->bIsCustomPropAssetPath = 1;
}
const UECodeGen_Private::FBoolPropertyParams Z_Construct_UScriptStruct_FAssetCreationSettings_Statics::NewProp_bIsCustomPropAssetPath = { "bIsCustomPropAssetPath", nullptr, (EPropertyFlags)0x0010000000000001, UECodeGen_Private::EPropertyGenFlags::Bool | UECodeGen_Private::EPropertyGenFlags::NativeBool, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, sizeof(bool), sizeof(FAssetCreationSettings), &Z_Construct_UScriptStruct_FAssetCreationSettings_Statics::NewProp_bIsCustomPropAssetPath_SetBit, METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_bIsCustomPropAssetPath_MetaData), NewProp_bIsCustomPropAssetPath_MetaData) };
const UECodeGen_Private::FStructPropertyParams Z_Construct_UScriptStruct_FAssetCreationSettings_Statics::NewProp_PropAssetFolderPath = { "PropAssetFolderPath", nullptr, (EPropertyFlags)0x0010000000000001, UECodeGen_Private::EPropertyGenFlags::Struct, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(FAssetCreationSettings, PropAssetFolderPath), Z_Construct_UScriptStruct_FDirectoryPath, METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_PropAssetFolderPath_MetaData), NewProp_PropAssetFolderPath_MetaData) };
const UECodeGen_Private::FPropertyParamsBase* const Z_Construct_UScriptStruct_FAssetCreationSettings_Statics::PropPointers[] = {
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UScriptStruct_FAssetCreationSettings_Statics::NewProp_bIsCustomStageAssetFolderPath,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UScriptStruct_FAssetCreationSettings_Statics::NewProp_StageAssetFolderPath,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UScriptStruct_FAssetCreationSettings_Statics::NewProp_bIsCustomPropAssetPath,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UScriptStruct_FAssetCreationSettings_Statics::NewProp_PropAssetFolderPath,
};
static_assert(UE_ARRAY_COUNT(Z_Construct_UScriptStruct_FAssetCreationSettings_Statics::PropPointers) < 2048);
const UECodeGen_Private::FStructParams Z_Construct_UScriptStruct_FAssetCreationSettings_Statics::StructParams = {
	(UObject* (*)())Z_Construct_UPackage__Script_StageEditor,
	nullptr,
	&NewStructOps,
	"AssetCreationSettings",
	Z_Construct_UScriptStruct_FAssetCreationSettings_Statics::PropPointers,
	UE_ARRAY_COUNT(Z_Construct_UScriptStruct_FAssetCreationSettings_Statics::PropPointers),
	sizeof(FAssetCreationSettings),
	alignof(FAssetCreationSettings),
	RF_Public|RF_Transient|RF_MarkAsNative,
	EStructFlags(0x00000001),
	METADATA_PARAMS(UE_ARRAY_COUNT(Z_Construct_UScriptStruct_FAssetCreationSettings_Statics::Struct_MetaDataParams), Z_Construct_UScriptStruct_FAssetCreationSettings_Statics::Struct_MetaDataParams)
};
UScriptStruct* Z_Construct_UScriptStruct_FAssetCreationSettings()
{
	if (!Z_Registration_Info_UScriptStruct_FAssetCreationSettings.InnerSingleton)
	{
		UECodeGen_Private::ConstructUScriptStruct(Z_Registration_Info_UScriptStruct_FAssetCreationSettings.InnerSingleton, Z_Construct_UScriptStruct_FAssetCreationSettings_Statics::StructParams);
	}
	return Z_Registration_Info_UScriptStruct_FAssetCreationSettings.InnerSingleton;
}
// ********** End ScriptStruct FAssetCreationSettings **********************************************

// ********** Begin Registration *******************************************************************
struct Z_CompiledInDeferFile_FID_ExtendEditor_Plugins_StageEditor_Source_StageEditor_Public_EditorUI_StageEditorPanel_h__Script_StageEditor_Statics
{
	static constexpr FStructRegisterCompiledInInfo ScriptStructInfo[] = {
		{ FAssetCreationSettings::StaticStruct, Z_Construct_UScriptStruct_FAssetCreationSettings_Statics::NewStructOps, TEXT("AssetCreationSettings"), &Z_Registration_Info_UScriptStruct_FAssetCreationSettings, CONSTRUCT_RELOAD_VERSION_INFO(FStructReloadVersionInfo, sizeof(FAssetCreationSettings), 2411241888U) },
	};
};
static FRegisterCompiledInInfo Z_CompiledInDeferFile_FID_ExtendEditor_Plugins_StageEditor_Source_StageEditor_Public_EditorUI_StageEditorPanel_h__Script_StageEditor_921015138(TEXT("/Script/StageEditor"),
	nullptr, 0,
	Z_CompiledInDeferFile_FID_ExtendEditor_Plugins_StageEditor_Source_StageEditor_Public_EditorUI_StageEditorPanel_h__Script_StageEditor_Statics::ScriptStructInfo, UE_ARRAY_COUNT(Z_CompiledInDeferFile_FID_ExtendEditor_Plugins_StageEditor_Source_StageEditor_Public_EditorUI_StageEditorPanel_h__Script_StageEditor_Statics::ScriptStructInfo),
	nullptr, 0);
// ********** End Registration *********************************************************************

PRAGMA_ENABLE_DEPRECATION_WARNINGS
