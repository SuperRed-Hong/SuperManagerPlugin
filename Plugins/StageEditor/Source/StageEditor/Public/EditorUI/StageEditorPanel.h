#pragma once

#pragma region Imports
#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Views/STreeView.h"
#include "EditorLogic/StageEditorController.h"
#include "Actors/Stage.h"
#include "StageEditorPanel.generated.h"
#pragma endregion Imports

#pragma region Types
enum class EStageTreeItemType
{
	Stage,
	ActsFolder,      // Acts folder
	PropsFolder,     // Registered Props folder
	Act,
	Prop
};

/**
 * Settings for asset creation paths
 */
USTRUCT()
struct FAssetCreationSettings
{
	GENERATED_BODY()

	/** If true, use custom path for Stage Blueprints. Otherwise use default plugin path. */
	UPROPERTY(EditAnywhere, Category = "Asset Creation")
	bool bIsCustomStageAssetFolderPath = false;

	/** Custom folder path for Stage Blueprint creation */
	UPROPERTY(EditAnywhere, Category = "Asset Creation", meta = (EditCondition = "bIsCustomStageAssetFolderPath", ContentDir, RelativeToGameContentDir))
	FDirectoryPath StageAssetFolderPath;

	/** If true, use custom path for Prop Blueprints. Otherwise use default plugin path. */
	UPROPERTY(EditAnywhere, Category = "Asset Creation")
	bool bIsCustomPropAssetPath = false;

	/** Custom folder path for Prop Blueprint creation */
	UPROPERTY(EditAnywhere, Category = "Asset Creation", meta = (EditCondition = "bIsCustomPropAssetPath", ContentDir, RelativeToGameContentDir))
	FDirectoryPath PropAssetFolderPath;

	/** If true, use custom path for DataLayer Assets. Otherwise use default plugin path. */
	UPROPERTY(EditAnywhere, Category = "Asset Creation")
	bool bIsCustomDataLayerAssetPath = false;

	/** Custom folder path for DataLayer Asset creation */
	UPROPERTY(EditAnywhere, Category = "Asset Creation", meta = (EditCondition = "bIsCustomDataLayerAssetPath", ContentDir, RelativeToGameContentDir))
	FDirectoryPath DataLayerAssetFolderPath;

	FAssetCreationSettings()
	{
		// Default paths to plugin Content folders (virtual paths)
		StageAssetFolderPath.Path = TEXT("/StageEditor/StagesBP");
		PropAssetFolderPath.Path = TEXT("/StageEditor/PropsBP");
		DataLayerAssetFolderPath.Path = TEXT("/StageEditor/DataLayers");
	}
};

struct FStageTreeItem : public TSharedFromThis<FStageTreeItem>
{
	EStageTreeItemType Type;
	FString DisplayName;
	int32 ID; // ActID or PropID
	TWeakObjectPtr<AActor> ActorPtr; // For Props
	TWeakObjectPtr<AStage> StagePtr; // For Stage Root
	TArray<TSharedPtr<FStageTreeItem>> Children;
	TWeakPtr<FStageTreeItem> Parent;
	FString ActorPath;
	int32 PropState = 0;
	bool bHasPropState = false;

	FStageTreeItem(EStageTreeItemType InType, const FString& InName, int32 InID = -1, AActor* InActor = nullptr, AStage* InStage = nullptr)
		: Type(InType)
		, DisplayName(InName)
		, ID(InID)
		, ActorPtr(InActor)
		, StagePtr(InStage)
	{}
};

/**
 * Custom drag-drop operation for Props
 */
class FPropDragDropOp : public FDragDropOperation
{
public:
	DRAG_DROP_OPERATOR_TYPE(FPropDragDropOp, FDragDropOperation)

	TArray<TSharedPtr<FStageTreeItem>> PropItems;
	FText DefaultHoverText;
	FText CurrentHoverText;

	virtual TSharedPtr<SWidget> GetDefaultDecorator() const override
	{
		return SNew(SBorder)
			.BorderImage(FAppStyle::GetBrush("Menu.Background"))
			.Content()
			[
				SNew(STextBlock)
				.Text(CurrentHoverText)
			];
	}
};
#pragma endregion Types

/**
 * @brief Main UI Panel for the Stage Editor
 * @details Displays the Stage hierarchy (Acts, Props) and provides controls for managing them.
 */
