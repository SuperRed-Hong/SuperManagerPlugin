#include "EditorUI/StageEditorPanel.h"

#include <DebugHeader.h>

#include "Widgets/Text/STextBlock.h"
#include "Editor.h"
#include "Engine/Blueprint.h"
#include "Subsystems/AssetEditorSubsystem.h"
#include "Selection.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "PropertyEditorModule.h"
#include "IStructureDetailsView.h"
#include "LevelEditor.h"
#include "ScopedTransaction.h"
#include "SceneOutlinerDragDrop.h"
#include "ActorTreeItem.h"

#define LOCTEXT_NAMESPACE "SStageEditorPanel"

void SStageEditorPanel::Construct(const FArguments& InArgs, TSharedPtr<FStageEditorController> InController)
{
	Controller = InController;

	// Bind to Controller updates
	if (Controller.IsValid())
	{
		Controller->OnModelChanged.AddSP(this, &SStageEditorPanel::RefreshUI);
	}
	
	// Create Creation Settings struct
	CreationSettings = MakeShared<FStructOnScope>(FAssetCreationSettings::StaticStruct());
	FAssetCreationSettings* Settings = (FAssetCreationSettings*)CreationSettings->GetStructMemory();
	*Settings = FAssetCreationSettings(); // Initialize with defaults
	
	// Create Details View for creation settings
	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	FStructureDetailsViewArgs StructureViewArgs;
	FDetailsViewArgs DetailsViewArgs;
	DetailsViewArgs.bAllowSearch = false;
	DetailsViewArgs.bHideSelectionTip = true;
	SettingsDetailsView = PropertyModule.CreateStructureDetailView(DetailsViewArgs, StructureViewArgs, CreationSettings);

	ChildSlot
	[
		SNew(SVerticalBox)
		
		// Toolbar
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(5)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				SNew(SButton)
				.Text(LOCTEXT("CreateAct", "Create Act"))
				.OnClicked(this, &SStageEditorPanel::OnCreateActClicked)
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(5, 0, 0, 0)
			[
				SNew(SButton)
				.Text(LOCTEXT("RegisterProps", "Register Selected Props"))
				.OnClicked(this, &SStageEditorPanel::OnRegisterSelectedPropsClicked)
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(5, 0, 0, 0)
			[
				SNew(SButton)
				.Text(LOCTEXT("Refresh", "Refresh"))
				.OnClicked(this, &SStageEditorPanel::OnRefreshClicked)
				.ToolTipText(LOCTEXT("Refresh_Tooltip", "Manually scan for Stages in the level"))
			]
		]
		
		// Quick Create Toolbar
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(5, 0, 5, 5)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				SNew(SButton)
				.Text(LOCTEXT("CreateStageBP", "Create Stage BP"))
				.OnClicked(this, &SStageEditorPanel::OnCreateStageBPClicked)
				.ToolTipText(LOCTEXT("CreateStageBP_Tooltip", "Create a new Stage Blueprint in Content Browser"))
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(5, 0, 0, 0)
			[
				SNew(SButton)
				.Text(LOCTEXT("CreatePropBP", "Create Prop BP"))
				.OnClicked(this, &SStageEditorPanel::OnCreatePropBPClicked)
				.ToolTipText(LOCTEXT("CreatePropBP_Tooltip", "Create a new Prop Blueprint in Content Browser"))
			]
		]
		
		// Settings Panel
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(5)
		[
			SNew(SBorder)
			.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
			.Padding(5)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(STextBlock)
					.Text(LOCTEXT("AssetCreationSettings", "Asset Creation Settings"))
					.Font(FCoreStyle::GetDefaultFontStyle("Bold", 10))
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0, 5, 0, 0)
				[
					SettingsDetailsView->GetWidget().ToSharedRef()
				]
			]
		]

		// Tree View
		+ SVerticalBox::Slot()
		.FillHeight(1.0f)
		[
			SNew(SBorder)
			.Padding(2)
			[
				SAssignNew(StageTreeView, STreeView<TSharedPtr<FStageTreeItem>>)
				.TreeItemsSource(&RootTreeItems)
				.OnGenerateRow(this, &SStageEditorPanel::OnGenerateRow)
				.OnGetChildren(this, &SStageEditorPanel::OnGetChildren)
				.OnSelectionChanged(this, &SStageEditorPanel::OnSelectionChanged)
				.OnContextMenuOpening(this, &SStageEditorPanel::OnContextMenuOpening)
			]
		]
	];

	RefreshUI();

	// Initialize Controller (scan for level actors) AFTER UI is built
	if (Controller.IsValid())
	{
		Controller->Initialize();
	}
}

