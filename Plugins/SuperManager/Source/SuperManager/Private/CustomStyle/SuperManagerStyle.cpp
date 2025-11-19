// Fill out your copyright notice in the Description page of Project Settings.


#include "CustomStyle/SuperManagerStyle.h"
#include "Styling/SlateStyleRegistry.h"

#include "Interfaces/IPluginManager.h"
FName FSuperManagerStyle::IconStyleSetName = FName("SuperManagerStyle");
TSharedPtr<FSlateStyleSet> FSuperManagerStyle::CreatedSlateStyleSet = nullptr;

const FLinearColor FSuperManagerStyle::PrimaryCardColor(0.07f, 0.09f, 0.15f, 0.92f);
const FLinearColor FSuperManagerStyle::PrimaryToolbarColor(0.12f, 0.16f, 0.26f, 0.9f);
const FLinearColor FSuperManagerStyle::PrimaryPanelColor(0.16f, 0.2f, 0.34f, 0.85f);
const FLinearColor FSuperManagerStyle::AccentColor(0.4f, 0.7f, 1.0f, 1.f);
const FLinearColor FSuperManagerStyle::AccentHoverColor(0.36f, 0.6f, 0.98f, 1.f);
const FLinearColor FSuperManagerStyle::NeutralButtonColor(0.36f, 0.4f, 0.55f, 0.9f);

const FSlateRoundedBoxBrush FSuperManagerStyle::PrimaryCardBrush(FSuperManagerStyle::PrimaryCardColor, 18.f);
const FSlateRoundedBoxBrush FSuperManagerStyle::PrimaryToolbarBrush(FSuperManagerStyle::PrimaryToolbarColor, 14.f);
const FSlateRoundedBoxBrush FSuperManagerStyle::PrimaryPanelBrush(FSuperManagerStyle::PrimaryPanelColor, 10.f);
const FSlateRoundedBoxBrush FSuperManagerStyle::EmphasisCellBrush(FLinearColor(0.16f, 0.2f, 0.35f, 0.7f), 10.f);
const FSlateRoundedBoxBrush FSuperManagerStyle::HighlightedRowBrush(FLinearColor(0.27f, 0.45f, 0.95f, 0.35f), 10.f);
const FSlateRoundedBoxBrush FSuperManagerStyle::InactiveRowBrush(FLinearColor(0.09f, 0.12f, 0.18f, 0.65f), 10.f);

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
	const FVector2D Icon8x8(8.0f, 8.0f);
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
	CustomStyleSet->Set("SceneOutliner.LockActorSelection",
	                    new FSlateImageBrush(IconDirectory / TEXT("LevelEditorMeneEntryIcons") / TEXT("Lock.png"),
	                                         Icon16x16));
	CustomStyleSet->Set("SceneOutliner.RowLockedImage",
	                    new FSlateImageBrush(IconDirectory / TEXT("LevelEditorMeneEntryIcons") / TEXT("Locking.png"),
	                                         Icon16x16));
	CustomStyleSet->Set("SceneOutliner.RowUnlockedImage",
	                    new FSlateImageBrush(IconDirectory / TEXT("LevelEditorMeneEntryIcons") / TEXT("Unlock_w.png"),
	                                         Icon16x16));
	const FCheckBoxStyle SelectionLockToggleButtonStyle = FCheckBoxStyle()
	                                                      .SetCheckBoxType(ESlateCheckBoxType::ToggleButton)
	                                                      .SetPadding(FMargin(10.0f))
	                                                      /** UnCheckedImage   */
	                                                      .SetUncheckedImage(FSlateImageBrush(
		                                                      IconDirectory / TEXT("LevelEditorMeneEntryIcons") / TEXT(
			                                                      "Unlock_w.png"),
		                                                      Icon8x8, FStyleColors::AccentGray))
	                                                      .SetUncheckedHoveredImage(FSlateImageBrush(
		                                                      IconDirectory / TEXT("LevelEditorMeneEntryIcons") / TEXT(
			                                                      "Unlock_w.png"),
		                                                      Icon8x8, FStyleColors::AccentBlue))
	                                                      .SetUncheckedPressedImage(FSlateImageBrush(
		                                                      IconDirectory / TEXT("LevelEditorMeneEntryIcons") / TEXT(
			                                                      "Unlock_w.png"),
		                                                      Icon8x8, FStyleColors::AccentWhite))
	                                                 
	                                                      //CheckedImage
	                                                      .SetCheckedImage(FSlateImageBrush(
		                                                      IconDirectory / TEXT("LevelEditorMeneEntryIcons") / TEXT(
			                                                      "Locking.png"),
		                                                      Icon8x8, FStyleColors::AccentWhite))
	                                                      .SetCheckedHoveredImage(FSlateImageBrush(
		                                                      IconDirectory / TEXT("LevelEditorMeneEntryIcons") / TEXT(
			                                                      "Locking.png"),
		                                                      Icon8x8, FStyleColors::White25))
	                                                      .SetCheckedPressedImage(FSlateImageBrush(
		                                                      IconDirectory / TEXT("LevelEditorMeneEntryIcons") / TEXT(
			                                                      "Locking.png"),
		                                                      Icon8x8, FStyleColors::Foreground));
	CustomStyleSet->Set("SceneOutliner.SelectionLock", SelectionLockToggleButtonStyle);
	return CustomStyleSet;
}

const ISlateStyle& FSuperManagerStyle::Get()
{
	check(CreatedSlateStyleSet.IsValid());
	return *CreatedSlateStyleSet.Get();
}

const FSlateBrush* FSuperManagerStyle::GetBrush(const FName BrushName)
{
	return CreatedSlateStyleSet.IsValid() ? CreatedSlateStyleSet->GetBrush(BrushName) : nullptr;
}

void FSuperManagerStyle::Shutdown()
{
	if (CreatedSlateStyleSet.IsValid())
	{
		FSlateStyleRegistry::UnRegisterSlateStyle(*CreatedSlateStyleSet);
		CreatedSlateStyleSet.Reset();
	}
}
