#include "Components/StagePropComponent.h"
#include "Actors/Stage.h"

UStagePropComponent::UStagePropComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UStagePropComponent::BeginPlay()
{
	Super::BeginPlay();
	
	// TODO: Auto-register with OwnerStage if configured
}

void UStagePropComponent::SetPropState(int32 NewState, bool bForce)
{
	if (PropState != NewState || bForce)
	{
		// Store previous state before updating
		PreviousPropState = PropState;
		PropState = NewState;

		// Notify listeners (Blueprints)
		OnPropStateChanged.Broadcast(PropState, PreviousPropState);

		UE_LOG(LogTemp, Verbose, TEXT("Prop Component [%s] (ID:%d) State Changed: %d -> %d"),
			   *GetOwner()->GetName(), SUID.PropID, PreviousPropState, PropState);

#if WITH_EDITOR
		// Force visual update in Editor
		if (AActor* Owner = GetOwner())
		{
			Owner->RerunConstructionScripts();
		}
#endif
	}
}

AStage* UStagePropComponent::GetOwnerStage() const
{
	return OwnerStage.Get();
}

void UStagePropComponent::IncrementState()
{
	SetPropState(PropState + 1);
}

void UStagePropComponent::DecrementState()
{
	SetPropState(PropState - 1);
}

void UStagePropComponent::ToggleState(int32 StateA, int32 StateB)
{
	if (PropState == StateA)
	{
		SetPropState(StateB);
	}
	else
	{
		SetPropState(StateA);
	}
}

bool UStagePropComponent::IsRegisteredToStage() const
{
	return OwnerStage.Get() != nullptr && SUID.PropID > 0;
}