void SStageEditorPanel::RefreshUI()
{
	if (!Controller.IsValid()) return;
	
	RootTreeItems.Empty();

	const TArray<TWeakObjectPtr<AStage>>& FoundStages = Controller->GetFoundStages();

	for (const auto& StagePtr : FoundStages)
	{
		if (AStage* Stage = StagePtr.Get())
		{
			// Create Stage Root Item
			// Create Stage Root Item
			FString StageName = Stage->GetActorLabel();
			TSharedPtr<FStageTreeItem> StageItem = MakeShared<FStageTreeItem>(EStageTreeItemType::Stage, StageName, Stage->StageID, nullptr, Stage);
			RootTreeItems.Add(StageItem);

			// 1. Create "Acts" Folder
			TSharedPtr<FStageTreeItem> ActsFolder = MakeShared<FStageTreeItem>(EStageTreeItemType::ActsFolder, TEXT("Acts"));
			StageItem->Children.Add(ActsFolder);
			ActsFolder->Parent = StageItem;

			// Populate Acts
			for (FAct& Act : Stage->Acts)
			{
				TSharedPtr<FStageTreeItem> ActItem = MakeShared<FStageTreeItem>(EStageTreeItemType::Act, Act.DisplayName, Act.ActID.ActID);
				ActsFolder->Children.Add(ActItem);
				ActItem->Parent = ActsFolder;

				// Populate Props in Act
				for (auto& Pair : Act.PropStateOverrides)
				{
					int32 PropID = Pair.Key;
					int32 State = Pair.Value;
					
					AActor* PropActor = Stage->GetPropByID(PropID);
					FString PropName = PropActor ? PropActor->GetActorLabel() : TEXT("Invalid Prop");
					FString DisplayName = FString::Printf(TEXT("%s (State: %d)"), *PropName, State);

					TSharedPtr<FStageTreeItem> PropItem = MakeShared<FStageTreeItem>(EStageTreeItemType::Prop, DisplayName, PropID, PropActor);
					ActItem->Children.Add(PropItem);
					PropItem->Parent = ActItem;
				}
			}

			// 2. Create "Registered Props" Folder
			TSharedPtr<FStageTreeItem> PropsFolder = MakeShared<FStageTreeItem>(EStageTreeItemType::PropsFolder, TEXT("Registered Props"));
			StageItem->Children.Add(PropsFolder);
			PropsFolder->Parent = StageItem;

			// Populate All Props
			for (auto& Pair : Stage->PropRegistry)
			{
				if (AActor* Actor = Pair.Value.Get())
				{
					FString DisplayName = FString::Printf(TEXT("%s (ID: %d)"), *Actor->GetActorLabel(), Pair.Key);
					TSharedPtr<FStageTreeItem> PropItem = MakeShared<FStageTreeItem>(EStageTreeItemType::Prop, DisplayName, Pair.Key, Actor);
					PropsFolder->Children.Add(PropItem);
					PropItem->Parent = PropsFolder;
				}
			}
		}
	}

	StageTreeView->RequestTreeRefresh();
	
	// Expand root folders by default
	for (const auto& Item : RootTreeItems)
	{
		StageTreeView->SetItemExpansion(Item, true);
	}
}

TSharedRef<ITableRow> SStageEditorPanel::OnGenerateRow(TSharedPtr<FStageTreeItem> Item, const TSharedRef<STableViewBase>& OwnerTable)
{
	// Select appropriate icon based on item type
	const FSlateBrush* IconBrush = nullptr;
	FSlateColor IconColor = FSlateColor::UseForeground();
	
	switch (Item->Type)
	{
	case EStageTreeItemType::Stage:
		IconBrush = FAppStyle::GetBrush("LevelEditor.Tabs.Levels");
		IconColor = FSlateColor(FLinearColor(0.3f, 0.6f, 1.0f)); // Blue
		break;
	case EStageTreeItemType::ActsFolder:
		IconBrush = FAppStyle::GetBrush("ContentBrowser.AssetTreeFolderClosed");
		IconColor = FSlateColor(FLinearColor(0.5f, 1.0f, 0.5f)); // Green (matching Acts)
		break;
	case EStageTreeItemType::PropsFolder:
		IconBrush = FAppStyle::GetBrush("ContentBrowser.AssetTreeFolderClosed");
		IconColor = FSlateColor(FLinearColor(1.0f, 0.6f, 0.3f)); // Orange (matching Props)
		break;
	case EStageTreeItemType::Act:
		IconBrush = FAppStyle::GetBrush("Sequencer.Tracks.Event");
		IconColor = FSlateColor(FLinearColor(0.5f, 1.0f, 0.5f)); // Green
		break;
	case EStageTreeItemType::Prop:
		IconBrush = FAppStyle::GetBrush("ClassIcon.StaticMeshActor");
		IconColor = FSlateColor(FLinearColor(1.0f, 0.6f, 0.3f)); // Orange
		break;
	}

	return SNew(STableRow<TSharedPtr<FStageTreeItem>>, OwnerTable)
	[
		SNew(SHorizontalBox)
		// Icon
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.VAlign(VAlign_Center)
		.Padding(2, 0, 4, 0)
		[
			SNew(SImage)
			.Image(IconBrush)
			.ColorAndOpacity(IconColor)
		]
		// Text
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.VAlign(VAlign_Center)
		.Padding(2)
		[
			SNew(STextBlock)
			.Text(FText::FromString(Item->DisplayName))
			.Font((Item->Type == EStageTreeItemType::ActsFolder || Item->Type == EStageTreeItemType::PropsFolder) 
				? FCoreStyle::GetDefaultFontStyle("Bold", 10) 
				: FCoreStyle::GetDefaultFontStyle("Regular", 10))
		]
	];
}

