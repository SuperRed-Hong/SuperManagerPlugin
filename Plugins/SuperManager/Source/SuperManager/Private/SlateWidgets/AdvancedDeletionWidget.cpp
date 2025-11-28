// Fill out your copyright notice in the Description page of Project Settings.


#include "SlateWidgets/AdvancedDeletionWidget.h"
#include "SlateBasics.h"
#include "EditorStyleSet.h"
#include "DebugHeader.h"
#include "SuperManager.h"
#include "SuperManagerEnums.h"
#include "Widgets/Views/SListView.h"
#include "Widgets/Views/STableViewBase.h" 
#include "Widgets/Views/STableRow.h"
#include "Widgets/Text/STextBlock.h"
#include "SlateExtras.h"
#include "Trace/Trace.inl"

#define ListAll TEXT("List All Avaliable Assets")
#define ListUnused TEXT("List Unused Assets")
#define ListSameNameAssets TEXT("List Same Name Assets")
void SAdvancedDeletionTab::Construct(const FArguments& InArgs)
{
	bCanSupportFocus = true;

	StoredAssetsData = InArgs._AssetsDataToStore;
	DisplayedAssetsData = StoredAssetsData;

	CheckBoxesArray.Empty();
	AssetsDataToDeleteArray.Empty();
	ComboboxOptions.Empty();
	
	ComboboxOptions.Add(MakeShared<FString>(ListAll));
	ComboboxOptions.Add(MakeShared<FString>(ListUnused));
	ComboboxOptions.Add(MakeShared<FString>(ListSameNameAssets));
	
	ChildSlot
	[
		//Main vertical box
		SNew(SVerticalBox)
		//First vertical slot for title text
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(STextBlock)
			.Text(FText::FromString(TEXT("Advanced Deletion")))
			.Font(TittleTextFont)
			.Justification(ETextJustify::Center)
			.ColorAndOpacity(FColor::White)
			//.Font(FEditorStyle::GetFontStyle("BoldFont"))
		]
		//Second Slot for drop down combobox slot to specify the listing condition

		+ SVerticalBox::Slot()
		.AutoHeight()
		//.VAlign(VAlign_Fill)
		[
			SNew(SHorizontalBox)
			// combobox slot
			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				ConstructConditionComboBox()
			]
			//help text for combo box slot
			+ SHorizontalBox::Slot()
			.FillWidth(.6f)
			[
				CreateComboBoxHelpTextBlock(TEXT("Specify the listing condition in the drop down."), ETextJustify::Center)
			]
			//Help Text for folder path
			+ SHorizontalBox::Slot()
			.FillWidth(.1f)
			[
				CreateComboBoxHelpTextBlock(TEXT("Folder Path: \n") + InArgs._SelectedFolder , ETextJustify::Right)
			]

		]
		//Third slot for the asset list
		+ SVerticalBox::Slot()
		.VAlign(VAlign_Fill)
		[
			SNew(SScrollBox)
			+ SScrollBox::Slot()
			[
				ConstructAssetListView()
			]
		]
		//Fourth slot for 3 buttons
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SHorizontalBox)
			//Button1 Slot
			+ SHorizontalBox::Slot()
			.FillWidth(1.0f)
			.Padding(5.f)
			[
				ConstructDeleteAllButton()
			]
			//Button for fixing redirectors
			+ SHorizontalBox::Slot()
			.FillWidth(1.0f)
			.Padding(5.f)
			[
				ConstructFixRedirectorsButton()
			]
			//Button2 Slot

			+ SHorizontalBox::Slot()
			.FillWidth(1.0f)
			.Padding(5.f)
			[
				ConstructSelectAllButton()
			]
			//Button3 Slot
			+ SHorizontalBox::Slot()
			.FillWidth(1.0f)
			.Padding(5.f)
			[
				ConstructDeselectAllButton()
			]
		]
	];
}
#pragma region FilteringAndConditionals


TSharedRef<SComboBox<TSharedPtr<FString>>> SAdvancedDeletionTab::ConstructConditionComboBox()
{
	TSharedRef<SComboBox<TSharedPtr<FString>>> ConstructedComboBox =
		SNew(SComboBox<TSharedPtr<FString>>)
		.OptionsSource(&ComboboxOptions)
		.OnGenerateWidget(this, &SAdvancedDeletionTab::OnGenerateComboboxWidget)
		.OnSelectionChanged(this, &SAdvancedDeletionTab::OnComboBoxSelectionChanged)
		[
			SAssignNew(ComboBoxDisplayTextBlock, STextBlock)
			.Text(FText::FromString(TEXT("List Assets Option")))
		];
		

	return ConstructedComboBox;
}

