// Fill out your copyright notice in the Description page of Project Settings.


#include "AssetActions/QuickMaterialCreationWidget.h"

#include "AssetToolsModule.h"
#include "DebugHeader.h"
#include "IAssetTools.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Factories/MaterialFactoryNew.h"
#include "EditorUtilityLibrary.h"
#include "SuperManager.h"
#include "AssetRegistry/AssetRegistryHelpers.h"
#include "Factories/MaterialInstanceConstantFactoryNew.h"
#include "Materials/MaterialInstanceConstant.h"
#include "ScopedTransaction.h"

#define LOCTEXT_NAMESPACE "QuickMaterialCreationWidget"

#pragma region QuickMaterialCreationCore

void UQuickMaterialCreationWidget::CreateMaterialFromSelectedTextures()
{
	//DebugHeader::Print("CreateMaterialFromSelectedTextures Working!", FColor::Green);
	//Check material name is valid

	if (bCustomMaterialName)
	{
		if (MaterialName.IsEmpty() || MaterialName == TEXT("M_"))
		{
			DebugHeader::ShowMsgDialog(EAppMsgType::Ok, TEXT("Please enter a valid material name."), true);
			return;
		}
	}


	TArray<FAssetData> SelectedAssetsData = UEditorUtilityLibrary::GetSelectedAssetData();
	TArray<UTexture2D*> SelectedTexturesArray;
	FString CreatedTexturePackagePath;
	uint32 PinsConnectedCounter = 0;
	if (!ProcessSelectedData(SelectedAssetsData, SelectedTexturesArray, CreatedTexturePackagePath))
	//sort Selected Textures from selected Asset
	{
		DebugHeader::Print("ProcessSelectedData Failed!", FColor::Red);
		return;
	}
	DebugHeader::Print("ProcessSelectedData Success!" + CreatedTexturePackagePath, FColor::Green);
	FString MaterialPackageName = CreatedTexturePackagePath + TEXT("/") + MaterialName;

	bool bIsNameUsed = CheckIsNameUsed(MaterialName, CreatedTexturePackagePath);
	if (bIsNameUsed)
	{
		FSuperManagerModule& SuperManagerModule = FModuleManager::LoadModuleChecked<FSuperManagerModule>(
			TEXT("SuperManager"));
		SuperManagerModule.SyncCBToClickedAssetForAssetList(MaterialPackageName);

		DebugHeader::Print("Material name is already used.", FColor::Red);
		DebugHeader::ShowMsgDialog(EAppMsgType::Ok, TEXT("Material name is already used."), true);
		MaterialName = TEXT("M_");
		return;
	}

	FScopedTransaction Transaction(LOCTEXT("CreateMaterialFromTexturesTransaction", "Create Material From Selected Textures"));

	UMaterial* CreatedMaterial = CreateMaterial(MaterialName, CreatedTexturePackagePath);
	if (!CreatedMaterial)
	{
		Transaction.Cancel();
		DebugHeader::ShowMsgDialog(EAppMsgType::Ok, TEXT("Failed to create material."), true);
		MaterialName = TEXT("M_");
		return;
	}
	CreatedMaterial->Modify();

	for (UTexture2D* SelectedTexture : SelectedTexturesArray)
	{
		if (!SelectedTexture) continue;
		switch (ChannelPackingType)
		{
		case E_ChannelPackingType::ECPT_NoChannelPacking:
			Default_CreateMaterialNodes(CreatedMaterial, SelectedTexture, PinsConnectedCounter);
			break;
		case E_ChannelPackingType::ECPT_ORM:
			ORM_CreateMaterialNodes(CreatedMaterial, SelectedTexture, PinsConnectedCounter);
			break;
		case E_ChannelPackingType::ECPT_MAX:
			break;
		default:
			checkNoEntry();
		}
	}

	if (PinsConnectedCounter > 0)
	{
		DebugHeader::ShowNotifyInfo(
			TEXT("Successfully connected ") + FString::FromInt(PinsConnectedCounter) + TEXT(" pins"));
	}
	MaterialName = TEXT("M_");
	if (bIsAutoCreateMI)
	{
		if (!CreateMaterialInstance(CreatedMaterial,CreatedTexturePackagePath))
		{
			DebugHeader::Print(TEXT("CreateMaterialInstance failed!"), FColor::Red);
		}
		
	}
}



