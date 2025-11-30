#pragma once

#pragma region Imports
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Core/StageCoreTypes.h"
#include "Actors/Prop.h"
#include "WorldPartition/DataLayer/DataLayerInstance.h" // For EDataLayerRuntimeState
#include "Stage.generated.h"
#pragma endregion Imports

// Forward declarations
class UDataLayerAsset;
class UStagePropComponent;
class UBoxComponent;
class UStageTriggerZoneComponent;
class UTriggerZoneComponentBase;

//----------------------------------------------------------------
// Delegate Declarations
//----------------------------------------------------------------

/** Broadcast when an Act is activated (added to ActiveActIDs). */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnActActivated, int32, ActID);

/** Broadcast when an Act is deactivated (removed from ActiveActIDs). */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnActDeactivated, int32, ActID);

/** Broadcast when the active Acts list changes (for UI refresh). */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnActiveActsChanged);

/** Broadcast when any Prop's state changes within this Stage. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnStagePropStateChanged, int32, PropID, int32, OldState, int32, NewState);

/**
 * @brief The "Director" of the stage.
 * Manages a list of Acts and a registry of Props.
 * Responsible for applying Act states to Props.
 */
UCLASS()
class STAGEEDITORRUNTIME_API AStage : public AActor
{
	GENERATED_BODY()
	
public:	
#pragma region Construction
	/**
	 * @brief Default constructor.
	 * Initializes the Stage with a default Act and sets up initial state.
	 */
	AStage();
#pragma endregion Construction

protected:
#pragma region Lifecycle
	/**
	 * @brief Called after object is loaded from disk (Editor and Runtime).
	 * Used to handle WP streaming load - ensures Stage is registered with Subsystem.
	 */
	virtual void PostLoad() override;

	/**
	 * @brief Called after components are initialized (Runtime only).
	 * Also handles Stage registration for PIE/Game.
	 */
	virtual void PostInitializeComponents() override;

	/**
	 * @brief Called when the game starts or when spawned.
	 */
	virtual void BeginPlay() override;

	/**
	 * @brief Called when actor is constructed (editor and runtime).
	 * Used to apply built-in zone visibility settings.
	 */
	virtual void OnConstruction(const FTransform& Transform) override;

	/**
	 * @brief Updates built-in zone visibility based on bDisableBuiltInZones.
	 * Hides zones completely when disabled (editor + runtime).
	 */
	void UpdateBuiltInZoneVisibility();
#pragma endregion Lifecycle

public:	
#pragma region Core Data
	//----------------------------------------------------------------
	// Core Data
	//----------------------------------------------------------------

