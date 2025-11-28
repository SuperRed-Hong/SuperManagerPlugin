#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Core/StageCoreTypes.h"
#include "StagePropComponent.generated.h"

class AStage;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPropStateChanged, int32, NewState, int32, OldState);

/**
 * @brief Core component that makes any Actor a controllable Prop in the Stage system.
 * Can be added to any Actor to make it respond to Stage state changes.
 */
UCLASS(ClassGroup=(StageEditor), meta=(BlueprintSpawnableComponent))
class STAGEEDITORRUNTIME_API UStagePropComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UStagePropComponent();

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

	/** Stage Unique ID. Contains StageID and PropID. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stage Prop", meta = (DisplayName = "SUID"))
	FSUID SUID;

	/**
	 * @brief Gets the Prop's unique ID within its Stage (convenience getter).
	 * @return The PropID from SUID.PropID
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Stage Prop")
	int32 GetPropID() const { return SUID.PropID; }

	/**
	 * @brief Gets the owning Stage's ID (convenience getter).
	 * @return The StageID from SUID.StageID
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Stage Prop")
	int32 GetOwnerStageID() const { return SUID.StageID; }

	/**
	 * Soft reference to the owning Stage Actor.
	 * Used to quickly access the Stage without lookup.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stage Prop")
	TSoftObjectPtr<AStage> OwnerStage;

	/**
	 * The current state of this Prop.
	 * Modified by the Stage Manager via SetPropState().
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stage Prop")
	int32 PropState = 0;

	/** The previous state before the last change. Useful for transition logic. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stage Prop")
	int32 PreviousPropState = 0;

	/** Event fired when PropState changes. Implement logic here in Blueprints. */
	UPROPERTY(BlueprintAssignable, Category = "Stage Prop")
	FOnPropStateChanged OnPropStateChanged;

	//----------------------------------------------------------------
	// State Control API
	//----------------------------------------------------------------

	/**
	 * @brief Sets the new state for this Prop.
	 * @param NewState The target state value.
	 * @param bForce If true, triggers update even if NewState == CurrentState.
	 */
	UFUNCTION(BlueprintCallable, Category = "Stage Prop")
	void SetPropState(int32 NewState, bool bForce = false);

	/**
	 * @brief Gets the current state of this Prop.
	 * @return The current PropState value.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Stage Prop")
	int32 GetPropState() const { return PropState; }

	/**
	 * @brief Gets the previous state before the last change.
	 * Useful for implementing transition logic in Blueprints.
	 * @return The previous PropState value.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Stage Prop")
	int32 GetPreviousPropState() const { return PreviousPropState; }

	/**
	 * @brief Increments the current state by 1.
	 * Useful for simple sequential state machines.
	 */
	UFUNCTION(BlueprintCallable, Category = "Stage Prop")
	void IncrementState();

	/**
	 * @brief Decrements the current state by 1.
	 * Useful for simple sequential state machines.
	 */
	UFUNCTION(BlueprintCallable, Category = "Stage Prop")
	void DecrementState();

	/**
	 * @brief Toggles between two state values.
	 * If current state equals StateA, switches to StateB, and vice versa.
	 * If current state is neither, switches to StateA.
	 * @param StateA First toggle state.
	 * @param StateB Second toggle state.
	 */
	UFUNCTION(BlueprintCallable, Category = "Stage Prop")
	void ToggleState(int32 StateA, int32 StateB);

	//----------------------------------------------------------------
	// Stage Interaction API
	//----------------------------------------------------------------

	/**
	 * @brief Gets the owning Stage Actor reference.
	 * @return The owning Stage, or nullptr if not registered.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Stage Prop")
	AStage* GetOwnerStage() const;

	/**
	 * @brief Checks if this Prop is registered to a Stage.
	 * @return True if registered (has valid OwnerStage and PropID > 0).
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Stage Prop")
	bool IsRegisteredToStage() const;
};
