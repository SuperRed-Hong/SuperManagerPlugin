#include "CustomStyle/SuperManagerStyle.h"

#include "Brushes/SlateImageBrush.h"
#include "Interfaces/IPluginManager.h"
#include "Styling/SlateStyleRegistry.h"
#include "Styling/StyleColors.h"
#include "Styling/SlateTypes.h"

const FName FSuperManagerStyleNames::StyleSetName(TEXT("SuperManager.Style"));
const FName FSuperManagerStyleNames::PrimaryCardBrushName(TEXT("SuperManager.Style.Card.Primary"));
const FName FSuperManagerStyleNames::PrimaryToolbarBrushName(TEXT("SuperManager.Style.Toolbar.Primary"));
const FName FSuperManagerStyleNames::PrimaryPanelBrushName(TEXT("SuperManager.Style.Panel.Primary"));
const FName FSuperManagerStyleNames::EmphasisCellBrushName(TEXT("SuperManager.Style.Cell.Emphasis"));
const FName FSuperManagerStyleNames::HighlightedRowBrushName(TEXT("SuperManager.Style.Row.Highlighted"));
const FName FSuperManagerStyleNames::InactiveRowBrushName(TEXT("SuperManager.Style.Row.Inactive"));

FSuperManagerPalette::FSuperManagerPalette()
	: PrimaryCardColor(FLinearColor::Transparent)
	, PrimaryToolbarColor(FLinearColor::Transparent)
	, PrimaryPanelColor(FLinearColor::Transparent)
	, AccentColor(FLinearColor::Transparent)
	, AccentHoverColor(FLinearColor::Transparent)
	, NeutralButtonColor(FLinearColor::Transparent)
	, PrimaryCardBrush(FSlateRoundedBoxBrush(FLinearColor::Transparent, 0.f))
	, PrimaryToolbarBrush(FSlateRoundedBoxBrush(FLinearColor::Transparent, 0.f))
	, PrimaryPanelBrush(FSlateRoundedBoxBrush(FLinearColor::Transparent, 0.f))
	, EmphasisCellBrush(FSlateRoundedBoxBrush(FLinearColor::Transparent, 0.f))
	, HighlightedRowBrush(FSlateRoundedBoxBrush(FLinearColor::Transparent, 0.f))
	, InactiveRowBrush(FSlateRoundedBoxBrush(FLinearColor::Transparent, 0.f))
{
}

FSuperManagerPalette FSuperManagerPalette::CreateDefault()
{
	FSuperManagerPalette Palette;
	Palette.PrimaryCardColor = FLinearColor(0.07f, 0.09f, 0.15f, 0.92f);
	Palette.PrimaryToolbarColor = FLinearColor(0.12f, 0.16f, 0.26f, 0.9f);
	Palette.PrimaryPanelColor = FLinearColor(0.16f, 0.2f, 0.34f, 0.85f);
	Palette.AccentColor = FLinearColor(0.4f, 0.7f, 1.0f, 1.f);
	Palette.AccentHoverColor = FLinearColor(0.36f, 0.6f, 0.98f, 1.f);
	Palette.NeutralButtonColor = FLinearColor(0.36f, 0.4f, 0.55f, 0.9f);

	Palette.PrimaryCardBrush = FSlateRoundedBoxBrush(Palette.PrimaryCardColor, 18.f);
	Palette.PrimaryToolbarBrush = FSlateRoundedBoxBrush(Palette.PrimaryToolbarColor, 14.f);
	Palette.PrimaryPanelBrush = FSlateRoundedBoxBrush(Palette.PrimaryPanelColor, 10.f);
	Palette.EmphasisCellBrush = FSlateRoundedBoxBrush(FLinearColor(0.16f, 0.2f, 0.35f, 0.7f), 10.f);
	Palette.HighlightedRowBrush = FSlateRoundedBoxBrush(FLinearColor(0.27f, 0.45f, 0.95f, 0.35f), 10.f);
	Palette.InactiveRowBrush = FSlateRoundedBoxBrush(FLinearColor(0.09f, 0.12f, 0.18f, 0.65f), 10.f);
	return Palette;
}

void FSuperManagerPalette::RegisterPaletteBrushes(FSlateStyleSet& StyleSet) const
{
	StyleSet.Set(
		FSuperManagerStyleNames::PrimaryCardBrushName, new FSlateRoundedBoxBrush(PrimaryCardBrush));
	StyleSet.Set(
		FSuperManagerStyleNames::PrimaryToolbarBrushName, new FSlateRoundedBoxBrush(PrimaryToolbarBrush));
	StyleSet.Set(
		FSuperManagerStyleNames::PrimaryPanelBrushName, new FSlateRoundedBoxBrush(PrimaryPanelBrush));
	StyleSet.Set(
		FSuperManagerStyleNames::EmphasisCellBrushName, new FSlateRoundedBoxBrush(EmphasisCellBrush));
	StyleSet.Set(
		FSuperManagerStyleNames::HighlightedRowBrushName, new FSlateRoundedBoxBrush(HighlightedRowBrush));
	StyleSet.Set(
		FSuperManagerStyleNames::InactiveRowBrushName, new FSlateRoundedBoxBrush(InactiveRowBrush));
}

bool FSuperManagerStyleSetRegistry::bIsInitialized = false;
TSharedPtr<FSlateStyleSet> FSuperManagerStyleSetRegistry::RegisteredStyleSet = nullptr;
TUniquePtr<FSuperManagerPalette> FSuperManagerStyleSetRegistry::ActivePalette;

