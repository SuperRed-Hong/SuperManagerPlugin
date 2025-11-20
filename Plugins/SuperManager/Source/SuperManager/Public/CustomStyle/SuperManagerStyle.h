#pragma once

#include "Brushes/SlateRoundedBoxBrush.h"
#include "Styling/SlateStyle.h"
#include "Templates/UniquePtr.h"

/**
 * 集中管理 SuperManager Style 名称，避免硬编码字符串。
 */
struct FSuperManagerStyleNames
{
	static const FName StyleSetName;
	static const FName PrimaryCardBrushName;
	static const FName PrimaryToolbarBrushName;
	static const FName PrimaryPanelBrushName;
	static const FName EmphasisCellBrushName;
	static const FName HighlightedRowBrushName;
	static const FName InactiveRowBrushName;
};

/**
 * SuperManager 调色盘，集中维护颜色与 Brush 数据。
 */
struct FSuperManagerPalette
{
	/** 构造调色盘，确保 Brush 成员获得有效初始值。 */
	FSuperManagerPalette();

	/** 创建默认调色盘配置。 */
	static FSuperManagerPalette CreateDefault();

	FLinearColor PrimaryCardColor;
	FLinearColor PrimaryToolbarColor;
	FLinearColor PrimaryPanelColor;
	FLinearColor AccentColor;
	FLinearColor AccentHoverColor;
	FLinearColor NeutralButtonColor;

	FSlateRoundedBoxBrush PrimaryCardBrush;
	FSlateRoundedBoxBrush PrimaryToolbarBrush;
	FSlateRoundedBoxBrush PrimaryPanelBrush;
	FSlateRoundedBoxBrush EmphasisCellBrush;
	FSlateRoundedBoxBrush HighlightedRowBrush;
	FSlateRoundedBoxBrush InactiveRowBrush;

	/** 按命名规范将 Brush 注册到指定 Style Set。 */
	void RegisterPaletteBrushes(FSlateStyleSet& StyleSet) const;
};

/**
 * Style 注册表，负责 FSlateStyleSet 的生命周期与访问接口。
 */
class FSuperManagerStyleSetRegistry
{
public:
	/** 在编辑器启动时注册 Style Set（仅执行一次）。 */
	static void Initialize();

	/** 在模块卸载时注销 Style Set 并清理 Palette。 */
	static void Shutdown();

	/**
	 * 判断 Style 是否已初始化。
	 * @return 如果已经调用 Initialize 并成功注册，则为 true
	 */
	static bool IsInitialized();

	/**
	 * 返回 Style Set 名称，可用于 FSlateIcon。
	 * @return SuperManager 的 Style Set 名称
	 */
	static FName GetStyleSetName();

	/**
	 * 获取 Slate Style 引用，访问 Widget 样式与 Brush。
	 */
	static const ISlateStyle& Get();

	/**
	 * 根据名称读取 Brush，未初始化时返回 nullptr。
	 * @param BrushName 已注册的 Brush 键
	 */
	static const FSlateBrush* GetBrush(const FName BrushName);

	/**
	 * 返回调色盘引用，供控件读取颜色值。
	 */
	static const FSuperManagerPalette& GetPalette();

private:
	static TSharedRef<FSlateStyleSet> CreateSlateStyleSet(const FSuperManagerPalette& Palette);

	static bool bIsInitialized;
	static TSharedPtr<FSlateStyleSet> RegisteredStyleSet;
	static TUniquePtr<FSuperManagerPalette> ActivePalette;
};
