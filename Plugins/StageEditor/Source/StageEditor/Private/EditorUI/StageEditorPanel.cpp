#include "EditorUI/StageEditorPanel.h"

#include <DebugHeader.h>
#include "StageEditorModule.h"

#include "Actors/Stage.h"
#include "Subsystems/StageManagerSubsystem.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Views/STableRow.h"
#include "Editor.h"
#include "Engine/Blueprint.h"
#include "Subsystems/AssetEditorSubsystem.h"
#include "Input/Events.h"
#include "IStructureDetailsView.h"
#include "Engine/Selection.h"
#include "DragAndDrop/ActorDragDropOp.h"
#include "Widgets/Input/SNumericEntryBox.h"
#include "Widgets/Input/SComboBox.h"
#include "Widgets/Input/SCheckBox.h"
#include "Templates/UnrealTemplate.h"
#include "Misc/MessageDialog.h"
#include "WorldPartition/DataLayer/DataLayerAsset.h"
#include "UObject/ObjectSaveContext.h"

#define LOCTEXT_NAMESPACE "SStageEditorPanel"

#pragma endregion Imports

#pragma region Helper Functions
/**
 * Helper to get the StageManagerSubsystem from PIE or Game world.
 * Used for Debug HUD watch feature in Editor Panel.
 */
static UStageManagerSubsystem* GetStageManagerSubsystemForWatch()
{
	if (!GEngine) return nullptr;

	// Try to find the game world (PIE or standalone)
	for (const FWorldContext& Context : GEngine->GetWorldContexts())
	{
		if (Context.WorldType == EWorldType::PIE || Context.WorldType == EWorldType::Game)
		{
			if (UWorld* World = Context.World())
			{
				return World->GetSubsystem<UStageManagerSubsystem>();
			}
		}
	}
	return nullptr;
}
#pragma endregion Helper Functions

#pragma region Construction

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

	// Bind to property changes to update Controller's DataLayer path
	if (SettingsDetailsView.IsValid())
	{
		SettingsDetailsView->GetOnFinishedChangingPropertiesDelegate().AddSP(this, &SStageEditorPanel::OnAssetCreationSettingsChanged);
	}

	// Initialize Controller's DataLayer asset folder path
	if (Controller.IsValid())
	{
		FString DataLayerPath = Settings->bIsCustomDataLayerAssetPath ? 
			Settings->DataLayerAssetFolderPath.Path : TEXT("/StageEditor/DataLayers");
		Controller->SetDataLayerAssetFolderPath(DataLayerPath);
	}

	// Check if World Partition is enabled
	const bool bIsWorldPartition = IsWorldPartitionLevel();
	bCachedIsWorldPartition = bIsWorldPartition;

	// Register for map change events (only once)
	if (!MapChangedHandle.IsValid())
	{
		MapChangedHandle = FEditorDelegates::OnMapOpened.AddSP(this, &SStageEditorPanel::OnMapOpened);
	}

	// Register for post-save world events to detect WorldPartition changes
	if (!PostSaveWorldHandle.IsValid())
	{
		PostSaveWorldHandle = FEditorDelegates::PostSaveWorldWithContext.AddSP(this, &SStageEditorPanel::OnPostSaveWorld);
	}

	// Register for stage data changed events (Import/Sync operations)
	if (!StageDataChangedHandle.IsValid())
	{
		if (UWorld* World = GEditor ? GEditor->GetEditorWorldContext().World() : nullptr)
		{
			if (UStageManagerSubsystem* Subsystem = World->GetSubsystem<UStageManagerSubsystem>())
			{
				StageDataChangedHandle = Subsystem->OnStageDataChanged.AddSP(this, &SStageEditorPanel::OnStageDataChanged);
			}
		}
	}

	if (!bIsWorldPartition)
	{
		// Show World Partition requirement warning
		ChildSlot
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.FillHeight(1.0f)
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			.Padding(20)
			[
				SNew(SBorder)
				.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
				.Padding(30)
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Center)
				[
					SNew(SVerticalBox)

					// Warning Icon & Title
					+ SVerticalBox::Slot()
					.AutoHeight()
					.HAlign(HAlign_Center)
					.Padding(0, 0, 0, 20)
					[
						SNew(STextBlock)
						.Text(LOCTEXT("WPRequired_Title", "⚠ World Partition Required"))
						.Font(FCoreStyle::GetDefaultFontStyle("Bold", 16))
						.ColorAndOpacity(FLinearColor(1.0f, 0.6f, 0.0f))
					]

					// Description
					+ SVerticalBox::Slot()
					.AutoHeight()
					.HAlign(HAlign_Center)
					.Padding(0, 0, 0, 30)
					[
						SNew(STextBlock)
						.Text(LOCTEXT("WPRequired_Description",
							"StageEditor requires a World Partition level to function properly.\n\n"
							"World Partition enables:\n"
							"• Dynamic resource streaming via DataLayers\n"
							"• Efficient memory management for large worlds\n"
							"• Proper Act-based scene state management\n\n"
							"Please convert your current level to World Partition to continue."))
						.Font(FCoreStyle::GetDefaultFontStyle("Regular", 10))
						.AutoWrapText(true)
						.Justification(ETextJustify::Center)
					]

					// Convert Button
					+ SVerticalBox::Slot()
					.AutoHeight()
					.HAlign(HAlign_Center)
					.Padding(0, 0, 0, 10)
					[
						SNew(SButton)
						.Text(LOCTEXT("ConvertToWP", "Convert to World Partition"))
						.ToolTipText(LOCTEXT("ConvertToWP_Tooltip", "Convert the current level to World Partition format"))
						.OnClicked(this, &SStageEditorPanel::OnConvertToWorldPartitionClicked)
						.ButtonStyle(FAppStyle::Get(), "PrimaryButton")
					]

					// Refresh Button
					+ SVerticalBox::Slot()
					.AutoHeight()
					.HAlign(HAlign_Center)
					.Padding(0, 0, 0, 20)
					[
						SNew(SButton)
						.Text(LOCTEXT("RefreshWPStatus", "Refresh Status"))
						.ToolTipText(LOCTEXT("RefreshWPStatus_Tooltip", "Refresh World Partition status check (use after conversion completes)"))
						.OnClicked(this, &SStageEditorPanel::OnRefreshWorldPartitionStatusClicked)
					]

					// Documentation Link
					+ SVerticalBox::Slot()
					.AutoHeight()
					.HAlign(HAlign_Center)
					[
						SNew(STextBlock)
						.Text(LOCTEXT("WPDocs", "Learn more about World Partition in UE5 documentation"))
						.Font(FCoreStyle::GetDefaultFontStyle("Italic", 9))
						.ColorAndOpacity(FLinearColor(0.7f, 0.7f, 0.7f))
					]
				]
			]
		];
	}
	else
	{
		// Normal StageEditor UI
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
					.Text(LOCTEXT("RegisterEntities", "Register Selected Entities"))
					.OnClicked(this, &SStageEditorPanel::OnRegisterSelectedEntitiesClicked)
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
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(5, 0, 0, 0)
				[
					SNew(SButton)
					.Text(LOCTEXT("CleanOrphanedEntities", "Clean Orphaned"))
					.OnClicked(this, &SStageEditorPanel::OnCleanOrphanedEntitiesClicked)
					.ToolTipText(LOCTEXT("CleanOrphanedEntities_Tooltip", "Clean orphaned Entities (Entities whose owner Stage was deleted)"))
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
					.Text(LOCTEXT("CreateEntityActorBP", "Create Entity Actor BP"))
					.OnClicked(this, &SStageEditorPanel::OnCreateEntityActorBPClicked)
					.ToolTipText(LOCTEXT("CreateEntityActorBP_Tooltip", "Create a new Entity Actor Blueprint in Content Browser"))
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(5, 0, 0, 0)
				[
					SNew(SButton)
					.Text(LOCTEXT("CreateEntityComponentBP", "Create Entity Component BP"))
					.OnClicked(this, &SStageEditorPanel::OnCreateEntityComponentBPClicked)
					.ToolTipText(LOCTEXT("CreateEntityComponentBP_Tooltip", "Create a new Entity Component Blueprint in Content Browser"))
				]

				// Settings Gear Button
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(10, 0, 0, 0)
				[
					SNew(SButton)
					.ButtonStyle(FAppStyle::Get(), "SimpleButton")
					.OnClicked(this, &SStageEditorPanel::OnOpenSettingsClicked)
					.ToolTipText(LOCTEXT("OpenSettings_Tooltip", "Open Asset Creation Settings"))
					[
						SNew(SImage)
						.Image(FAppStyle::GetBrush("Icons.Settings"))
						.ColorAndOpacity(FSlateColor::UseForeground())
					]
				]
			]

			// Tree View with Header Row
			+ SVerticalBox::Slot()
			.FillHeight(1.0f)
			[
				SNew(SBorder)
				.Padding(2)
				[
					SAssignNew(StageTreeView, STreeView<TSharedPtr<FStageTreeItem>>)
					.TreeItemsSource(&RootTreeItems)
					.SelectionMode(ESelectionMode::Multi)  // Enable Ctrl/Shift multi-select
					.OnGenerateRow(this, &SStageEditorPanel::OnGenerateRow)
					.OnGetChildren(this, &SStageEditorPanel::OnGetChildren)
					.OnSelectionChanged(this, &SStageEditorPanel::OnSelectionChanged)
					.OnContextMenuOpening(this, &SStageEditorPanel::OnContextMenuOpening)
					.OnMouseButtonDoubleClick(this, &SStageEditorPanel::OnRowDoubleClicked)
					.HeaderRow
					(
						SAssignNew(HeaderRow, SHeaderRow)
						+ SHeaderRow::Column(StageTreeColumns::Watch)
						.DefaultLabel(FText::GetEmpty())  // Icon only in header
						.ManualWidth(24.0f)
						.HeaderContentPadding(FMargin(2, 0, 0, 0))
						.HeaderContent()
						[
							SNew(SImage)
							.Image(FAppStyle::GetBrush("Level.VisibleIcon16x"))
							.ToolTipText(LOCTEXT("WatchColumnHeader_Tooltip", "Toggle Stage watch state for Debug HUD"))
						]

						+ SHeaderRow::Column(StageTreeColumns::ID)
						.DefaultLabel(LOCTEXT("SUIDColumn", "SUID"))
						.ManualWidth(150.0f)
						.HeaderContentPadding(FMargin(4, 0, 0, 0))

						+ SHeaderRow::Column(StageTreeColumns::Name)
						.DefaultLabel(LOCTEXT("NameColumn", "Name"))
						.ManualWidth(300.0f)
						.HeaderContentPadding(FMargin(4, 0, 0, 0))

						+ SHeaderRow::Column(StageTreeColumns::Actions)
						.DefaultLabel(LOCTEXT("ActionsColumn", "Actions"))
						.ManualWidth(120.0f)
						.HeaderContentPadding(FMargin(4, 0, 0, 0))
					)
				]
			]
		];
	}

	RefreshUI();

	// ❌ Disabled: We want one-way sync (Panel → Viewport only)
	// Viewport selection should NOT affect Panel selection
	// RegisterViewportSelectionListener();

	// Initialize Controller (scan for level actors) AFTER UI is built
	if (Controller.IsValid())
	{
		Controller->Initialize();
	}
}

