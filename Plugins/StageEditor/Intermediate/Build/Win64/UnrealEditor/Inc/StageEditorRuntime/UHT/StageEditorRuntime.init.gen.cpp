// Copyright Epic Games, Inc. All Rights Reserved.
/*===========================================================================
	Generated code exported from UnrealHeaderTool.
	DO NOT modify this manually! Edit the corresponding .h files instead!
===========================================================================*/

#include "UObject/GeneratedCppIncludes.h"
PRAGMA_DISABLE_DEPRECATION_WARNINGS
void EmptyLinkFunctionForGeneratedCodeStageEditorRuntime_init() {}
	STAGEEDITORRUNTIME_API UFunction* Z_Construct_UDelegateFunction_StageEditorRuntime_OnPropStateChanged__DelegateSignature();
	static FPackageRegistrationInfo Z_Registration_Info_UPackage__Script_StageEditorRuntime;
	FORCENOINLINE UPackage* Z_Construct_UPackage__Script_StageEditorRuntime()
	{
		if (!Z_Registration_Info_UPackage__Script_StageEditorRuntime.OuterSingleton)
		{
			static UObject* (*const SingletonFuncArray[])() = {
				(UObject* (*)())Z_Construct_UDelegateFunction_StageEditorRuntime_OnPropStateChanged__DelegateSignature,
			};
			static const UECodeGen_Private::FPackageParams PackageParams = {
				"/Script/StageEditorRuntime",
				SingletonFuncArray,
				UE_ARRAY_COUNT(SingletonFuncArray),
				PKG_CompiledIn | 0x00000000,
				0x4DA5CEF4,
				0xC27B94C5,
				METADATA_PARAMS(0, nullptr)
			};
			UECodeGen_Private::ConstructUPackage(Z_Registration_Info_UPackage__Script_StageEditorRuntime.OuterSingleton, PackageParams);
		}
		return Z_Registration_Info_UPackage__Script_StageEditorRuntime.OuterSingleton;
	}
	static FRegisterCompiledInInfo Z_CompiledInDeferPackage_UPackage__Script_StageEditorRuntime(Z_Construct_UPackage__Script_StageEditorRuntime, TEXT("/Script/StageEditorRuntime"), Z_Registration_Info_UPackage__Script_StageEditorRuntime, CONSTRUCT_RELOAD_VERSION_INFO(FPackageReloadVersionInfo, 0x4DA5CEF4, 0xC27B94C5));
PRAGMA_ENABLE_DEPRECATION_WARNINGS