bool UQuickMaterialCreationWidget::ProcessSelectedData(const TArray<FAssetData>& SelectedAssetsDataToProcess,
                                                       TArray<UTexture2D*>& OutSelectedTexturesArray,
                                                       FString& OutCreatedTexturePackagePath)
{
	if (SelectedAssetsDataToProcess.Num() == 0)
	{
		DebugHeader::ShowMsgDialog(EAppMsgType::Ok, TEXT("Please select at least one texture."), true);
		return false;
	}
	bool bPackagePathSet = false;
	bool bMaterialNameSet = false;
	for (const FAssetData& AssetData : SelectedAssetsDataToProcess)
	{
		//DebugHeader::Print(AssetData.ObjectPath, FColor::Green);
		UObject* SelectedObject = AssetData.GetAsset();
		UTexture2D* SelectedTexture = Cast<UTexture2D>(SelectedObject);
		if (!SelectedTexture)
		{
			DebugHeader::ShowMsgDialog(EAppMsgType::Ok,
			                           TEXT("Please select only texture ") + SelectedObject->GetName() + TEXT(
				                           " is not texture"), true);
			return false;
		}
		OutSelectedTexturesArray.Add(SelectedTexture);
		if (bPackagePathSet == false)
		{
			OutCreatedTexturePackagePath = AssetData.PackagePath.ToString();
			bPackagePathSet = true;
		}
		if (!bCustomMaterialName && !bMaterialNameSet)
		{
			MaterialName = AssetData.AssetName.ToString();
			MaterialName.RemoveFromStart(TEXT("T_"));
			MaterialName.RemoveFromEnd(TEXT("_Inst"));
			MaterialName = TEXT("M_") + MaterialName;
			bMaterialNameSet = true;
		}
	}
	return true;
}

bool UQuickMaterialCreationWidget::CheckIsNameUsed(const FString& NameToCheck, const FString& FolderPath)
{
	IAssetRegistry& AssetRegistry = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry")).
		Get();
	FString FullPath = FString::Printf(TEXT("%s/%s.%s"), *FolderPath, *NameToCheck, *NameToCheck);
	FSoftObjectPath SoftObjectPathToCheck(FullPath);
	FAssetData ExistingAssetData = AssetRegistry.GetAssetByObjectPath(SoftObjectPathToCheck);
	return ExistingAssetData.IsValid();
}

UMaterial* UQuickMaterialCreationWidget::CreateMaterial(const FString& InMaterialName,
                                                        const FString& MaterialPackagePath)
{
	FAssetToolsModule& AssetToolsModule = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools");
	UMaterialFactoryNew* MaterialFactory = NewObject<UMaterialFactoryNew>();
	UObject* CreatedMaterial = AssetToolsModule.Get().CreateAsset(InMaterialName, MaterialPackagePath,
	                                                              UMaterial::StaticClass(), MaterialFactory);
	return Cast<UMaterial>(CreatedMaterial);
}

/*UMaterialInstanceConstant* UQuickMaterialCreationWidget::CreateMaterialInstance(UMaterial* CreatedMaterial, const FString& InMaterialPackagePath)
{
	FAssetToolsModule& AssetToolsModule = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools");
	FString MaterialInstanceName = CreatedMaterial->GetName();
	MaterialInstanceName.InsertAt(1, TEXT("I"));
	FAssetData AssetData = UAssetRegistryHelpers::CreateAssetData(CreatedMaterial);
	FString FolderPath = AssetData.PackagePath.ToString();
	FString PackagePath = FolderPath + TEXT("/") + MaterialInstanceName;
	UMaterialInstanceConstantFactoryNew* MIFactory = NewObject<UMaterialInstanceConstantFactoryNew>();
	UObject* CreatedMI = AssetToolsModule.Get().CreateAsset(MaterialInstanceName, PackagePath,UMaterialInstance::StaticClass(), MIFactory);
	if (!CreatedMI)
	{
		DebugHeader::Print(TEXT("Creat MI Fail"), FColor::Red);
		return nullptr;
	}
	return Cast<UMaterialInstanceConstant>(CreatedMI);
}*/

