#pragma once

#pragma region Imports
#include "CoreMinimal.h"
#include "EditorSubsystem.h"
#include "StageEditorSubsystem.generated.h"
#pragma endregion Imports

#pragma region Forward Declarations
class AStage;
#pragma endregion Forward Declarations

/**
 * @brief Editor Subsystem for managing Stage registration and ID allocation.
 *
 * This subsystem maintains a central registry of all Stage actors in the editor
 * and provides unique ID allocation for each Stage. It ensures that every Stage
 * has a globally unique StageID that persists across sessions.
 *
 * The Subsystem follows these key design decisions:
 * - StageID starts from 1 (0 is reserved for invalid/unregistered)
 * - IDs are never reused even after Stage deletion (to prevent confusion)
 * - Uses TWeakObjectPtr to avoid preventing garbage collection
 *
 * @see AStage
 */
UCLASS()
class STAGEEDITOR_API UStageEditorSubsystem : public UEditorSubsystem
{
	GENERATED_BODY()

public:
#pragma region Lifecycle
	//----------------------------------------------------------------
	// Lifecycle
	//----------------------------------------------------------------

	/** Called when the subsystem is initialized. */
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	/** Called when the subsystem is deinitialized. */
	virtual void Deinitialize() override;
#pragma endregion Lifecycle

public:
#pragma region Stage Registration API
	//----------------------------------------------------------------
	// Stage Registration API
	//----------------------------------------------------------------

	/**
	 * @brief Register a Stage actor and allocate a unique StageID.
	 *
	 * If the Stage already has a valid StageID (> 0), it will be registered
	 * with its existing ID. Otherwise, a new ID will be allocated.
	 *
	 * @param Stage - The Stage actor to register
	 * @return The allocated StageID, or -1 if registration failed
	 */
	UFUNCTION(BlueprintCallable, Category = "Stage Editor")
	int32 RegisterStage(AStage* Stage);

	/**
	 * @brief Unregister a Stage actor by its ID.
	 *
	 * The ID will not be reused after unregistration.
	 *
	 * @param StageID - The ID of the Stage to unregister
	 */
	UFUNCTION(BlueprintCallable, Category = "Stage Editor")
	void UnregisterStage(int32 StageID);

	/**
	 * @brief Get a Stage actor by its ID.
	 *
	 * @param StageID - The ID of the Stage to retrieve
	 * @return The Stage actor, or nullptr if not found or garbage collected
	 */
	UFUNCTION(BlueprintCallable, Category = "Stage Editor")
	AStage* GetStage(int32 StageID) const;

	/**
	 * @brief Get all currently registered Stage actors.
	 *
	 * This filters out any garbage collected entries automatically.
	 *
	 * @return Array of all valid registered Stages
	 */
	UFUNCTION(BlueprintCallable, Category = "Stage Editor")
	TArray<AStage*> GetAllStages() const;

	/**
	 * @brief Check if a StageID is currently registered and valid.
	 *
	 * @param StageID - The ID to check
	 * @return True if the ID is registered and the Stage is still valid
	 */
	UFUNCTION(BlueprintCallable, Category = "Stage Editor")
	bool IsStageIDRegistered(int32 StageID) const;

	/**
	 * @brief Get the next available StageID (for preview purposes).
	 *
	 * @return The next ID that would be allocated
	 */
	UFUNCTION(BlueprintCallable, Category = "Stage Editor")
	int32 GetNextStageID() const { return NextStageID; }
#pragma endregion Stage Registration API

private:
#pragma region Internal State
	//----------------------------------------------------------------
	// Internal State
	//----------------------------------------------------------------

	/**
	 * Registry of all Stages.
	 * Key: StageID, Value: Weak pointer to Stage actor
	 */
	TMap<int32, TWeakObjectPtr<AStage>> StageRegistry;

	/** Next available StageID to allocate (starts at 1, never decreases) */
	int32 NextStageID = 1;
#pragma endregion Internal State

#pragma region Internal Methods
	//----------------------------------------------------------------
	// Internal Methods
	//----------------------------------------------------------------

	/**
	 * @brief Allocate a new unique StageID.
	 *
	 * IDs are allocated sequentially and never reused.
	 *
	 * @return A unique StageID
	 */
	int32 AllocateStageID();

	/**
	 * @brief Clean up invalid entries in the registry.
	 *
	 * Removes entries where the Stage actor has been garbage collected.
	 */
	void CleanupRegistry();

	/**
	 * @brief Scan the world for existing Stages and ensure they are registered.
	 *
	 * Called during initialization to pick up Stages that were placed before
	 * the subsystem was created.
	 */
	void ScanWorldForExistingStages();
#pragma endregion Internal Methods
};
