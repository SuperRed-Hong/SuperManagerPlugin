// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EditorUtilityWidget.h"
#include "Materials/MaterialExpressionTextureSample.h"

#include "QuickMaterialCreationWidget.generated.h"

/** 描述材质创建时的贴图打包方式。 */
UENUM(BlueprintType)
enum class E_ChannelPackingType : uint8
{
	ECPT_NoChannelPacking UMETA(DisplayName = "No Channel Packing"),
	ECPT_ORM UMETA(DisplayName = "ORM Channel Packing"),
	ECPT_MAX UMETA(DisplayName = "Default Max Channel Packing"),
};

/** 快速将贴图转换为材质/材质实例的编辑器实用面板。 */
UCLASS()
class SUPERMANAGER_API UQuickMaterialCreationWidget : public UEditorUtilityWidget
{
	GENERATED_BODY()
public:
#pragma region QuickMaterialCreationData

	/** 基于当前选中的贴图创建材质。 */
	UFUNCTION(BlueprintCallable, Category = "QuickMaterialCreationCore")
	void CreateMaterialFromSelectedTextures();

	/** 贴图打包类型。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CreateMaterialFromSelectedTextures")
	E_ChannelPackingType ChannelPackingType = E_ChannelPackingType::ECPT_NoChannelPacking;
	/** 是否自定义材质名称。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CreateMaterialFromSelectedTextures")
	bool bCustomMaterialName = false;
	/** 自定义材质名称前缀。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CreateMaterialFromSelectedTextures", meta = (EditCondition = "bCustomMaterialName"))
	FString MaterialName = TEXT("M_");
	/** 是否自动创建材质实例。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CreateMaterialFromSelectedTextures")
	bool bIsAutoCreateMI = false;
#pragma endregion
#pragma region Supported Texture Names
	/** 支持的 BaseColor 贴图名称关键字。 */
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category = "Supported Texture Names")
	TArray<FString> BaseColorArray = {
		TEXT("_BaseColor"),
		TEXT("_Albedo"),
		TEXT("_Diffuse"),
		TEXT("_diff")
		};
	
	/** 支持的 Metallic 贴图名称关键字。 */
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category = "Supported Texture Names")
	TArray<FString> MetallicArray = {
		TEXT("_Metallic"),
		TEXT("_metal")
		};

	/** 支持的 Roughness 贴图名称关键字。 */
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category = "Supported Texture Names")
	TArray<FString> RoughnessArray = {
		TEXT("_Roughness"),
		TEXT("_RoughnessMap"),
		TEXT("_rough")
		};

	/** 支持的 Normal 贴图名称关键字。 */
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category = "Supported Texture Names")
	TArray<FString> NormalArray = {
		TEXT("_Normal"),
		TEXT("_NormalMap"),
		TEXT("_nor")
		};
	              
	/** 支持的 AO 贴图名称关键字。 */
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category = "Supported Texture Names")
	TArray<FString> AmbientOcclusionArray = {
		TEXT("_AmbientOcclusion"),
		TEXT("_AmbientOcclusionMap"),
		TEXT("_AO")
		};
	/** 支持的 ORM 贴图名称关键字。 */
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category = "Supported Texture Names")
	TArray<FString> ORMNameArray = {
		TEXT("_arm"),
		TEXT("OcclusionRoughnessMetallic"),
		TEXT("_ORM")
	};
#pragma endregion
private:
#pragma region QuickMaterialCreationCore
	/** 解析所选资产并提取贴图及输出路径。 */
	bool ProcessSelectedData(const TArray<FAssetData>& SelectedAssetsDataToProcess,
	                         TArray<UTexture2D*>& OutSelectedTexturesArray, FString& OutCreatedTexturePackagePath);
	/** 检查名称是否在路径内已被使用。 */
	static bool CheckIsNameUsed(const FString& NameToCheck, const FString& FolderPath);
	/** 根据配置创建材质资源。 */
	UMaterial* CreateMaterial(const FString& InMaterialName, const FString& MaterialPackagePath);
	/** 默认贴图连线逻辑。 */
	void Default_CreateMaterialNodes(UMaterial* CreatedMaterial, UTexture2D* SelectedTexture, uint32& PinsConnectedCounter);
	/** ORM 贴图连线逻辑。 */
	void ORM_CreateMaterialNodes(UMaterial* CreatedMaterial, UTexture2D* SelectedTexture, uint32& PinsConnectedCounter);
#pragma endregion
	
#pragma region CreateMaterialNodesConnectPins

	/** 尝试连接 BaseColor 栅格。 */
	bool TryConnectBaseColor(UMaterialExpressionTextureSample* TextureSampleNode, UTexture2D* SelectedTexture, UMaterial* CreatedMaterial);
	/** 尝试连接 Metallic 通道。 */
	bool TryConnectMetallic(UMaterialExpressionTextureSample* TextureSampleNode, UTexture2D* SelectedTexture, UMaterial* CreatedMaterial);
	/** 尝试连接 Roughness 通道。 */
	bool TryConnectRoughness(UMaterialExpressionTextureSample* TextureSampleNode, UTexture2D* SelectedTexture, UMaterial* CreatedMaterial);
	/** 尝试连接 Normal 通道。 */
	bool TryConnectNormal(UMaterialExpressionTextureSample* TextureSampleNode, UTexture2D* SelectedTexture, UMaterial* CreatedMaterial);
	/** 尝试连接 AO 通道。 */
	bool TryConnectAO(UMaterialExpressionTextureSample* TextureSampleNode, UTexture2D* SelectedTexture, UMaterial* CreatedMaterial);
	/** 尝试连接 ORM 打包贴图。 */
	bool TryConnectORM(UMaterialExpressionTextureSample* TextureSampleNode, UTexture2D* SelectedTexture, UMaterial* CreatedMaterial);
#pragma endregion

	
	/** 对指定材质生成材质实例并保存。 */
	UFUNCTION(BlueprintCallable, Category = "QuickMaterialCreationCore")
	UMaterialInstanceConstant* CreateMaterialInstance(UMaterial* CreatedMaterial, const FString& PathToPutMI);
};