void UQuickMaterialCreationWidget::Default_CreateMaterialNodes(UMaterial* CreatedMaterial,
                                                               UTexture2D* SelectedTexture,
                                                               uint32& PinsConnectedCounter)
{
	if (CreatedMaterial)
	{
		CreatedMaterial->Modify();
	}
	UMaterialExpressionTextureSample* TextureSampleNode =
		NewObject<UMaterialExpressionTextureSample>(CreatedMaterial);
	if (!TextureSampleNode) return;
	if (!CreatedMaterial->HasBaseColorConnected())
	{
		if (TryConnectBaseColor(TextureSampleNode, SelectedTexture, CreatedMaterial))
		{
			PinsConnectedCounter++;
			return;
		}
	}
	if (!CreatedMaterial->HasMetallicConnected())
	{
		if (TryConnectMetallic(TextureSampleNode, SelectedTexture, CreatedMaterial))
		{
			PinsConnectedCounter++;
			return;
		}
	}
	if (!CreatedMaterial->HasRoughnessConnected())
	{
		if (TryConnectRoughness(TextureSampleNode, SelectedTexture, CreatedMaterial))
		{
			PinsConnectedCounter++;
			return;
		}
	}
	if (!CreatedMaterial->HasNormalConnected())
	{
		if (TryConnectNormal(TextureSampleNode, SelectedTexture, CreatedMaterial))
		{
			PinsConnectedCounter++;
			return;
		}
	}
	if (!CreatedMaterial->HasAmbientOcclusionConnected())
	{
		if (TryConnectAO(TextureSampleNode, SelectedTexture, CreatedMaterial))
		{
			PinsConnectedCounter++;
			return;
		}
	}
	DebugHeader::Print(TEXT("Failed to connect the Texture") + SelectedTexture->GetName(), FColor::Red);
}

void UQuickMaterialCreationWidget::ORM_CreateMaterialNodes(UMaterial* CreatedMaterial, UTexture2D* SelectedTexture,
	uint32& PinsConnectedCounter)
{
	if (CreatedMaterial)
	{
		CreatedMaterial->Modify();
	}
	UMaterialExpressionTextureSample* TextureSampleNode =
		NewObject<UMaterialExpressionTextureSample>(CreatedMaterial);
	if (!TextureSampleNode) return;
	if (!CreatedMaterial->HasBaseColorConnected())
	{
		if (TryConnectBaseColor(TextureSampleNode, SelectedTexture, CreatedMaterial))
		{
			PinsConnectedCounter++;
			return;
		}
	}
	if (!CreatedMaterial->HasNormalConnected())
	{
		if (TryConnectNormal(TextureSampleNode, SelectedTexture, CreatedMaterial))
		{
			PinsConnectedCounter++;
			return;
		}
	}
	if (!CreatedMaterial->HasRoughnessConnected())
	{
		if (TryConnectORM(TextureSampleNode, SelectedTexture, CreatedMaterial))
		{
			PinsConnectedCounter += 3;
			return;
		}
	}
}
#pragma endregion
#pragma region CreateMaterialNodesConnectPins