TSharedRef<SWidget> SAdvancedDeletionTab::OnGenerateComboboxWidget(TSharedPtr<FString> OptionItem)
{
	TSharedRef<SWidget> ConstructedComboBoxText =
		SNew(STextBlock).Text(FText::FromString(*OptionItem.Get()))
		.ApplyLineHeightToBottomLine(true);

	return ConstructedComboBoxText;
}
TSharedRef<STextBlock> SAdvancedDeletionTab::CreateComboBoxHelpTextBlock(const FString& TextContent,
	ETextJustify::Type Justification)
{
	TSharedRef<STextBlock> ConstructedTextBlock =
		SNew(STextBlock)
		.Text(FText::FromString(TextContent))
		.Justification(Justification)
		.AutoWrapText(true);
		
	return ConstructedTextBlock;
}
void SAdvancedDeletionTab::OnComboBoxSelectionChanged(TSharedPtr<FString> SelectedOption,
	ESelectInfo::Type InSelectInfo)
{
	DebugHeader::Print(*SelectedOption.Get(),FColor::Cyan);
	ComboBoxDisplayTextBlock->SetText(FText::FromString(*SelectedOption.Get()));
	FSuperManagerModule&	SuperManagerModule =
		FModuleManager::Get().LoadModuleChecked<FSuperManagerModule>(TEXT("SuperManager"));
	
	//Pass stored data to module
	
	if (*SelectedOption.Get() == ListAll)
	{
		SuperManagerModule.UpdateDisplayedData(StoredAssetsData, DisplayedAssetsData, EComboBoxOptions::E_ListAll);
		RebuildAssetListView();
		
		// List all stored asset data
	}
	else if (*SelectedOption.Get() == ListUnused)
	{
		//List all unused assets
		SuperManagerModule.UpdateDisplayedData(StoredAssetsData, DisplayedAssetsData, EComboBoxOptions::E_ListUnused);
		RebuildAssetListView();
	}
	else if (*SelectedOption.Get() == ListSameNameAssets)
	{
		//List Same Name Assets
		SuperManagerModule.UpdateDisplayedData(StoredAssetsData, DisplayedAssetsData, EComboBoxOptions::E_ListSameNameAssets);
		RebuildAssetListView();
	}
	

	
}


#pragma endregion
#pragma region TabButtons
TSharedRef<SButton> SAdvancedDeletionTab::ConstructDeleteAllButton()
{
	TSharedRef<SButton> DeleteAllButton = SNew(SButton)
		.ContentPadding(FMargin(5.f))
		.OnClicked(this, &SAdvancedDeletionTab::OnDeleteAllButtonClicked);
	DeleteAllButton->SetContent(ConstructTextForTabButtons(TEXT("Delete All"), ButtonTextFont));
	return DeleteAllButton;
}

TSharedRef<SButton> SAdvancedDeletionTab::ConstructFixRedirectorsButton()
{
	TSharedRef<SButton> FixRedirectorsButton = SNew(SButton)
		.ContentPadding(FMargin(5.f))
		.OnClicked(this, &SAdvancedDeletionTab::OnFixRedirectorsButtonClicked);
	FixRedirectorsButton->SetContent(ConstructTextForTabButtons(TEXT("Fix Redirectors"), ButtonTextFont));
	return FixRedirectorsButton;
}

TSharedRef<SButton> SAdvancedDeletionTab::ConstructSelectAllButton()
{
	TSharedRef<SButton> SelectAllButton = SNew(SButton)
		.ContentPadding(FMargin(5.f))
		.OnClicked(this, &SAdvancedDeletionTab::OnSelectAllButtonClicked);
	SelectAllButton->SetContent(ConstructTextForTabButtons(TEXT("Select All"), ButtonTextFont));
	return SelectAllButton;
}

TSharedRef<SButton> SAdvancedDeletionTab::ConstructDeselectAllButton()
{
	TSharedRef<SButton> DeselectAllButton = SNew(SButton)
		.ContentPadding(FMargin(5.f))
		.OnClicked(this, &SAdvancedDeletionTab::OnDeselectAllButtonClicked);
	DeselectAllButton->SetContent(ConstructTextForTabButtons(TEXT("Deselect All"), ButtonTextFont));
	return DeselectAllButton;
}

