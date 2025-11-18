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

	/** 深色卡片背景，适用于面板根布局。 */
	static const FLinearColor PrimaryCardColor;
	/** 工具栏/标题背景色。 */
	static const FLinearColor PrimaryToolbarColor;
	/** 控件容器背景色。 */
	static const FLinearColor PrimaryPanelColor;
	/** 主强调色，按钮/链接使用。 */
	static const FLinearColor AccentColor;
	/** 鼠标 Hover 时的强调色。 */
	static const FLinearColor AccentHoverColor;
	/** 中性色按钮背景，用于刷新等动作。 */
	static const FLinearColor NeutralButtonColor;

	/** 对应卡片背景的圆角 Brush。 */
	static const FSlateRoundedBoxBrush PrimaryCardBrush;
	/** 工具栏专用圆角 Brush。 */
	static const FSlateRoundedBoxBrush PrimaryToolbarBrush;
	/** 控件容器圆角 Brush。 */
	static const FSlateRoundedBoxBrush PrimaryPanelBrush;
	/** 强调单元格背景（如锁按钮）。 */
	static const FSlateRoundedBoxBrush EmphasisCellBrush;
	/** 列表选中/高亮行背景。 */
	static const FSlateRoundedBoxBrush HighlightedRowBrush;
	/** 列表未选中行背景。 */
	static const FSlateRoundedBoxBrush InactiveRowBrush;
private:
	static FName IconStyleSetName;

	static TSharedRef<FSlateStyleSet> CreateSlateStyleSet();
	static TSharedPtr<FSlateStyleSet> CreatedSlateStyleSet;
};