bool UQuickMaterialCreationWidget::TryConnectBaseColor(UMaterialExpressionTextureSample* TextureSampleNode,
                                                       UTexture2D* SelectedTexture, UMaterial* CreatedMaterial)
{
	for (const FString& BaseColorName : BaseColorArray)
	{
		if (SelectedTexture->GetName().Contains(BaseColorName))
		{
			CreatedMaterial->Modify();
			//connect pins to base color socket
			TextureSampleNode->Texture = SelectedTexture;
			CreatedMaterial->GetExpressionCollection().AddExpression(TextureSampleNode);
			CreatedMaterial->GetExpressionInputForProperty(MP_BaseColor)->Connect(0, TextureSampleNode);
			CreatedMaterial->PostEditChange();
			TextureSampleNode->MaterialExpressionEditorX -= 600;
			return true;
		}
	}
	return false;
}

bool UQuickMaterialCreationWidget::TryConnectMetallic(UMaterialExpressionTextureSample* TextureSampleNode,
                                                      UTexture2D* SelectedTexture, UMaterial* CreatedMaterial)
{
	for (const FString& MetallicName : MetallicArray)
	{
		if (SelectedTexture->GetName().Contains(MetallicName))
		{
			SelectedTexture->Modify();
			SelectedTexture->CompressionSettings = TC_Default;
			SelectedTexture->SRGB = false;
			SelectedTexture->PostEditChange();

			CreatedMaterial->Modify();
			TextureSampleNode->Texture = SelectedTexture;
			TextureSampleNode->SamplerType = SAMPLERTYPE_LinearColor;
			CreatedMaterial->GetExpressionCollection().AddExpression(TextureSampleNode);
			CreatedMaterial->GetExpressionInputForProperty(MP_Metallic)->Connect(0, TextureSampleNode);
			CreatedMaterial->PostEditChange();
			TextureSampleNode->MaterialExpressionEditorX -= 600;
			TextureSampleNode->MaterialExpressionEditorY += 240;

			return true;
		}
	}
	return false;
}

bool UQuickMaterialCreationWidget::TryConnectRoughness(UMaterialExpressionTextureSample* TextureSampleNode,
                                                       UTexture2D* SelectedTexture, UMaterial* CreatedMaterial)
{
	for (const FString& RoughnessName : RoughnessArray)
	{
		if (SelectedTexture->GetName().Contains(RoughnessName))
		{
			SelectedTexture->Modify();
			SelectedTexture->CompressionSettings = TC_Default;
			SelectedTexture->SRGB = false;
			SelectedTexture->PostEditChange();

			CreatedMaterial->Modify();

			TextureSampleNode->Texture = SelectedTexture;
			TextureSampleNode->SamplerType = SAMPLERTYPE_LinearColor;

			CreatedMaterial->GetExpressionCollection().AddExpression(TextureSampleNode);
			CreatedMaterial->GetExpressionInputForProperty(MP_Roughness)->Connect(0, TextureSampleNode);
			CreatedMaterial->PostEditChange();
			TextureSampleNode->MaterialExpressionEditorX -= 600;
			TextureSampleNode->MaterialExpressionEditorY += 240 * 2;
			return true;
		}
	}
	return false;
}

bool UQuickMaterialCreationWidget::TryConnectNormal(UMaterialExpressionTextureSample* TextureSampleNode,
                                                    UTexture2D* SelectedTexture, UMaterial* CreatedMaterial)
{
	for (const FString& NormalName : NormalArray)
	{
		if (SelectedTexture->GetName().Contains(NormalName))
		{
			SelectedTexture->Modify();
			SelectedTexture->CompressionSettings = TC_Normalmap;
			SelectedTexture->SRGB = false;
			SelectedTexture->PostEditChange();

			CreatedMaterial->Modify();
			TextureSampleNode->Texture = SelectedTexture;
			TextureSampleNode->SamplerType = SAMPLERTYPE_Normal;
			CreatedMaterial->GetExpressionCollection().AddExpression(TextureSampleNode);
			CreatedMaterial->GetExpressionInputForProperty(MP_Normal)->Connect(0, TextureSampleNode);
			CreatedMaterial->PostEditChange();

			TextureSampleNode->MaterialExpressionEditorX -= 600;
			TextureSampleNode->MaterialExpressionEditorY += 240 * 3;
			return true;
		}
	}
	return false;
}

