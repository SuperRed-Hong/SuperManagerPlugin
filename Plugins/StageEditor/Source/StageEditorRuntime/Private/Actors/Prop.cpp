#pragma region Imports
#include "Actors/Prop.h"
#pragma endregion Imports

#pragma region Construction
AProp::AProp()
{
	PrimaryActorTick.bCanEverTick = false;

	// Create the core PropComponent
	// Note: Blueprint subclasses of AProp can override the component class in the editor
	PropComponent = CreateDefaultSubobject<UStagePropComponent>(TEXT("PropComponent"));
}
#pragma endregion Construction

#pragma region Lifecycle
void AProp::BeginPlay()
{
	Super::BeginPlay();
}
#pragma endregion Lifecycle

#pragma region Convenience Wrappers
void AProp::SetPropState(int32 NewState, bool bForce)
{
	if (PropComponent)
	{
		PropComponent->SetPropState(NewState, bForce);
	}
}
#pragma endregion Convenience Wrappers
