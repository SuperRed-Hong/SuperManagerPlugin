// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Widgets/SCompoundWidget.h"
#include "AssetRegistry/AssetData.h"

// --- 类声明 ---
class SAdvancedDeletionTab : public SCompoundWidget
{
public:
	// --- 1. SLATE 参数 ---
	SLATE_BEGIN_ARGS(SAdvancedDeletionTab)
		{
		}

		/** Assets data passed in from the manager module to be stored and displayed. */
		SLATE_ARGUMENT(TArray<TSharedPtr<FAssetData>>, AssetsDataToStore)
		SLATE_ARGUMENT(FString, SelectedFolder)
	SLATE_END_ARGS()

	/** Constructs this widget with arguments. */
	void Construct(const FArguments& InArgs);

private:
#pragma region DataMembers

	// --- 1. 数据存储 ---
	/** The main array storing all asset data for the list view. */
	TArray<TSharedPtr<FAssetData>> StoredAssetsData;
	TArray<TSharedPtr<FAssetData>> DisplayedAssetsData;
	/** Assets selected for deletion via checkboxes. */
	TArray<TSharedPtr<FAssetData>> AssetsDataToDeleteArray;
	/** References to all SCheckBox widgets to control their states. */
	TArray<TSharedRef<SCheckBox>> CheckBoxesArray;

	// --- 2. 字体定义 ---
	FSlateFontInfo TittleTextFont = GetEmboseedTextFont(30);
	FSlateFontInfo ButtonTextFont = GetEmboseedTextFont(15);
	// ComboBox Options

#pragma endregion

	// ----------------------------------------------------------------------

#pragma region UIComponents

	/** The actual ListView widget constructed for displaying assets. */
	TSharedPtr<SListView<TSharedPtr<FAssetData>>> ConstructedAssetListView;

#pragma endregion

	// ----------------------------------------------------------------------

#pragma region AssetListView_LogicAndConstruction

	void OnRowWidgetDobuleClicked(TSharedPtr<FAssetData> ClickedAssetData);
	/** Creates and returns the main asset list view widget. */
	TSharedRef<SListView<TSharedPtr<FAssetData>>> ConstructAssetListView();

	/** Delegate to generate a row widget for the list view. */
	TSharedRef<ITableRow> OnGenerateRowForList(TSharedPtr<FAssetData> AssetDataToDisplay,
	                                           const TSharedRef<STableViewBase>& OwnerTable);

	/** Constructs the reusable text block widget for a row. */
	TSharedRef<STextBlock> ConstructTextForRowWidget(const FString& TextContent, const FSlateFontInfo& FontInfo,
	                                                 FColor Color = FColor::White);

	/** Constructs the checkbox for a row. */
	TSharedRef<SCheckBox> ConstructCheckBox(TSharedPtr<FAssetData> AssetDataToDisplay);

	/** Constructs the 'Remove' button for a row. */
	TSharedRef<SButton> ConstructButtonForRowWidget(TSharedPtr<FAssetData> AssetDataToDisplay);

	/** Updates the list view after changes (deletion, selection, etc.). */
	void RefreshAssetListView(bool bIsEmptyAssetsDataToDeleteArray = false);
	void RebuildAssetListView();
#pragma endregion

	// ----------------------------------------------------------------------

#pragma region DelegateCallbacks

	// --- 1. Row Callbacks ---
	/** Handles the state change of a row checkbox. */
	void OnCheckBoxStateChanged(ECheckBoxState NewState, TSharedPtr<FAssetData> AssetData);
	/** Handles the click event for the 'Remove' button on a row. */
	FReply OnRemoveButtonClicked(TSharedPtr<FAssetData> ClickedAssetData);

	// --- 2. Tab Button Callbacks ---
	/** Handles the click event for deleting all selected assets. */
	FReply OnDeleteAllButtonClicked();
	/** Handles the click event for fixing redirectors manually. */
	FReply OnFixRedirectorsButtonClicked();
	/** Handles the click event for selecting all assets. */
	FReply OnSelectAllButtonClicked();
	/** Handles the click event for deselecting all assets. */
	FReply OnDeselectAllButtonClicked();

#pragma endregion

	// ----------------------------------------------------------------------

#pragma region TabButtons_Construction

	/** Constructs the 'Delete All' button widget. */
	TSharedRef<SButton> ConstructDeleteAllButton();
	/** Constructs the 'Fix Redirectors' button widget. */
	TSharedRef<SButton> ConstructFixRedirectorsButton();
	/** Constructs the 'Select All' button widget. */
	TSharedRef<SButton> ConstructSelectAllButton();
	/** Constructs the 'Deselect All' button widget. */
	TSharedRef<SButton> ConstructDeselectAllButton();
	/** Constructs reusable text block for tab control buttons. */
	TSharedRef<STextBlock> ConstructTextForTabButtons(const FString& TextContent, const FSlateFontInfo& FontInfo);

#pragma endregion

	// ----------------------------------------------------------------------

#pragma region HelperFunctions

	/** Static helper to retrieve and configure the embossed style font. */
	static FSlateFontInfo GetEmboseedTextFont(float fontsize = 15)
	{
		FSlateFontInfo FontInfo = FCoreStyle::Get().GetFontStyle(FName("EmbossedText"));
		FontInfo.Size = fontsize;
		return FontInfo;
	}

#pragma endregion

	// ----------------------------------------------------------------------

#pragma region FilteringAndConditionals


	// Placeholder for ComboBox logic, if implemented later
	// TSharedRef<SComboBox<...>> ConstructConditionComboBox();
	TSharedRef<SComboBox<TSharedPtr<FString>>> ConstructConditionComboBox();
	TArray<TSharedPtr<FString>> ComboboxOptions;


	/** 构造 ComboBox 菜单项的委托函数签名 */
	TSharedRef<SWidget> OnGenerateComboboxWidget(TSharedPtr<FString> OptionItem);

	void OnComboBoxSelectionChanged(TSharedPtr<FString> SelectedOption, ESelectInfo::Type InSelectInfo);

	TSharedPtr<STextBlock> ComboBoxDisplayTextBlock;
	TSharedRef<STextBlock> CreateComboBoxHelpTextBlock(const FString& TextContent, ETextJustify::Type Justification);

#pragma endregion
};
