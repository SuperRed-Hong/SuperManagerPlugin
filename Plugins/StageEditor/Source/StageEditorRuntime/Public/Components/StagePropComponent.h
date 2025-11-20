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

public:	
	//----------------------------------------------------------------
	// Core Properties
	//----------------------------------------------------------------

	/** The unique ID of this Prop within its owning Stage. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stage Prop")
	int32 PropID = 0;

	/** 
	 * Soft reference to the owning Stage Actor.
	 * Used to dynamically resolve StageID without storing it directly.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stage Prop")
	TSoftObjectPtr<AStage> OwnerStage;

	/** 
	 * The current state of this Prop. 
	 * Modified by the Stage Manager via SetPropState().
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stage Prop")
	int32 PropState = 0;

	/** Event fired when PropState changes. Implement logic here in Blueprints. */
	UPROPERTY(BlueprintAssignable, Category = "Stage Prop")
	FOnPropStateChanged OnPropStateChanged;

	//----------------------------------------------------------------
	// API
	//----------------------------------------------------------------

	/**
	 * @brief Sets the new state for this Prop.
	 * @param NewState The target state value.
	 * @param bForce If true, triggers update even if NewState == CurrentState.
	 */
	UFUNCTION(BlueprintCallable, Category = "Stage Prop")
	void SetPropState(int32 NewState, bool bForce = false);

	/**
	 * @brief Get the full hierarchical ID by resolving the OwnerStage reference.
	 * @return StageID from the OwnerStage, or -1 if invalid.
	 */
	UFUNCTION(BlueprintCallable, Category = "Stage Prop")
	int32 GetResolvedStageID() const;
};