FReply SAdvancedDeletionTab::OnDeleteAllButtonClicked()
{
	if (AssetsDataToDeleteArray.Num() == 0)
	{
		DebugHeader::ShowMsgDialog(EAppMsgType::Ok, TEXT("Please select some assets first."), true);
		return FReply::Handled();
	}

	//pass data to our deletion
	TArray<FAssetData> AssetsToDelete;
	for (const TSharedPtr<FAssetData>& AssetData : AssetsDataToDeleteArray)
	{
		AssetsToDelete.AddUnique(*AssetData.Get());
	}
	if (AssetsToDelete.Num() > 0)
	{
		DebugHeader::ShowNotifyInfo(TEXT("Deleting selected assets..."));
		FSuperManagerModule& SuperManagerModule =
			FModuleManager::LoadModuleChecked<FSuperManagerModule>(TEXT("SuperManager"));
		const bool bAssetsDeleted = SuperManagerModule.DeleteAssetsForAssetList(AssetsToDelete);
		if (bAssetsDeleted)
		{
			for (const TSharedPtr<FAssetData>& AssetData : AssetsDataToDeleteArray)
			{
				StoredAssetsData.Remove(AssetData);
				DisplayedAssetsData.Remove(AssetData);
			}
			RefreshAssetListView(true);
		}
		DebugHeader::ShowNotifyInfo(bAssetsDeleted ? TEXT("Finished deleting selected assets.") : TEXT("Failed to delete selected assets."));
	}
	return FReply::Handled();
}

FReply SAdvancedDeletionTab::OnFixRedirectorsButtonClicked()
{
	FSuperManagerModule& SuperManagerModule =
		FModuleManager::LoadModuleChecked<FSuperManagerModule>(TEXT("SuperManager"));
	SuperManagerModule.ForceFixUpRedirectors();
	DebugHeader::ShowNotifyInfo(TEXT("Finished fixing redirectors under /Game."));
	return FReply::Handled();
}

FReply SAdvancedDeletionTab::OnSelectAllButtonClicked()
{
	if (CheckBoxesArray.Num() == 0)
	{
		return FReply::Handled();
	}
	for (const TSharedRef<SCheckBox>& CheckBox : CheckBoxesArray)
	{
		if (!CheckBox->IsChecked())
		{
			CheckBox->ToggleCheckedState();
		}
	}
	return FReply::Handled();
}

FReply SAdvancedDeletionTab::OnDeselectAllButtonClicked()
{
	for (const TSharedRef<SCheckBox>& CheckBox : CheckBoxesArray)
	{
		if (CheckBox->IsChecked())
		{
			CheckBox->ToggleCheckedState();
		}
	}
	return FReply::Handled();
}

TSharedRef<STextBlock> SAdvancedDeletionTab::ConstructTextForTabButtons(const FString& TextContent,
                                                                        const FSlateFontInfo& FontInfo)
{
	TSharedRef<STextBlock> ConstructedTextBlock = SNew(STextBlock)
		.Text(FText::FromString(TextContent))
		.Font(FontInfo)
		.Justification(ETextJustify::Center)
		.ColorAndOpacity(FColor::White);
	return ConstructedTextBlock;
}
#pragma endregion