bool UQuickMaterialCreationWidget::TryConnectAO(UMaterialExpressionTextureSample* TextureSampleNode,
                                                UTexture2D* SelectedTexture, UMaterial* CreatedMaterial)
{
	for (const FString& AOName : AmbientOcclusionArray)
	{
		if (SelectedTexture->GetName().Contains(AOName))
		{
			SelectedTexture->Modify();
			SelectedTexture->CompressionSettings = TC_Default;
			SelectedTexture->SRGB = false;
			SelectedTexture->PostEditChange();
			CreatedMaterial->Modify();
			TextureSampleNode->Texture = SelectedTexture;
			TextureSampleNode->SamplerType = SAMPLERTYPE_LinearColor;
			CreatedMaterial->GetExpressionCollection().AddExpression(TextureSampleNode);
			CreatedMaterial->GetExpressionInputForProperty(MP_AmbientOcclusion)->Connect(0, TextureSampleNode);
			CreatedMaterial->PostEditChange();
			TextureSampleNode->MaterialExpressionEditorX -= 600;
			TextureSampleNode->MaterialExpressionEditorY += 240 * 4;
			return true;
		}
	}
	return false;
}

bool UQuickMaterialCreationWidget::TryConnectORM(UMaterialExpressionTextureSample* TextureSampleNode,
	UTexture2D* SelectedTexture, UMaterial* CreatedMaterial)
{
	for (const FString& ORMName : ORMNameArray)
	{
		if (SelectedTexture->GetName().Contains(ORMName))
		{
			SelectedTexture->Modify();
			SelectedTexture->CompressionSettings = TC_Masks;
			SelectedTexture->SRGB = false;
			SelectedTexture->PostEditChange();

			CreatedMaterial->Modify();
			TextureSampleNode->Texture = SelectedTexture;
			TextureSampleNode->SamplerType = SAMPLERTYPE_Masks;
			CreatedMaterial->GetExpressionCollection().AddExpression(TextureSampleNode);
			CreatedMaterial->GetExpressionInputForProperty(MP_AmbientOcclusion)->Connect(1, TextureSampleNode);
			CreatedMaterial->GetExpressionInputForProperty(MP_Roughness)->Connect(2, TextureSampleNode);
			CreatedMaterial->GetExpressionInputForProperty(MP_Metallic)->Connect(3, TextureSampleNode);
			CreatedMaterial->PostEditChange();
			TextureSampleNode->MaterialExpressionEditorX -= 600;
			TextureSampleNode->MaterialExpressionEditorY += 240*2;
			return true;
		}
	}
	return false;
}
#pragma endregion

UMaterialInstanceConstant* UQuickMaterialCreationWidget::CreateMaterialInstance(UMaterial* CreatedMaterial,
	const FString& PathToPutMI)
{
	FAssetToolsModule& AssetToolsModule = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools");
	FString MaterialInstanceName = CreatedMaterial->GetName();
	MaterialInstanceName.InsertAt(1, TEXT("I"));
	FString PathToCreateMI = PathToPutMI + "/" + MaterialInstanceName;
	UMaterialInstanceConstantFactoryNew* Factory = NewObject<UMaterialInstanceConstantFactoryNew>();
	Factory->InitialParent = CreatedMaterial;
	UObject* Object = AssetToolsModule.Get().CreateAsset(MaterialInstanceName,PathToPutMI,UMaterialInstanceConstant::StaticClass(),Factory);
	if (UMaterialInstanceConstant* CreatedMI = Cast<UMaterialInstanceConstant>(Object))
	{
		CreatedMI->Modify();
		CreatedMaterial->Modify();
		CreatedMI->SetParentEditorOnly(CreatedMaterial);
		CreatedMI->PostEditChange();
		CreatedMaterial->PostEditChange();
		return CreatedMI;
	}
	return nullptr;

}

#undef LOCTEXT_NAMESPACE
