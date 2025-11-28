#pragma once

#pragma region Imports
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StagePropComponent.h"
#include "Prop.generated.h"
#pragma endregion Imports

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
#pragma region Construction
	/**
	 * @brief Default constructor.
	 * Initializes the PropComponent.
	 */
	AProp();
#pragma endregion Construction

protected:
#pragma region Lifecycle
	/**
	 * @brief Called when the game starts or when spawned.
	 */
	virtual void BeginPlay() override;
#pragma endregion Lifecycle

public:	
#pragma region Components
	/** The core Prop component that holds all logic. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stage Prop")
	TObjectPtr<UStagePropComponent> PropComponent;
#pragma endregion Components

#pragma region Convenience Wrappers
	//----------------------------------------------------------------
	// Convenience Wrappers (delegates to Component)
	//----------------------------------------------------------------

	/**
	 * @brief Sets the new state for this Prop.
	 * Convenience wrapper that calls the component's SetPropState.
	 * @param NewState The new state value to set.
	 * @param bForce If true, forces the state update even if the value hasn't changed.
	 */
	UFUNCTION(BlueprintCallable, Category = "Stage Prop")
	void SetPropState(int32 NewState, bool bForce = false);

	/**
	 * @brief Get the current PropState.
	 * @return The current state value.
	 */
	UFUNCTION(BlueprintCallable, Category = "Stage Prop")
	int32 GetPropState() const { return PropComponent ? PropComponent->PropState : 0; }
#pragma endregion Convenience Wrappers
};