SStageEditorPanel::~SStageEditorPanel()
{
	// Unregister map changed delegate
	if (MapChangedHandle.IsValid())
	{
		FEditorDelegates::OnMapOpened.Remove(MapChangedHandle);
		MapChangedHandle.Reset();
	}

	// Unregister post-save world delegate
	if (PostSaveWorldHandle.IsValid())
	{
		FEditorDelegates::PostSaveWorldWithContext.Remove(PostSaveWorldHandle);
		PostSaveWorldHandle.Reset();
	}

	// Unregister stage data changed delegate
	if (StageDataChangedHandle.IsValid())
	{
		if (UWorld* World = GEditor ? GEditor->GetEditorWorldContext().World() : nullptr)
		{
			if (UStageManagerSubsystem* Subsystem = World->GetSubsystem<UStageManagerSubsystem>())
			{
				Subsystem->OnStageDataChanged.Remove(StageDataChangedHandle);
			}
		}
		StageDataChangedHandle.Reset();
	}

	// ❌ Disabled: Viewport listener was not registered
	// UnregisterViewportSelectionListener();
}

void SStageEditorPanel::OnMapOpened(const FString& Filename, bool bAsTemplate)
{
	CheckAndRefreshWorldPartitionStatus();
}

void SStageEditorPanel::OnPostSaveWorld(UWorld* World, FObjectPostSaveContext ObjectSaveContext)
{
	// After saving, check if WorldPartition status changed
	// (conversion to WP requires save)
	CheckAndRefreshWorldPartitionStatus();
}

void SStageEditorPanel::OnStageDataChanged(AStage* Stage)
{
	// Refresh UI when Stage data changes (after Import/Sync operations)
	// This ensures the StageEditorPanel reflects the latest state
	if (Controller.IsValid())
	{
		// Rescan the world for stages in case new ones were created
		Controller->FindStageInWorld();
	}
	RefreshUI();
}

void SStageEditorPanel::CheckAndRefreshWorldPartitionStatus()
{
	const bool bNewIsWorldPartition = IsWorldPartitionLevel();

	if (bNewIsWorldPartition != bCachedIsWorldPartition)
	{
		UE_LOG(LogTemp, Log, TEXT("StageEditorPanel: WorldPartition status changed from %s to %s"),
			bCachedIsWorldPartition ? TEXT("true") : TEXT("false"),
			bNewIsWorldPartition ? TEXT("true") : TEXT("false"));

		bCachedIsWorldPartition = bNewIsWorldPartition;
		RebuildUI();

		// Show notification to user
		if (bNewIsWorldPartition)
		{
			DebugHeader::ShowNotifyInfo(TEXT("World Partition detected! Stage Editor is now available."));
		}
	}
	else
	{
		// Same state, just refresh the data
		RefreshUI();
	}
}

void SStageEditorPanel::RebuildUI()
{
	// Clear current content and rebuild the entire UI
	// This is needed when switching between WP/non-WP states
	ChildSlot.DetachWidget();

	// Re-run construction logic by calling Construct again
	// We need to preserve the Controller reference
	TSharedPtr<FStageEditorController> SavedController = Controller;

	// Build new arguments
	FArguments Args;

	// Re-construct
	Construct(Args, SavedController);

	UE_LOG(LogTemp, Log, TEXT("StageEditorPanel: UI rebuilt. WorldPartition=%s"),
		bCachedIsWorldPartition ? TEXT("true") : TEXT("false"));
}


#pragma endregion Construction

#pragma region Core API

