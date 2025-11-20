#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StagePropComponent.h"
#include "Prop.generated.h"

/**
 * @brief Convenience base class for Prop Actors.
 * Automatically includes UStagePropComponent for quick setup.
 * All core logic is in the component - this is just a wrapper.
 */
UCLASS(Abstract, Blueprintable)
class STAGEEDITORRUNTIME_API AProp : public AActor
{
	GENERATED_BODY()
	
public:	
	AProp();

protected:
	virtual void BeginPlay() override;

public:	
	/** The core Prop component that holds all logic. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stage Prop")
	TObjectPtr<UStagePropComponent> PropComponent;

	//----------------------------------------------------------------
	// Convenience Wrappers (delegates to Component)
	//----------------------------------------------------------------

	/**
	 * @brief Sets the new state for this Prop.
	 * Convenience wrapper that calls the component's SetPropState.
	 */
	UFUNCTION(BlueprintCallable, Category = "Stage Prop")
	void SetPropState(int32 NewState, bool bForce = false);

	/**
	 * @brief Get the current PropState.
	 */
	UFUNCTION(BlueprintCallable, Category = "Stage Prop")
	int32 GetPropState() const { return PropComponent ? PropComponent->PropState : 0; }
};