	/** Stage Unique ID. Contains the globally unique StageID. Assigned by StageManagerSubsystem, read-only. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stage", meta = (DisplayName = "SUID"))
	FSUID SUID;

	/**
	 * User-friendly display name for this Stage.
	 * Auto-synced from ActorLabel in editor. Available at runtime for Debug HUD.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stage", meta = (DisplayName = "Stage Name"))
	FString StageName;

	/**
	 * @brief Gets the display name for this Stage.
	 * Returns StageName if set, otherwise falls back to ActorLabel (editor) or GetName() (runtime).
	 * @return The display name for UI/debug purposes.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Stage")
	FString GetStageName() const;

	/**
	 * @brief Gets the Stage's unique ID (convenience getter).
	 * @return The StageID from SUID.StageID
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Stage")
	int32 GetStageID() const { return SUID.StageID; }

	/** List of all defined Acts for this Stage. Managed via StageEditorController. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stage")
	TArray<FAct> Acts;

	/**
	 * Registry of Props managed by this Stage.
	 * Key: PropID, Value: Soft reference to the Prop Actor.
	 * Managed via StageEditorController - do not edit directly.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stage")
	TMap<int32, TSoftObjectPtr<AActor>> PropRegistry;
#pragma endregion Core Data

#pragma region Trigger Zones
	//----------------------------------------------------------------
	// Trigger Zones (for automatic Stage DataLayer loading)
	//----------------------------------------------------------------

	/**
	 * Built-in LoadZone - Outer trigger zone (80% use case).
	 * When player enters: Stage transitions to Preloading → Loaded
	 * When player leaves: Stage transitions to Unloading → Unloaded
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stage|Trigger")
	TObjectPtr<UStageTriggerZoneComponent> BuiltInLoadZone;

	/**
	 * Built-in ActivateZone - Inner trigger zone (80% use case).
	 * When player enters: Stage transitions from Loaded → Active
	 * When player leaves (but still in LoadZone): Stage stays Active
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stage|Trigger")
	TObjectPtr<UStageTriggerZoneComponent> BuiltInActivateZone;

	/** Default extent for LoadZone (half-size in each axis). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stage|Trigger", meta = (DisplayName = "LoadZone Extent"))
	FVector LoadZoneExtent = FVector(2000.0f, 2000.0f, 500.0f);

	/** Default extent for ActivateZone (half-size in each axis). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stage|Trigger", meta = (DisplayName = "ActivateZone Extent"))
	FVector ActivateZoneExtent = FVector(1000.0f, 1000.0f, 400.0f);

	//----------------------------------------------------------------
	// Trigger Zone Filtering (shared settings for built-in zones)
	//----------------------------------------------------------------

	/**
	 * If true, built-in zones use the shared filtering settings below.
	 * If false, each zone uses its own filtering settings (see component Advanced options).
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stage|Trigger|Filtering",
		meta = (DisplayName = "Use Shared Filtering"))
	bool bUseSharedFiltering = true;

	/**
	 * Shared tag filters for triggering actors (applies to both LoadZone and ActivateZone).
	 * If not empty, only actors with ANY of these tags will trigger zone events.
	 * If empty, defaults to allowing only Pawn class actors.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stage|Trigger|Filtering",
		meta = (DisplayName = "Trigger Actor Tags", EditCondition = "bUseSharedFiltering"))
	TArray<FName> SharedTriggerActorTags;

	/**
	 * If true, requires the actor to be a Pawn in addition to having the tag.
	 * Only relevant when SharedTriggerActorTags is not empty.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stage|Trigger|Filtering",
		meta = (DisplayName = "Require Pawn With Tag", EditCondition = "bUseSharedFiltering && SharedTriggerActorTags.Num() > 0"))
	bool bSharedRequirePawnWithTag = false;

	//----------------------------------------------------------------
	// Advanced: External TriggerZone Configuration (20% use case)
	//----------------------------------------------------------------

	/**
	 * Optional external TriggerZone references.
	 * Use this when you need custom-shaped zones or zones placed on other actors.
	 * These zones will be bound during InitializeTriggerZones().
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stage|Trigger|Advanced")
	TArray<TSoftObjectPtr<UStageTriggerZoneComponent>> ExternalTriggerZones;

	/**
	 * If true, disables the built-in LoadZone and ActivateZone.
	 * The built-in zones will be completely hidden (editor + runtime).
	 * Use this for:
	 * - Initial/spawn stages that should be Active from the start
	 * - Stages managed entirely by external zones or script
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stage|Trigger|Advanced",
		meta = (DisplayName = "Disable Built-in Zones"))
	bool bDisableBuiltInZones = false;

	/**
	 * Initial state when bDisableBuiltInZones is true.
	 * This determines the Stage's state at BeginPlay when not managed by built-in zones.
	 * - Unloaded: Wait for external trigger or script to load
	 * - Loaded: DataLayer loaded but not activated
	 * - Active: Fully activated from the start (default for spawn stages)
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stage|Trigger|Advanced",
		meta = (DisplayName = "Initial State", EditCondition = "bDisableBuiltInZones"))
	EStageRuntimeState InitialStageState = EStageRuntimeState::Active;

	//----------------------------------------------------------------
	// Runtime TriggerZone Tracking
	//----------------------------------------------------------------

	/** All LoadZones registered to this Stage (built-in + external). */
	UPROPERTY(Transient)
	TArray<TObjectPtr<UStageTriggerZoneComponent>> RegisteredLoadZones;

	/** All ActivateZones registered to this Stage (built-in + external). */
	UPROPERTY(Transient)
	TArray<TObjectPtr<UStageTriggerZoneComponent>> RegisteredActivateZones;

	/**
	 * @brief Initializes all TriggerZones for this Stage.
	 * Binds built-in zones (if enabled) and external zones.
	 * Called automatically in BeginPlay.
	 */
	UFUNCTION(BlueprintCallable, Category = "Stage|Trigger")
	void InitializeTriggerZones();

	/**
	 * @brief Called by TriggerZoneComponent when an actor enters the zone.
	 * Updates actor tracking sets and triggers state transitions.
	 * @param Zone The zone that was entered.
	 * @param OtherActor The actor that entered.
	 */
	void HandleZoneBeginOverlap(UStageTriggerZoneComponent* Zone, AActor* OtherActor);

