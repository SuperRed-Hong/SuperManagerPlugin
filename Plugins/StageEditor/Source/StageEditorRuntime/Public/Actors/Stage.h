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

//----------------------------------------------------------------
// Delegate Declarations
//----------------------------------------------------------------

/** Broadcast when an Act is activated. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnActActivated, int32, ActID);

/** Broadcast when an Act is deactivated. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnActDeactivated, int32, ActID);

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
	 * @brief Called when the game starts or when spawned.
	 */
	virtual void BeginPlay() override;
#pragma endregion Lifecycle

public:	
#pragma region Core Data
	//----------------------------------------------------------------
	// Core Data
	//----------------------------------------------------------------

	/** Stage Unique ID. Contains the globally unique StageID. Assigned by StageEditorSubsystem, read-only. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stage", meta = (DisplayName = "SUID"))
	FSUID SUID;

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

#pragma region Runtime State
	//----------------------------------------------------------------
	// Runtime State
	//----------------------------------------------------------------

	/** The ID of the currently active Act. -1 if no Act is active. */
	UPROPERTY(Transient, BlueprintReadOnly, Category = "Stage|Runtime")
	int32 CurrentActID = -1;

	/** The currently active Data Layer. */
	UPROPERTY(Transient, BlueprintReadOnly, Category = "Stage|Runtime")
	TObjectPtr<class UDataLayerAsset> CurrentDataLayer;

	/** The Stage's root Data Layer Asset. Auto-created by StageEditorController. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stage|DataLayer", meta = (DisplayName = "Stage DataLayer Asset"))
	TObjectPtr<class UDataLayerAsset> StageDataLayerAsset;

	/** The name of the Stage's root Data Layer (auto-created). */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stage|DataLayer", meta = (DisplayName = "DataLayer Name (Display Only)"))
	FString StageDataLayerName;
#pragma endregion Runtime State

#pragma region Events
	//----------------------------------------------------------------
	// Events / Delegates
	//----------------------------------------------------------------

	/** Broadcast when an Act is activated. */
	UPROPERTY(BlueprintAssignable, Category = "Stage|Events")
	FOnActActivated OnActActivated;

	/** Broadcast when an Act is deactivated. */
	UPROPERTY(BlueprintAssignable, Category = "Stage|Events")
	FOnActDeactivated OnActDeactivated;

	/** Broadcast when any Prop's state changes within this Stage. */
	UPROPERTY(BlueprintAssignable, Category = "Stage|Events")
	FOnStagePropStateChanged OnStagePropStateChanged;
#pragma endregion Events

#pragma region Runtime Logic
	//----------------------------------------------------------------
	// Runtime Logic
	//----------------------------------------------------------------

	/**
	 * @brief Activates a specific Act by its local ID.
	 * Applies the PropState overrides defined in the Act to the registered Props.
	 * Also activates the Act's associated DataLayer.
	 * @param ActID The ID of the Act to activate.
	 */
	UFUNCTION(BlueprintCallable, Category = "Stage")
	void ActivateAct(int32 ActID);

	/**
	 * @brief Deactivates the current Act (optional logic, e.g., reset props).
	 * Unloads the current Act's DataLayer.
	 */
	UFUNCTION(BlueprintCallable, Category = "Stage")
	void DeactivateAct();

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
	 * @brief Gets the currently active Act ID.
	 * @return The current ActID, or -1 if no Act is active.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Stage|Acts")
	int32 GetCurrentActID() const { return CurrentActID; }

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
	 * Does NOT change CurrentActID or apply PropState overrides.
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
