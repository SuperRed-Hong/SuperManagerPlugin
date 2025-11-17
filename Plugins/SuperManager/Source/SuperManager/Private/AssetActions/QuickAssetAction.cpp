// Fill out your copyright notice in the Description page of Project Settings.
#include "AssetActions/QuickAssetAction.h"
#include "DebugHeader.h"
#include "EditorUtilityLibrary.h"
#include "EditorAssetLibrary.h"
#include "Editor.h"
#include "ObjectTools.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetToolsModule.h"
#include "AssetViewUtils.h"
#include "DSP/PassiveFilter.h"
#include "Misc/DefaultValueHelper.h"
#include "ScopedTransaction.h"

#define LOCTEXT_NAMESPACE "QuickAssetAction"

void UQuickAssetAction::DuplicateAssets(const int NumOfDuplicates)
{
	// ... (省略边界检查和获取选中资产的代码) ...
	if (NumOfDuplicates <= 0)
	{
		/*Print(TEXT("Please enter a POSITIVE number of duplicates"), FColor::Red);*/
		DebugHeader::ShowMsgDialog(EAppMsgType::Ok, TEXT("Please enter a POSITIVE number of duplicates"), true);


		return;
	}
	TArray<FAssetData> SelectedAssetsData = UEditorUtilityLibrary::GetSelectedAssetData();
	if (SelectedAssetsData.Num() == 0)
	{
		DebugHeader::ShowMsgDialog(EAppMsgType::Ok, TEXT("Please select at least one asset to duplicate."), true);
		return;
	}

	FScopedTransaction Transaction(LOCTEXT("DuplicateAssetsTransaction", "Duplicate Selected Assets"));
	uint32 Counter = 0;

	for (const FAssetData& SelectedAssetData : SelectedAssetsData) // 优化：使用 const 引用
	{
		// 外部循环的开始：对于每一个选中的资产

		// --- 新增逻辑的关键点 ---
		// 1. 找到当前资产的"基准名称"（不带任何后缀）
		const FString BaseAssetName = SelectedAssetData.AssetName.ToString();
		const FString PackagePath = SelectedAssetData.PackagePath.ToString();

		// ********** 优化点 A：预先计算起始 VersionNumber **********
		int32 VersionNumber = GetNextAvailableVersionNumber(PackagePath, BaseAssetName); 
		// ^^^ 这个新函数只需调用一次 Asset Registry

		for (int i = 0; i < NumOfDuplicates; i++)
		{
			// 1. 直接构造新名称 (不再需要 do-while 循环！)
			const FString FinalNewAssetName = BaseAssetName + TEXT("_") + FString::FromInt(VersionNumber);
			const FString FinalNewAssetPath = FPaths::Combine(PackagePath, FinalNewAssetName);

			// 2. 准备复制和保存
			const FString SourceAssetPath = SelectedAssetData.GetObjectPathString();
        
			if (UEditorAssetLibrary::DuplicateAsset(SourceAssetPath, FinalNewAssetPath))
			{
				UEditorAssetLibrary::SaveAsset(FinalNewAssetPath, false);
				Counter++;
			}

			// 3. 复制成功后，递增 VersionNumber，为下一个副本做准备
			VersionNumber++;
		
		}
	}

	if (Counter > 0)
	{
		// 成功的消息建议使用 FString::Printf 和 FColor::Green
		DebugHeader::ShowNotifyInfo(TEXT("Successfully duplicated " + FString::FromInt(Counter) + " files"));
	}
	else
	{
		Transaction.Cancel();
		DebugHeader::ShowNotifyInfo(TEXT("Duplicate Fail"));
	}
}

void UQuickAssetAction::AddPrefixes()
{
	FScopedTransaction Transaction(LOCTEXT("AddPrefixesTransaction", "Add Asset Prefixes"));
	uint32 Counter = 0;
	TArray<UObject*> SelectedAssets = UEditorUtilityLibrary::GetSelectedAssets();
	for (UObject* SelectedAsset : SelectedAssets)
	{
		if (!SelectedAsset) continue;
		FString* PrefixFound = PrefixMap.Find(SelectedAsset->GetClass());
		if (!PrefixFound || PrefixFound->IsEmpty())
		{
			DebugHeader::Print(TEXT("Failed to find prefix for class") + SelectedAsset->GetClass()->GetName(), FColor::Red);
			continue;
		}
		FString BaseAssetName = SelectedAsset->GetName();
		if (BaseAssetName.StartsWith(*PrefixFound))
		{
			DebugHeader::Print(BaseAssetName + TEXT(" already has prefix added"), FColor::Yellow);
		}
		if (SelectedAsset->IsA<UMaterialInstanceConstant>())
		{
			BaseAssetName.RemoveFromStart(TEXT("M_"));
			BaseAssetName.RemoveFromEnd(TEXT("_Inst"));
		}
		const FString NewAssetName = *PrefixFound + BaseAssetName;
		SelectedAsset->Modify();
		UEditorUtilityLibrary::RenameAsset(SelectedAsset, NewAssetName);
		Counter++;
	}
	if (Counter > 0)
	{
		DebugHeader::ShowNotifyInfo(TEXT("Successfully renamed "+ FString::FromInt(Counter) + "assets"));
	}
	else
	{
		Transaction.Cancel();
	}
}

