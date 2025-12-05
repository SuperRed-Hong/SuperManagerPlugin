#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Core/StageCoreTypes.h"
#include "StageEntityComponent.generated.h"

class AStage;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnEntityStateChanged, int32, NewState, int32, OldState);

/**
 * @brief Core component that makes any Actor a controllable Entity in the Stage system.
 * Can be added to any Actor to make it respond to Stage state changes.
 */
UCLASS(ClassGroup=(StageEditor), Blueprintable, meta=(BlueprintSpawnableComponent))
class STAGEEDITORRUNTIME_API UStageEntityComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UStageEntityComponent();

protected:
	virtual void BeginPlay() override;

#if WITH_EDITOR
	/**
	 * Called when component is registered.
	 * Used to prevent adding this component to Stage actors (nested Stage is not allowed).
	 */
	virtual void OnRegister() override;
#endif

public:	
	//----------------------------------------------------------------
	// Core Properties
	//----------------------------------------------------------------

	/** Stage Unique ID. Contains StageID and EntityID. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stage Entity", meta = (DisplayName = "SUID"))
	FSUID SUID;

	/**
	 * @brief Gets the Entity's unique ID within its Stage (convenience getter).
	 * @return The EntityID from SUID.EntityID
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Stage Entity")
	int32 GetEntityID() const { return SUID.EntityID; }

	/**
	 * @brief Gets the owning Stage's ID (convenience getter).
	 * @return The StageID from SUID.StageID
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Stage Entity")
	int32 GetOwnerStageID() const { return SUID.StageID; }

	/**
	 * Soft reference to the owning Stage Actor.
	 * Used to quickly access the Stage without lookup.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stage Entity")
	TSoftObjectPtr<AStage> OwnerStage;

	/**
	 * The current state of this Entity.
	 * Modified by the Stage Manager via SetEntityState().
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stage Entity")
	int32 EntityState = 0;

	/** The previous state before the last change. Useful for transition logic. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stage Entity")
	int32 PreviousEntityState = 0;

	/** Event fired when EntityState changes. Implement logic here in Blueprints. */
	UPROPERTY(BlueprintAssignable, Category = "Stage Entity")
	FOnEntityStateChanged OnEntityStateChanged;

	//----------------------------------------------------------------
	// State Control API
	//----------------------------------------------------------------

	/**
	 * @brief Sets the new state for this Entity.
	 * @param NewState The target state value.
	 * @param bForce If true, triggers update even if NewState == CurrentState.
	 */
	UFUNCTION(BlueprintCallable, Category = "Stage Entity")
	void SetEntityState(int32 NewState, bool bForce = false);

	/**
	 * @brief Gets the current state of this Entity.
	 * @return The current EntityState value.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Stage Entity")
	int32 GetEntityState() const { return EntityState; }

	/**
	 * @brief Gets the previous state before the last change.
	 * Useful for implementing transition logic in Blueprints.
	 * @return The previous EntityState value.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Stage Entity")
	int32 GetPreviousEntityState() const { return PreviousEntityState; }

	/**
	 * @brief Increments the current state by 1.
	 * Useful for simple sequential state machines.
	 */
	UFUNCTION(BlueprintCallable, Category = "Stage Entity")
	void IncrementState();

	/**
	 * @brief Decrements the current state by 1.
	 * Useful for simple sequential state machines.
	 */
	UFUNCTION(BlueprintCallable, Category = "Stage Entity")
	void DecrementState();

	/**
	 * @brief Toggles between two state values.
	 * If current state equals StateA, switches to StateB, and vice versa.
	 * If current state is neither, switches to StateA.
	 * @param StateA First toggle state.
	 * @param StateB Second toggle state.
	 */
	UFUNCTION(BlueprintCallable, Category = "Stage Entity")
	void ToggleState(int32 StateA, int32 StateB);

	//----------------------------------------------------------------
	// Stage Interaction API
	//----------------------------------------------------------------

	/**
	 * @brief Gets the owning Stage Actor reference.
	 * @return The owning Stage, or nullptr if not registered.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Stage Entity")
	AStage* GetOwnerStage() const;

	/**
	 * @brief Checks if this Entity is registered to a Stage.
	 * @return True if registered (has valid OwnerStage and EntityID > 0).
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Stage Entity")
	bool IsRegisteredToStage() const;

	/**
	 * @brief Checks if this Entity is orphaned (OwnerStage was deleted).
	 * An orphaned Entity has an EntityID but no valid OwnerStage reference.
	 * @return True if orphaned (has EntityID >= 0 but OwnerStage is invalid).
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Stage Entity")
	bool IsOrphaned() const;

	/**
	 * @brief Clears orphaned state, resetting to unregistered state.
	 * Used to clean up Entities whose owner Stage was deleted.
	 * Resets SUID, OwnerStage, and EntityState to default values.
	 */
	UFUNCTION(BlueprintCallable, Category = "Stage Entity")
	void ClearOrphanedState();
};