#pragma region Row Widgets For Asset List View
TSharedRef<SListView<TSharedPtr<FAssetData>>> SAdvancedDeletionTab::ConstructAssetListView()
{
	// 1. 构建 SHeaderRow
	/*
	TSharedRef<SHeaderRow> HeaderRow = 
		SNew(SHeaderRow)

		// 第一列：Checkbox 列 (宽度很小)
		+ SHeaderRow::Column(TEXT("Selection")) // Header ID: Selection
		.DefaultLabel(FText::GetEmpty())         // 标题行不显示文本，只占位
		.HAlignHeader(HAlign_Left)
		.VAlignHeader(VAlign_Center)
		.ManualWidth(20.0f) // 固定小宽度，与您的 checkbox 匹配

		// 第二列：Asset Class (资产类名)
		+ SHeaderRow::Column(TEXT("AssetClass")) // Header ID: AssetClass
		.DefaultLabel(FText::FromString(TEXT("Asset Class"))) // 列标题文本
		.HAlignHeader(HAlign_Center)
		.VAlignHeader(VAlign_Center)
		.FillWidth(0.5f) // 填充宽度 0.5f，与您的第二列匹配

		// 第三列：Asset Name (资产名称)
		+ SHeaderRow::Column(TEXT("AssetName")) // Header ID: AssetName
		.DefaultLabel(FText::FromString(TEXT("Asset Name"))) // 列标题文本
		.HAlignHeader(HAlign_Left) // 资产名称通常靠左对齐
		.VAlignHeader(VAlign_Center)
		.FillWidth(1.0f) // 填充宽度 1.0f，占据剩余空间

		// 第四列：Remove Button (按钮操作)
		+ SHeaderRow::Column(TEXT("Action")) // Header ID: Action
		.DefaultLabel(FText::FromString(TEXT("Action")))
		.HAlignHeader(HAlign_Right) // 按钮操作通常靠右对齐
		.VAlignHeader(VAlign_Center)
		.ManualWidth(60.0f); // 按钮使用固定宽度，例如 60.0f
		*/

	
	ConstructedAssetListView = SNew(SListView<TSharedPtr<FAssetData>>)

		.ListItemsSource(&DisplayedAssetsData)
		.OnGenerateRow(this, &SAdvancedDeletionTab::OnGenerateRowForList)
		.OnMouseButtonDoubleClick(this, &SAdvancedDeletionTab::OnRowWidgetDobuleClicked);
	//	.HeaderRow(HeaderRow);
	
	return ConstructedAssetListView.ToSharedRef();
}

TSharedRef<ITableRow> SAdvancedDeletionTab::OnGenerateRowForList(TSharedPtr<FAssetData> AssetDataToDisplay,
                                                                 const TSharedRef<STableViewBase>& OwnerTable)
{
	if (!AssetDataToDisplay.IsValid()) return SNew(STableRow<TSharedPtr<FAssetData>>, OwnerTable);

	const FString DisplayAssetName = AssetDataToDisplay->AssetName.ToString();
	const FString DisplayAssetClassName = AssetDataToDisplay->AssetClassPath.GetAssetName().ToString();

	FSlateFontInfo AssetClassNameFont = GetEmboseedTextFont();
	AssetClassNameFont.Size = 10;
	FSlateFontInfo AssetNameFont = GetEmboseedTextFont();
	AssetNameFont.Size = 15;

	TSharedRef<STableRow<TSharedPtr<FAssetData>>> ListViewRowWidget =
		SNew(STableRow<TSharedPtr<FAssetData>>, OwnerTable)
		.Padding(FMargin(0.5f, 0.0f, 0.0f, 0.0f))
		
		[
			SNew(SHorizontalBox)

			//First slot for check box
			
			+ SHorizontalBox::Slot()
			.HAlign(HAlign_Left)
			.VAlign(VAlign_Center)
			.FillWidth(0.05f)
			[
				ConstructCheckBox(AssetDataToDisplay)
			]

			//Second slot for displaying asset class name
			+ SHorizontalBox::Slot()
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Fill)
			.FillWidth(0.5f)
			[
				ConstructTextForRowWidget(DisplayAssetClassName, AssetClassNameFont)
			]
			//Third slot for displaying asset name
			+ SHorizontalBox::Slot()
			[
				ConstructTextForRowWidget(DisplayAssetName, AssetNameFont)

			]

			// Fourth slot for a button
			+ SHorizontalBox::Slot()
			.HAlign(HAlign_Right)
			.VAlign(VAlign_Fill)
			[
				ConstructButtonForRowWidget(AssetDataToDisplay)
			]
		];

	return ListViewRowWidget;
}

void SAdvancedDeletionTab::OnRowWidgetDobuleClicked(TSharedPtr<FAssetData> ClickedAssetData)
{
	if (ClickedAssetData.IsValid())
	{
		DebugHeader::Print(ClickedAssetData->AssetName.ToString() + TEXT(" is clicked"), FColor::Cyan);
		DebugHeader::PrintLog(ClickedAssetData->AssetName.ToString() + TEXT(" is clicked"));
		FSuperManagerModule& SuperManagerModule =
			FModuleManager::LoadModuleChecked<FSuperManagerModule>(TEXT("SuperManager"));
		
		SuperManagerModule.SyncCBToClickedAssetForAssetList(ClickedAssetData->GetObjectPathString());
		
	}
}
void SAdvancedDeletionTab::RefreshAssetListView(bool bIsEmptyAssetsDataToDeleteArray)
{
	if (bIsEmptyAssetsDataToDeleteArray) { AssetsDataToDeleteArray.Empty(); }

	if (ConstructedAssetListView.IsValid())
	{
		ConstructedAssetListView->RequestListRefresh();
	}
}

