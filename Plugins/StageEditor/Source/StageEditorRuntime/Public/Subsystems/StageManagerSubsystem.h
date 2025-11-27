#pragma once

#pragma region Imports
#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Core/StageCoreTypes.h"
#include "StageManagerSubsystem.generated.h"
#pragma endregion Imports

#pragma region Forward Declarations
class AStage;
#pragma endregion Forward Declarations

/**
 * @brief World Subsystem for managing Stage registration, ID allocation, and cross-Stage communication.
 *
 * This subsystem maintains a central registry of all Stage actors and provides:
 * - Unique ID allocation for each Stage (editor-time)
 * - Stage lookup by ID (editor + runtime)
 * - Cross-Stage communication (runtime, H-005)
 * - State lock mechanism (runtime, H-005)
 *
 * Key design decisions:
 * - StageID starts from 1 (0 is reserved for invalid/unregistered)
 * - IDs are never reused even after Stage deletion (to prevent confusion)
 * - Uses TWeakObjectPtr to avoid preventing garbage collection
 * - Inherits from UWorldSubsystem for both editor and runtime availability
 *
 * Note: In WorldPartition projects, map switching is rare, so Subsystem lifecycle is stable.
 *
 * @see AStage
 */
UCLASS()
class STAGEEDITORRUNTIME_API UStageManagerSubsystem : public UWorldSubsystem
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

	/** Determines if this subsystem should be created for the given world. */
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
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
	UFUNCTION(BlueprintCallable, Category = "Stage Manager")
	int32 RegisterStage(AStage* Stage);

	/**
	 * @brief Unregister a Stage actor by its ID.
	 *
	 * The ID will not be reused after unregistration.
	 *
	 * @param StageID - The ID of the Stage to unregister
	 */
	UFUNCTION(BlueprintCallable, Category = "Stage Manager")
	void UnregisterStage(int32 StageID);

	/**
	 * @brief Get a Stage actor by its ID.
	 *
	 * @param StageID - The ID of the Stage to retrieve
	 * @return The Stage actor, or nullptr if not found or garbage collected
	 */
	UFUNCTION(BlueprintCallable, Category = "Stage Manager")
	AStage* GetStage(int32 StageID) const;

	/**
	 * @brief Get all currently registered Stage actors.
	 *
	 * This filters out any garbage collected entries automatically.
	 *
	 * @return Array of all valid registered Stages
	 */
	UFUNCTION(BlueprintCallable, Category = "Stage Manager")
	TArray<AStage*> GetAllStages() const;

	/**
	 * @brief Check if a StageID is currently registered and valid.
	 *
	 * @param StageID - The ID to check
	 * @return True if the ID is registered and the Stage is still valid
	 */
	UFUNCTION(BlueprintCallable, Category = "Stage Manager")
	bool IsStageIDRegistered(int32 StageID) const;

	/**
	 * @brief Get the next available StageID (for preview purposes).
	 *
	 * @return The next ID that would be allocated
	 */
	UFUNCTION(BlueprintCallable, Category = "Stage Manager")
	int32 GetNextStageID() const { return NextStageID; }

	/**
	 * @brief Get the number of currently registered Stages.
	 *
	 * @return The count of valid registered Stages
	 */
	UFUNCTION(BlueprintCallable, Category = "Stage Manager")
	int32 GetRegisteredStageCount() const;
#pragma endregion Stage Registration API

#pragma region Cross-Stage Communication API (H-005)
	//----------------------------------------------------------------
	// Cross-Stage Communication API (H-005)
	//----------------------------------------------------------------

	/**
	 * @brief Force activate a Stage by its ID.
	 *
	 * Forces the Stage to the Active state, bypassing normal TriggerZone logic.
	 * If bLockState is true, the Stage will stay in Active state until
	 * ReleaseStageOverride is called.
	 *
	 * @param StageID - The ID of the Stage to activate
	 * @param bLockState - If true, lock the Stage in Active state
	 */
	UFUNCTION(BlueprintCallable, Category = "Stage Manager|Control",
		meta = (DisplayName = "Force Activate Stage"))
	void ForceActivateStage(int32 StageID, bool bLockState = false);

	/**
	 * @brief Force unload a Stage by its ID.
	 *
	 * Forces the Stage to the Unloaded state, bypassing normal TriggerZone logic.
	 * If bLockState is true, the Stage will stay in Unloaded state until
	 * ReleaseStageOverride is called.
	 *
	 * @param StageID - The ID of the Stage to unload
	 * @param bLockState - If true, lock the Stage in Unloaded state
	 */
	UFUNCTION(BlueprintCallable, Category = "Stage Manager|Control",
		meta = (DisplayName = "Force Unload Stage"))
	void ForceUnloadStage(int32 StageID, bool bLockState = false);

	/**
	 * @brief Force a Stage to a specific state by its ID.
	 *
	 * General-purpose method to force a Stage to any state.
	 * Prefer ForceActivateStage/ForceUnloadStage for common use cases.
	 *
	 * @param StageID - The ID of the Stage to control
	 * @param NewState - The target state
	 * @param bLockState - If true, lock the Stage in the target state
	 */
	UFUNCTION(BlueprintCallable, Category = "Stage Manager|Control",
		meta = (DisplayName = "Force Stage State"))
	void ForceStageState(int32 StageID, EStageRuntimeState NewState, bool bLockState = false);

	/**
	 * @brief Release the state override on a Stage.
	 *
	 * Allows the Stage to resume normal TriggerZone-based state management.
	 * The Stage will immediately re-evaluate its state based on current
	 * TriggerZone overlaps.
	 *
	 * @param StageID - The ID of the Stage to release
	 */
	UFUNCTION(BlueprintCallable, Category = "Stage Manager|Control",
		meta = (DisplayName = "Release Stage Override"))
	void ReleaseStageOverride(int32 StageID);

	/**
	 * @brief Check if a Stage is currently overridden by this Subsystem.
	 *
	 * @param StageID - The ID of the Stage to check
	 * @return True if the Stage is being controlled by this Subsystem
	 */
	UFUNCTION(BlueprintCallable, Category = "Stage Manager|Control")
	bool IsStageOverridden(int32 StageID) const;

	/**
	 * @brief Get the overridden state of a Stage (if any).
	 *
	 * @param StageID - The ID of the Stage to query
	 * @param OutState - Output parameter for the overridden state
	 * @return True if the Stage is overridden, false otherwise
	 */
	UFUNCTION(BlueprintCallable, Category = "Stage Manager|Control")
	bool GetStageOverrideState(int32 StageID, EStageRuntimeState& OutState) const;

	/**
	 * @brief Release all overrides on all Stages.
	 *
	 * Useful for cleanup or resetting all Stages to automatic management.
	 */
	UFUNCTION(BlueprintCallable, Category = "Stage Manager|Control")
	void ReleaseAllStageOverrides();

#pragma endregion Cross-Stage Communication API

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

	/**
	 * Tracks Stages that are currently overridden by this Subsystem.
	 * Key: StageID, Value: The state the Stage is locked to.
	 *
	 * This mirrors the AStage::bIsStageStateLocked state at the Subsystem level,
	 * making it clear which overrides originated from this Subsystem.
	 */
	TMap<int32, EStageRuntimeState> OverriddenStageStates;
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
