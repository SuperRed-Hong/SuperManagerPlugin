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

	/** Registers a list of Actors as Props to the Active Stage. */
	bool RegisterProps(const TArray<AActor*>& ActorsToRegister);

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

	//----------------------------------------------------------------
	// Editor Delegates
	//----------------------------------------------------------------

	void BindEditorDelegates();
	void UnbindEditorDelegates();

	void OnLevelActorAdded(AActor* InActor);
	void OnLevelActorDeleted(AActor* InActor);
};
