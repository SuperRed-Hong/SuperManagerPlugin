#pragma once

#include "CoreMinimal.h"
#include "Core/StageCoreTypes.h"

class AStage;
class AActor;

/**
 * @brief Logic controller for the Stage Editor.
 * Handles transactions and data modification for AStage.
 */
class STAGEEDITOR_API FStageEditorController : public TSharedFromThis<FStageEditorController>
{
public:
	FStageEditorController();
	~FStageEditorController();

	//----------------------------------------------------------------
	// Core API
	//----------------------------------------------------------------

	/** Sets the Stage currently being edited/active. */
	void SetActiveStage(AStage* NewStage);

	/** Returns the currently active Stage (for operations). */
	AStage* GetActiveStage() const;

	/** Returns all stages found in the level. */
	const TArray<TWeakObjectPtr<AStage>>& GetFoundStages() const { return FoundStages; }

	/** Creates a new Act in the Active Stage. */
	bool CreateNewAct();

	/** Registers a list of Actors as Props to the Active Stage (or specific TargetStage). */
	bool RegisterProps(const TArray<AActor*>& ActorsToRegister, AStage* TargetStage = nullptr);

	/** Quick create blueprint assets with custom default paths */
	void CreateStageBlueprint(const FString& DefaultPath);
	void CreatePropBlueprint(const FString& DefaultPath);

	/**
	 * @brief Initializes the controller, scanning the world for existing Stages.
	 */
	void Initialize();

	/**
	 * @brief Scans the current editor world for ALL AStage actors.
	 * Populates FoundStages.
	 */
	void FindStageInWorld();

	/**
	 * @brief Previews the specified Act by applying its Prop States.
	 * @param ActID The ID of the Act to preview.
	 */
	void PreviewAct(int32 ActID);

	//----------------------------------------------------------------
	// Prop Management
	//----------------------------------------------------------------

	/** Sets the state of a Prop within a specific Act. */
	bool SetPropStateInAct(int32 PropID, int32 ActID, int32 NewState);

	/** Removes a Prop from a specific Act (keeps it registered to Stage). */
	bool RemovePropFromAct(int32 PropID, int32 ActID);

	/** Removes all Props from a specific Act. */
	bool RemoveAllPropsFromAct(int32 ActID);

	/** Unregisters a Prop from the Stage entirely (removes from all Acts and PropRegistry). */
	bool UnregisterProp(int32 PropID);

	/** Unregisters all Props from the Stage. */
	bool UnregisterAllProps();

	//----------------------------------------------------------------
	// Act Management
	//----------------------------------------------------------------

	/** Deletes an Act from the Stage (cannot delete Default Act). */
	bool DeleteAct(int32 ActID);

	//----------------------------------------------------------------
	// Data Layer Integration
	//----------------------------------------------------------------

	/** Sets the folder path where DataLayer assets will be created. */
	void SetDataLayerAssetFolderPath(const FString& Path) { DataLayerAssetFolderPath = Path; }

	/** Creates a DataLayerAsset and saves it to Content folder. */
	class UDataLayerAsset* CreateDataLayerAsset(const FString& AssetName, const FString& FolderPath = TEXT("/StageEditor/DataLayers"));

	/** Gets or creates a DataLayerInstance for the given Asset. */
	class UDataLayerInstance* GetOrCreateInstanceForAsset(class UDataLayerAsset* Asset);

	/** Creates the root Data Layer for a Stage (Asset + Instance). */
	bool CreateDataLayerForStage(AStage* Stage);

	/** Creates a new Data Layer for an Act (Asset + Instance), as child of Stage's DataLayer. */
	bool CreateDataLayerForAct(int32 ActID);

	/** Deletes the Data Layer associated with an Act. */
	bool DeleteDataLayerForAct(int32 ActID);

	/** Assigns an existing Data Layer to the specified Act. */
	bool AssignDataLayerToAct(int32 ActID, class UDataLayerAsset* DataLayer);

	/** Syncs a Prop to the Act's Data Layer (adds the actor to the data layer). */
	bool SyncPropToDataLayer(int32 PropID, int32 ActID);

	/** Assigns a Prop to the Stage's root Data Layer. */
	bool AssignPropToStageDataLayer(int32 PropID);

	/** Removes a Prop from the Stage's root Data Layer. */
	bool RemovePropFromStageDataLayer(int32 PropID);

	/** Assigns a Prop to an Act's Data Layer. */
	bool AssignPropToActDataLayer(int32 PropID, int32 ActID);

	/** Removes a Prop from an Act's Data Layer. */
	bool RemovePropFromActDataLayer(int32 PropID, int32 ActID);

	/** Finds the DataLayerInstance for a Stage's root DataLayer. */
	class UDataLayerInstance* FindStageDataLayerInstance(AStage* Stage);

	//----------------------------------------------------------------
	// World Partition Support
	//----------------------------------------------------------------

	/** Checks if the current world has World Partition enabled. */
	bool IsWorldPartitionActive() const;

	/** Converts the current world to World Partition. */
	void ConvertToWorldPartition();

	//----------------------------------------------------------------
	// Delegates
	//----------------------------------------------------------------
	
	/** Broadcasts when the Stage data changes (to update UI). */
	DECLARE_MULTICAST_DELEGATE(FOnStageModelChanged);
	FOnStageModelChanged OnModelChanged;

private:
	void CreateBlueprintAsset(UClass* ParentClass, const FString& BaseName, const FString& DefaultPath);

	/** List of all Stages found in the level. */
	TArray<TWeakObjectPtr<AStage>> FoundStages;

	/** The Stage currently selected or active for operations. */
	TWeakObjectPtr<AStage> ActiveStage;

	/** Path where DataLayer assets are created. */
	FString DataLayerAssetFolderPath = TEXT("/StageEditor/DataLayers");

	/** Helper to get the StageManagerSubsystem. */
	class UStageManagerSubsystem* GetSubsystem() const;

	//----------------------------------------------------------------
	// Editor Delegates
	//----------------------------------------------------------------

	void BindEditorDelegates();
	void UnbindEditorDelegates();

	void OnLevelActorAdded(AActor* InActor);
	void OnLevelActorDeleted(AActor* InActor);
};
