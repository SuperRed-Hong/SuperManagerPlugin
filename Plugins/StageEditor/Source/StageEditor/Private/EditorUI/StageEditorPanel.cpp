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
	
	// Save Expansion State
	TSet<FString> ExpansionState;
	SaveExpansionState(ExpansionState);

	RootTreeItems.Empty();

	const TArray<TWeakObjectPtr<AStage>>& FoundStages = Controller->GetFoundStages();

	for (const auto& StagePtr : FoundStages)
	{
		if (AStage* Stage = StagePtr.Get())
		{
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

	// Add Create Act button for Acts folder
	if (Item->Type == EStageTreeItemType::ActsFolder)
	{
		RowContent->AddSlot()
		.AutoWidth()
		.VAlign(VAlign_Center)
		.Padding(4, 0, 0, 0)
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
	[
		SNew(SBorder)
		.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
		.BorderBackgroundColor_Lambda([this, Item]() -> FSlateColor
		{
			// Highlight the Stage and all its descendants when being dragged over
			bool bIsDragTarget = IsItemOrDescendantOf(Item, DraggedOverItem);
			// Use a brighter, more visible blue with higher opacity
			return bIsDragTarget ? FLinearColor(0.0f, 0.5f, 1.0f, 0.6f) : FLinearColor::Transparent;
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
	if (!Operation.IsValid() || !Operation->IsOfType<FActorDragDropOp>())
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
	// Check if this is an actor drag operation from World Outliner
	TSharedPtr<FDragDropOperation> Operation = DragDropEvent.GetOperation();
	if (!Operation.IsValid() || !Operation->IsOfType<FActorDragDropOp>())
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
