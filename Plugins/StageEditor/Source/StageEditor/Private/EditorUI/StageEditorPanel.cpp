#include "EditorUI/StageEditorPanel.h"

#include <DebugHeader.h>

#include "Widgets/Text/STextBlock.h"
#include "Editor.h"
#include "Engine/Blueprint.h"
#include "Subsystems/AssetEditorSubsystem.h"
#include "Input/Events.h"
#include "IStructureDetailsView.h"
#include "Engine/Selection.h"
#include "DragAndDrop/ActorDragDropOp.h"
#include "Widgets/Input/SNumericEntryBox.h"
#include "Templates/UnrealTemplate.h"

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
				.OnMouseButtonDoubleClick(this, &SStageEditorPanel::OnRowDoubleClicked)
			]
		]
	];

	RefreshUI();

	RegisterViewportSelectionListener();

	// Initialize Controller (scan for level actors) AFTER UI is built
	if (Controller.IsValid())
	{
		Controller->Initialize();
	}
}

SStageEditorPanel::~SStageEditorPanel()
{
	UnregisterViewportSelectionListener();
}

void SStageEditorPanel::RefreshUI()
{
	if (!Controller.IsValid()) return;
	
	// Save Expansion State
	TSet<FString> ExpansionState;
	SaveExpansionState(ExpansionState);

	RootTreeItems.Empty();
	ActorPathToTreeItem.Empty();

	const TArray<TWeakObjectPtr<AStage>>& FoundStages = Controller->GetFoundStages();

	for (const auto& StagePtr : FoundStages)
	{
		if (AStage* Stage = StagePtr.Get())
		{
			// Create Stage Root Item
			FString StageName = Stage->GetActorLabel();
			TSharedPtr<FStageTreeItem> StageItem = MakeShared<FStageTreeItem>(EStageTreeItemType::Stage, StageName, Stage->StageID, nullptr, Stage);
			StageItem->ActorPtr = Stage;
			StageItem->ActorPath = Stage->GetPathName();
			RootTreeItems.Add(StageItem);
			if (!StageItem->ActorPath.IsEmpty())
			{
				ActorPathToTreeItem.Add(StageItem->ActorPath, StageItem);
			}

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
					PropItem->StagePtr = Stage;
					PropItem->PropState = State;
					PropItem->bHasPropState = true;
					if (PropActor)
					{
						PropItem->ActorPath = PropActor->GetPathName();
						if (!PropItem->ActorPath.IsEmpty() && !ActorPathToTreeItem.Contains(PropItem->ActorPath))
						{
							ActorPathToTreeItem.Add(PropItem->ActorPath, PropItem);
						}
					}
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
					PropItem->StagePtr = Stage;
					PropItem->ActorPath = Actor->GetPathName();
					if (!PropItem->ActorPath.IsEmpty() && !ActorPathToTreeItem.Contains(PropItem->ActorPath))
					{
						ActorPathToTreeItem.Add(PropItem->ActorPath, PropItem);
					}
				}
			}
		}
	}

	StageTreeView->RequestTreeRefresh();
	
	// Restore Expansion State
	if (ExpansionState.Num() > 0)
	{
		RestoreExpansionState(ExpansionState);
	}
	else
	{
		// Default expansion for first load
		for (const auto& Item : RootTreeItems)
		{
			StageTreeView->SetItemExpansion(Item, true);
		}
	}

	HandleViewportSelectionChanged(nullptr);
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

	TSharedPtr<FStageTreeItem> ParentItem = Item->Parent.Pin();
	const bool bParentIsAct = ParentItem.IsValid() && ParentItem->Type == EStageTreeItemType::Act;
	const bool bParentIsPropsFolder = ParentItem.IsValid() && ParentItem->Type == EStageTreeItemType::PropsFolder;

	TSharedRef<SHorizontalBox> RowContent = SNew(SHorizontalBox)
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
		];

	if (Item->Type == EStageTreeItemType::Prop && bParentIsAct && Item->bHasPropState)
	{
		RowContent->AddSlot()
		.AutoWidth()
		.VAlign(VAlign_Center)
		.Padding(8, 0, 0, 0)
		[
			SNew(SNumericEntryBox<int32>)
			.Value(Item->PropState)
			.AllowSpin(true)
			.Delta(1)
			.MinDesiredValueWidth(50.0f)
			.ToolTipText(LOCTEXT("PropStateInlineEdit_Tooltip", "Adjust the Prop state applied within this Act"))
			.OnValueCommitted_Lambda([this, Item, ParentItem](int32 NewValue, ETextCommit::Type)
			{
				ApplyPropStateChange(Item, ParentItem, NewValue);
			})
		];
	}

	// Add spacer to push action buttons to the right
	RowContent->AddSlot()
	.FillWidth(1.0f);

	// Add Create Act button for Acts folder
	if (Item->Type == EStageTreeItemType::ActsFolder)
	{
		RowContent->AddSlot()
		.AutoWidth()
		.VAlign(VAlign_Center)
		.Padding(8, 0, 0, 0)  // Increased padding to prevent accidental clicks
		[
			SNew(SButton)
			.ButtonStyle(FAppStyle::Get(), "SimpleButton")
			.ToolTipText(LOCTEXT("CreateActInline_Tooltip", "Create a new Act in this Stage"))
			.OnClicked_Lambda([this, Item]()
			{
				if (Controller.IsValid() && Item->Parent.IsValid())
				{
					TSharedPtr<FStageTreeItem> ParentStage = Item->Parent.Pin();
					if (ParentStage.IsValid() && ParentStage->StagePtr.IsValid())
					{
						Controller->SetActiveStage(ParentStage->StagePtr.Get());
						Controller->CreateNewAct();
					}
				}
				return FReply::Handled();
			})
			[
				SNew(SImage)
				.Image(FAppStyle::GetBrush("Icons.Plus"))
				.ColorAndOpacity(FSlateColor::UseForeground())
			]
		];
	}

	// Add Delete button for Act items (except Default Act)
	if (Item->Type == EStageTreeItemType::Act && Item->ID != 0)
	{
		RowContent->AddSlot()
		.AutoWidth()
		.VAlign(VAlign_Center)
		.Padding(8, 0, 0, 0)  // Increased padding to prevent accidental clicks
		[
			SNew(SButton)
			.ButtonStyle(FAppStyle::Get(), "SimpleButton")
			.ToolTipText(LOCTEXT("DeleteActInline_Tooltip", "Delete this Act"))
			.OnClicked_Lambda([this, Item]()
			{
				if (Controller.IsValid())
				{
					// Find parent Stage
					TSharedPtr<FStageTreeItem> CurrentItem = Item;
					while (CurrentItem.IsValid())
					{
						if (CurrentItem->Type == EStageTreeItemType::Stage && CurrentItem->StagePtr.IsValid())
						{
							Controller->SetActiveStage(CurrentItem->StagePtr.Get());
							
							// Confirmation dialog
							FText ConfirmTitle = LOCTEXT("ConfirmDeleteAct", "Confirm Delete Act");
							FText ConfirmMessage = FText::Format(
								LOCTEXT("ConfirmDeleteActMsg", "Are you sure you want to delete Act '{0}'?"),
								FText::FromString(Item->DisplayName)
							);
							
							EAppReturnType::Type Result = FMessageDialog::Open(EAppMsgType::YesNo, ConfirmMessage, ConfirmTitle);
							if (Result == EAppReturnType::Yes)
							{
								Controller->DeleteAct(Item->ID);
							}
							break;
						}
						CurrentItem = CurrentItem->Parent.Pin();
					}
				}
				return FReply::Handled();
			})
			[
				SNew(SImage)
				.Image(FAppStyle::GetBrush("Icons.Delete"))
				.ColorAndOpacity(FSlateColor::UseForeground())
			]
		];
	}

	// Add Delete button for Prop items (context-aware)
	if (Item->Type == EStageTreeItemType::Prop && (bParentIsAct || bParentIsPropsFolder))
	{
		FText TooltipText = bParentIsAct
			? LOCTEXT("RemovePropFromActInline_Tooltip", "Remove this Prop from the Act")
			: LOCTEXT("UnregisterPropInline_Tooltip", "Unregister this Prop from the Stage");

		RowContent->AddSlot()
		.AutoWidth()
		.VAlign(VAlign_Center)
		.Padding(8, 0, 0, 0)
		[
			SNew(SButton)
			.ButtonStyle(FAppStyle::Get(), "SimpleButton")
			.ToolTipText(TooltipText)
			.OnClicked_Lambda([this, Item, ParentItem, bParentIsAct]()
			{
				if (Controller.IsValid())
				{
					TSharedPtr<FStageTreeItem> CurrentItem = bParentIsAct ? ParentItem : Item;
					while (CurrentItem.IsValid())
					{
						if (CurrentItem->Type == EStageTreeItemType::Stage && CurrentItem->StagePtr.IsValid())
						{
							Controller->SetActiveStage(CurrentItem->StagePtr.Get());

							if (bParentIsAct)
							{
								Controller->RemovePropFromAct(Item->ID, ParentItem->ID);
							}
							else
							{
								FText ConfirmTitle = LOCTEXT("ConfirmUnregisterPropInline", "Confirm Unregister");
								FText ConfirmMessage = FText::Format(
									LOCTEXT("ConfirmUnregisterPropInlineMsg", "Are you sure you want to unregister '{0}' from the Stage?\n\nThis will remove it from all Acts."),
									FText::FromString(Item->DisplayName)
								);

								EAppReturnType::Type Result = FMessageDialog::Open(EAppMsgType::YesNo, ConfirmMessage, ConfirmTitle);
								if (Result == EAppReturnType::Yes)
								{
									Controller->UnregisterProp(Item->ID);
								}
							}
							break;
						}
						CurrentItem = CurrentItem->Parent.Pin();
					}
				}
				return FReply::Handled();
			})
			[
				SNew(SImage)
				.Image(FAppStyle::GetBrush("Icons.Delete"))
				.ColorAndOpacity(FSlateColor::UseForeground())
			]
		];
	}

	// Enable dragging for Props in Registered Props folder
	
	// Determine if this row should be highlighted during drag
	// Use a lambda attribute so the color updates dynamically when DraggedOverItem changes
	TSharedRef<STableRow<TSharedPtr<FStageTreeItem>>> TableRow = SNew(STableRow<TSharedPtr<FStageTreeItem>>, OwnerTable)
	.OnDrop_Lambda([this, Item](const FDragDropEvent& DragDropEvent) -> FReply
	{
		return OnRowDrop(FGeometry(), DragDropEvent, Item);
	})
	.OnDragEnter_Lambda([this, Item](const FDragDropEvent& DragDropEvent)
	{
		OnRowDragEnter(DragDropEvent, Item);
	})
	.OnDragLeave_Lambda([this, Item](const FDragDropEvent& DragDropEvent)
	{
		OnRowDragLeave(DragDropEvent, Item);
	})
	.OnDragDetected_Lambda([this, Item, bParentIsPropsFolder](const FGeometry&, const FPointerEvent&) -> FReply
	{
		// Only allow dragging Props from Registered Props folder
		if (Item->Type == EStageTreeItemType::Prop && bParentIsPropsFolder)
		{
			// Get selected items for multi-select drag
			TArray<TSharedPtr<FStageTreeItem>> SelectedItems;
			if (StageTreeView.IsValid())
			{
				StageTreeView->GetSelectedItems(SelectedItems);
				
				// Filter to only include Props from Registered Props folder
				SelectedItems = SelectedItems.FilterByPredicate([](const TSharedPtr<FStageTreeItem>& SelectedItem) {
					if (!SelectedItem.IsValid()) return false;
					if (SelectedItem->Type != EStageTreeItemType::Prop) return false;
					TSharedPtr<FStageTreeItem> Parent = SelectedItem->Parent.Pin();
					return Parent.IsValid() && Parent->Type == EStageTreeItemType::PropsFolder;
				});
				
				// If current item is not in selection, use just current item
				if (!SelectedItems.Contains(Item))
				{
					SelectedItems.Empty();
					SelectedItems.Add(Item);
				}
			}
			else
			{
				SelectedItems.Add(Item);
			}
			
			if (SelectedItems.Num() > 0)
			{
				// Create a custom drag-drop operation with PropIDs
				TSharedRef<FPropDragDropOp> DragOp = MakeShared<FPropDragDropOp>();
				DragOp->PropItems = SelectedItems;
				
				// Create drag visual
				FString DragText = SelectedItems.Num() == 1 
					? SelectedItems[0]->DisplayName 
					: FString::Printf(TEXT("%d Props"), SelectedItems.Num());
				
				DragOp->DefaultHoverText = FText::FromString(DragText);
				DragOp->CurrentHoverText = DragOp->DefaultHoverText;
				
				return FReply::Handled().BeginDragDrop(DragOp);
			}
		}
		
		return FReply::Unhandled();
	})
	[
		SNew(SBorder)
		.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
		.BorderBackgroundColor_Lambda([this, Item]() -> FSlateColor
		{
			// Highlight the Stage and all its descendants when being dragged over
			bool bIsDragTarget = IsItemOrDescendantOf(Item, DraggedOverItem);
			// Use white color with 80% opacity for better visibility
			return bIsDragTarget ? FLinearColor(1.0f, 1.0f, 1.0f, 0.8f) : FLinearColor::Transparent;
		})
		.Padding(2)
		[
			RowContent
		]
	];

	return TableRow;
}

