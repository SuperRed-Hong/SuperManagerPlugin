#include "Components/StageEntityComponent.h"
#include "Actors/Stage.h"

#if WITH_EDITOR
#include "Misc/MessageDialog.h"
#endif

UStageEntityComponent::UStageEntityComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

#if WITH_EDITOR
void UStageEntityComponent::OnRegister()
{
	Super::OnRegister();

	// === Prevent adding StageEntityComponent to Stage actors ===
	// Stage actors cannot be Entities (nested Stage is dangerous and not allowed)
	if (AActor* Owner = GetOwner())
	{
		if (Owner->IsA<AStage>())
		{
			// Log the error
			UE_LOG(LogTemp, Error,
				TEXT("StageEntityComponent cannot be added to Stage actor '%s'! "
				     "Stage actors cannot be registered as Entities. This component will be destroyed."),
				*Owner->GetName());

			// Show warning dialog to user
			FMessageDialog::Open(
				EAppMsgType::Ok,
				FText::Format(
					NSLOCTEXT("StageEditor", "StageCannotBeEntity",
						"Cannot add StageEntityComponent to Stage actor!\n\n"
						"Stage: {0}\n\n"
						"Stage actors cannot be registered as Entities.\n"
						"This is a dangerous nested operation and is not allowed.\n\n"
						"The component will be removed."),
					FText::FromString(Owner->GetName())),
				NSLOCTEXT("StageEditor", "StageCannotBeEntityTitle", "Invalid Operation")
			);

			// Mark component for destruction
			DestroyComponent();
			return;
		}
	}
}
#endif

void UStageEntityComponent::BeginPlay()
{
	Super::BeginPlay();

	// TODO: Auto-register with OwnerStage if configured
}

void UStageEntityComponent::SetEntityState(int32 NewState, bool bForce)
{
	if (EntityState != NewState || bForce)
	{
		// Store previous state before updating
		PreviousEntityState = EntityState;
		EntityState = NewState;

		// Notify listeners (Blueprints)
		OnEntityStateChanged.Broadcast(EntityState, PreviousEntityState);

		UE_LOG(LogTemp, Verbose, TEXT("Entity Component [%s] (ID:%d) State Changed: %d -> %d"),
			   *GetOwner()->GetName(), SUID.EntityID, PreviousEntityState, EntityState);

#if WITH_EDITOR
		// Force visual update in Editor
		if (AActor* Owner = GetOwner())
		{
			Owner->RerunConstructionScripts();
		}
#endif
	}
}

AStage* UStageEntityComponent::GetOwnerStage() const
{
	return OwnerStage.Get();
}

void UStageEntityComponent::IncrementState()
{
	SetEntityState(EntityState + 1);
}

void UStageEntityComponent::DecrementState()
{
	SetEntityState(EntityState - 1);
}

void UStageEntityComponent::ToggleState(int32 StateA, int32 StateB)
{
	if (EntityState == StateA)
	{
		SetEntityState(StateB);
	}
	else
	{
		SetEntityState(StateA);
	}
}

bool UStageEntityComponent::IsRegisteredToStage() const
{
	return OwnerStage.Get() != nullptr && SUID.EntityID > 0;
}

bool UStageEntityComponent::IsOrphaned() const
{
	// Has EntityID but OwnerStage is invalid = orphaned
	return (SUID.EntityID >= 0 && !OwnerStage.IsValid());
}

void UStageEntityComponent::ClearOrphanedState()
{
	if (!IsOrphaned())
	{
		return;
	}

#if WITH_EDITOR
	Modify();
#endif

	// Reset to unregistered state
	SUID.StageID = -1;
	SUID.EntityID = -1;
	OwnerStage = nullptr;
	EntityState = 0;
	PreviousEntityState = 0;

	UE_LOG(LogTemp, Log, TEXT("Cleared orphaned state for Entity '%s'"),
		*GetOwner()->GetActorLabel());
}
