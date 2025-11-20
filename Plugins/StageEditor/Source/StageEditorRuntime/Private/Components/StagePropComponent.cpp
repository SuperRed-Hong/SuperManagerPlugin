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
		const int32 OldState = PropState;
		PropState = NewState;
		
		// Notify listeners (Blueprints)
		OnPropStateChanged.Broadcast(PropState, OldState);

		UE_LOG(LogTemp, Verbose, TEXT("Prop Component [%s] (ID:%d) State Changed: %d -> %d"), 
			   *GetOwner()->GetName(), PropID, OldState, PropState);

#if WITH_EDITOR
		// Force visual update in Editor
		if (AActor* Owner = GetOwner())
		{
			Owner->RerunConstructionScripts();
		}
#endif
	}
}

int32 UStagePropComponent::GetResolvedStageID() const
{
	if (OwnerStage.IsValid())
	{
		if (AStage* Stage = OwnerStage.Get())
		{
			return Stage->StageID;
		}
	}
	return -1;
}