void SStageEditorPanel::OnGetChildren(TSharedPtr<FStageTreeItem> Item, TArray<TSharedPtr<FStageTreeItem>>& OutChildren)
{
	OutChildren = Item->Children;
}

void SStageEditorPanel::OnSelectionChanged(TSharedPtr<FStageTreeItem> Item, ESelectInfo::Type SelectInfo)
{
	if (bUpdatingTreeSelectionFromViewport || !Item.IsValid() || !Controller.IsValid())
	{
		return;
	}

	if (Item->Type == EStageTreeItemType::Stage)
	{
		if (AStage* Stage = Item->StagePtr.Get())
		{
			Controller->SetActiveStage(Stage);
			SelectActorInViewport(Stage);
		}
	}
	else if (Item->Type == EStageTreeItemType::Act)
	{
		if (TSharedPtr<FStageTreeItem> StageItem = FindStageAncestor(Item))
		{
			if (StageItem->StagePtr.IsValid())
			{
				Controller->SetActiveStage(StageItem->StagePtr.Get());
			}
		}
		Controller->PreviewAct(Item->ID);
	}
	else if (Item->Type == EStageTreeItemType::Prop)
	{
		if (TSharedPtr<FStageTreeItem> StageItem = FindStageAncestor(Item))
		{
			if (StageItem->StagePtr.IsValid())
			{
				Controller->SetActiveStage(StageItem->StagePtr.Get());
			}
		}

		if (AActor* PropActor = Item->ActorPtr.Get())
		{
			SelectActorInViewport(PropActor);
		}
	}
}

