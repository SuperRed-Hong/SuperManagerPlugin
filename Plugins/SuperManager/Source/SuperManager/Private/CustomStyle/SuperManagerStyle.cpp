// Fill out your copyright notice in the Description page of Project Settings.


#include "CustomStyle/SuperManagerStyle.h"
#include "Styling/SlateStyleRegistry.h"

#include "Interfaces/IPluginManager.h"
FName FSuperManagerStyle::IconStyleSetName = FName("SuperManagerStyle");
TSharedPtr<FSlateStyleSet> FSuperManagerStyle::CreatedSlateStyleSet = nullptr;

namespace SuperManagerStyle
{
namespace LockedActors
{
static const FLinearColor CardColor(0.07f, 0.09f, 0.15f, 0.92f);
static const FLinearColor ToolbarColor(0.12f, 0.16f, 0.26f, 0.9f);
static const FLinearColor ControlColor(0.16f, 0.2f, 0.34f, 0.85f);
static const FLinearColor AccentColor(0.4f, 0.7f, 1.0f, 1.f);
static const FLinearColor AccentHoverColor(0.36f, 0.6f, 0.98f, 1.f);
static const FLinearColor NeutralButtonColor(0.36f, 0.4f, 0.55f, 0.9f);

static const FSlateRoundedBoxBrush GlassCardBrush(CardColor, 18.f);
static const FSlateRoundedBoxBrush ToolbarBrush(ToolbarColor, 14.f);
static const FSlateRoundedBoxBrush ControlBrush(ControlColor, 10.f);
static const FSlateRoundedBoxBrush LockCellBrush(FLinearColor(0.16f, 0.2f, 0.35f, 0.7f), 10.f);
static const FSlateRoundedBoxBrush RowActiveBrush(FLinearColor(0.27f, 0.45f, 0.95f, 0.35f), 10.f);
static const FSlateRoundedBoxBrush RowInactiveBrush(FLinearColor(0.09f, 0.12f, 0.18f, 0.65f), 10.f);
}
}

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

FLinearColor FSuperManagerStyle::FLockedActorsListStyle::GetCardColor()
{
	return SuperManagerStyle::LockedActors::CardColor;
}

FLinearColor FSuperManagerStyle::FLockedActorsListStyle::GetToolbarColor()
{
	return SuperManagerStyle::LockedActors::ToolbarColor;
}

FLinearColor FSuperManagerStyle::FLockedActorsListStyle::GetControlColor()
{
	return SuperManagerStyle::LockedActors::ControlColor;
}

FLinearColor FSuperManagerStyle::FLockedActorsListStyle::GetAccentColor()
{
	return SuperManagerStyle::LockedActors::AccentColor;
}

FLinearColor FSuperManagerStyle::FLockedActorsListStyle::GetAccentHoverColor()
{
	return SuperManagerStyle::LockedActors::AccentHoverColor;
}

FLinearColor FSuperManagerStyle::FLockedActorsListStyle::GetNeutralButtonColor()
{
	return SuperManagerStyle::LockedActors::NeutralButtonColor;
}

const FSlateRoundedBoxBrush& FSuperManagerStyle::FLockedActorsListStyle::GetGlassCardBrush()
{
	return SuperManagerStyle::LockedActors::GlassCardBrush;
}

const FSlateRoundedBoxBrush& FSuperManagerStyle::FLockedActorsListStyle::GetToolbarBrush()
{
	return SuperManagerStyle::LockedActors::ToolbarBrush;
}

const FSlateRoundedBoxBrush& FSuperManagerStyle::FLockedActorsListStyle::GetControlBrush()
{
	return SuperManagerStyle::LockedActors::ControlBrush;
}

const FSlateRoundedBoxBrush& FSuperManagerStyle::FLockedActorsListStyle::GetLockCellBrush()
{
	return SuperManagerStyle::LockedActors::LockCellBrush;
}

const FSlateRoundedBoxBrush& FSuperManagerStyle::FLockedActorsListStyle::GetRowActiveBrush()
{
	return SuperManagerStyle::LockedActors::RowActiveBrush;
}

const FSlateRoundedBoxBrush& FSuperManagerStyle::FLockedActorsListStyle::GetRowInactiveBrush()
{
	return SuperManagerStyle::LockedActors::RowInactiveBrush;
}