	/**
	 * @brief Called by TriggerZoneComponent when an actor leaves the zone.
	 * Updates actor tracking sets and triggers state transitions.
	 * @param Zone The zone that was exited.
	 * @param OtherActor The actor that exited.
	 */
	void HandleZoneEndOverlap(UStageTriggerZoneComponent* Zone, AActor* OtherActor);

	/**
	 * @brief Checks for actors already inside TriggerZones at BeginPlay.
	 * Fixes the issue where player spawning inside a zone doesn't trigger BeginOverlap.
	 * Called automatically after InitializeTriggerZones().
	 */
	void CheckInitialOverlaps();

	//----------------------------------------------------------------
	// Generic TriggerZone Registration (for UTriggerZoneComponentBase)
	//----------------------------------------------------------------

	/**
	 * All generic TriggerZones registered to this Stage.
	 * Includes both built-in zones and external zones using UTriggerZoneComponentBase.
	 * Used for debugging and flow visualization.
	 */
	UPROPERTY(Transient, BlueprintReadOnly, Category = "Stage|Trigger")
	TArray<TObjectPtr<UTriggerZoneComponentBase>> RegisteredTriggerZones;

	/**
	 * @brief Registers a TriggerZone to this Stage.
	 * Called automatically by UTriggerZoneComponentBase when OwnerStage is set.
	 * @param Zone The zone to register.
	 */
	UFUNCTION(BlueprintCallable, Category = "Stage|Trigger")
	void RegisterTriggerZone(UTriggerZoneComponentBase* Zone);

	/**
	 * @brief Unregisters a TriggerZone from this Stage.
	 * Called automatically by UTriggerZoneComponentBase on EndPlay.
	 * @param Zone The zone to unregister.
	 */
	UFUNCTION(BlueprintCallable, Category = "Stage|Trigger")
	void UnregisterTriggerZone(UTriggerZoneComponentBase* Zone);

	/**
	 * @brief Gets all registered TriggerZones.
	 * Useful for debugging and flow visualization.
	 * @return Array of all registered zones.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Stage|Trigger")
	TArray<UTriggerZoneComponentBase*> GetAllTriggerZones() const;

	/**
	 * @brief Gets the count of registered TriggerZones.
	 * @return Number of registered zones.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Stage|Trigger")
	int32 GetTriggerZoneCount() const { return RegisteredTriggerZones.Num(); }

#pragma endregion Trigger Zones

#pragma region Runtime State
	//----------------------------------------------------------------
	// Runtime State (Multi-Act Activation System)
	//----------------------------------------------------------------

	/**
	 * Currently active Act IDs, ordered by activation time.
	 * Last element has highest priority (most recently activated).
	 * When multiple Acts define the same Prop's state, the highest priority Act wins.
	 */
	UPROPERTY(Transient, BlueprintReadOnly, Category = "Stage|Runtime")
	TArray<int32> ActiveActIDs;

	/** The most recently activated Act ID. Updated on each ActivateAct call. */
	UPROPERTY(Transient, BlueprintReadOnly, Category = "Stage|Runtime")
	int32 RecentActivatedActID = -1;

	/** The currently active Data Layer (of highest priority Act). */
	UPROPERTY(Transient, BlueprintReadOnly, Category = "Stage|Runtime")
	TObjectPtr<class UDataLayerAsset> CurrentDataLayer;

	/** The Stage's root Data Layer Asset. Auto-created by StageEditorController. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stage|DataLayer", meta = (DisplayName = "Stage DataLayer Asset"))
	TObjectPtr<class UDataLayerAsset> StageDataLayerAsset;

	/**
	 * Cached display name for the Stage's root Data Layer.
	 * Use GetStageDataLayerDisplayName() for reliable access.
	 * @note This is kept for Blueprint compatibility. Prefer using the getter.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stage|DataLayer", meta = (DisplayName = "DataLayer Name (Cached)"))
	FString StageDataLayerName;

	/**
	 * @brief Gets the display name of the Stage's DataLayer.
	 * Prioritizes the actual Asset name if available, falls back to cached StageDataLayerName.
	 * @return The DataLayer display name, or empty string if no DataLayer.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Stage|DataLayer")
	FString GetStageDataLayerDisplayName() const;

	//----------------------------------------------------------------
	// Stage State Machine
	//----------------------------------------------------------------

	/** Current runtime state of the Stage's DataLayer lifecycle. */
	UPROPERTY(Transient, BlueprintReadOnly, Category = "Stage|Runtime")
	EStageRuntimeState CurrentStageState = EStageRuntimeState::Unloaded;