void SStageEditorPanel::OnGetChildren(TSharedPtr<FStageTreeItem> Item, TArray<TSharedPtr<FStageTreeItem>>& OutChildren)
{
	OutChildren = Item->Children;
}

void SStageEditorPanel::OnSelectionChanged(TSharedPtr<FStageTreeItem> Item, ESelectInfo::Type SelectInfo)
{
	if (Item.IsValid() && Controller.IsValid())
	{
		// Handle Active Stage Selection
		if (Item->Type == EStageTreeItemType::Stage)
		{
			if (AStage* Stage = Item->StagePtr.Get())
			{
				Controller->SetActiveStage(Stage);
			}
		}
		else if (Item->Type == EStageTreeItemType::Act)
		{
			Controller->PreviewAct(Item->ID);
			
			// Also set active stage from parent if possible (simplified for now, assuming user selects Stage first)
			// Ideally we traverse up to find the Stage
		}
	}
}

FReply SStageEditorPanel::OnCreateActClicked()
{
	if (Controller.IsValid())
	{
		Controller->CreateNewAct();
	}
	return FReply::Handled();
}

FReply SStageEditorPanel::OnRegisterSelectedPropsClicked()
{
	if (Controller.IsValid() && GEditor)
	{
		TArray<AActor*> SelectedActors;
		GEditor->GetSelectedActors()->GetSelectedObjects<AActor>(SelectedActors);
		Controller->RegisterProps(SelectedActors);
	}
	return FReply::Handled();
}

FReply SStageEditorPanel::OnCreateStageBPClicked()
{
	if (Controller.IsValid() && CreationSettings.IsValid())
	{
		FAssetCreationSettings* Settings = (FAssetCreationSettings*)CreationSettings->GetStructMemory();
		
		FString Path;
		if (Settings->bIsCustomStageAssetFolderPath)
		{
			// Convert physical path to virtual path
			FString PhysicalPath = Settings->StageAssetFolderPath.Path;
			FString ProjectContentDir = FPaths::ProjectContentDir();
			
			if (PhysicalPath.StartsWith(ProjectContentDir))
			{
				FString RelativePath = PhysicalPath.RightChop(ProjectContentDir.Len());
				Path = TEXT("/Game/") + RelativePath;
			}
			else
			{
				// Assume it's already a virtual path
				Path = PhysicalPath;
			}
		}
		else
		{
			Path = TEXT("/StageEditor/StagesBP");
		}
		
		Controller->CreateStageBlueprint(Path);
	}
	return FReply::Handled();
}

FReply SStageEditorPanel::OnCreatePropBPClicked()
{
	if (Controller.IsValid() && CreationSettings.IsValid())
	{
		FAssetCreationSettings* Settings = (FAssetCreationSettings*)CreationSettings->GetStructMemory();
		
		FString Path;
		if (Settings->bIsCustomPropAssetPath)
		{
			// Convert physical path to virtual path
			FString PhysicalPath = Settings->PropAssetFolderPath.Path;
			FString ProjectContentDir = FPaths::ProjectContentDir();
			
			if (PhysicalPath.StartsWith(ProjectContentDir))
			{
				FString RelativePath = PhysicalPath.RightChop(ProjectContentDir.Len());
				Path = TEXT("/Game/") + RelativePath;
			}
			else
			{
				// Assume it's already a virtual path
				Path = PhysicalPath;
			}
		}
		else
		{
			Path = TEXT("/StageEditor/PropsBP");
		}
		
		Controller->CreatePropBlueprint(Path);
	}
	return FReply::Handled();
}