void SStageEditorPanel::RefreshUI()
{
	if (!Controller.IsValid()) return;

	// If StageTreeView doesn't exist (e.g., in non-World Partition level), skip refresh
	if (!StageTreeView.IsValid())
	{
		return;
	}

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
			// Create Stage Root Item - DisplayName stores label for other uses (e.g., drag text)
			FString StageName = Stage->GetActorLabel();
			TSharedPtr<FStageTreeItem> StageItem = MakeShared<FStageTreeItem>(EStageTreeItemType::Stage, StageName, Stage->GetStageID(), nullptr, Stage);
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
				// DisplayName stores the user-friendly name for other uses (e.g., context menu, drag text)
				TSharedPtr<FStageTreeItem> ActItem = MakeShared<FStageTreeItem>(EStageTreeItemType::Act, Act.DisplayName, Act.SUID.ActID);
				ActsFolder->Children.Add(ActItem);
				ActItem->Parent = ActsFolder;

				// Populate Entities in Act
				for (auto& Pair : Act.EntityStateOverrides)
				{
					int32 EntityID = Pair.Key;
					int32 State = Pair.Value;

					AActor* EntityActor = Stage->GetEntityByID(EntityID);
					// DisplayName stores actor label for other uses
					FString EntityDisplayName = EntityActor ? EntityActor->GetActorLabel() : TEXT("Invalid Entity");

					TSharedPtr<FStageTreeItem> EntityItem = MakeShared<FStageTreeItem>(EStageTreeItemType::Entity, EntityDisplayName, EntityID, EntityActor);
					ActItem->Children.Add(EntityItem);
					EntityItem->Parent = ActItem;
					EntityItem->StagePtr = Stage;
					EntityItem->EntityState = State;
					EntityItem->bHasEntityState = true;
					if (EntityActor)
					{
						EntityItem->ActorPath = EntityActor->GetPathName();
						if (!EntityItem->ActorPath.IsEmpty() && !ActorPathToTreeItem.Contains(EntityItem->ActorPath))
						{
							ActorPathToTreeItem.Add(EntityItem->ActorPath, EntityItem);
						}
					}
				}
			}

			// 2. Create "Registered Entities" Folder
			TSharedPtr<FStageTreeItem> EntitiesFolder = MakeShared<FStageTreeItem>(EStageTreeItemType::EntitiesFolder, TEXT("Registered Entities"));
			StageItem->Children.Add(EntitiesFolder);
			EntitiesFolder->Parent = StageItem;

			// Populate All Entities
			for (auto& Pair : Stage->EntityRegistry)
			{
				if (AActor* Actor = Pair.Value.Get())
				{
					// DisplayName stores actor label for other uses
					FString EntityDisplayName = Actor->GetActorLabel();
					TSharedPtr<FStageTreeItem> EntityItem = MakeShared<FStageTreeItem>(EStageTreeItemType::Entity, EntityDisplayName, Pair.Key, Actor);
					EntitiesFolder->Children.Add(EntityItem);
					EntityItem->Parent = EntitiesFolder;
					EntityItem->StagePtr = Stage;
					EntityItem->ActorPath = Actor->GetPathName();
					if (!EntityItem->ActorPath.IsEmpty() && !ActorPathToTreeItem.Contains(EntityItem->ActorPath))
					{
						ActorPathToTreeItem.Add(EntityItem->ActorPath, EntityItem);
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

	// HandleViewportSelectionChanged(nullptr);
}
#pragma endregion Core API

#pragma region SStageTreeRow

/**
 * Custom multi-column table row for Stage tree items
 * Supports resizable columns via SHeaderRow integration
 */
class SStageTreeRow : public SMultiColumnTableRow<TSharedPtr<FStageTreeItem>>
{
public:
	SLATE_BEGIN_ARGS(SStageTreeRow) {}
		SLATE_ARGUMENT(TSharedPtr<FStageTreeItem>, Item)
		SLATE_ARGUMENT(SStageEditorPanel*, OwnerPanel)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& InOwnerTable)
	{
		Item = InArgs._Item;
		OwnerPanel = InArgs._OwnerPanel;

		// Cache parent info
		ParentItem = Item.IsValid() ? Item->Parent.Pin() : nullptr;
		bParentIsAct = ParentItem.IsValid() && ParentItem->Type == EStageTreeItemType::Act;
		bParentIsEntitiesFolder = ParentItem.IsValid() && ParentItem->Type == EStageTreeItemType::EntitiesFolder;
		bIsFolder = Item.IsValid() && (Item->Type == EStageTreeItemType::ActsFolder || Item->Type == EStageTreeItemType::EntitiesFolder);

		// Determine icon
		DetermineIcon();

		// Build ID and Name text
		BuildDisplayText();

		SMultiColumnTableRow<TSharedPtr<FStageTreeItem>>::Construct(
			FSuperRowType::FArguments()
			.Style(&FAppStyle::Get().GetWidgetStyle<FTableRowStyle>("TableView.AlternatingRow"))
			.OnDrop_Lambda([this](const FDragDropEvent& DragDropEvent) -> FReply
			{
				if (OwnerPanel)
				{
					return OwnerPanel->OnRowDrop(FGeometry(), DragDropEvent, Item);
				}
				return FReply::Unhandled();
			})
			.OnDragEnter_Lambda([this](const FDragDropEvent& DragDropEvent)
			{
				if (OwnerPanel)
				{
					OwnerPanel->OnRowDragEnter(DragDropEvent, Item);
					// Request refresh to update highlight (lighter than RebuildList)
					if (OwnerPanel->StageTreeView.IsValid())
					{
						OwnerPanel->StageTreeView->RequestListRefresh();
					}
				}
			})
			.OnDragLeave_Lambda([this](const FDragDropEvent& DragDropEvent)
			{
				if (OwnerPanel)
				{
					OwnerPanel->OnRowDragLeave(DragDropEvent, Item);
					// Request refresh when leaving to clear highlight
					if (OwnerPanel->StageTreeView.IsValid())
					{
						OwnerPanel->StageTreeView->RequestListRefresh();
					}
				}
			}),
			InOwnerTable
		);
	}

	/**
	 * Override OnDragDetected to support dragging Entities from Registered Entities folder
	 * Only draggable items return Handled, others use default TreeView behavior
	 */
	virtual FReply OnDragDetected(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override
	{
		// Only allow dragging Entities from Registered Entities folder
		if (Item.IsValid() && Item->Type == EStageTreeItemType::Entity && bParentIsEntitiesFolder && OwnerPanel)
		{
			// Get selected items for multi-select drag
			TArray<TSharedPtr<FStageTreeItem>> SelectedItems;
			if (OwnerPanel->StageTreeView.IsValid())
			{
				OwnerPanel->StageTreeView->GetSelectedItems(SelectedItems);

				// Filter to only include Entities from Registered Entities folder
				SelectedItems = SelectedItems.FilterByPredicate([](const TSharedPtr<FStageTreeItem>& SelectedItem) {
					if (!SelectedItem.IsValid()) return false;
					if (SelectedItem->Type != EStageTreeItemType::Entity) return false;
					TSharedPtr<FStageTreeItem> Parent = SelectedItem->Parent.Pin();
					return Parent.IsValid() && Parent->Type == EStageTreeItemType::EntitiesFolder;
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
				// Create a custom drag-drop operation with EntityIDs
				TSharedRef<FEntityDragDropOp> DragOp = MakeShared<FEntityDragDropOp>();
				DragOp->EntityItems = SelectedItems;

				// Create drag visual
				FString DragText = SelectedItems.Num() == 1
					? SelectedItems[0]->DisplayName
					: FString::Printf(TEXT("%d Entities"), SelectedItems.Num());

				DragOp->DefaultHoverText = FText::FromString(DragText);
				DragOp->CurrentHoverText = DragOp->DefaultHoverText;

				return FReply::Handled().BeginDragDrop(DragOp);
			}
		}

		// For non-draggable items, let the parent class handle it (returns Unhandled)
		return SMultiColumnTableRow<TSharedPtr<FStageTreeItem>>::OnDragDetected(MyGeometry, MouseEvent);
	}

	/** Check if this row is currently a drag target */
	bool IsDragTarget() const
	{
		if (!OwnerPanel || !Item.IsValid()) return false;
		return OwnerPanel->IsItemOrDescendantOf(Item, OwnerPanel->DraggedOverItem);
	}

	/** Override to provide drag highlight effect similar to hover */
	virtual const FSlateBrush* GetBorder() const override
	{
		// Check if this row should be highlighted as drag target
		if (IsDragTarget())
		{
			// Use the same brush as hover state from the style
			const FTableRowStyle* RowStyle = &FAppStyle::Get().GetWidgetStyle<FTableRowStyle>("TableView.AlternatingRow");
			return &RowStyle->ActiveHoveredBrush;
		}

		// Default behavior from parent class
		return SMultiColumnTableRow<TSharedPtr<FStageTreeItem>>::GetBorder();
	}

	virtual TSharedRef<SWidget> GenerateWidgetForColumn(const FName& InColumnName) override
	{
		if (InColumnName == StageTreeColumns::Watch)
		{
			return GenerateWatchColumnWidget();
		}
		else if (InColumnName == StageTreeColumns::ID)
		{
			return GenerateIDColumnWidget();
		}
		else if (InColumnName == StageTreeColumns::Name)
		{
			return GenerateNameColumnWidget();
		}
		else if (InColumnName == StageTreeColumns::Actions)
		{
			return GenerateActionsColumnWidget();
		}

		return SNullWidget::NullWidget;
	}

private:
	TSharedPtr<FStageTreeItem> Item;
	SStageEditorPanel* OwnerPanel = nullptr;
	TSharedPtr<FStageTreeItem> ParentItem;
	bool bParentIsAct = false;
	bool bParentIsEntitiesFolder = false;
	bool bIsFolder = false;

	const FSlateBrush* IconBrush = nullptr;
	FSlateColor IconColor = FSlateColor::UseForeground();
	FString IDText;
	FString NameText;

	void DetermineIcon()
	{
		if (!Item.IsValid()) return;

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
		case EStageTreeItemType::EntitiesFolder:
			IconBrush = FAppStyle::GetBrush("ContentBrowser.AssetTreeFolderClosed");
			IconColor = FSlateColor(FLinearColor(1.0f, 0.6f, 0.3f)); // Orange (matching Entities)
			break;
		case EStageTreeItemType::Act:
			IconBrush = FAppStyle::GetBrush("Sequencer.Tracks.Event");
			IconColor = FSlateColor(FLinearColor(0.5f, 1.0f, 0.5f)); // Green
			break;
		case EStageTreeItemType::Entity:
			IconBrush = FAppStyle::GetBrush("ClassIcon.StaticMeshActor");
			IconColor = FSlateColor(FLinearColor(1.0f, 0.6f, 0.3f)); // Orange
			break;
		}
	}

	void BuildDisplayText()
	{
		if (!Item.IsValid()) return;

		// Helper to get StageID from ancestor
		auto GetStageID = [this]() -> int32
		{
			if (Item->StagePtr.IsValid())
			{
				return Item->StagePtr->GetStageID();
			}
			// Walk up parent chain to find Stage
			TSharedPtr<FStageTreeItem> Current = Item->Parent.Pin();
			while (Current.IsValid())
			{
				if (Current->Type == EStageTreeItemType::Stage && Current->StagePtr.IsValid())
				{
					return Current->StagePtr->GetStageID();
				}
				Current = Current->Parent.Pin();
			}
			return 0;
		};

		switch (Item->Type)
		{
		case EStageTreeItemType::Stage:
			// Display: S_StageID.0.0
			IDText = FString::Printf(TEXT("S_%d.0.0"), Item->ID);
			NameText = Item->StagePtr.IsValid() ? Item->StagePtr->GetActorLabel() : TEXT("");
			break;
		case EStageTreeItemType::Act:
			// Display: A_StageID.ActID.0 (ActID starts from 1, Default Act = 1)
			IDText = FString::Printf(TEXT("A_%d.%d.0"), GetStageID(), Item->ID);
			NameText = Item->DisplayName;
			break;
		case EStageTreeItemType::Entity:
			// Display: P_StageID.0.EntityID
			IDText = FString::Printf(TEXT("P_%d.0.%d"), GetStageID(), Item->ID);
			NameText = Item->ActorPtr.IsValid() ? Item->ActorPtr->GetActorLabel() : TEXT("Invalid");
			break;
		case EStageTreeItemType::ActsFolder:
		case EStageTreeItemType::EntitiesFolder:
			// Folders: show name in SUID column, Name column empty
			IDText = Item->DisplayName;
			NameText = TEXT("");
			break;
		default:
			IDText = TEXT("");
			NameText = Item->DisplayName;
			break;
		}
	}

	TSharedRef<SWidget> GenerateWatchColumnWidget()
	{
		// Only show watch toggle for Stage rows
		if (!Item.IsValid() || Item->Type != EStageTreeItemType::Stage || !Item->StagePtr.IsValid())
		{
			return SNullWidget::NullWidget;
		}

		// Get Stage pointer for this row
		TWeakObjectPtr<AStage> WeakStage = Item->StagePtr;
		int32 StageID = Item->ID;

		// Create eye icon button
		return SNew(SBox)
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			.WidthOverride(20.0f)
			[
				SNew(SButton)
				.ButtonStyle(FAppStyle::Get(), "NoBorder")
				.ToolTipText_Lambda([WeakStage, StageID]()
				{
					// Check PIE first
					UStageManagerSubsystem* Subsystem = GetStageManagerSubsystemForWatch();
					if (Subsystem)
					{
						bool bIsWatched = Subsystem->IsStageWatched(StageID);
						return bIsWatched
							? LOCTEXT("WatchButton_Unwatch_PIE", "Click to stop watching (PIE active)")
							: LOCTEXT("WatchButton_Watch_PIE", "Click to watch in Debug HUD (PIE active)");
					}

					// Editor mode - show editor watch state
					if (AStage* Stage = WeakStage.Get())
					{
						return Stage->bEditorWatched
							? LOCTEXT("WatchButton_Unwatch_Editor", "Click to unmark for Debug HUD (will sync when PIE starts)")
							: LOCTEXT("WatchButton_Watch_Editor", "Click to mark for Debug HUD (will sync when PIE starts)");
					}

					return LOCTEXT("WatchButton_Invalid", "Stage not available");
				})
				.OnClicked_Lambda([WeakStage, StageID]()
				{
					// Check PIE first
					UStageManagerSubsystem* Subsystem = GetStageManagerSubsystemForWatch();
					if (Subsystem)
					{
						// PIE mode - toggle in Subsystem AND sync back to Stage
						if (Subsystem->IsStageWatched(StageID))
						{
							Subsystem->UnwatchStage(StageID);
							if (AStage* Stage = WeakStage.Get())
							{
								Stage->bEditorWatched = false;
							}
						}
						else
						{
							Subsystem->WatchStage(StageID);
							if (AStage* Stage = WeakStage.Get())
							{
								Stage->bEditorWatched = true;
							}
						}
					}
					else
					{
						// Editor mode - toggle bEditorWatched
						if (AStage* Stage = WeakStage.Get())
						{
							Stage->Modify();  // Support Undo
							Stage->bEditorWatched = !Stage->bEditorWatched;
						}
					}
					return FReply::Handled();
				})
				[
					SNew(SImage)
					.Image_Lambda([WeakStage, StageID]() -> const FSlateBrush*
					{
						// Check PIE first
						UStageManagerSubsystem* Subsystem = GetStageManagerSubsystemForWatch();
						if (Subsystem)
						{
							bool bIsWatched = Subsystem->IsStageWatched(StageID);
							return bIsWatched
								? FAppStyle::GetBrush("Level.VisibleIcon16x")
								: FAppStyle::GetBrush("Level.NotVisibleIcon16x");
						}

						// Editor mode - use bEditorWatched
						if (AStage* Stage = WeakStage.Get())
						{
							return Stage->bEditorWatched
								? FAppStyle::GetBrush("Level.VisibleIcon16x")
								: FAppStyle::GetBrush("Level.NotVisibleIcon16x");
						}

						return FAppStyle::GetBrush("Level.NotVisibleIcon16x");
					})
					.ColorAndOpacity_Lambda([WeakStage, StageID]() -> FSlateColor
					{
						// Check PIE first
						UStageManagerSubsystem* Subsystem = GetStageManagerSubsystemForWatch();
						if (Subsystem)
						{
							bool bIsWatched = Subsystem->IsStageWatched(StageID);
							return bIsWatched
								? FSlateColor(FLinearColor(0.2f, 0.8f, 0.2f))  // Green when watched
								: FSlateColor::UseForeground();
						}

						// Editor mode - use bEditorWatched with different color
						if (AStage* Stage = WeakStage.Get())
						{
							return Stage->bEditorWatched
								? FSlateColor(FLinearColor(0.5f, 0.7f, 1.0f))  // Light blue for editor preset
								: FSlateColor::UseForeground();
						}

						return FSlateColor(FLinearColor(0.3f, 0.3f, 0.3f, 0.5f));
					})
				]
			];
	}

	TSharedRef<SWidget> GenerateIDColumnWidget()
	{
		TSharedRef<SHorizontalBox> ColumnContent = SNew(SHorizontalBox);

		// Expander Arrow (for tree hierarchy)
		ColumnContent->AddSlot()
		.AutoWidth()
		.VAlign(VAlign_Center)
		[
			SNew(SExpanderArrow, SharedThis(this))
			.IndentAmount(16)
		];

		// Icon
		ColumnContent->AddSlot()
		.AutoWidth()
		.VAlign(VAlign_Center)
		.Padding(2, 0, 4, 0)
		[
			SNew(SImage)
			.Image(IconBrush)
			.ColorAndOpacity(IconColor)
		];

		// ID Text (or folder name for folders)
		ColumnContent->AddSlot()
		.FillWidth(1.0f)
		.VAlign(VAlign_Center)
		[
			SNew(STextBlock)
			.Text(FText::FromString(IDText))
			.Font(bIsFolder ? FCoreStyle::GetDefaultFontStyle("Bold", 10) : FCoreStyle::GetDefaultFontStyle("Regular", 10))
		];

		return ColumnContent;
	}

	TSharedRef<SWidget> GenerateNameColumnWidget()
	{
		TSharedRef<SHorizontalBox> ColumnContent = SNew(SHorizontalBox);

		// Name Text (also used for folder display names)
		ColumnContent->AddSlot()
		.FillWidth(1.0f)
		.VAlign(VAlign_Center)
		.Padding(4, 0, 0, 0)
		[
			SNew(STextBlock)
			.Text(FText::FromString(NameText))
			.Font(FCoreStyle::GetDefaultFontStyle("Regular", 10))
			.ColorAndOpacity(bIsFolder
				? FSlateColor::UseForeground()  // Folders use normal color
				: FSlateColor(FLinearColor(0.7f, 0.7f, 0.7f))) // Others slightly dimmed
		];

		// EntityState inline edit - only for Entities under Act
		if (Item.IsValid() && Item->Type == EStageTreeItemType::Entity && bParentIsAct && Item->bHasEntityState)
		{
			TSharedPtr<FStageTreeItem> CapturedItem = Item;
			TSharedPtr<FStageTreeItem> CapturedParent = ParentItem;
			SStageEditorPanel* CapturedPanel = OwnerPanel;

			ColumnContent->AddSlot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			.Padding(4, 0, 0, 0)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				[
					SNew(STextBlock)
					.Text(FText::FromString(TEXT("State:")))
					.Font(FCoreStyle::GetDefaultFontStyle("Regular", 9))
					.ColorAndOpacity(FSlateColor(FLinearColor(0.5f, 0.5f, 0.5f)))
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				.Padding(4, 0, 0, 0)
				[
					SNew(SNumericEntryBox<int32>)
					.Value(Item->EntityState)
					.AllowSpin(false)
					.MinDesiredValueWidth(40.0f)
					.ToolTipText(LOCTEXT("EntityStateInlineEdit_Tooltip", "Adjust the Entity state applied within this Act"))
					.OnValueCommitted_Lambda([CapturedPanel, CapturedItem, CapturedParent](int32 NewValue, ETextCommit::Type)
					{
						if (CapturedPanel)
						{
							CapturedPanel->ApplyEntityStateChange(CapturedItem, CapturedParent, NewValue);
						}
					})
				]
			];
		}

		return ColumnContent;
	}

	/** Recursively sets expansion state for an item and all its children */
	void SetExpansionRecursive(TSharedPtr<FStageTreeItem> InItem, bool bExpand)
	{
		if (!InItem.IsValid() || !OwnerPanel || !OwnerPanel->StageTreeView.IsValid()) return;

		OwnerPanel->StageTreeView->SetItemExpansion(InItem, bExpand);
		for (const TSharedPtr<FStageTreeItem>& Child : InItem->Children)
		{
			SetExpansionRecursive(Child, bExpand);
		}
	}

	TSharedRef<SWidget> GenerateActionsColumnWidget()
	{
		TSharedRef<SHorizontalBox> ColumnContent = SNew(SHorizontalBox);

		if (!Item.IsValid() || !OwnerPanel) return ColumnContent;

		TSharedPtr<FStageTreeItem> CapturedItem = Item;
		TSharedPtr<FStageTreeItem> CapturedParent = ParentItem;
		SStageEditorPanel* CapturedPanel = OwnerPanel;

		// Expand All / Collapse All buttons for Stage items
		if (Item->Type == EStageTreeItemType::Stage)
		{
			// Expand All button
			ColumnContent->AddSlot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			.Padding(2, 0, 0, 0)
			[
				SNew(SButton)
				.ButtonStyle(FAppStyle::Get(), "SimpleButton")
				.ToolTipText(LOCTEXT("ExpandAllStage_Tooltip", "Expand all children of this Stage"))
				.OnClicked_Lambda([this, CapturedItem]()
				{
					SetExpansionRecursive(CapturedItem, true);
					return FReply::Handled();
				})
				[
					SNew(SImage)
					.Image(FAppStyle::GetBrush("TreeArrow_Expanded"))
					.ColorAndOpacity(FSlateColor::UseForeground())
				]
			];

			// Collapse All button
			ColumnContent->AddSlot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			.Padding(2, 0, 0, 0)
			[
				SNew(SButton)
				.ButtonStyle(FAppStyle::Get(), "SimpleButton")
				.ToolTipText(LOCTEXT("CollapseAllStage_Tooltip", "Collapse all children of this Stage"))
				.OnClicked_Lambda([this, CapturedItem]()
				{
					SetExpansionRecursive(CapturedItem, false);
					return FReply::Handled();
				})
				[
					SNew(SImage)
					.Image(FAppStyle::GetBrush("TreeArrow_Collapsed"))
					.ColorAndOpacity(FSlateColor::UseForeground())
				]
			];
		}

		// Create Act button for Acts folder
		if (Item->Type == EStageTreeItemType::ActsFolder)
		{
			ColumnContent->AddSlot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			.Padding(4, 0, 0, 0)
			[
				SNew(SButton)
				.ButtonStyle(FAppStyle::Get(), "SimpleButton")
				.ToolTipText(LOCTEXT("CreateActInline_Tooltip", "Create a new Act in this Stage"))
				.OnClicked_Lambda([CapturedPanel, CapturedItem]()
				{
					if (CapturedPanel && CapturedPanel->Controller.IsValid() && CapturedItem->Parent.IsValid())
					{
						TSharedPtr<FStageTreeItem> ParentStage = CapturedItem->Parent.Pin();
						if (ParentStage.IsValid() && ParentStage->StagePtr.IsValid())
						{
							CapturedPanel->Controller->SetActiveStage(ParentStage->StagePtr.Get());
							CapturedPanel->Controller->CreateNewAct();
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

		// Browse to Asset and Edit BP buttons for Stage items
		if (Item->Type == EStageTreeItemType::Stage && Item->StagePtr.IsValid())
		{
			// Browse to Asset button
			ColumnContent->AddSlot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			.Padding(4, 0, 0, 0)
			[
				SNew(SButton)
				.ButtonStyle(FAppStyle::Get(), "SimpleButton")
				.ToolTipText(LOCTEXT("BrowseToStageBP_Tooltip", "Browse to Stage Blueprint in Content Browser"))
				.OnClicked_Lambda([CapturedItem]()
				{
					if (CapturedItem->StagePtr.IsValid())
					{
						if (UBlueprint* Blueprint = Cast<UBlueprint>(CapturedItem->StagePtr->GetClass()->ClassGeneratedBy))
						{
							TArray<UObject*> Assets;
							Assets.Add(Blueprint);
							GEditor->SyncBrowserToObjects(Assets);
						}
					}
					return FReply::Handled();
				})
				[
					SNew(SImage)
					.Image(FAppStyle::GetBrush("Icons.BrowseContent"))
					.ColorAndOpacity(FSlateColor::UseForeground())
				]
			];

			// Edit BP button
			ColumnContent->AddSlot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			.Padding(4, 0, 0, 0)
			[
				SNew(SButton)
				.ButtonStyle(FAppStyle::Get(), "SimpleButton")
				.ToolTipText(LOCTEXT("EditStageBP_Tooltip", "Edit Stage Blueprint"))
				.OnClicked_Lambda([CapturedItem]()
				{
					if (CapturedItem->StagePtr.IsValid())
					{
						if (UBlueprint* Blueprint = Cast<UBlueprint>(CapturedItem->StagePtr->GetClass()->ClassGeneratedBy))
						{
							GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()->OpenEditorForAsset(Blueprint);
						}
					}
					return FReply::Handled();
				})
				[
					SNew(SImage)
					.Image(FAppStyle::GetBrush("Icons.Edit"))
					.ColorAndOpacity(FSlateColor::UseForeground())
				]
			];

			// Delete Stage button
			ColumnContent->AddSlot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			.Padding(4, 0, 0, 0)
			[
				SNew(SButton)
				.ButtonStyle(FAppStyle::Get(), "SimpleButton")
				.ToolTipText(LOCTEXT("DeleteStage_Tooltip", "Delete this Stage (with confirmation)"))
				.OnClicked_Lambda([CapturedPanel, CapturedItem]()
				{
					if (CapturedPanel && CapturedPanel->Controller.IsValid() && CapturedItem->StagePtr.IsValid())
					{
						CapturedPanel->Controller->DeleteStageWithConfirmation(CapturedItem->StagePtr.Get());
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

		// FollowStageState checkbox and InitialDataLayerState dropdown for Act items
		if (Item->Type == EStageTreeItemType::Act)
		{
			// Helper lambda to find parent Stage
			auto FindParentStage = [CapturedItem]() -> AStage*
			{
				TSharedPtr<FStageTreeItem> Current = CapturedItem->Parent.Pin();
				while (Current.IsValid())
				{
					if (Current->Type == EStageTreeItemType::Stage && Current->StagePtr.IsValid())
					{
						return Current->StagePtr.Get();
					}
					Current = Current->Parent.Pin();
				}
				return nullptr;
			};

			// Get current values for this Act
			int32 ActID = Item->ID;
			bool bFollowStageState = false;
			EDataLayerRuntimeState CurrentState = EDataLayerRuntimeState::Unloaded;
			if (AStage* Stage = FindParentStage())
			{
				for (const FAct& Act : Stage->Acts)
				{
					if (Act.SUID.ActID == ActID)
					{
						bFollowStageState = Act.bFollowStageState;
						CurrentState = Act.InitialDataLayerState;
						break;
					}
				}
			}

			// FollowStageState checkbox
			ColumnContent->AddSlot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			.Padding(4, 0, 0, 0)
			[
				SNew(SCheckBox)
				.IsChecked_Lambda([CapturedItem]() -> ECheckBoxState
				{
					// Find parent Stage and get current bFollowStageState
					AStage* Stage = nullptr;
					TSharedPtr<FStageTreeItem> Current = CapturedItem.IsValid() ? CapturedItem->Parent.Pin() : nullptr;
					while (Current.IsValid())
					{
						if (Current->Type == EStageTreeItemType::Stage && Current->StagePtr.IsValid())
						{
							Stage = Current->StagePtr.Get();
							break;
						}
						Current = Current->Parent.Pin();
					}

					if (!Stage || !CapturedItem.IsValid()) return ECheckBoxState::Unchecked;

					int32 ActID = CapturedItem->ID;
					for (const FAct& Act : Stage->Acts)
					{
						if (Act.SUID.ActID == ActID)
						{
							return Act.bFollowStageState ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
						}
					}
					return ECheckBoxState::Unchecked;
				})
				.OnCheckStateChanged_Lambda([CapturedItem, CapturedPanel](ECheckBoxState NewState)
				{
					if (!CapturedItem.IsValid() || !CapturedPanel) return;

					// Find parent Stage
					AStage* Stage = nullptr;
					TSharedPtr<FStageTreeItem> Current = CapturedItem->Parent.Pin();
					while (Current.IsValid())
					{
						if (Current->Type == EStageTreeItemType::Stage && Current->StagePtr.IsValid())
						{
							Stage = Current->StagePtr.Get();
							break;
						}
						Current = Current->Parent.Pin();
					}

					if (!Stage) return;

					// Find and modify the Act
					int32 ActID = CapturedItem->ID;
					bool bNewFollowState = (NewState == ECheckBoxState::Checked);
					for (FAct& Act : Stage->Acts)
					{
						if (Act.SUID.ActID == ActID)
						{
							if (Act.bFollowStageState != bNewFollowState)
							{
								FScopedTransaction Transaction(LOCTEXT("SetActFollowStageState", "Set Act Follow Stage State"));
								Stage->Modify();
								Act.bFollowStageState = bNewFollowState;

								// Notify Controller to broadcast model change (triggers UI refresh)
								if (CapturedPanel->Controller.IsValid())
								{
									CapturedPanel->Controller->OnModelChanged.Broadcast();
								}
							}
							break;
						}
					}
				})
				.ToolTipText(LOCTEXT("FollowStageState_Tooltip", "When checked, this Act's DataLayer state mirrors the Stage's state (Stage Loaded → Act Loaded, Stage Active → Act Active)"))
			];

			// "Follow" label
			ColumnContent->AddSlot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			.Padding(2, 0, 4, 0)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("FollowLabel", "Follow"))
				.Font(FCoreStyle::GetDefaultFontStyle("Regular", 8))
				.ColorAndOpacity_Lambda([CapturedItem]() -> FSlateColor
				{
					// Dim the text if Follow is unchecked
					AStage* Stage = nullptr;
					TSharedPtr<FStageTreeItem> Current = CapturedItem.IsValid() ? CapturedItem->Parent.Pin() : nullptr;
					while (Current.IsValid())
					{
						if (Current->Type == EStageTreeItemType::Stage && Current->StagePtr.IsValid())
						{
							Stage = Current->StagePtr.Get();
							break;
						}
						Current = Current->Parent.Pin();
					}

					if (!Stage || !CapturedItem.IsValid()) return FSlateColor::UseSubduedForeground();

					int32 ActID = CapturedItem->ID;
					for (const FAct& Act : Stage->Acts)
					{
						if (Act.SUID.ActID == ActID)
						{
							return Act.bFollowStageState ? FSlateColor::UseForeground() : FSlateColor::UseSubduedForeground();
						}
					}
					return FSlateColor::UseSubduedForeground();
				})
			];

			// Build state options list
			static TArray<TSharedPtr<FText>> DataLayerStateOptions;
			if (DataLayerStateOptions.Num() == 0)
			{
				DataLayerStateOptions.Add(MakeShared<FText>(LOCTEXT("DLS_Unloaded", "Unloaded")));
				DataLayerStateOptions.Add(MakeShared<FText>(LOCTEXT("DLS_Loaded", "Loaded")));
				DataLayerStateOptions.Add(MakeShared<FText>(LOCTEXT("DLS_Activated", "Activated")));
			}

			// Convert current state to option index
			int32 CurrentIndex = 0;
			switch (CurrentState)
			{
			case EDataLayerRuntimeState::Unloaded: CurrentIndex = 0; break;
			case EDataLayerRuntimeState::Loaded: CurrentIndex = 1; break;
			case EDataLayerRuntimeState::Activated: CurrentIndex = 2; break;
			default: CurrentIndex = 0; break;
			}

			// InitialDataLayerState dropdown (disabled when FollowStageState is checked)
			ColumnContent->AddSlot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			.Padding(0, 0, 0, 0)
			[
				SNew(SBox)
				.WidthOverride(80.0f)
				.IsEnabled_Lambda([CapturedItem]() -> bool
				{
					// Disable dropdown when bFollowStageState is checked
					AStage* Stage = nullptr;
					TSharedPtr<FStageTreeItem> Current = CapturedItem.IsValid() ? CapturedItem->Parent.Pin() : nullptr;
					while (Current.IsValid())
					{
						if (Current->Type == EStageTreeItemType::Stage && Current->StagePtr.IsValid())
						{
							Stage = Current->StagePtr.Get();
							break;
						}
						Current = Current->Parent.Pin();
					}

					if (!Stage || !CapturedItem.IsValid()) return true;

					int32 ActID = CapturedItem->ID;
					for (const FAct& Act : Stage->Acts)
					{
						if (Act.SUID.ActID == ActID)
						{
							return !Act.bFollowStageState; // Disable if following
						}
					}
					return true;
				})
				[
					SNew(SComboBox<TSharedPtr<FText>>)
					.OptionsSource(&DataLayerStateOptions)
					.InitiallySelectedItem(DataLayerStateOptions[CurrentIndex])
					.ToolTipText(LOCTEXT("ActInitialDataLayerState_Tooltip", "Initial DataLayer state when Stage becomes Active (only used when 'Follow' is unchecked)"))
					.OnGenerateWidget_Lambda([](TSharedPtr<FText> InOption)
					{
						return SNew(STextBlock)
							.Text(*InOption)
							.Font(FCoreStyle::GetDefaultFontStyle("Regular", 9));
					})
					.OnSelectionChanged_Lambda([CapturedItem, CapturedPanel](TSharedPtr<FText> NewSelection, ESelectInfo::Type SelectInfo)
					{
						if (SelectInfo == ESelectInfo::Direct || !NewSelection.IsValid() || !CapturedItem.IsValid())
						{
							return;
						}

						// Find parent Stage
						AStage* Stage = nullptr;
						TSharedPtr<FStageTreeItem> Current = CapturedItem->Parent.Pin();
						while (Current.IsValid())
						{
							if (Current->Type == EStageTreeItemType::Stage && Current->StagePtr.IsValid())
							{
								Stage = Current->StagePtr.Get();
								break;
							}
							Current = Current->Parent.Pin();
						}

						if (!Stage) return;

						// Determine new state from selection
						EDataLayerRuntimeState NewState = EDataLayerRuntimeState::Unloaded;
						FString SelectionText = NewSelection->ToString();
						if (SelectionText.Contains(TEXT("Activated")))
						{
							NewState = EDataLayerRuntimeState::Activated;
						}
						else if (SelectionText.Contains(TEXT("Unloaded")))
						{
							// Check Unloaded BEFORE Loaded because "Unloaded" contains "Loaded"
							NewState = EDataLayerRuntimeState::Unloaded;
						}
						else if (SelectionText.Contains(TEXT("Loaded")))
						{
							NewState = EDataLayerRuntimeState::Loaded;
						}

						// Find and modify the Act
						int32 ActID = CapturedItem->ID;
						for (FAct& Act : Stage->Acts)
						{
							if (Act.SUID.ActID == ActID)
							{
								if (Act.InitialDataLayerState != NewState)
								{
									FScopedTransaction Transaction(LOCTEXT("SetActInitialDataLayerState", "Set Act Initial DataLayer State"));
									Stage->Modify();
									Act.InitialDataLayerState = NewState;
								}
								break;
							}
						}
					})
					[
						SNew(STextBlock)
						.Text_Lambda([CapturedItem]() -> FText
						{
							// Find parent Stage and get current state
							AStage* Stage = nullptr;
							TSharedPtr<FStageTreeItem> Current = CapturedItem.IsValid() ? CapturedItem->Parent.Pin() : nullptr;
							while (Current.IsValid())
							{
								if (Current->Type == EStageTreeItemType::Stage && Current->StagePtr.IsValid())
								{
									Stage = Current->StagePtr.Get();
									break;
								}
								Current = Current->Parent.Pin();
							}

							if (!Stage || !CapturedItem.IsValid()) return LOCTEXT("DLS_Unknown", "?");

							int32 ActID = CapturedItem->ID;
							for (const FAct& Act : Stage->Acts)
							{
								if (Act.SUID.ActID == ActID)
								{
									// Show "(Follow)" if following Stage state
									if (Act.bFollowStageState)
									{
										return LOCTEXT("DLS_Follow", "(Follow)");
									}
									switch (Act.InitialDataLayerState)
									{
									case EDataLayerRuntimeState::Unloaded: return LOCTEXT("DLS_U", "Unloaded");
									case EDataLayerRuntimeState::Loaded: return LOCTEXT("DLS_L", "Loaded");
									case EDataLayerRuntimeState::Activated: return LOCTEXT("DLS_A", "Activated");
									default: return LOCTEXT("DLS_Unknown2", "?");
									}
								}
							}
							return LOCTEXT("DLS_NotFound", "?");
						})
						.Font(FCoreStyle::GetDefaultFontStyle("Regular", 9))
					]
				]
			];
		}

		// Delete button for Act items (except Default Act)
		if (Item->Type == EStageTreeItemType::Act && Item->ID != 0)
		{
			ColumnContent->AddSlot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			.Padding(4, 0, 0, 0)
			[
				SNew(SButton)
				.ButtonStyle(FAppStyle::Get(), "SimpleButton")
				.ToolTipText(LOCTEXT("DeleteActInline_Tooltip", "Delete this Act"))
				.OnClicked_Lambda([CapturedPanel, CapturedItem]()
				{
					if (CapturedPanel && CapturedPanel->Controller.IsValid())
					{
						if (TSharedPtr<FStageTreeItem> StageItem = CapturedPanel->FindStageAncestor(CapturedItem))
						{
							if (StageItem->StagePtr.IsValid())
							{
								CapturedPanel->Controller->SetActiveStage(StageItem->StagePtr.Get());

								// Confirmation dialog
								FText ConfirmTitle = LOCTEXT("ConfirmDeleteAct", "Confirm Delete Act");
								FText ConfirmMessage = FText::Format(
									LOCTEXT("ConfirmDeleteActMsg", "Are you sure you want to delete Act '{0}'?"),
									FText::FromString(CapturedItem->DisplayName)
								);

								if (CapturedPanel->ShowConfirmDialog(ConfirmTitle, ConfirmMessage))
								{
									CapturedPanel->Controller->DeleteAct(CapturedItem->ID);
								}
							}
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

		// Delete button for Entity items
		if (Item->Type == EStageTreeItemType::Entity && (bParentIsAct || bParentIsEntitiesFolder))
		{
			FText TooltipText = bParentIsAct
				? LOCTEXT("RemoveEntityFromActInline_Tooltip", "Remove this Entity from the Act")
				: LOCTEXT("UnregisterEntityInline_Tooltip", "Unregister this Entity from the Stage");

			ColumnContent->AddSlot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			.Padding(4, 0, 0, 0)
			[
				SNew(SButton)
				.ButtonStyle(FAppStyle::Get(), "SimpleButton")
				.ToolTipText(TooltipText)
				.OnClicked_Lambda([CapturedPanel, CapturedItem, CapturedParent, bIsAct = bParentIsAct]()
				{
					if (CapturedPanel && CapturedPanel->Controller.IsValid())
					{
						TSharedPtr<FStageTreeItem> StartItem = bIsAct ? CapturedParent : CapturedItem;
						if (TSharedPtr<FStageTreeItem> StageItem = CapturedPanel->FindStageAncestor(StartItem))
						{
							if (StageItem->StagePtr.IsValid())
							{
								CapturedPanel->Controller->SetActiveStage(StageItem->StagePtr.Get());

								if (bIsAct)
								{
									CapturedPanel->Controller->RemoveEntityFromAct(CapturedItem->ID, CapturedParent->ID);
								}
								else
								{
									FText ConfirmTitle = LOCTEXT("ConfirmUnregisterEntityInline", "Confirm Unregister");
									FText ConfirmMessage = FText::Format(
										LOCTEXT("ConfirmUnregisterEntityInlineMsg", "Are you sure you want to unregister '{0}' from the Stage?\n\nThis will remove it from all Acts."),
										FText::FromString(CapturedItem->DisplayName)
									);

									if (CapturedPanel->ShowConfirmDialog(ConfirmTitle, ConfirmMessage))
									{
										CapturedPanel->Controller->UnregisterAllEntities(CapturedItem->ID);
									}
								}
							}
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

		return ColumnContent;
	}
};

#pragma endregion SStageTreeRow

#pragma region Callbacks

TSharedRef<ITableRow> SStageEditorPanel::OnGenerateRow(TSharedPtr<FStageTreeItem> Item, const TSharedRef<STableViewBase>& OwnerTable)
{
	return SNew(SStageTreeRow, OwnerTable)
		.Item(Item)
		.OwnerPanel(this);
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

	// IMPORTANT: Do NOT call any Controller methods here that would trigger OnModelChanged!
	// OnModelChanged -> RefreshUI -> Tree rebuild -> Selection lost
	//
	// All Controller operations (SetActiveStage, PreviewAct, etc.) are deferred to:
	// - OnRowDoubleClicked: for viewport sync and act preview
	// - Context menu actions: for explicit operations
	// - Button clicks: for explicit operations
	//
	// Single-click only selects the tree item without side effects.
}

void SStageEditorPanel::OnRowDoubleClicked(TSharedPtr<FStageTreeItem> Item)
{
	if (!Item.IsValid() || !Controller.IsValid()) return;

	// Set active stage and perform type-specific actions
	AActor* ActorToFocus = nullptr;

	if (Item->Type == EStageTreeItemType::Stage && Item->StagePtr.IsValid())
	{
		Controller->SetActiveStage(Item->StagePtr.Get());
		ActorToFocus = Item->StagePtr.Get();
	}
	else if (Item->Type == EStageTreeItemType::Act)
	{
		// Set active stage first
		if (TSharedPtr<FStageTreeItem> StageItem = FindStageAncestor(Item))
		{
			if (StageItem->StagePtr.IsValid())
			{
				Controller->SetActiveStage(StageItem->StagePtr.Get());
			}
		}
		// Preview the Act (activates DataLayer etc.)
		Controller->PreviewAct(Item->ID);
	}
	else if (Item->Type == EStageTreeItemType::Entity && Item->ActorPtr.IsValid())
	{
		// Set active stage first
		if (TSharedPtr<FStageTreeItem> StageItem = FindStageAncestor(Item))
		{
			if (StageItem->StagePtr.IsValid())
			{
				Controller->SetActiveStage(StageItem->StagePtr.Get());
			}
		}
		ActorToFocus = Item->ActorPtr.Get();
	}
	else if (Item->Type == EStageTreeItemType::ActsFolder || Item->Type == EStageTreeItemType::EntitiesFolder)
	{
		// Set active stage for folder items
		if (TSharedPtr<FStageTreeItem> StageItem = FindStageAncestor(Item))
		{
			if (StageItem->StagePtr.IsValid())
			{
				Controller->SetActiveStage(StageItem->StagePtr.Get());
			}
		}
	}

	// Focus viewport on actor if applicable
	if (ActorToFocus && GEditor)
	{
		GEditor->SelectNone(false, true);
		GEditor->SelectActor(ActorToFocus, true, true);
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

FReply SStageEditorPanel::OnRegisterSelectedEntitiesClicked()
{
	if (Controller.IsValid() && GEditor)
	{
		TArray<AActor*> SelectedActors;
		GEditor->GetSelectedActors()->GetSelectedObjects<AActor>(SelectedActors);
		Controller->RegisterEntities(SelectedActors);
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

		// Load default parent class from settings
		UClass* DefaultParentClass = nullptr;

		// Check if path is not null (don't use IsValid() as it only returns true for already-loaded assets)
		if (!Settings->DefaultStageBlueprintParentClass.IsNull())
		{
			DefaultParentClass = Settings->DefaultStageBlueprintParentClass.LoadSynchronous();
			UE_LOG(LogStageEditor, Log, TEXT("Loaded DefaultStageBlueprintParentClass: %s"),
				DefaultParentClass ? *DefaultParentClass->GetName() : TEXT("nullptr (failed to load)"));
		}
		else
		{
			UE_LOG(LogStageEditor, Warning, TEXT("DefaultStageBlueprintParentClass is null"));
		}

		// Call with default parent class and name prefix
		Controller->CreateStageBlueprint(Path, DefaultParentClass, TEXT("BP_Stage_"));
	}
	return FReply::Handled();
}

FReply SStageEditorPanel::OnCreateEntityActorBPClicked()
{
	if (Controller.IsValid() && CreationSettings.IsValid())
	{
		FAssetCreationSettings* Settings = (FAssetCreationSettings*)CreationSettings->GetStructMemory();

		FString Path;
		if (Settings->bIsCustomEntityActorAssetPath)
		{
			// Convert physical path to virtual path
			FString PhysicalPath = Settings->EntityActorAssetFolderPath.Path;
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
			Path = TEXT("/StageEditor/EntitiesBP");
		}

		// Load default parent class from settings
		UClass* DefaultParentClass = nullptr;
		if (Settings->DefaultEntityActorBlueprintParentClass.IsValid())
		{
			DefaultParentClass = Settings->DefaultEntityActorBlueprintParentClass.LoadSynchronous();
		}

		Controller->CreateEntityActorBlueprint(Path, DefaultParentClass);
	}
	return FReply::Handled();
}

FReply SStageEditorPanel::OnCreateEntityComponentBPClicked()
{
	if (Controller.IsValid() && CreationSettings.IsValid())
	{
		FAssetCreationSettings* Settings = (FAssetCreationSettings*)CreationSettings->GetStructMemory();

		FString Path;
		if (Settings->bIsCustomEntityComponentAssetPath)
		{
			// Convert physical path to virtual path
			FString PhysicalPath = Settings->EntityComponentAssetFolderPath.Path;
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
			Path = TEXT("/StageEditor/EntitiesBP");
		}

		// Load default parent class from settings
		UClass* DefaultParentClass = nullptr;
		if (Settings->DefaultEntityComponentBlueprintParentClass.IsValid())
		{
			DefaultParentClass = Settings->DefaultEntityComponentBlueprintParentClass.LoadSynchronous();
		}

		Controller->CreateEntityComponentBlueprint(Path, DefaultParentClass);
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

FReply SStageEditorPanel::OnCleanOrphanedEntitiesClicked()
{
	if (Controller.IsValid())
	{
		int32 CleanedCount = Controller->CleanOrphanedEntities();

		// Show feedback message
		FText Message = FText::Format(
			LOCTEXT("OrphanedEntitiesCleaned", "Cleaned {0} orphaned Entity(ies)."),
			FText::AsNumber(CleanedCount)
		);

		FMessageDialog::Open(
			EAppMsgType::Ok,
			Message,
			LOCTEXT("CleanOrphanedEntitiesTitle", "Clean Orphaned Entities")
		);
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
					OnRegisterSelectedEntitiesClicked(); 
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
			LOCTEXT("UnregisterAllEntities", "Unregister All Entities"),
			LOCTEXT("UnregisterAllEntities_Tooltip", "Unregister all Entities from this Stage"),
			FSlateIcon(),
			FUIAction(FExecuteAction::CreateLambda([this, Item]()
			{
				if (Controller.IsValid() && Item->StagePtr.IsValid())
				{
					// Confirmation dialog
					FText ConfirmTitle = LOCTEXT("ConfirmUnregisterAll", "Confirm Unregister All");
					FText ConfirmMessage = LOCTEXT("ConfirmUnregisterAllMsg", "Are you sure you want to unregister all Entities from this Stage?\n\nThis will remove them from all Acts.");

					if (ShowConfirmDialog(ConfirmTitle, ConfirmMessage))
					{
						Controller->SetActiveStage(Item->StagePtr.Get());
						Controller->UnregisterAllEntities();
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
					if (TSharedPtr<FStageTreeItem> StageItem = FindStageAncestor(Item))
					{
						if (StageItem->StagePtr.IsValid())
						{
							Controller->SetActiveStage(StageItem->StagePtr.Get());
							Controller->CreateDataLayerForAct(Item->ID);
						}
					}
				}
			}))
		);

		MenuBuilder.AddMenuEntry(
			LOCTEXT("LinkExistingDataLayer", "Link Existing DataLayer"),
			LOCTEXT("LinkExistingDataLayer_Tooltip", "Link an existing DataLayer to this Act"),
			FSlateIcon(FAppStyle::GetAppStyleSetName(), "Icons.Link"),
			FUIAction(FExecuteAction::CreateLambda([this, Item]()
			{
				if (Controller.IsValid())
				{
					if (TSharedPtr<FStageTreeItem> StageItem = FindStageAncestor(Item))
					{
						if (StageItem->StagePtr.IsValid())
						{
							Controller->SetActiveStage(StageItem->StagePtr.Get());
							ShowLinkDataLayerDialog(Item->ID);
						}
					}
				}
			}))
		);

		MenuBuilder.AddMenuSeparator();

		MenuBuilder.AddMenuEntry(
			LOCTEXT("RemoveAllEntitiesFromAct", "Remove All Entities from Act"),
			LOCTEXT("RemoveAllEntitiesFromAct_Tooltip", "Remove all Entities from this Act (they remain registered to the Stage)"),
			FSlateIcon(),
			FUIAction(FExecuteAction::CreateLambda([this, Item]()
			{
				if (Controller.IsValid())
				{
					if (TSharedPtr<FStageTreeItem> StageItem = FindStageAncestor(Item))
					{
						if (StageItem->StagePtr.IsValid())
						{
							Controller->SetActiveStage(StageItem->StagePtr.Get());
							Controller->RemoveAllEntitiesFromAct(Item->ID);
						}
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
						if (TSharedPtr<FStageTreeItem> StageItem = FindStageAncestor(Item))
						{
							if (StageItem->StagePtr.IsValid())
							{
								Controller->SetActiveStage(StageItem->StagePtr.Get());

								// Confirmation dialog
								FText ConfirmTitle = LOCTEXT("ConfirmDeleteActMenu", "Confirm Delete Act");
								FText ConfirmMessage = FText::Format(
									LOCTEXT("ConfirmDeleteActMenuMsg", "Are you sure you want to delete Act '{0}'?"),
									FText::FromString(Item->DisplayName)
								);

								if (ShowConfirmDialog(ConfirmTitle, ConfirmMessage))
								{
									Controller->DeleteAct(Item->ID);
								}
							}
						}
					}
				}))
			);
		}
	}

	//----------------------------------------------------------------
	// Entity Context Menu
	//----------------------------------------------------------------
	else if (Item->Type == EStageTreeItemType::Entity)
	{
		// Check if Entity is under an Act or under Registered Entities folder
		TSharedPtr<FStageTreeItem> ParentItem = Item->Parent.Pin();
		bool bIsInAct = ParentItem.IsValid() && ParentItem->Type == EStageTreeItemType::Act;

		// Collect all selected Entities that are under the same Act (for batch operations)
		TArray<TSharedPtr<FStageTreeItem>> EntitiesInSameAct;
		int32 SharedActID = -1;
		TSharedPtr<FStageTreeItem> SharedActItem;

		if (bIsInAct)
		{
			SharedActID = ParentItem->ID;
			SharedActItem = ParentItem;

			for (const TSharedPtr<FStageTreeItem>& SelItem : SelectedItems)
			{
				if (SelItem.IsValid() && SelItem->Type == EStageTreeItemType::Entity)
				{
					TSharedPtr<FStageTreeItem> SelParent = SelItem->Parent.Pin();
					if (SelParent.IsValid() && SelParent->Type == EStageTreeItemType::Act && SelParent->ID == SharedActID)
					{
						EntitiesInSameAct.Add(SelItem);
					}
				}
			}
		}

		bool bMultipleEntitiesSelected = EntitiesInSameAct.Num() > 1;

		if (bIsInAct)
		{
			// Single Entity: Set State operation
			if (!bMultipleEntitiesSelected)
			{
				MenuBuilder.AddMenuEntry(
					LOCTEXT("SetEntityState", "Set State..."),
					LOCTEXT("SetEntityState_Tooltip", "Change the state value of this Entity in the Act"),
					FSlateIcon(),
					FUIAction(FExecuteAction::CreateLambda([this, Item, ParentItem]()
					{
						if (Controller.IsValid())
						{
							// Simple input dialog
							TSharedRef<SWindow> InputWindow = SNew(SWindow)
								.Title(LOCTEXT("SetStateTitle", "Set Entity State"))
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

												if (TSharedPtr<FStageTreeItem> StageItem = FindStageAncestor(ParentItem))
												{
													if (StageItem->StagePtr.IsValid())
													{
														Controller->SetActiveStage(StageItem->StagePtr.Get());
														Controller->SetEntityStateInAct(Item->ID, ParentItem->ID, NewState);
													}
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
			}

			// Remove from Act - single or batch
			if (bMultipleEntitiesSelected)
			{
				// Batch remove: multiple Entities selected under same Act
				MenuBuilder.AddMenuEntry(
					FText::Format(LOCTEXT("RemoveSelectedEntitiesFromAct", "Remove {0} Entities from Act"), FText::AsNumber(EntitiesInSameAct.Num())),
					LOCTEXT("RemoveSelectedEntitiesFromAct_Tooltip", "Remove all selected Entities from this Act (they remain registered to the Stage)"),
					FSlateIcon(),
					FUIAction(FExecuteAction::CreateLambda([this, EntitiesInSameAct, SharedActItem]()
					{
						if (Controller.IsValid() && SharedActItem.IsValid())
						{
							if (TSharedPtr<FStageTreeItem> StageItem = FindStageAncestor(SharedActItem))
							{
								if (StageItem->StagePtr.IsValid())
								{
									Controller->SetActiveStage(StageItem->StagePtr.Get());

									// Remove all selected Entities from the Act
									for (const TSharedPtr<FStageTreeItem>& EntityItem : EntitiesInSameAct)
									{
										if (EntityItem.IsValid())
										{
											Controller->RemoveEntityFromAct(EntityItem->ID, SharedActItem->ID);
										}
									}
								}
							}
						}
					}))
				);
			}
			else
			{
				// Single remove
				MenuBuilder.AddMenuEntry(
					LOCTEXT("RemoveEntityFromAct", "Remove from Act"),
					LOCTEXT("RemoveEntityFromAct_Tooltip", "Remove this Entity from the Act (it remains registered to the Stage)"),
					FSlateIcon(),
					FUIAction(FExecuteAction::CreateLambda([this, Item, ParentItem]()
					{
						if (Controller.IsValid())
						{
							if (TSharedPtr<FStageTreeItem> StageItem = FindStageAncestor(ParentItem))
							{
								if (StageItem->StagePtr.IsValid())
								{
									Controller->SetActiveStage(StageItem->StagePtr.Get());
									Controller->RemoveEntityFromAct(Item->ID, ParentItem->ID);
								}
							}
						}
					}))
				);
			}

			MenuBuilder.AddMenuSeparator();
		}

		//----------------------------------------------------------------
		// Add to Act Operations (for all Entities, regardless of parent)
		//----------------------------------------------------------------
		
		// Collect all selected Entities (from any parent - Act or Registered Entities folder)
		TArray<TSharedPtr<FStageTreeItem>> AllSelectedEntities;
		for (const TSharedPtr<FStageTreeItem>& SelItem : SelectedItems)
		{
			if (SelItem.IsValid() && SelItem->Type == EStageTreeItemType::Entity)
			{
				AllSelectedEntities.Add(SelItem);
			}
		}

		if (AllSelectedEntities.Num() > 0)
		{
			MenuBuilder.AddMenuSeparator();

			// "Add to New Act" - Creates a new Act and adds selected Entities to it
			FText AddToNewActLabel = AllSelectedEntities.Num() > 1
				? FText::Format(LOCTEXT("AddEntitiesToNewAct", "Add {0} Entities to New Act"), FText::AsNumber(AllSelectedEntities.Num()))
				: LOCTEXT("AddEntityToNewAct", "Add to New Act");

			MenuBuilder.AddMenuEntry(
				AddToNewActLabel,
				LOCTEXT("AddToNewAct_Tooltip", "Create a new Act and add selected Entity(s) to it"),
				FSlateIcon(FAppStyle::GetAppStyleSetName(), "Icons.Plus"),
				FUIAction(FExecuteAction::CreateLambda([this, AllSelectedEntities]()
				{
					if (Controller.IsValid() && AllSelectedEntities.Num() > 0)
					{
						if (TSharedPtr<FStageTreeItem> StageItem = FindStageAncestor(AllSelectedEntities[0]))
						{
							if (StageItem->StagePtr.IsValid())
							{
								Controller->SetActiveStage(StageItem->StagePtr.Get());

								// Create new Act
								int32 NewActID = Controller->CreateNewAct();
								if (NewActID != -1)
								{
									// Add all selected Entities to the new Act with default state 0
									for (const TSharedPtr<FStageTreeItem>& EntityItem : AllSelectedEntities)
									{
										if (EntityItem.IsValid())
										{
											Controller->SetEntityStateInAct(EntityItem->ID, NewActID, 0);
										}
									}

									FString Message = AllSelectedEntities.Num() > 1
										? FString::Printf(TEXT("Added %d Entities to new Act"), AllSelectedEntities.Num())
										: TEXT("Added Entity to new Act");
									DebugHeader::ShowNotifyInfo(Message);
								}
							}
						}
					}
				}))
			);

			// "Add to Existing Act" - Submenu with list of all Acts
			MenuBuilder.AddSubMenu(
				LOCTEXT("AddToExistingAct", "Add to Existing Act"),
				LOCTEXT("AddToExistingAct_Tooltip", "Add selected Entity(s) to an existing Act"),
				FNewMenuDelegate::CreateLambda([this, AllSelectedEntities](FMenuBuilder& SubMenuBuilder)
				{
					if (Controller.IsValid() && AllSelectedEntities.Num() > 0)
					{
						// Find parent Stage
						AStage* Stage = nullptr;
						if (TSharedPtr<FStageTreeItem> StageItem = FindStageAncestor(AllSelectedEntities[0]))
						{
							if (StageItem->StagePtr.IsValid())
							{
								Stage = StageItem->StagePtr.Get();
							}
						}

						if (Stage)
						{
							// Add menu entry for each Act
							for (const FAct& Act : Stage->Acts)
							{
								FText ActLabel = FText::FromString(Act.DisplayName);
								int32 ActID = Act.SUID.ActID;

								SubMenuBuilder.AddMenuEntry(
									ActLabel,
									FText::Format(LOCTEXT("AddToActTooltip", "Add selected Entity(s) to {0}"), ActLabel),
									FSlateIcon(),
									FUIAction(FExecuteAction::CreateLambda([this, AllSelectedEntities, Stage, ActID]()
									{
										if (Controller.IsValid())
										{
											Controller->SetActiveStage(Stage);
											
											// Add all selected Entities to the chosen Act with default state 0
											for (const TSharedPtr<FStageTreeItem>& EntityItem : AllSelectedEntities)
											{
												if (EntityItem.IsValid())
												{
													Controller->SetEntityStateInAct(EntityItem->ID, ActID, 0);
												}
											}
											
											FString Message = AllSelectedEntities.Num() > 1
												? FString::Printf(TEXT("Added %d Entities to Act"), AllSelectedEntities.Num())
												: TEXT("Added Entity to Act");
											DebugHeader::ShowNotifyInfo(Message);
										}
									}))
								);
							}
						}
					}
				}),
				false,
				FSlateIcon(FAppStyle::GetAppStyleSetName(), "Icons.Link")
			);
		}

		MenuBuilder.AddMenuSeparator();

		// Unregister from Stage - single or batch
		if (SelectedItems.Num() > 1)
		{
			// Count all Entities in selection (regardless of parent)
			TArray<TSharedPtr<FStageTreeItem>> AllSelectedEntitiesForUnregister;
			for (const TSharedPtr<FStageTreeItem>& SelItem : SelectedItems)
			{
				if (SelItem.IsValid() && SelItem->Type == EStageTreeItemType::Entity)
				{
					AllSelectedEntitiesForUnregister.Add(SelItem);
				}
			}

			if (AllSelectedEntitiesForUnregister.Num() > 1)
			{
				MenuBuilder.AddMenuEntry(
					FText::Format(LOCTEXT("UnregisterSelectedEntities", "Unregister {0} Entities from Stage"), FText::AsNumber(AllSelectedEntitiesForUnregister.Num())),
					LOCTEXT("UnregisterSelectedEntities_Tooltip", "Completely unregister all selected Entities from the Stage (removes from all Acts)"),
					FSlateIcon(),
					FUIAction(FExecuteAction::CreateLambda([this, AllSelectedEntitiesForUnregister]()
					{
						if (Controller.IsValid())
						{
							// Confirmation dialog
							FText ConfirmTitle = LOCTEXT("ConfirmUnregisterMultiple", "Confirm Unregister");
							FText ConfirmMessage = FText::Format(
								LOCTEXT("ConfirmUnregisterMultipleMsg", "Are you sure you want to unregister {0} Entities from the Stage?\n\nThis will remove them from all Acts."),
								FText::AsNumber(AllSelectedEntitiesForUnregister.Num())
							);

							if (ShowConfirmDialog(ConfirmTitle, ConfirmMessage))
							{
								if (TSharedPtr<FStageTreeItem> StageItem = FindStageAncestor(AllSelectedEntitiesForUnregister[0]))
								{
									if (StageItem->StagePtr.IsValid())
									{
										Controller->SetActiveStage(StageItem->StagePtr.Get());

										// Unregister all selected Entities
										for (const TSharedPtr<FStageTreeItem>& EntityItem : AllSelectedEntitiesForUnregister)
										{
											if (EntityItem.IsValid())
											{
												Controller->UnregisterAllEntities(EntityItem->ID);
											}
										}
									}
								}
							}
						}
					}))
				);
			}
			else
			{
				// Single unregister
				MenuBuilder.AddMenuEntry(
					LOCTEXT("UnregisterEntity", "Unregister from Stage"),
					LOCTEXT("UnregisterEntity_Tooltip", "Completely unregister this Entity from the Stage (removes from all Acts)"),
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

							if (ShowConfirmDialog(ConfirmTitle, ConfirmMessage))
							{
								if (TSharedPtr<FStageTreeItem> StageItem = FindStageAncestor(Item))
								{
									if (StageItem->StagePtr.IsValid())
									{
										Controller->SetActiveStage(StageItem->StagePtr.Get());
										Controller->UnregisterAllEntities(Item->ID);
									}
								}
							}
						}
					}))
				);
			}
		}
		else
		{
			// Single unregister
			MenuBuilder.AddMenuEntry(
				LOCTEXT("UnregisterEntity", "Unregister from Stage"),
				LOCTEXT("UnregisterEntity_Tooltip", "Completely unregister this Entity from the Stage (removes from all Acts)"),
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

						if (ShowConfirmDialog(ConfirmTitle, ConfirmMessage))
						{
							if (TSharedPtr<FStageTreeItem> StageItem = FindStageAncestor(Item))
							{
								if (StageItem->StagePtr.IsValid())
								{
									Controller->SetActiveStage(StageItem->StagePtr.Get());
									Controller->UnregisterAllEntities(Item->ID);
								}
							}
						}
					}
				}))
			);
		}
	}

	return MenuBuilder.MakeWidget();
}
#pragma endregion Callbacks

#pragma region Drag & Drop Support
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

	// Handle Entity Drag Drop (Internal)
	if (Operation->IsOfType<FEntityDragDropOp>())
	{
		TSharedPtr<FEntityDragDropOp> EntityDragDrop = StaticCastSharedPtr<FEntityDragDropOp>(Operation);
		
		// Identify Target Act
		int32 TargetActID = -1;
		if (TargetItem->Type == EStageTreeItemType::Act)
		{
			TargetActID = TargetItem->ID;
		}
		else if (TargetItem->Type == EStageTreeItemType::Entity)
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
			for (const TSharedPtr<FStageTreeItem>& EntityItem : EntityDragDrop->EntityItems)
			{
				if (EntityItem.IsValid() && EntityItem->ID != -1)
				{
					// Add entity to Act with default state 0
					// SetEntityStateInAct will add it if not present, or update if present
					if (Controller->SetEntityStateInAct(EntityItem->ID, TargetActID, 0))
					{
						AddedCount++;
					}
				}
			}
			
			if (AddedCount > 0)
			{
				DebugHeader::ShowNotifyInfo(FString::Printf(TEXT("Added %d Entities to Act"), AddedCount));
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

	// Register actors as entities to the specific Stage
	if (ActorsToRegister.Num() > 0 && Controller.IsValid())
	{
		if (Controller->RegisterEntities(ActorsToRegister, TargetStage))
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
#pragma endregion Drag & Drop Support

#pragma region Private Helpers

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
	// Check if this is an actor drag operation from World Outliner or internal Entity drag
	TSharedPtr<FDragDropOperation> Operation = DragDropEvent.GetOperation();
	bool bIsActorDrag = Operation.IsValid() && Operation->IsOfType<FActorDragDropOp>();
	bool bIsEntityDrag = Operation.IsValid() && Operation->IsOfType<FEntityDragDropOp>();

	if (!bIsActorDrag && !bIsEntityDrag)
	{
		return;
	}

	// For internal Entity drag (from Registered Entities to Act), highlight the target Act only
	if (bIsEntityDrag)
	{
		// Find the Act item that would receive this drop
		TSharedPtr<FStageTreeItem> ActItem;
		TSharedPtr<FStageTreeItem> Current = TargetItem;
		while (Current.IsValid())
		{
			if (Current->Type == EStageTreeItemType::Act)
			{
				ActItem = Current;
				break;
			}
			Current = Current->Parent.Pin();
		}

		// Only highlight if we found a valid Act target
		DraggedOverItem = ActItem; // Will be nullptr if not over an Act
		return;
	}

	// For Actor drag from World Outliner, highlight the entire Stage
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

bool SStageEditorPanel::ShowConfirmDialog(const FText& Title, const FText& Message) const
{
	return FMessageDialog::Open(EAppMsgType::YesNo, Message, Title) == EAppReturnType::Yes;
}

void SStageEditorPanel::ApplyEntityStateChange(TSharedPtr<FStageTreeItem> EntityItem, TSharedPtr<FStageTreeItem> ParentActItem, int32 NewState)
{
	if (!Controller.IsValid() || !EntityItem.IsValid() || !ParentActItem.IsValid())
	{
		return;
	}

	if (EntityItem->bHasEntityState && EntityItem->EntityState == NewState)
	{
		return;
	}

	TSharedPtr<FStageTreeItem> StageItem = FindStageAncestor(ParentActItem);
	if (!StageItem.IsValid() || !StageItem->StagePtr.IsValid())
	{
		return;
	}

	Controller->SetActiveStage(StageItem->StagePtr.Get());
	Controller->SetEntityStateInAct(EntityItem->ID, ParentActItem->ID, NewState);
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

void SStageEditorPanel::ShowLinkDataLayerDialog(int32 ActID)
{
	if (!Controller.IsValid()) return;

	// TODO: Implement DataLayer linking dialog
	// This feature requires AnalyzeDataLayerHierarchy and LinkDataLayerToAct functions
	FMessageDialog::Open(EAppMsgType::Ok,
		LOCTEXT("LinkDataLayerNotImplemented", "DataLayer linking dialog is not yet implemented.\n\nUse the Details panel to assign DataLayers to Acts."));
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

bool SStageEditorPanel::IsWorldPartitionLevel() const
{
	if (!Controller.IsValid())
	{
		return false;
	}

	return Controller->IsWorldPartitionActive();
}

FReply SStageEditorPanel::OnConvertToWorldPartitionClicked()
{
	if (!Controller.IsValid())
	{
		return FReply::Handled();
	}

	// Show confirmation dialog
	const FText Title = LOCTEXT("ConvertConfirmTitle", "Convert to World Partition?");
	const FText Message = LOCTEXT("ConvertConfirmMessage",
		"This will convert the current level to World Partition format.\n\n"
		"Important:\n"
		"- This operation cannot be undone\n"
		"- The level will be saved and reloaded\n"
		"- Make sure you have saved all your work\n\n"
		"Do you want to continue?");

	if (ShowConfirmDialog(Title, Message))
	{
		// Call conversion (this opens UE's native conversion dialog)
		Controller->ConvertToWorldPartition();

		// Check if conversion succeeded (user completed the dialog)
		const bool bNowIsWorldPartition = IsWorldPartitionLevel();

		if (bNowIsWorldPartition)
		{
			// Conversion succeeded - show success message
			DebugHeader::ShowMsgDialog(
				EAppMsgType::Ok,
				TEXT("World Partition conversion completed successfully!\n\n"
					"The level is now using World Partition.\n"
					"Stage Editor features are now available."),
				false  // Not a warning, it's a success
			);

			// Update cache and rebuild UI
			bCachedIsWorldPartition = true;
			RebuildUI();
		}
		else
		{
			// Conversion was cancelled or failed
			DebugHeader::ShowMsgDialog(
				EAppMsgType::Ok,
				TEXT("World Partition conversion was cancelled or failed.\n\n"
					"The level is still not using World Partition.\n"
					"Please try again or convert manually via the Level menu."),
				true  // Show as warning
			);
		}
	}

	return FReply::Handled();
}

FReply SStageEditorPanel::OnRefreshWorldPartitionStatusClicked()
{
	const bool bNowIsWorldPartition = IsWorldPartitionLevel();

	if (bNowIsWorldPartition)
	{
		// World Partition is now active - show success and rebuild
		DebugHeader::ShowMsgDialog(
			EAppMsgType::Ok,
			TEXT("World Partition detected!\n\n"
				"Stage Editor features are now available."),
			false
		);

		bCachedIsWorldPartition = true;
		RebuildUI();
	}
	else
	{
		// Still not World Partition
		DebugHeader::ShowMsgDialog(
			EAppMsgType::Ok,
			TEXT("World Partition is still not active.\n\n"
				"Please complete the conversion process first.\n"
				"If you just converted, try saving and reloading the level."),
			true
		);
	}

	return FReply::Handled();
}

void SStageEditorPanel::OnAssetCreationSettingsChanged(const FPropertyChangedEvent& PropertyChangedEvent)
{
	if (!Controller.IsValid() || !CreationSettings.IsValid())
	{
		return;
	}

	FAssetCreationSettings* Settings = (FAssetCreationSettings*)CreationSettings->GetStructMemory();
	if (!Settings)
	{
		return;
	}

	// Update Controller's DataLayer asset folder path when settings change
	FString DataLayerPath;
	if (Settings->bIsCustomDataLayerAssetPath)
	{
		// Convert physical path to virtual path if needed
		FString PhysicalPath = Settings->DataLayerAssetFolderPath.Path;
		FString ProjectContentDir = FPaths::ProjectContentDir();

		if (PhysicalPath.StartsWith(ProjectContentDir))
		{
			FString RelativePath = PhysicalPath.RightChop(ProjectContentDir.Len());
			DataLayerPath = TEXT("/Game/") + RelativePath;
		}
		else
		{
			// Assume it's already a virtual path
			DataLayerPath = PhysicalPath;
		}
	}
	else
	{
		DataLayerPath = TEXT("/StageEditor/DataLayers");
	}

	Controller->SetDataLayerAssetFolderPath(DataLayerPath);
}

FReply SStageEditorPanel::OnOpenSettingsClicked()
{
	// If window already exists and is valid, bring it to front
	if (SettingsWindow.IsValid())
	{
		TSharedPtr<SWindow> ExistingWindow = SettingsWindow.Pin();
		if (ExistingWindow.IsValid())
		{
			ExistingWindow->BringToFront();
			return FReply::Handled();
		}
	}

	// Create a new settings window
	TSharedRef<SWindow> NewWindow = SNew(SWindow)
		.Title(LOCTEXT("SettingsWindowTitle", "Asset Creation Settings"))
		.SizingRule(ESizingRule::Autosized)
		.SupportsMaximize(false)
		.SupportsMinimize(false)
		.HasCloseButton(true)
		[
			SNew(SBorder)
			.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
			.Padding(10)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SettingsDetailsView->GetWidget().ToSharedRef()
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0, 10, 0, 0)
				.HAlign(HAlign_Right)
				[
					SNew(SButton)
					.Text(LOCTEXT("CloseSettings", "Close"))
					.OnClicked_Lambda([this]()
					{
						CloseSettingsWindow();
						return FReply::Handled();
					})
				]
			]
		];

	// Store weak reference
	SettingsWindow = NewWindow;

	// Add as a standalone window
	FSlateApplication::Get().AddWindow(NewWindow);

	return FReply::Handled();
}

void SStageEditorPanel::CloseSettingsWindow()
{
	if (SettingsWindow.IsValid())
	{
		TSharedPtr<SWindow> Window = SettingsWindow.Pin();
		if (Window.IsValid())
		{
			Window->RequestDestroyWindow();
		}
	}
	SettingsWindow.Reset();
}

#undef LOCTEXT_NAMESPACE
#pragma endregion Private Helpers