	/** Actors currently overlapping the LoadZone. Used for reference counting. */
	UPROPERTY(Transient)
	TSet<TObjectPtr<AActor>> OverlappingLoadZoneActors;

	/** Actors currently overlapping the ActivateZone. Used for reference counting. */
	UPROPERTY(Transient)
	TSet<TObjectPtr<AActor>> OverlappingActivateZoneActors;

	//----------------------------------------------------------------
	// State Lock Mechanism (for Subsystem control)
	//----------------------------------------------------------------

	/**
	 * When true, Stage state is locked. TriggerZone auto-transitions are ignored.
	 * Only ForceStageStateOverride() can change state when locked.
	 */
	UPROPERTY(Transient, BlueprintReadOnly, Category = "Stage|Runtime")
	bool bIsStageStateLocked = false;

	/**
	 * The state the Stage is locked to (only valid when bIsStageStateLocked is true).
	 */
	UPROPERTY(Transient, BlueprintReadOnly, Category = "Stage|Runtime")
	EStageRuntimeState LockedStageState = EStageRuntimeState::Unloaded;

	/**
	 * Set of Act IDs that are locked (cannot be deactivated).
	 * Managed by LockAct() / UnlockAct().
	 */
	UPROPERTY(Transient)
	TSet<int32> LockedActIDs;

	//----------------------------------------------------------------
	// State Lock API (for Subsystem/external control)
	//----------------------------------------------------------------

	/**
	 * @brief Forces the Stage to a specific state and optionally locks it.
	 * Bypasses normal TriggerZone transitions.
	 * Used by StageManagerSubsystem for cross-Stage coordination.
	 * @param NewState The state to force the Stage to.
	 * @param bLockState If true, Stage will stay in this state until ReleaseStageStateOverride.
	 */
	UFUNCTION(BlueprintCallable, Category = "Stage|Runtime")
	void ForceStageStateOverride(EStageRuntimeState NewState, bool bLockState = false);

	/**
	 * @brief Releases the Stage state lock, allowing normal TriggerZone transitions.
	 * If TriggerZone overlap state doesn't match current state, transitions immediately.
	 */
	UFUNCTION(BlueprintCallable, Category = "Stage|Runtime")
	void ReleaseStageStateOverride();

	/**
	 * @brief Checks if the Stage state is currently locked.
	 * @return True if state is locked.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Stage|Runtime")
	bool IsStageStateLocked() const { return bIsStageStateLocked; }

	/**
	 * @brief Locks an Act, preventing it from being deactivated.
	 * @param ActID The Act to lock.
	 */
	UFUNCTION(BlueprintCallable, Category = "Stage|Acts")
	void LockAct(int32 ActID);

	/**
	 * @brief Unlocks an Act, allowing it to be deactivated.
	 * @param ActID The Act to unlock.
	 */
	UFUNCTION(BlueprintCallable, Category = "Stage|Acts")
	void UnlockAct(int32 ActID);

	/**
	 * @brief Checks if a specific Act is locked.
	 * @param ActID The Act to check.
	 * @return True if the Act is locked.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Stage|Acts")
	bool IsActLocked(int32 ActID) const { return LockedActIDs.Contains(ActID); }

	/**
	 * @brief Gets all currently locked Act IDs.
	 * @return Array of locked ActIDs.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Stage|Acts")
	TArray<int32> GetLockedActIDs() const { return LockedActIDs.Array(); }

protected:
	//----------------------------------------------------------------
	// State Machine Core Functions
	//----------------------------------------------------------------

	/**
	 * @brief [Internal] Safely transitions to a new runtime state.
	 * Calls OnExitState for current, updates CurrentStageState, then calls OnEnterState.
	 * Respects state lock if active.
	 *
	 * NOTE: This is the internal 5-state API. For user code, use GotoState(EStageState) instead.
	 *
	 * @param NewState The target runtime state.
	 */
	void InternalGotoState(EStageRuntimeState NewState);

	/**
	 * @brief Called when entering a new state. Handles state-specific initialization.
	 * @param State The state being entered.
	 */
	void OnEnterState(EStageRuntimeState State);

	/**
	 * @brief Called when exiting a state. Handles state-specific cleanup.
	 * @param State The state being exited.
	 */
	void OnExitState(EStageRuntimeState State);