FReply SStageEditorPanel::OnRefreshClicked()
{
	if (Controller.IsValid())
	{
		Controller->FindStageInWorld();
	}
	return FReply::Handled();
}

TSharedPtr<SWidget> SStageEditorPanel::OnContextMenuOpening()
{
	if (!Controller.IsValid()) return nullptr;

	TArray<TSharedPtr<FStageTreeItem>> SelectedItems;
	StageTreeView->GetSelectedItems(SelectedItems);

	if (SelectedItems.Num() == 0) return nullptr;

	TSharedPtr<FStageTreeItem> Item = SelectedItems[0];
	if (!Item.IsValid()) return nullptr;

	FMenuBuilder MenuBuilder(true, nullptr);

	if (Item->Type == EStageTreeItemType::Stage)
	{
		MenuBuilder.AddMenuEntry(
			LOCTEXT("RegisterSelectedActors", "Register Selected Actors"),
			LOCTEXT("RegisterSelectedActors_Tooltip", "Register currently selected level actors to this Stage"),
			FSlateIcon(),
			FUIAction(FExecuteAction::CreateLambda([this, Item]()
			{
				if (Controller.IsValid() && Item->StagePtr.IsValid())
				{
					Controller->SetActiveStage(Item->StagePtr.Get());
					// Trigger the register logic
					OnRegisterSelectedPropsClicked(); 
				}
			}))
		);

		MenuBuilder.AddMenuEntry(
			LOCTEXT("EditStageBlueprint", "Edit Blueprint"),
			LOCTEXT("EditStageBlueprint_Tooltip", "Open the Blueprint Editor for this Stage"),
			FSlateIcon(),
			FUIAction(FExecuteAction::CreateLambda([this, Item]()
			{
				if (Item->StagePtr.IsValid())
				{
					if (UBlueprint* Blueprint = Cast<UBlueprint>(Item->StagePtr->GetClass()->ClassGeneratedBy))
					{
						GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()->OpenEditorForAsset(Blueprint);
					}
				}
			}))
		);
	}

	return MenuBuilder.MakeWidget();
}

//----------------------------------------------------------------
// Drag & Drop Support
//----------------------------------------------------------------

void SStageEditorPanel::OnDragEnter(const FGeometry& MyGeometry, const FDragDropEvent& DragDropEvent)
{
	SCompoundWidget::OnDragEnter(MyGeometry, DragDropEvent);
}

void SStageEditorPanel::OnDragLeave(const FDragDropEvent& DragDropEvent)
{
	SCompoundWidget::OnDragLeave(DragDropEvent);
}

FReply SStageEditorPanel::OnDragOver(const FGeometry& MyGeometry, const FDragDropEvent& DragDropEvent)
{
	// Check if this is an Actor drag operation from World Outliner
	TSharedPtr<FDragDropOperation> Operation = DragDropEvent.GetOperation();
	if (Operation.IsValid() && Operation->IsOfType<FActorDragDropOp>())
	{
		return FReply::Handled();
	}
	
	return FReply::Unhandled();
}

FReply SStageEditorPanel::OnDrop(const FGeometry& MyGeometry, const FDragDropEvent& DragDropEvent)
{
	TSharedPtr<FDragDropOperation> Operation = DragDropEvent.GetOperation();
	if (!Operation.IsValid() || !Operation->IsOfType<FActorDragDropOp>())
	{
		return FReply::Unhandled();
	}

	TSharedPtr<FActorDragDropOp> ActorDragDrop = StaticCastSharedPtr<FActorDragDropOp>(Operation);
	if (!ActorDragDrop.IsValid())
	{
		return FReply::Unhandled();
	}

	// Extract actors from drag operation
	TArray<AActor*> ActorsToRegister;
	for (TWeakObjectPtr<AActor> WeakActor : ActorDragDrop->Actors)
	{
		if (AActor* Actor = WeakActor.Get())
		{
			ActorsToRegister.Add(Actor);
		}
	}

	// Register actors as props
	if (ActorsToRegister.Num() > 0 && Controller.IsValid())
	{
		Controller->RegisterProps(ActorsToRegister);
		DebugHeader::ShowNotifyInfo(FString::Printf(TEXT("Registered %d actors via Drag & Drop"), ActorsToRegister.Num()));
	}

	return FReply::Handled();
}

#undef LOCTEXT_NAMESPACE
