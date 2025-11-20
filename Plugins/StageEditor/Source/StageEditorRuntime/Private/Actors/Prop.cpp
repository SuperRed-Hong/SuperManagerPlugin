#include "Actors/Prop.h"

AProp::AProp()
{
	PrimaryActorTick.bCanEverTick = false;

	// Create the core PropComponent
	PropComponent = CreateDefaultSubobject<UStagePropComponent>(TEXT("PropComponent"));
}

void AProp::BeginPlay()
{
	Super::BeginPlay();
}

void AProp::SetPropState(int32 NewState, bool bForce)
{
	if (PropComponent)
	{
		PropComponent->SetPropState(NewState, bForce);
	}
}
