// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Brushes/SlateRoundedBoxBrush.h"
#include "Styling/SlateStyle.h"

class FSuperManagerStyle 
{
public:
	static void InitializeIcons();
	static void Shutdown();
	static FName GetStylesSetName() {return IconStyleSetName;}
	static const ISlateStyle& Get();
	static const FSlateBrush* GetBrush(const FName BrushName);

	struct FLockedActorsListStyle
	{
		static FLinearColor GetCardColor();
		static FLinearColor GetToolbarColor();
		static FLinearColor GetControlColor();
		static FLinearColor GetAccentColor();
		static FLinearColor GetAccentHoverColor();
		static FLinearColor GetNeutralButtonColor();

		static const FSlateRoundedBoxBrush& GetGlassCardBrush();
		static const FSlateRoundedBoxBrush& GetToolbarBrush();
		static const FSlateRoundedBoxBrush& GetControlBrush();
		static const FSlateRoundedBoxBrush& GetLockCellBrush();
		static const FSlateRoundedBoxBrush& GetRowActiveBrush();
		static const FSlateRoundedBoxBrush& GetRowInactiveBrush();
	};
private:
	static FName IconStyleSetName;

	static TSharedRef<FSlateStyleSet> CreateSlateStyleSet();
	static TSharedPtr<FSlateStyleSet> CreatedSlateStyleSet;
};