class SStageEditorPanel : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SStageEditorPanel) {}
	SLATE_END_ARGS()

#pragma region Construction
	/**
	 * @brief Constructs the widget
	 * @param InArgs Slate arguments
	 * @param InController The logic controller for the editor
	 */
	void Construct(const FArguments& InArgs, TSharedPtr<FStageEditorController> InController);
	
	virtual ~SStageEditorPanel();
#pragma endregion Construction

#pragma region Core API
	/**
	 * @brief Refreshes the UI from the Controller's data.
	 * @details Rebuilds the tree view items based on the current state of the Stage and its Acts/Props.
	 */
	void RefreshUI();
#pragma endregion Core API

#pragma region Drag & Drop Support
	//----------------------------------------------------------------
	// Drag & Drop Support
	//----------------------------------------------------------------
	
	/**
	 * @brief Called when a drag operation enters the panel.
	 */
	virtual void OnDragEnter(const FGeometry& MyGeometry, const FDragDropEvent& DragDropEvent) override;
	
	/**
	 * @brief Called when a drag operation leaves the panel.
	 */
	virtual void OnDragLeave(const FDragDropEvent& DragDropEvent) override;
	
	/**
	 * @brief Called when a drag operation moves over the panel.
	 */
	virtual FReply OnDragOver(const FGeometry& MyGeometry, const FDragDropEvent& DragDropEvent) override;
	
	/**
	 * @brief Called when a payload is dropped onto the panel.
	 */
	virtual FReply OnDrop(const FGeometry& MyGeometry, const FDragDropEvent& DragDropEvent) override;

	/** Settings for asset creation, exposed to the details view. */
	TSharedPtr<FStructOnScope> CreationSettings;
#pragma endregion Drag & Drop Support

#pragma region Callbacks
	//----------------------------------------------------------------
	// Callbacks
	//----------------------------------------------------------------
	
	/** Generates a row widget for the tree view. */
	TSharedRef<ITableRow> OnGenerateRow(TSharedPtr<FStageTreeItem> Item, const TSharedRef<STableViewBase>& OwnerTable);
	
	/** Gets the children of a tree item. */
	void OnGetChildren(TSharedPtr<FStageTreeItem> Item, TArray<TSharedPtr<FStageTreeItem>>& OutChildren);
	
	/** Handles selection changes in the tree view. */
	void OnSelectionChanged(TSharedPtr<FStageTreeItem> Item, ESelectInfo::Type SelectInfo);
	
	/** Handler for "Create Act" button click. */
	FReply OnCreateActClicked();
	
	/** Handler for "Register Selected Props" button click. */
	FReply OnRegisterSelectedPropsClicked();
	
	/** Handler for "Create Stage BP" button click. */
	FReply OnCreateStageBPClicked();
	
	/** Handler for "Create Prop BP" button click. */
	FReply OnCreatePropBPClicked();
	
	/** Handler for "Refresh" button click. */
	FReply OnRefreshClicked();
	
	/** Handler for "Convert to World Partition" button click. */
	FReply OnConvertToWorldPartitionClicked();
	
	/** Handles dropping an item onto a row. */
	FReply OnRowDrop(const FGeometry& MyGeometry, const FDragDropEvent& DragDropEvent, TSharedPtr<FStageTreeItem> TargetItem);

	/** Handles double-clicking a row. */
	void OnRowDoubleClicked(TSharedPtr<FStageTreeItem> Item);

	/** Opens the context menu for a row. */
	TSharedPtr<SWidget> OnContextMenuOpening();
#pragma endregion Callbacks

