#pragma once

#pragma region Imports
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Core/StageCoreTypes.h"
#include "Actors/Prop.h"
#include "Stage.generated.h"
#pragma endregion Imports

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

	/** Globally unique ID for this Stage. Assigned by StageEditorSubsystem, read-only. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stage", meta = (DisplayName = "Stage ID"))
	int32 StageID = 0;

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

#pragma region Runtime Logic
	//----------------------------------------------------------------
	// Runtime Logic
	//----------------------------------------------------------------

	/**
	 * @brief Activates a specific Act by its local ID.
	 * Applies the PropState overrides defined in the Act to the registered Props.
	 * @param ActID The ID of the Act to activate.
	 */
	UFUNCTION(BlueprintCallable, Category = "Stage")
	void ActivateAct(int32 ActID);

	/**
	 * @brief Deactivates the current Act (optional logic, e.g., reset props).
	 */
	UFUNCTION(BlueprintCallable, Category = "Stage")
	void DeactivateAct();
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