void SAdvancedDeletionTab::RebuildAssetListView()
{

	if (ConstructedAssetListView.IsValid())
	{
		AssetsDataToDeleteArray.Empty();
		CheckBoxesArray.Empty();
		ConstructedAssetListView->RebuildList();	
	}
}

TSharedRef<SCheckBox> SAdvancedDeletionTab::ConstructCheckBox(TSharedPtr<FAssetData> AssetDataToDisplay)
{
	TSharedRef<SCheckBox> ConstructedCheckBox =
		SNew(SCheckBox)
		.Type(ESlateCheckBoxType::CheckBox)
		.OnCheckStateChanged(this, &SAdvancedDeletionTab::OnCheckBoxStateChanged, AssetDataToDisplay)
		.Visibility(EVisibility::Visible);

	CheckBoxesArray.AddUnique(ConstructedCheckBox);

	return ConstructedCheckBox;
}


void SAdvancedDeletionTab::OnCheckBoxStateChanged(ECheckBoxState NewState, TSharedPtr<FAssetData> AssetData)
{
	switch (NewState)
	{
	case ECheckBoxState::Unchecked:
		if (AssetsDataToDeleteArray.Contains(AssetData))
		{
			AssetsDataToDeleteArray.Remove(AssetData);
		}
		break;
	case ECheckBoxState::Checked:
		AssetsDataToDeleteArray.AddUnique(AssetData);
		break;
	case ECheckBoxState::Undetermined:
		break;
	default: ;
	}
}


TSharedRef<STextBlock> SAdvancedDeletionTab::ConstructTextForRowWidget(const FString& TextContent,
                                                                       const FSlateFontInfo& FontInfo,
                                                                       FColor Color)
{
	TSharedRef<STextBlock> TextBlock =
		SNew(STextBlock)
		.Text(FText::FromString(TextContent))
		.Font(FontInfo)
		.ColorAndOpacity(Color);

	return TextBlock;
}

TSharedRef<SButton> SAdvancedDeletionTab::ConstructButtonForRowWidget(TSharedPtr<FAssetData> AssetDataToDisplay)
{
	TSharedRef<SButton> ConstructedButton =
		SNew(SButton)
		.Text(FText::FromString(TEXT("Remove")))
		.OnClicked(this, &SAdvancedDeletionTab::OnRemoveButtonClicked, AssetDataToDisplay);
	return ConstructedButton;
}

FReply SAdvancedDeletionTab::OnRemoveButtonClicked(TSharedPtr<FAssetData> ClickedAssetData)
{
	if (!ClickedAssetData.IsValid())
	{
		return FReply::Handled();
	}
	TRACE_CPUPROFILER_EVENT_SCOPE(SAdvancedDeletionTab_RemoveSingleAsset);
	TRACE_BOOKMARK(TEXT("AdvancedDeletion_RemoveSingle"));
	FSuperManagerModule& SuperManagerModule =
		FModuleManager::LoadModuleChecked<FSuperManagerModule>(TEXT("SuperManager"));
	DebugHeader::ShowNotifyInfo(TEXT("Deleting selected asset..."));
	const bool bAssetDeleted = SuperManagerModule.DeleteSingleAssetForAssetList(*ClickedAssetData.Get());
	if (bAssetDeleted)
	{
		StoredAssetsData.Remove(ClickedAssetData);
		DisplayedAssetsData.Remove(ClickedAssetData);
		AssetsDataToDeleteArray.Remove(ClickedAssetData);
		if (ConstructedAssetListView.IsValid())
		{
			ConstructedAssetListView->RequestListRefresh();
		}
		DebugHeader::ShowNotifyInfo(TEXT("Asset deleted successfully."));
	}
	else
	{
		DebugHeader::ShowNotifyInfo(TEXT("Failed to delete asset."));
	}
	return FReply::Handled();
}
#pragma endregion