#pragma region Private Helpers
private:
	/** Checks if the current level is a World Partition level. */
	bool IsWorldPartitionLevel() const;

	/** Helper to get a unique hash/string for a tree item to persist expansion state. */
	FString GetItemHash(TSharedPtr<FStageTreeItem> Item);

	/** Saves the current expansion state of the TreeView. */
	void SaveExpansionState(TSet<FString>& OutExpansionState);

	/** Restores the expansion state of the TreeView. */
	void RestoreExpansionState(const TSet<FString>& InExpansionState);

	/**
	 * @brief Tracks the Stage item currently being hovered during drag operations
	 * @details Used to provide visual feedback by highlighting the target Stage row.
	 *          Updated via OnRowDragEnter/Leave callbacks.
	 */
	TSharedPtr<FStageTreeItem> DraggedOverItem;

	/**
	 * @brief Handles drag enter events on tree view rows
	 * @details Identifies parent Stage and sets DraggedOverItem for visual highlighting
	 * @param DragDropEvent The drag and drop event
	 * @param TargetItem The tree item being hovered
	 */
	void OnRowDragEnter(const FDragDropEvent& DragDropEvent, TSharedPtr<FStageTreeItem> TargetItem);

	/**
	 * @brief Checks if an item is the drag target or one of its descendants
	 * @param Item The item to check
	 * @param DragTarget The current drag target (Stage item)
	 * @return true if Item is DragTarget or a descendant of DragTarget
	 */
	bool IsItemOrDescendantOf(TSharedPtr<FStageTreeItem> Item, TSharedPtr<FStageTreeItem> DragTarget);

	/**
	 * @brief Handles drag leave events on tree view rows
	 * @details Intentionally does not clear DraggedOverItem to avoid flicker between child rows
	 * @param DragDropEvent The drag and drop event
	 * @param TargetItem The tree item being left
	 */
	void OnRowDragLeave(const FDragDropEvent& DragDropEvent, TSharedPtr<FStageTreeItem> TargetItem);

	/** Registers delegates to listen for viewport selection changes. */
	void RegisterViewportSelectionListener();

	/** Removes viewport selection delegates. */
	void UnregisterViewportSelectionListener();

	/** Handles selection events originating from the viewport/world outliner. */
	void HandleViewportSelectionChanged(UObject* SelectedObject);

	/** Expands all parent items leading to the specified tree item. */
	void ExpandAncestors(TSharedPtr<FStageTreeItem> Item);

	/** Finds the stage ancestor for a given item. */
	TSharedPtr<FStageTreeItem> FindStageAncestor(TSharedPtr<FStageTreeItem> Item) const;

	/** Applies a Prop state change for items nested under Acts. */
	void ApplyPropStateChange(TSharedPtr<FStageTreeItem> PropItem, TSharedPtr<FStageTreeItem> ParentActItem, int32 NewState);

	/** Selects an actor inside the viewport/editor. */
	void SelectActorInViewport(AActor* ActorToSelect);

	/** Shows dialog for linking existing DataLayer to Act. */
	void ShowLinkDataLayerDialog(int32 ActID);

	/** Handles changes to asset creation settings. */
	void OnAssetCreationSettingsChanged(const FPropertyChangedEvent& PropertyChangedEvent);

	/** Opens the settings window when gear button is clicked. */
	FReply OnOpenSettingsClicked();

	/** Closes the settings window. */
	void CloseSettingsWindow();
#pragma endregion Private Helpers

#pragma region Private State
private:
	/** Reference to the Controller (MVC bridge) */
	TSharedPtr<FStageEditorController> Controller;

	/** The tree view widget displaying the Stage hierarchy */
	TSharedPtr<STreeView<TSharedPtr<FStageTreeItem>>> StageTreeView;

	/** Root items for the tree view (typically Stage items) */
	TArray<TSharedPtr<FStageTreeItem>> RootTreeItems;

	/** Details view for asset creation settings */
	TSharedPtr<class IStructureDetailsView> SettingsDetailsView;

	/** Weak reference to the settings popup window (to prevent multiple windows) */
	TWeakPtr<SWindow> SettingsWindow;

	/** Map of actor path to the corresponding tree item for quick selection sync. */
	TMap<FString, TWeakPtr<FStageTreeItem>> ActorPathToTreeItem;

	/** Delegate handle for selection change events. */
	FDelegateHandle ViewportSelectionDelegateHandle;

	/** Delegate handle for map changed events. */
	FDelegateHandle MapChangedHandle;

	/** Cached World Partition state to detect changes. */
	bool bCachedIsWorldPartition = false;

	/** Selection object we bound to (cached for removal). */
	TWeakObjectPtr<class USelection> ActorSelectionPtr;

	/** Called when a map is opened/changed. */
	void OnMapOpened(const FString& Filename, bool bAsTemplate);

	/** Rebuilds the entire UI (used when World Partition state changes). */
	void RebuildUI();

	/** Guards to prevent recursive selection updates. */
	bool bUpdatingTreeSelectionFromViewport = false;
	bool bUpdatingViewportSelectionFromPanel = false;
#pragma endregion Private State
};