	/**
	 * @brief Called when Stage DataLayer finishes loading (async callback).
	 * Transitions from Preloading to Loaded.
	 */
	void OnStageDataLayerLoaded();

	/**
	 * @brief Called when Stage DataLayer finishes unloading (async callback).
	 * Transitions from Unloading to Unloaded.
	 */
	void OnStageDataLayerUnloaded();

	/**
	 * @brief Applies state to Acts that have bFollowStageState=true.
	 * Called from OnEnterState() when Stage state changes.
	 * @param TargetState The DataLayer state to apply (Loaded, Activated, or Unloaded).
	 */
	void ApplyFollowingActStates(EDataLayerRuntimeState TargetState);

	/**
	 * @brief Applies InitialDataLayerState for Acts that have bFollowStageState=false.
	 * Called from OnEnterState(Active).
	 * - Acts with Activated state are added to ActiveActIDs
	 * - Acts with Loaded state have their DataLayer preloaded
	 * - Acts with Unloaded state remain unloaded
	 */
	void ApplyInitialActDataLayerStates();

	/**
	 * @brief Unloads ALL Act DataLayers regardless of bFollowStageState.
	 * Called from OnEnterState(Unloading).
	 * This is necessary because child DataLayers "remember" their state when parent unloads,
	 * and will restore to that state when parent reloads.
	 */
	void UnloadAllActDataLayers();

public:
	//----------------------------------------------------------------
	// Stage State Query API
	//----------------------------------------------------------------

	/** Gets the current Stage runtime state. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Stage|Runtime")
	EStageRuntimeState GetCurrentStageState() const { return CurrentStageState; }

	/** Checks if the Stage is in a transition state (Preloading or Unloading). */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Stage|Runtime")
	bool IsInTransitionState() const;

	/** Checks if the Stage DataLayer is loaded (Loaded or Active state). */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Stage|Runtime")
	bool IsStageLoaded() const;

	/** Checks if the Stage is fully active. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Stage|Runtime")
	bool IsStageActive() const { return CurrentStageState == EStageRuntimeState::Active; }

	//----------------------------------------------------------------
	// User API (3-State)
	//----------------------------------------------------------------

	/**
	 * @brief Gets the simplified Stage state (3-state user view).
	 * Maps the internal 5-state to user-friendly 3-state.
	 *
	 * Mapping:
	 * - Unloaded, Unloading → EStageState::Unloaded
	 * - Preloading, Loaded → EStageState::Loaded
	 * - Active → EStageState::Active
	 *
	 * @return The simplified stage state.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Stage",
		meta = (DisplayName = "Get Stage State"))
	EStageState GetStageState() const;

	/**
	 * @brief Requests the Stage to transition to a target state.
	 * This is the primary user API for controlling Stage state.
	 *
	 * State transitions:
	 * - GotoState(Unloaded) → Triggers unload sequence
	 * - GotoState(Loaded)   → Triggers load sequence (preload → loaded)
	 * - GotoState(Active)   → Triggers full activation (load if needed → active)
	 *
	 * Safe to call in any state - invalid transitions are gracefully ignored.
	 *
	 * @param TargetState The desired state to transition to.
	 */
	UFUNCTION(BlueprintCallable, Category = "Stage",
		meta = (DisplayName = "Goto State"))
	void GotoState(EStageState TargetState);

	/**
	 * @brief Convenience method: Request Stage to load.
	 * Equivalent to GotoState(EStageState::Loaded).
	 */
	UFUNCTION(BlueprintCallable, Category = "Stage",
		meta = (DisplayName = "Load Stage"))
	void LoadStage() { GotoState(EStageState::Loaded); }

	/**
	 * @brief Convenience method: Request Stage to activate.
	 * Equivalent to GotoState(EStageState::Active).
	 */
	UFUNCTION(BlueprintCallable, Category = "Stage",
		meta = (DisplayName = "Activate Stage"))
	void ActivateStage() { GotoState(EStageState::Active); }

	/**
	 * @brief Convenience method: Request Stage to unload.
	 * Equivalent to GotoState(EStageState::Unloaded).
	 */
	UFUNCTION(BlueprintCallable, Category = "Stage",
		meta = (DisplayName = "Unload Stage"))
	void UnloadStage() { GotoState(EStageState::Unloaded); }

