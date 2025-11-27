#include "Components/StagePropComponent.h"
#include "Actors/Stage.h"

#if WITH_EDITOR
#include "Misc/MessageDialog.h"
#endif

UStagePropComponent::UStagePropComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

#if WITH_EDITOR
void UStagePropComponent::OnRegister()
{
	Super::OnRegister();

	// === Prevent adding StagePropComponent to Stage actors ===
	// Stage actors cannot be Props (nested Stage is dangerous and not allowed)
	if (AActor* Owner = GetOwner())
	{
		if (Owner->IsA<AStage>())
		{
			// Log the error
			UE_LOG(LogTemp, Error,
				TEXT("StagePropComponent cannot be added to Stage actor '%s'! "
				     "Stage actors cannot be registered as Props. This component will be destroyed."),
				*Owner->GetName());

			// Show warning dialog to user
			FMessageDialog::Open(
				EAppMsgType::Ok,
				FText::Format(
					NSLOCTEXT("StageEditor", "StageCannotBeProp",
						"Cannot add StagePropComponent to Stage actor!\n\n"
						"Stage: {0}\n\n"
						"Stage actors cannot be registered as Props.\n"
						"This is a dangerous nested operation and is not allowed.\n\n"
						"The component will be removed."),
					FText::FromString(Owner->GetName())),
				NSLOCTEXT("StageEditor", "StageCannotBePropTitle", "Invalid Operation")
			);

			// Mark component for destruction
			DestroyComponent();
			return;
		}
	}
}
#endif

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