void FSuperManagerStyleSetRegistry::Initialize()
{
	if (bIsInitialized)
	{
		ensureMsgf(false, TEXT("SuperManagerStyleRegistry already initialized."));
		return;
	}

	ActivePalette = MakeUnique<FSuperManagerPalette>(FSuperManagerPalette::CreateDefault());
	RegisteredStyleSet = CreateSlateStyleSet(*ActivePalette);
	FSlateStyleRegistry::RegisterSlateStyle(*RegisteredStyleSet);
	bIsInitialized = true;
}

void FSuperManagerStyleSetRegistry::Shutdown()
{
	if (!bIsInitialized)
	{
		return;
	}

	if (RegisteredStyleSet.IsValid())
	{
		FSlateStyleRegistry::UnRegisterSlateStyle(*RegisteredStyleSet);
		RegisteredStyleSet.Reset();
	}
	ActivePalette.Reset();
	bIsInitialized = false;
}

bool FSuperManagerStyleSetRegistry::IsInitialized()
{
	return bIsInitialized;
}

FName FSuperManagerStyleSetRegistry::GetStyleSetName()
{
	return FSuperManagerStyleNames::StyleSetName;
}

const ISlateStyle& FSuperManagerStyleSetRegistry::Get()
{
	checkf(RegisteredStyleSet.IsValid(), TEXT("SuperManagerStyleRegistry::Get() called before Initialize."));
	return *RegisteredStyleSet.Get();
}

const FSlateBrush* FSuperManagerStyleSetRegistry::GetBrush(const FName BrushName)
{
	return RegisteredStyleSet.IsValid() ? RegisteredStyleSet->GetBrush(BrushName) : nullptr;
}

const FSuperManagerPalette& FSuperManagerStyleSetRegistry::GetPalette()
{
	if (!ActivePalette.IsValid())
	{
		static const FSuperManagerPalette FallbackPalette = FSuperManagerPalette::CreateDefault();
		ensureMsgf(false, TEXT("SuperManager palette requested before initialization."));
		return FallbackPalette;
	}
	return *ActivePalette.Get();
}

TSharedRef<FSlateStyleSet> FSuperManagerStyleSetRegistry::CreateSlateStyleSet(const FSuperManagerPalette& Palette)
{
	TSharedRef<FSlateStyleSet> CustomStyleSet = MakeShared<FSlateStyleSet>(FSuperManagerStyleNames::StyleSetName);
	const TSharedPtr<IPlugin> SuperManagerPlugin = IPluginManager::Get().FindPlugin(TEXT("SuperManager"));
	checkf(SuperManagerPlugin.IsValid(), TEXT("SuperManager plugin descriptor not found when building styles."));
	const FString IconDirectory = SuperManagerPlugin->GetBaseDir() / TEXT("Resources/Icons");
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
			IconDirectory / TEXT("LevelEditorMeneEntryIcons") / TEXT("SelectionLock.png"), Icon16x16));
	CustomStyleSet->Set("LevelEditor.UnlockActorSelection",
		new FSlateImageBrush(
			IconDirectory / TEXT("LevelEditorMeneEntryIcons") / TEXT("SelectionUnlock.png"), Icon16x16));
	CustomStyleSet->Set("LevelEditor.SubMenu",
		new FSlateImageBrush(
			IconDirectory / TEXT("LevelEditorMeneEntryIcons") / TEXT("Superman.png"), Icon16x16));
	CustomStyleSet->Set("SceneOutliner.LockActorSelection",
		new FSlateImageBrush(
			IconDirectory / TEXT("LevelEditorMeneEntryIcons") / TEXT("Lock.png"), Icon16x16));
	CustomStyleSet->Set("SceneOutliner.RowLockedImage",
		new FSlateImageBrush(
			IconDirectory / TEXT("LevelEditorMeneEntryIcons") / TEXT("Locking.png"), Icon16x16));
	CustomStyleSet->Set("SceneOutliner.RowUnlockedImage",
		new FSlateImageBrush(
			IconDirectory / TEXT("LevelEditorMeneEntryIcons") / TEXT("Unlock_w.png"), Icon16x16));

	const FCheckBoxStyle SelectionLockToggleButtonStyle = FCheckBoxStyle()
		.SetCheckBoxType(ESlateCheckBoxType::ToggleButton)
		.SetPadding(FMargin(8.f))
		.SetUncheckedImage(FSlateImageBrush(
			IconDirectory / TEXT("LevelEditorMeneEntryIcons") / TEXT("Unlock_w.png"), Icon8x8,
			FStyleColors::AccentBlack))
		.SetUncheckedHoveredImage(FSlateImageBrush(
			IconDirectory / TEXT("LevelEditorMeneEntryIcons") / TEXT("Unlock_w.png"), Icon16x16,
			FStyleColors::AccentBlue))
		.SetUncheckedPressedImage(FSlateImageBrush(
			IconDirectory / TEXT("LevelEditorMeneEntryIcons") / TEXT("Unlock_w.png"), Icon16x16,
			FStyleColors::AccentWhite))
		.SetCheckedImage(FSlateImageBrush(
			IconDirectory / TEXT("LevelEditorMeneEntryIcons") / TEXT("Locking.png"), Icon16x16,
			FStyleColors::AccentWhite))
		.SetCheckedHoveredImage(FSlateImageBrush(
			IconDirectory / TEXT("LevelEditorMeneEntryIcons") / TEXT("Locking.png"), Icon16x16,
			FStyleColors::White25))
		.SetCheckedPressedImage(FSlateImageBrush(
			IconDirectory / TEXT("LevelEditorMeneEntryIcons") / TEXT("Locking.png"), Icon16x16,
			FStyleColors::Foreground));
	CustomStyleSet->Set("SceneOutliner.SelectionLock", SelectionLockToggleButtonStyle);

	Palette.RegisterPaletteBrushes(*CustomStyleSet);
	return CustomStyleSet;
}