void SStageEditorPanel::OnRowDoubleClicked(TSharedPtr<FStageTreeItem> Item)
{
	if (!Item.IsValid()) return;

	// Focus on the actor in viewport when double-clicked
	AActor* ActorToFocus = nullptr;
	
	if (Item->Type == EStageTreeItemType::Stage && Item->StagePtr.IsValid())
	{
		ActorToFocus = Item->StagePtr.Get();
	}
	else if (Item->Type == EStageTreeItemType::Prop && Item->ActorPtr.IsValid())
	{
		ActorToFocus = Item->ActorPtr.Get();
	}
	
	if (ActorToFocus && GEditor)
	{
		// Select the actor
		GEditor->SelectNone(false, true);
		GEditor->SelectActor(ActorToFocus, true, true);
		
		// Focus viewport on the actor
		GEditor->MoveViewportCamerasToActor(*ActorToFocus, false);
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

	//----------------------------------------------------------------
	// Stage Context Menu
	//----------------------------------------------------------------
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

		MenuBuilder.AddMenuSeparator();

		MenuBuilder.AddMenuEntry(
			LOCTEXT("UnregisterAllProps", "Unregister All Props"),
			LOCTEXT("UnregisterAllProps_Tooltip", "Unregister all Props from this Stage"),
			FSlateIcon(),
			FUIAction(FExecuteAction::CreateLambda([this, Item]()
			{
				if (Controller.IsValid() && Item->StagePtr.IsValid())
				{
					// Confirmation dialog
					FText ConfirmTitle = LOCTEXT("ConfirmUnregisterAll", "Confirm Unregister All");
					FText ConfirmMessage = LOCTEXT("ConfirmUnregisterAllMsg", "Are you sure you want to unregister all Props from this Stage?\n\nThis will remove them from all Acts.");
					
					EAppReturnType::Type Result = FMessageDialog::Open(EAppMsgType::YesNo, ConfirmMessage, ConfirmTitle);
					if (Result == EAppReturnType::Yes)
					{
						Controller->SetActiveStage(Item->StagePtr.Get());
						Controller->UnregisterAllProps();
					}
				}
			}))
		);
	}

	//----------------------------------------------------------------
	// Act Context Menu
	//----------------------------------------------------------------
	else if (Item->Type == EStageTreeItemType::Act)
	{
		MenuBuilder.AddMenuEntry(
			LOCTEXT("CreateDataLayer", "Create Data Layer"),
			LOCTEXT("CreateDataLayer_Tooltip", "Create a new Data Layer for this Act"),
			FSlateIcon(FAppStyle::GetAppStyleSetName(), "LevelEditor.Tabs.DataLayers"),
			FUIAction(FExecuteAction::CreateLambda([this, Item]()
			{
				if (Controller.IsValid())
				{
					// Find parent Stage
					TSharedPtr<FStageTreeItem> CurrentItem = Item;
					while (CurrentItem.IsValid())
					{
						if (CurrentItem->Type == EStageTreeItemType::Stage && CurrentItem->StagePtr.IsValid())
						{
							Controller->SetActiveStage(CurrentItem->StagePtr.Get());
							Controller->CreateDataLayerForAct(Item->ID);
							break;
						}
						CurrentItem = CurrentItem->Parent.Pin();
					}
				}
			}))
		);

		MenuBuilder.AddMenuSeparator();

		MenuBuilder.AddMenuEntry(
			LOCTEXT("RemoveAllPropsFromAct", "Remove All Props from Act"),
			LOCTEXT("RemoveAllPropsFromAct_Tooltip", "Remove all Props from this Act (they remain registered to the Stage)"),
			FSlateIcon(),
			FUIAction(FExecuteAction::CreateLambda([this, Item]()
			{
				if (Controller.IsValid())
				{
					// Find parent Stage
					TSharedPtr<FStageTreeItem> CurrentItem = Item;
					while (CurrentItem.IsValid())
					{
						if (CurrentItem->Type == EStageTreeItemType::Stage && CurrentItem->StagePtr.IsValid())
						{
							Controller->SetActiveStage(CurrentItem->StagePtr.Get());
							Controller->RemoveAllPropsFromAct(Item->ID);
							break;
						}
						CurrentItem = CurrentItem->Parent.Pin();
					}
				}
			}))
		);

		// Don't allow deleting Default Act (ID 0)
		if (Item->ID != 0)
		{
			MenuBuilder.AddMenuSeparator();

			MenuBuilder.AddMenuEntry(
				LOCTEXT("DeleteAct", "Delete Act"),
				LOCTEXT("DeleteAct_Tooltip", "Delete this Act"),
				FSlateIcon(FAppStyle::GetAppStyleSetName(), "Icons.Delete"),
				FUIAction(FExecuteAction::CreateLambda([this, Item]()
				{
					if (Controller.IsValid())
					{
						// Find parent Stage
						TSharedPtr<FStageTreeItem> CurrentItem = Item;
						while (CurrentItem.IsValid())
						{
							if (CurrentItem->Type == EStageTreeItemType::Stage && CurrentItem->StagePtr.IsValid())
							{
								Controller->SetActiveStage(CurrentItem->StagePtr.Get());
								
								// Confirmation dialog
								FText ConfirmTitle = LOCTEXT("ConfirmDeleteActMenu", "Confirm Delete Act");
								FText ConfirmMessage = FText::Format(
									LOCTEXT("ConfirmDeleteActMenuMsg", "Are you sure you want to delete Act '{0}'?"),
									FText::FromString(Item->DisplayName)
								);
								
								EAppReturnType::Type Result = FMessageDialog::Open(EAppMsgType::YesNo, ConfirmMessage, ConfirmTitle);
								if (Result == EAppReturnType::Yes)
								{
									Controller->DeleteAct(Item->ID);
								}
								break;
							}
							CurrentItem = CurrentItem->Parent.Pin();
						}
					}
				}))
			);
		}
	}

	//----------------------------------------------------------------
	// Prop Context Menu
	//----------------------------------------------------------------
	else if (Item->Type == EStageTreeItemType::Prop)
	{
		// Check if Prop is under an Act or under Registered Props folder
		TSharedPtr<FStageTreeItem> ParentItem = Item->Parent.Pin();
		bool bIsInAct = ParentItem.IsValid() && ParentItem->Type == EStageTreeItemType::Act;

		if (bIsInAct)
		{
			// Prop is under an Act - show Act-specific operations
			MenuBuilder.AddMenuEntry(
				LOCTEXT("SetPropState", "Set State..."),
				LOCTEXT("SetPropState_Tooltip", "Change the state value of this Prop in the Act"),
				FSlateIcon(),
				FUIAction(FExecuteAction::CreateLambda([this, Item, ParentItem]()
				{
					if (Controller.IsValid())
					{
						// Simple input dialog
						TSharedRef<SWindow> InputWindow = SNew(SWindow)
							.Title(LOCTEXT("SetStateTitle", "Set Prop State"))
							.SizingRule(ESizingRule::Autosized)
							.SupportsMinimize(false)
							.SupportsMaximize(false);

						TSharedPtr<SEditableTextBox> TextBox;
						
						InputWindow->SetContent(
							SNew(SBorder)
							.Padding(10)
							[
								SNew(SVerticalBox)
								+ SVerticalBox::Slot()
								.AutoHeight()
								.Padding(0, 0, 0, 10)
								[
									SNew(STextBlock)
									.Text(LOCTEXT("EnterStateValue", "Enter new state value:"))
								]
								+ SVerticalBox::Slot()
								.AutoHeight()
								.Padding(0, 0, 0, 10)
								[
									SAssignNew(TextBox, SEditableTextBox)
									.Text(FText::FromString("0"))
								]
								+ SVerticalBox::Slot()
								.AutoHeight()
								[
									SNew(SHorizontalBox)
									+ SHorizontalBox::Slot()
									.FillWidth(1.0f)
									.HAlign(HAlign_Right)
									[
										SNew(SButton)
										.Text(LOCTEXT("OK", "OK"))
										.OnClicked_Lambda([this, Item, ParentItem, TextBox, InputWindow]()
										{
											FString InputText = TextBox->GetText().ToString();
											int32 NewState = FCString::Atoi(*InputText);
											
											// Find parent Stage
											TSharedPtr<FStageTreeItem> CurrentItem = ParentItem;
											while (CurrentItem.IsValid())
											{
												if (CurrentItem->Type == EStageTreeItemType::Stage && CurrentItem->StagePtr.IsValid())
												{
													Controller->SetActiveStage(CurrentItem->StagePtr.Get());
													Controller->SetPropStateInAct(Item->ID, ParentItem->ID, NewState);
													break;
												}
												CurrentItem = CurrentItem->Parent.Pin();
											}
											
											InputWindow->RequestDestroyWindow();
											return FReply::Handled();
										})
									]
									+ SHorizontalBox::Slot()
									.AutoWidth()
									.Padding(5, 0, 0, 0)
									[
										SNew(SButton)
										.Text(LOCTEXT("Cancel", "Cancel"))
										.OnClicked_Lambda([InputWindow]()
										{
											InputWindow->RequestDestroyWindow();
											return FReply::Handled();
										})
									]
								]
							]
						);

						FSlateApplication::Get().AddModalWindow(InputWindow, FSlateApplication::Get().GetActiveTopLevelWindow());
					}
				}))
			);

			MenuBuilder.AddMenuEntry(
				LOCTEXT("RemovePropFromAct", "Remove from Act"),
				LOCTEXT("RemovePropFromAct_Tooltip", "Remove this Prop from the Act (it remains registered to the Stage)"),
				FSlateIcon(),
				FUIAction(FExecuteAction::CreateLambda([this, Item, ParentItem]()
				{
					if (Controller.IsValid())
					{
						// Find parent Stage
						TSharedPtr<FStageTreeItem> CurrentItem = ParentItem;
						while (CurrentItem.IsValid())
						{
							if (CurrentItem->Type == EStageTreeItemType::Stage && CurrentItem->StagePtr.IsValid())
							{
								Controller->SetActiveStage(CurrentItem->StagePtr.Get());
								Controller->RemovePropFromAct(Item->ID, ParentItem->ID);
								break;
							}
							CurrentItem = CurrentItem->Parent.Pin();
						}
					}
				}))
			);

			MenuBuilder.AddMenuSeparator();
		}

		// Unregister from Stage (available for both Act and Registered Props contexts)
		MenuBuilder.AddMenuEntry(
			LOCTEXT("UnregisterProp", "Unregister from Stage"),
			LOCTEXT("UnregisterProp_Tooltip", "Completely unregister this Prop from the Stage (removes from all Acts)"),
			FSlateIcon(),
			FUIAction(FExecuteAction::CreateLambda([this, Item]()
			{
				if (Controller.IsValid())
				{
					// Confirmation dialog
					FText ConfirmTitle = LOCTEXT("ConfirmUnregister", "Confirm Unregister");
					FText ConfirmMessage = FText::Format(
						LOCTEXT("ConfirmUnregisterMsg", "Are you sure you want to unregister '{0}' from the Stage?\n\nThis will remove it from all Acts."),
						FText::FromString(Item->DisplayName)
					);
					
					EAppReturnType::Type Result = FMessageDialog::Open(EAppMsgType::YesNo, ConfirmMessage, ConfirmTitle);
					if (Result == EAppReturnType::Yes)
					{
						// Find parent Stage
						TSharedPtr<FStageTreeItem> CurrentItem = Item;
						while (CurrentItem.IsValid())
						{
							if (CurrentItem->Type == EStageTreeItemType::Stage && CurrentItem->StagePtr.IsValid())
							{
								Controller->SetActiveStage(CurrentItem->StagePtr.Get());
								Controller->UnregisterProp(Item->ID);
								break;
							}
							CurrentItem = CurrentItem->Parent.Pin();
						}
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
	// Fallback drop handler (e.g. dropping on empty space)
	// For now, we can just ignore it or default to active stage
	return FReply::Unhandled();
}

/**
 * @brief Handles drop events on tree view rows
 * @details Processes actor drops from World Outliner, registers them to the appropriate
 *          Stage, and provides user feedback via notifications.
 * @param MyGeometry The geometry of the widget (unused, passed as default)
 * @param DragDropEvent The drag and drop event containing the dragged actors
 * @param TargetItem The tree item where the drop occurred
 * @return FReply::Handled() if drop was processed, FReply::Unhandled() otherwise
 */
FReply SStageEditorPanel::OnRowDrop(const FGeometry& MyGeometry, const FDragDropEvent& DragDropEvent, TSharedPtr<FStageTreeItem> TargetItem)
{
	// Clear drag highlight when drop occurs
	DraggedOverItem.Reset();

	TSharedPtr<FDragDropOperation> Operation = DragDropEvent.GetOperation();
	if (!Operation.IsValid())
	{
		return FReply::Unhandled();
	}

	// Handle Prop Drag Drop (Internal)
	if (Operation->IsOfType<FPropDragDropOp>())
	{
		TSharedPtr<FPropDragDropOp> PropDragDrop = StaticCastSharedPtr<FPropDragDropOp>(Operation);
		
		// Identify Target Act
		int32 TargetActID = -1;
		if (TargetItem->Type == EStageTreeItemType::Act)
		{
			TargetActID = TargetItem->ID;
		}
		else if (TargetItem->Type == EStageTreeItemType::Prop)
		{
			TSharedPtr<FStageTreeItem> Parent = TargetItem->Parent.Pin();
			if (Parent.IsValid() && Parent->Type == EStageTreeItemType::Act)
			{
				TargetActID = Parent->ID;
			}
		}

		if (TargetActID != -1 && Controller.IsValid())
		{
			// Find parent Stage to ensure we are in the correct context
			TSharedPtr<FStageTreeItem> Current = TargetItem;
			while (Current.IsValid())
			{
				if (Current->Type == EStageTreeItemType::Stage && Current->StagePtr.IsValid())
				{
					Controller->SetActiveStage(Current->StagePtr.Get());
					break;
				}
				Current = Current->Parent.Pin();
			}

			int32 AddedCount = 0;
			for (const TSharedPtr<FStageTreeItem>& PropItem : PropDragDrop->PropItems)
			{
				if (PropItem.IsValid() && PropItem->ID != -1)
				{
					// Add prop to Act with default state 0
					// SetPropStateInAct will add it if not present, or update if present
					if (Controller->SetPropStateInAct(PropItem->ID, TargetActID, 0))
					{
						AddedCount++;
					}
				}
			}
			
			if (AddedCount > 0)
			{
				DebugHeader::ShowNotifyInfo(FString::Printf(TEXT("Added %d Props to Act"), AddedCount));
			}
			
			return FReply::Handled();
		}
		
		return FReply::Unhandled();
	}

	// Handle Actor Drag Drop (From World Outliner)
	if (!Operation->IsOfType<FActorDragDropOp>())
	{
		return FReply::Unhandled();
	}

	TSharedPtr<FActorDragDropOp> ActorDragDrop = StaticCastSharedPtr<FActorDragDropOp>(Operation);
	if (!ActorDragDrop.IsValid())
	{
		return FReply::Unhandled();
	}

	// Find Target Stage from the dropped item
	AStage* TargetStage = nullptr;
	TSharedPtr<FStageTreeItem> Current = TargetItem;
	while (Current.IsValid())
	{
		if (Current->Type == EStageTreeItemType::Stage && Current->StagePtr.IsValid())
		{
			TargetStage = Current->StagePtr.Get();
			break;
		}
		Current = Current->Parent.Pin();
	}

	if (!TargetStage)
	{
		DebugHeader::ShowNotifyInfo(TEXT("Drop Failed: Could not determine Target Stage."));
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

	// Register actors as props to the specific Stage
	if (ActorsToRegister.Num() > 0 && Controller.IsValid())
	{
		if (Controller->RegisterProps(ActorsToRegister, TargetStage))
		{
			DebugHeader::ShowNotifyInfo(FString::Printf(TEXT("Registered %d actors to Stage '%s'"), ActorsToRegister.Num(), *TargetStage->GetActorLabel()));
		}
		else
		{
			DebugHeader::ShowNotifyInfo(TEXT("Registration Failed: No valid actors or already registered."));
		}
	}

	return FReply::Handled();
}

FString SStageEditorPanel::GetItemHash(TSharedPtr<FStageTreeItem> Item)
{
	if (!Item.IsValid()) return TEXT("");

	FString Hash = FString::Printf(TEXT("%d_%s"), (int32)Item->Type, *Item->DisplayName);
	
	if (Item->Parent.IsValid())
	{
		Hash = GetItemHash(Item->Parent.Pin()) + TEXT("/") + Hash;
	}
	
	return Hash;
}

void SStageEditorPanel::SaveExpansionState(TSet<FString>& OutExpansionState)
{
	OutExpansionState.Empty();
	if (!StageTreeView.IsValid()) return;

	// Traverse all items to check expansion (recursive helper or iterative)
	// Since STreeView doesn't expose a simple "GetExpandedItems", we iterate our known RootTreeItems and their children
	
	TArray<TSharedPtr<FStageTreeItem>> Stack = RootTreeItems;
	while (Stack.Num() > 0)
	{
		TSharedPtr<FStageTreeItem> Current = Stack.Pop();
		if (StageTreeView->IsItemExpanded(Current))
		{
			OutExpansionState.Add(GetItemHash(Current));
			Stack.Append(Current->Children);
		}
	}
}

void SStageEditorPanel::RestoreExpansionState(const TSet<FString>& InExpansionState)
{
	if (!StageTreeView.IsValid()) return;

	TArray<TSharedPtr<FStageTreeItem>> Stack = RootTreeItems;
	while (Stack.Num() > 0)
	{
		TSharedPtr<FStageTreeItem> Current = Stack.Pop();
		FString Hash = GetItemHash(Current);
		
		if (InExpansionState.Contains(Hash))
		{
			StageTreeView->SetItemExpansion(Current, true);
			Stack.Append(Current->Children);
		}
	}
}

/**
 * @brief Checks if an item is the drag target or one of its descendants
 * @param Item The item to check
 * @param DragTarget The current drag target (Stage item)
 * @return true if Item is DragTarget or a descendant of DragTarget
 */
bool SStageEditorPanel::IsItemOrDescendantOf(TSharedPtr<FStageTreeItem> Item, TSharedPtr<FStageTreeItem> DragTarget)
{
	if (!Item.IsValid() || !DragTarget.IsValid()) return false;
	if (Item == DragTarget) return true;
	
	// Traverse up the parent chain
	TSharedPtr<FStageTreeItem> Current = Item->Parent.Pin();
	while (Current.IsValid())
	{
		if (Current == DragTarget) return true;
		Current = Current->Parent.Pin();
	}
	return false;
}

/**
 * @brief Handles drag enter events on tree view rows
 * @details Called when a drag operation enters a tree row. Identifies the parent Stage
 *          of the hovered row and sets it as the drag target for visual feedback.
 * @param DragDropEvent The drag and drop event containing operation details
 * @param TargetItem The tree item that was entered during the drag operation
 */
void SStageEditorPanel::OnRowDragEnter(const FDragDropEvent& DragDropEvent, TSharedPtr<FStageTreeItem> TargetItem)
{
	// Check if this is an actor drag operation from World Outliner or internal Prop drag
	TSharedPtr<FDragDropOperation> Operation = DragDropEvent.GetOperation();
	bool bIsActorDrag = Operation.IsValid() && Operation->IsOfType<FActorDragDropOp>();
	bool bIsPropDrag = Operation.IsValid() && Operation->IsOfType<FPropDragDropOp>();

	if (!bIsActorDrag && !bIsPropDrag)
	{
		return;
	}

	// Traverse up the hierarchy to find the parent Stage node
	TSharedPtr<FStageTreeItem> StageItem;
	TSharedPtr<FStageTreeItem> Current = TargetItem;
	while (Current.IsValid())
	{
		if (Current->Type == EStageTreeItemType::Stage)
		{
			StageItem = Current;
			break;
		}
		Current = Current->Parent.Pin();
	}

	// Update the drag target (triggers visual update via TAttribute binding)
	if (StageItem.IsValid())
	{
		DraggedOverItem = StageItem;
	}
}

/**
 * @brief Handles drag leave events on tree view rows
 * @details Called when a drag operation leaves a tree row. Clears the drag target
 *          to remove visual highlighting feedback.
 * @param DragDropEvent The drag and drop event
 * @param TargetItem The tree item that was left during the drag operation
 */
void SStageEditorPanel::OnRowDragLeave(const FDragDropEvent& DragDropEvent, TSharedPtr<FStageTreeItem> TargetItem)
{
	// Note: We don't clear DraggedOverItem here because drag leave might fire
	// when moving between child rows. The drag target should only be cleared
	// when the drag operation ends or enters a different Stage.
	// The OnDrop or drag end will handle cleanup.
}

void SStageEditorPanel::RegisterViewportSelectionListener()
{
	if (!GEditor)
	{
		return;
	}

	if (USelection* ActorSelection = GEditor->GetSelectedActors())
	{
		ActorSelectionPtr = ActorSelection;

		if (!ViewportSelectionDelegateHandle.IsValid())
		{
			ViewportSelectionDelegateHandle = ActorSelection->SelectObjectEvent.AddSP(this, &SStageEditorPanel::HandleViewportSelectionChanged);
		}

		HandleViewportSelectionChanged(nullptr);
	}
}

void SStageEditorPanel::UnregisterViewportSelectionListener()
{
	if (ActorSelectionPtr.IsValid() && ViewportSelectionDelegateHandle.IsValid())
	{
		ActorSelectionPtr->SelectObjectEvent.Remove(ViewportSelectionDelegateHandle);
	}

	ViewportSelectionDelegateHandle = FDelegateHandle();
	ActorSelectionPtr.Reset();
}

void SStageEditorPanel::HandleViewportSelectionChanged(UObject* SelectedObject)
{
	if (bUpdatingViewportSelectionFromPanel || !StageTreeView.IsValid())
	{
		return;
	}

	AActor* SelectedActor = Cast<AActor>(SelectedObject);

	if (!SelectedActor && GEditor)
	{
		if (USelection* SelectedActors = GEditor->GetSelectedActors())
		{
			for (FSelectionIterator It(*SelectedActors); It; ++It)
			{
				if (AActor* Actor = Cast<AActor>(*It))
				{
					SelectedActor = Actor;
					break;
				}
			}
		}
	}

	if (!SelectedActor)
	{
		TGuardValue<bool> Guard(bUpdatingTreeSelectionFromViewport, true);
		StageTreeView->ClearSelection();
		return;
	}

	const FString ActorPath = SelectedActor->GetPathName();
	if (ActorPath.IsEmpty())
	{
		return;
	}

	if (TWeakPtr<FStageTreeItem>* ItemPtr = ActorPathToTreeItem.Find(ActorPath))
	{
		if (TSharedPtr<FStageTreeItem> TreeItem = ItemPtr->Pin())
		{
			ExpandAncestors(TreeItem);
			TGuardValue<bool> Guard(bUpdatingTreeSelectionFromViewport, true);
			StageTreeView->SetSelection(TreeItem);
			StageTreeView->RequestScrollIntoView(TreeItem);
		}
		else
		{
			ActorPathToTreeItem.Remove(ActorPath);
		}
	}
}

void SStageEditorPanel::ExpandAncestors(TSharedPtr<FStageTreeItem> Item)
{
	if (!StageTreeView.IsValid() || !Item.IsValid())
	{
		return;
	}

	TSharedPtr<FStageTreeItem> CurrentParent = Item->Parent.Pin();
	while (CurrentParent.IsValid())
	{
		StageTreeView->SetItemExpansion(CurrentParent, true);
		CurrentParent = CurrentParent->Parent.Pin();
	}
}

TSharedPtr<FStageTreeItem> SStageEditorPanel::FindStageAncestor(TSharedPtr<FStageTreeItem> Item) const
{
	TSharedPtr<FStageTreeItem> CurrentItem = Item;
	while (CurrentItem.IsValid())
	{
		if (CurrentItem->Type == EStageTreeItemType::Stage)
		{
			return CurrentItem;
		}
		CurrentItem = CurrentItem->Parent.Pin();
	}

	return nullptr;
}

void SStageEditorPanel::ApplyPropStateChange(TSharedPtr<FStageTreeItem> PropItem, TSharedPtr<FStageTreeItem> ParentActItem, int32 NewState)
{
	if (!Controller.IsValid() || !PropItem.IsValid() || !ParentActItem.IsValid())
	{
		return;
	}

	if (PropItem->bHasPropState && PropItem->PropState == NewState)
	{
		return;
	}

	TSharedPtr<FStageTreeItem> StageItem = FindStageAncestor(ParentActItem);
	if (!StageItem.IsValid() || !StageItem->StagePtr.IsValid())
	{
		return;
	}

	Controller->SetActiveStage(StageItem->StagePtr.Get());
	Controller->SetPropStateInAct(PropItem->ID, ParentActItem->ID, NewState);
}

void SStageEditorPanel::SelectActorInViewport(AActor* ActorToSelect)
{
	if (!GEditor || !ActorToSelect)
	{
		return;
	}

	TGuardValue<bool> Guard(bUpdatingViewportSelectionFromPanel, true);
	GEditor->SelectNone(false, true);
	GEditor->SelectActor(ActorToSelect, true, true);
}

/**
 * @brief Handles drop events on tree view rows
 * @details Processes actor drops from World Outliner, registers them to the appropriate
 *          Stage, and provides user feedback via notifications.
 * @param MyGeometry The geometry of the widget (unused, passed as default)
 * @param DragDropEvent The drag and drop event containing the dragged actors
 * @param TargetItem The tree item where the drop occurred
 * @return FReply::Handled() if drop was processed, FReply::Unhandled() otherwise
 */

#undef LOCTEXT_NAMESPACE