	/**
	 * @brief Gets the internal runtime state (5-state developer view).
	 * Use this for debugging and detailed state inspection.
	 * For normal gameplay code, prefer GetStageState().
	 * @return The detailed internal state.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Stage|Debug",
		meta = (DisplayName = "Get Runtime State (Debug)"))
	EStageRuntimeState GetRuntimeState() const { return CurrentStageState; }

#pragma endregion Runtime State

#pragma region Events
	//----------------------------------------------------------------
	// Events / Delegates
	//----------------------------------------------------------------

	/** Broadcast when an Act is activated (added to ActiveActIDs). */
	UPROPERTY(BlueprintAssignable, Category = "Stage|Events")
	FOnActActivated OnActActivated;

	/** Broadcast when an Act is deactivated (removed from ActiveActIDs). */
	UPROPERTY(BlueprintAssignable, Category = "Stage|Events")
	FOnActDeactivated OnActDeactivated;

	/** Broadcast when the active Acts list changes (for UI refresh). */
	UPROPERTY(BlueprintAssignable, Category = "Stage|Events")
	FOnActiveActsChanged OnActiveActsChanged;

	/** Broadcast when any Prop's state changes within this Stage. */
	UPROPERTY(BlueprintAssignable, Category = "Stage|Events")
	FOnStagePropStateChanged OnStagePropStateChanged;
#pragma endregion Events

#pragma region Runtime Logic
	//----------------------------------------------------------------
	// Multi-Act Activation Control API
	//----------------------------------------------------------------

	/**
	 * @brief Activates a specific Act, adding it to ActiveActIDs.
	 * - If already active, moves to end (highest priority)
	 * - Applies PropState overrides for this Act
	 * - Activates the Act's associated DataLayer
	 * - Updates RecentActivatedActID
	 * @param ActID The ID of the Act to activate.
	 */
	UFUNCTION(BlueprintCallable, Category = "Stage|Acts")
	void ActivateAct(int32 ActID);

	/**
	 * @brief Deactivates a specific Act, removing it from ActiveActIDs.
	 * - Prop states are NOT automatically reverted
	 * - Unloads the Act's associated DataLayer
	 * @param ActID The ID of the Act to deactivate.
	 */
	UFUNCTION(BlueprintCallable, Category = "Stage|Acts")
	void DeactivateAct(int32 ActID);

	/**
	 * @brief Activates multiple Acts in order (last one has highest priority).
	 * Equivalent to calling ActivateAct for each in sequence.
	 * @param ActIDs Array of Act IDs to activate.
	 */
	UFUNCTION(BlueprintCallable, Category = "Stage|Acts")
	void ActivateActs(const TArray<int32>& ActIDs);

	/**
	 * @brief Deactivates all currently active Acts.
	 * Prop states are NOT automatically reverted.
	 */
	UFUNCTION(BlueprintCallable, Category = "Stage|Acts")
	void DeactivateAllActs();

	//----------------------------------------------------------------
	// Multi-Act Query API
	//----------------------------------------------------------------

	/**
	 * @brief Gets the list of currently active Act IDs (ordered by priority).
	 * Last element has highest priority.
	 * @return Array of active ActIDs.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Stage|Acts")
	TArray<int32> GetActiveActIDs() const { return ActiveActIDs; }

	/**
	 * @brief Checks if a specific Act is currently active.
	 * @param ActID The Act to check.
	 * @return True if Act is in ActiveActIDs.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Stage|Acts")
	bool IsActActive(int32 ActID) const;

	/**
	 * @brief Gets the Act with highest priority (most recently activated).
	 * @return The ActID at end of ActiveActIDs, or -1 if none active.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Stage|Acts")
	int32 GetHighestPriorityActID() const;

	/**
	 * @brief Gets the most recently activated Act ID.
	 * @return RecentActivatedActID, or -1 if never activated.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Stage|Acts")
	int32 GetRecentActivatedActID() const { return RecentActivatedActID; }

	/**
	 * @brief Gets the number of currently active Acts.
	 * @return Count of ActiveActIDs.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Stage|Acts")
	int32 GetActiveActCount() const { return ActiveActIDs.Num(); }

	//----------------------------------------------------------------
	// Prop Effective State API (Multi-Act)
	//----------------------------------------------------------------

	/**
	 * @brief Gets the effective state of a Prop considering all active Acts.
	 * Iterates from highest to lowest priority, returns first defined state.
	 * @param PropID The Prop to query.
	 * @return The effective state, or current actual state if no Act defines it.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Stage|Props")
	int32 GetEffectivePropState(int32 PropID) const;

	/**
	 * @brief Gets which active Act is controlling a Prop's state.
	 * @param PropID The Prop to query.
	 * @return The ActID with highest priority that defines this Prop, or -1 if none.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Stage|Props")
	int32 GetControllingActForProp(int32 PropID) const;

	//----------------------------------------------------------------
	// Legacy/Utility API
	//----------------------------------------------------------------

	/**
	 * @brief Applies an Act's PropState overrides WITHOUT changing DataLayer.
	 * Use this when you want to preview Act states without streaming.
	 * @param ActID The Act whose PropStates to apply.
	 * @return True if Act was found and states were applied.
	 */
	UFUNCTION(BlueprintCallable, Category = "Stage")
	bool ApplyActPropStatesOnly(int32 ActID);

