// Fill out your copyright notice in the Description page of Project Settings.


#include "CustomStyle/SuperManagerStyle.h"
#include "Styling/SlateStyleRegistry.h"

#include "Interfaces/IPluginManager.h"
FName FSuperManagerStyle::IconStyleSetName = FName("SuperManagerStyle");
TSharedPtr<FSlateStyleSet> FSuperManagerStyle::CreatedSlateStyleSet = nullptr;

void FSuperManagerStyle::InitializeIcons()
{
	if (CreatedSlateStyleSet.IsValid() == false)
	{
		CreatedSlateStyleSet = CreateSlateStyleSet();
		FSlateStyleRegistry::RegisterSlateStyle(*CreatedSlateStyleSet.Get());
	}
}


TSharedRef<FSlateStyleSet> FSuperManagerStyle::CreateSlateStyleSet()
{
	TSharedRef<FSlateStyleSet> CustomStyleSet = MakeShareable(new FSlateStyleSet(IconStyleSetName));
	const FString IconDirectory =
		IPluginManager::Get().FindPlugin("SuperManager")->GetBaseDir() / TEXT("Resources/Icons");
	CustomStyleSet->SetContentRoot(IconDirectory);
	const FVector2D Icon16x16(16.0f, 16.0f);
	CustomStyleSet->Set("ContentBrowser.DeleteUnusedAssets",
	                    new FSlateImageBrush(IconDirectory / TEXT("DeleteUnusedAsset.png"), Icon16x16));
	CustomStyleSet->Set("ContentBrowser.AdvancedDeletion",
	                    new FSlateImageBrush(IconDirectory / TEXT("AdvancedDeletion.png"), Icon16x16));
	CustomStyleSet->Set("ContentBrowser.DeleteEmptyFolders",
	                    new FSlateImageBrush(IconDirectory / TEXT("DeleteEmptyFolders.png"), Icon16x16));
	CustomStyleSet->Set("LevelEditor.LockActorSelection",
	                    new FSlateImageBrush(
		                    IconDirectory / TEXT("LevelEditorMeneEntryIcons") / TEXT("SelectionLock.png"),
		                    Icon16x16));
	CustomStyleSet->Set("LevelEditor.UnlockActorSelection",
	                    new FSlateImageBrush(
		                    IconDirectory / TEXT("LevelEditorMeneEntryIcons") / TEXT("SelectionUnlock.png"),
		                    Icon16x16));

	CustomStyleSet->Set("LevelEditor.SubMenu",
	                    new FSlateImageBrush(IconDirectory / TEXT("LevelEditorMeneEntryIcons") / TEXT("Superman.png"),
	                                         Icon16x16));
	return CustomStyleSet;
}

void FSuperManagerStyle::Shutdown()
{
	if (CreatedSlateStyleSet.IsValid())
	{
		FSlateStyleRegistry::UnRegisterSlateStyle(*CreatedSlateStyleSet);
		CreatedSlateStyleSet.Reset();
	}
}