void UQuickAssetAction::RemoveUnusedAsset()
{
	TArray<FAssetData> SelectedAssetsData = UEditorUtilityLibrary::GetSelectedAssetData();
	TArray<FAssetData> UnusedAssetsData;
	
	FixUpRedirectors();
	
	for (const FAssetData& SelectedAssetData : SelectedAssetsData)
	{
		if (UEditorAssetLibrary::FindPackageReferencersForAsset(SelectedAssetData.GetObjectPathString()).IsEmpty())
		//if (UEditorAssetLibrary::FindPackageReferencersForAsset(SelectedAssetData.ObjectPath.ToString()).Num() == 0)
		{
			UnusedAssetsData.Add(SelectedAssetData);
		}
	}
	if (UnusedAssetsData.Num() == 0)
	{
		DebugHeader::ShowMsgDialog(EAppMsgType::Ok, ("No unused asset found among selected assets"), false);
		return;
	}
	FScopedTransaction Transaction(LOCTEXT("RemoveUnusedAssetsTransaction", "Remove Unused Assets"));
	int32 NumOfAssetsDeleted = ObjectTools::DeleteAssets(UnusedAssetsData);
	if (NumOfAssetsDeleted == 0)
	{
		Transaction.Cancel();
		return;
	}

	DebugHeader::ShowNotifyInfo(TEXT("Successfully deleted " + FString::FromInt(NumOfAssetsDeleted)) + TEXT(" unused Asset data"));
}

void UQuickAssetAction::BatchRenameAsset()
{
}

void UQuickAssetAction::FixUpRedirectors()
{

	IAssetRegistry& AssetRegistry = 
	FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry")).Get();
 
	// Form a filter from the paths
	FARFilter Filter;
	Filter.bRecursivePaths = true;
	Filter.PackagePaths.Emplace("/Game");
 
 
	Filter.ClassPaths.Add(UObjectRedirector::StaticClass()->GetClassPathName());
 
	// Query for a list of assets in the selected paths
	TArray<FAssetData> AssetList;
	AssetRegistry.GetAssets(Filter, AssetList);
 
	if (AssetList.Num() == 0) return;
	
	TArray<FString> ObjectPaths;
	for (const FAssetData& Asset : AssetList)
	{			
		ObjectPaths.Add(Asset.GetObjectPathString());	
	}
 
	TArray<UObject*> Objects;
	const bool bAllowedToPromptToLoadAssets = true;
	const bool bLoadRedirects = true;
 
	AssetViewUtils::FLoadAssetsSettings Settings;
	Settings.bFollowRedirectors = false;
	Settings.bAllowCancel = true;
 
	AssetViewUtils::ELoadAssetsResult Result = AssetViewUtils::LoadAssetsIfNeeded(ObjectPaths,Objects,Settings);
 
	if (Result != AssetViewUtils::ELoadAssetsResult::Cancelled)
	{
		// Transform Objects array to ObjectRedirectors array
		TArray<UObjectRedirector*> Redirectors;
		for (UObject* Object : Objects)
		{
			Redirectors.Add(CastChecked<UObjectRedirector>(Object));
		}
	
		// Load the asset tools module
		FAssetToolsModule& AssetToolsModule = FModuleManager::LoadModuleChecked<FAssetToolsModule>(TEXT("AssetTools"));
		AssetToolsModule.Get().FixupReferencers(Redirectors);
 
	}
}

int32 UQuickAssetAction::GetNextAvailableVersionNumber(const FString& PackagePath, const FString& BaseAssetName)
{
	// 1. 获取资产注册表
	IAssetRegistry& AssetRegistry = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry")).Get();

	// 2. 构建筛选器
	FARFilter Filter;
	// 只在指定路径下搜索
	Filter.PackagePaths.Add(*PackagePath); 
	// 递归查找设为 false，因为我们只关心当前目录下的文件

	// 3. 查询资产
	TArray<FAssetData> AssetList;
	AssetRegistry.GetAssets(Filter, AssetList);

	int32 MaxVersion = 0;
	const FString Prefix = BaseAssetName + TEXT("_");
    
	// 4. 解析所有匹配的资产名称
	for (const FAssetData& AssetData : AssetList)
	{
		FString AssetName = AssetData.AssetName.ToString();
		// 检查资产名称是否以 BaseAssetName_ 开头
		if (AssetName.StartsWith(Prefix))
		{
			// 尝试提取后缀数字
			FString Suffix = AssetName.RightChop(Prefix.Len());
			int32 CurrentVersion;
			if (FDefaultValueHelper::ParseInt(Suffix, CurrentVersion))
			{
				MaxVersion = FMath::Max(MaxVersion, CurrentVersion);
			}
		}
	}
    
	// 返回 MaxVersion + 1 作为下一个可用的版本号
	return MaxVersion + 1;
}

#undef LOCTEXT_NAMESPACE