	//----------------------------------------------------------------
	// Prop State Control API
	//----------------------------------------------------------------

	/**
	 * @brief Sets the state of a specific Prop by its ID.
	 * @param PropID The ID of the Prop to modify.
	 * @param NewState The new state value.
	 * @param bForce If true, triggers update even if state is unchanged.
	 * @return True if Prop was found and state was set.
	 */
	UFUNCTION(BlueprintCallable, Category = "Stage|Props")
	bool SetPropStateByID(int32 PropID, int32 NewState, bool bForce = false);

	/**
	 * @brief Gets the current state of a specific Prop.
	 * @param PropID The ID of the Prop to query.
	 * @return The current state, or -1 if Prop not found.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Stage|Props")
	int32 GetPropStateByID(int32 PropID) const;

	/**
	 * @brief Sets multiple Prop states at once.
	 * @param PropStates Map of PropID to desired state.
	 */
	UFUNCTION(BlueprintCallable, Category = "Stage|Props")
	void SetMultiplePropStates(const TMap<int32, int32>& PropStates);

	//----------------------------------------------------------------
	// Prop Query API
	//----------------------------------------------------------------

	/**
	 * @brief Finds a Prop Actor by its ID.
	 * @param PropID The ID of the Prop to find.
	 * @return Pointer to the Prop Actor, or nullptr if not found.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Stage|Props")
	AActor* GetPropActorByID(int32 PropID) const;

	/**
	 * @brief Gets the StagePropComponent of a Prop by its ID.
	 * @param PropID The ID of the Prop.
	 * @return The StagePropComponent, or nullptr if not found.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Stage|Props")
	UStagePropComponent* GetPropComponentByID(int32 PropID) const;

	/**
	 * @brief Gets all registered Prop IDs.
	 * @return Array of all PropIDs in this Stage.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Stage|Props")
	TArray<int32> GetAllPropIDs() const;

	/**
	 * @brief Gets all registered Prop Actors.
	 * @return Array of all Prop Actors in this Stage.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Stage|Props")
	TArray<AActor*> GetAllPropActors() const;

	/**
	 * @brief Gets the number of registered Props.
	 * @return The Prop count.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Stage|Props")
	int32 GetPropCount() const;

	/**
	 * @brief Checks if a Prop with the given ID exists.
	 * @param PropID The ID to check.
	 * @return True if Prop exists.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Stage|Props")
	bool DoesPropExist(int32 PropID) const;

	//----------------------------------------------------------------
	// Act Query API
	//----------------------------------------------------------------

	/**
	 * @brief Gets the display name of an Act.
	 * @param ActID The Act to query.
	 * @return The display name, or empty string if not found.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Stage|Acts")
	FString GetActDisplayName(int32 ActID) const;

	/**
	 * @brief Gets the PropState overrides configured for an Act.
	 * @param ActID The Act to query.
	 * @return Map of PropID to state values for this Act.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Stage|Acts")
	TMap<int32, int32> GetActPropStates(int32 ActID) const;

	/**
	 * @brief Gets all Act IDs in this Stage.
	 * @return Array of all ActIDs.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Stage|Acts")
	TArray<int32> GetAllActIDs() const;

	/**
	 * @brief Gets the number of Acts.
	 * @return The Act count.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Stage|Acts")
	int32 GetActCount() const { return Acts.Num(); }

	/**
	 * @brief Checks if an Act with the given ID exists.
	 * @param ActID The ID to check.
	 * @return True if Act exists.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Stage|Acts")
	bool DoesActExist(int32 ActID) const;

	//----------------------------------------------------------------
	// DataLayer Runtime Control API
	//----------------------------------------------------------------

	/**
	 * @brief Sets the runtime state of a specific Act's DataLayer.
	 * Does NOT change active Acts or apply PropState overrides.
	 * Use this for fine-grained DataLayer streaming control.
	 * @param ActID The Act whose DataLayer to control.
	 * @param NewState The desired runtime state (Unloaded/Loaded/Activated).
	 * @return True if successful, false if Act not found or has no DataLayer.
	 */
	UFUNCTION(BlueprintCallable, Category = "Stage|DataLayer")
	bool SetActDataLayerState(int32 ActID, EDataLayerRuntimeState NewState);

