// Copyright Epic Games, Inc. All Rights Reserved.
/*===========================================================================
	Generated code exported from UnrealHeaderTool.
	DO NOT modify this manually! Edit the corresponding .h files instead!
===========================================================================*/

#include "UObject/GeneratedCppIncludes.h"
PRAGMA_DISABLE_DEPRECATION_WARNINGS
void EmptyLinkFunctionForGeneratedCodeStageEditor_init() {}
	static FPackageRegistrationInfo Z_Registration_Info_UPackage__Script_StageEditor;
	FORCENOINLINE UPackage* Z_Construct_UPackage__Script_StageEditor()
	{
		if (!Z_Registration_Info_UPackage__Script_StageEditor.OuterSingleton)
		{
			static const UECodeGen_Private::FPackageParams PackageParams = {
				"/Script/StageEditor",
				nullptr,
				0,
				PKG_CompiledIn | 0x00000040,
				0xE430FBC8,
				0x302855BE,
				METADATA_PARAMS(0, nullptr)
			};
			UECodeGen_Private::ConstructUPackage(Z_Registration_Info_UPackage__Script_StageEditor.OuterSingleton, PackageParams);
		}
		return Z_Registration_Info_UPackage__Script_StageEditor.OuterSingleton;
	}
	static FRegisterCompiledInInfo Z_CompiledInDeferPackage_UPackage__Script_StageEditor(Z_Construct_UPackage__Script_StageEditor, TEXT("/Script/StageEditor"), Z_Registration_Info_UPackage__Script_StageEditor, CONSTRUCT_RELOAD_VERSION_INFO(FPackageReloadVersionInfo, 0xE430FBC8, 0x302855BE));
PRAGMA_ENABLE_DEPRECATION_WARNINGS
