#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Views/STreeView.h"
#include "EditorLogic/StageEditorController.h"
#include "Actors/Stage.h"
#include "StageEditorPanel.generated.h"

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

	FAssetCreationSettings()
	{
		// Default paths to plugin Content folders (virtual paths)
		StageAssetFolderPath.Path = TEXT("/StageEditor/StagesBP");
		PropAssetFolderPath.Path = TEXT("/StageEditor/PropsBP");
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

class SStageEditorPanel : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SStageEditorPanel) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, TSharedPtr<FStageEditorController> InController);
	virtual ~SStageEditorPanel();

	/** Refreshes the UI from the Controller's data. */
	void RefreshUI();

	//----------------------------------------------------------------
	// Drag & Drop Support
	//----------------------------------------------------------------
	virtual void OnDragEnter(const FGeometry& MyGeometry, const FDragDropEvent& DragDropEvent) override;
	virtual void OnDragLeave(const FDragDropEvent& DragDropEvent) override;
	virtual FReply OnDragOver(const FGeometry& MyGeometry, const FDragDropEvent& DragDropEvent) override;
	TSharedPtr<FStructOnScope> CreationSettings;

	//----------------------------------------------------------------
	// Callbacks
	//----------------------------------------------------------------
	TSharedRef<ITableRow> OnGenerateRow(TSharedPtr<FStageTreeItem> Item, const TSharedRef<STableViewBase>& OwnerTable);
	void OnGetChildren(TSharedPtr<FStageTreeItem> Item, TArray<TSharedPtr<FStageTreeItem>>& OutChildren);
	void OnSelectionChanged(TSharedPtr<FStageTreeItem> Item, ESelectInfo::Type SelectInfo);
	
	FReply OnCreateActClicked();
	FReply OnRegisterSelectedPropsClicked();
	FReply OnCreateStageBPClicked();
	FReply OnCreatePropBPClicked();
	FReply OnRefreshClicked();
	
	FReply OnRowDrop(const FGeometry& MyGeometry, const FDragDropEvent& DragDropEvent, TSharedPtr<FStageTreeItem> TargetItem);

	void OnRowDoubleClicked(TSharedPtr<FStageTreeItem> Item);

	TSharedPtr<SWidget> OnContextMenuOpening();

private:
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

private:
	/** Map of actor path to the corresponding tree item for quick selection sync. */
	TMap<FString, TWeakPtr<FStageTreeItem>> ActorPathToTreeItem;

	/** Delegate handle for selection change events. */
	FDelegateHandle ViewportSelectionDelegateHandle;

	/** Selection object we bound to (cached for removal). */
	TWeakObjectPtr<class USelection> ActorSelectionPtr;

	/** Guards to prevent recursive selection updates. */
	bool bUpdatingTreeSelectionFromViewport = false;
	bool bUpdatingViewportSelectionFromPanel = false;
};
