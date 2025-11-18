// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AssetActionUtility.h"

#include "Materials/Material.h"
#include "Materials/MaterialInstanceConstant.h"
#include "Particles/ParticleSystem.h"	
#include "Sound/SoundCue.h"
#include "Sound/SoundWave.h"
#include "Engine/Texture.h"
#include "Blueprint/UserWidget.h"
#include "Components/SkeletalMeshComponent.h"
#include "NiagaraSystem.h"
#include "NiagaraEmitter.h"

#include "QuickAssetAction.generated.h"
/** 批量处理资产复制、命名和清理的编辑器实用工具。 */
UCLASS()
class SUPERMANAGER_API UQuickAssetAction : public UAssetActionUtility
{
	GENERATED_BODY()
public:
/** 为所选资产创建指定数量的副本。 */
UFUNCTION(CallInEditor)
void DuplicateAssets(const int32 NumOfDuplicates);

/** 按资产类型批量添加命名前缀。 */
UFUNCTION(CallInEditor)
void AddPrefixes();

/** 移除未被引用的资产，保持工程整洁。 */
UFUNCTION(CallInEditor)
void RemoveUnusedAsset();

/** 根据前缀映射批量重命名资产。 */
UFUNCTION(CallInEditor)
void BatchRenameAsset();
private:
	TMap<UClass*, FString> PrefixMap = 
	{
		{UBlueprint::StaticClass(), TEXT("BP_")},
		{UBlueprint::StaticClass(),TEXT("BP_")},
		{UStaticMesh::StaticClass(),TEXT("SM_")},
		{UMaterial::StaticClass(), TEXT("M_")},
		{UMaterialInstanceConstant::StaticClass(),TEXT("MI_")},
		{UMaterialFunctionInterface::StaticClass(), TEXT("MF_")},
		{UParticleSystem::StaticClass(), TEXT("PS_")},
		{USoundCue::StaticClass(), TEXT("SC_")},
		{USoundWave::StaticClass(), TEXT("SW_")},
		{UTexture::StaticClass(), TEXT("T_")},
		{UTexture2D::StaticClass(), TEXT("T_")},
		{UUserWidget::StaticClass(), TEXT("WBP_")},
		{USkeletalMeshComponent::StaticClass(), TEXT("SK_")},
		{UNiagaraSystem::StaticClass(), TEXT("NS_")},
		{UNiagaraEmitter::StaticClass(), TEXT("NE_")}
	};

/** 运行重定向器修复以避免残留引用。 */
UFUNCTION()
void FixUpRedirectors();

/** 在指定路径中计算资产的下一个版本号。 */
int32 GetNextAvailableVersionNumber(const FString& PackagePath, const FString& BaseAssetName);
};