	/**
	 * @brief Gets the current runtime state of a specific Act's DataLayer.
	 * @param ActID The Act whose DataLayer state to query.
	 * @return The current runtime state, or Unloaded if Act/DataLayer not found.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Stage|DataLayer")
	EDataLayerRuntimeState GetActDataLayerState(int32 ActID) const;

	/**
	 * @brief Checks if a specific Act's DataLayer is currently loaded or activated.
	 * @param ActID The Act to check.
	 * @return True if DataLayer is Loaded or Activated.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Stage|DataLayer")
	bool IsActDataLayerLoaded(int32 ActID) const;

	/**
	 * @brief Convenience function to load (activate) an Act's DataLayer.
	 * @param ActID The Act whose DataLayer to load.
	 * @return True if successful.
	 */
	UFUNCTION(BlueprintCallable, Category = "Stage|DataLayer")
	bool LoadActDataLayer(int32 ActID);

	/**
	 * @brief Convenience function to unload an Act's DataLayer.
	 * @param ActID The Act whose DataLayer to unload.
	 * @return True if successful.
	 */
	UFUNCTION(BlueprintCallable, Category = "Stage|DataLayer")
	bool UnloadActDataLayer(int32 ActID);

	/**
	 * @brief Sets the runtime state of the Stage's root DataLayer.
	 * @param NewState The desired runtime state.
	 * @return True if successful.
	 */
	UFUNCTION(BlueprintCallable, Category = "Stage|DataLayer")
	bool SetStageDataLayerState(EDataLayerRuntimeState NewState);

	/**
	 * @brief Gets the current runtime state of the Stage's root DataLayer.
	 * @return The current runtime state, or Unloaded if no DataLayer.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Stage|DataLayer")
	EDataLayerRuntimeState GetStageDataLayerState() const;

	/**
	 * @brief Gets the DataLayer Asset associated with a specific Act.
	 * @param ActID The Act to query.
	 * @return The DataLayer Asset, or nullptr if not found.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Stage|DataLayer")
	UDataLayerAsset* GetActDataLayerAsset(int32 ActID) const;

#pragma endregion Runtime Logic

#pragma region Editor API
	//----------------------------------------------------------------
	// Editor/Setup API
	//----------------------------------------------------------------

	/**
	 * If true, this Stage will be automatically added to the Debug HUD watch list when PIE starts.
	 * This setting persists across editor sessions and is saved with the level.
	 * Toggle via the eye icon in StageEditor Panel or Stage Details.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stage|Debug",
		meta = (DisplayName = "Watch in Debug HUD"))
	bool bEditorWatched = false;

#if WITH_EDITOR
	/** Ensures the Stage has a root DataLayer created. Called when Stage is placed. */
	void EnsureStageDataLayer();

	virtual void PostActorCreated() override;
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual void BeginDestroy() override;
#endif

	/** 
	 * @brief Registers a Prop to this Stage.
	 * @param NewProp The prop actor to register.
	 * @return The assigned PropID, or -1 if registration failed.
	 */
	int32 RegisterProp(AActor* NewProp);

	/** 
	 * @brief Unregisters a Prop by ID. Removes from PropRegistry and all Acts.
	 * @param PropID The ID of the prop to unregister.
	 */
	void UnregisterProp(int32 PropID);

	/** 
	 * @brief Removes a Prop from a specific Act's PropStateOverrides.
	 * @param PropID The ID of the prop to remove.
	 * @param ActID The ID of the act to remove the prop from.
	 */
	void RemovePropFromAct(int32 PropID, int32 ActID);

	/** 
	 * @brief Removes an Act by its ActID.
	 * @param ActID The ID of the act to remove.
	 */
	void RemoveAct(int32 ActID);

	/** 
	 * @brief Finds a Prop by ID.
	 * @param PropID The ID of the prop to find.
	 * @return Pointer to the prop actor, or nullptr if not found.
	 */
	AActor* GetPropByID(int32 PropID) const;
#pragma endregion Editor API
};
