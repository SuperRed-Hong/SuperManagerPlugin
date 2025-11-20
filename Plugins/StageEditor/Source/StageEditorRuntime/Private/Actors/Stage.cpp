#include "Actors/Stage.h"
#include "Components/StagePropComponent.h"

AStage::AStage()
{
	PrimaryActorTick.bCanEverTick = false;

	// Ensure Default Act exists
	FAct DefaultAct;
	DefaultAct.ActID.StageID = StageID; // Note: StageID might be 0 at this point
	DefaultAct.ActID.ActID = 0; // 0 is reserved for Default Act
	DefaultAct.DisplayName = TEXT("Default Act");
	Acts.Add(DefaultAct);
}

void AStage::BeginPlay()
{
	Super::BeginPlay();
}

void AStage::ActivateAct(int32 ActID)
{
	// Find the Act
	const FAct* TargetAct = Acts.FindByPredicate([ActID](const FAct& Act) {
		return Act.ActID.ActID == ActID;
	});

	if (!TargetAct)
	{
		UE_LOG(LogTemp, Warning, TEXT("Stage [%s]: Failed to activate Act %d. Act not found."), *GetName(), ActID);
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("Stage [%s]: Activating Act '%s' (ID:%d)"), *GetName(), *TargetAct->DisplayName, ActID);

	// Apply Prop States
	for (const auto& Pair : TargetAct->PropStateOverrides)
	{
		int32 PropID = Pair.Key;
		int32 TargetState = Pair.Value;

		if (AActor* PropActor = GetPropByID(PropID))
		{
			if (UStagePropComponent* PropComp = PropActor->FindComponentByClass<UStagePropComponent>())
			{
				PropComp->SetPropState(TargetState);
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("Stage [%s]: Prop Actor '%s' (ID:%d) has no UStagePropComponent."), *GetName(), *PropActor->GetName(), PropID);
			}
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Stage [%s]: Prop ID %d not found or invalid during Act activation."), *GetName(), PropID);
		}
	}
}

void AStage::DeactivateAct()
{
	// Logic to reset props or unload resources could go here.
	UE_LOG(LogTemp, Log, TEXT("Stage [%s]: Deactivating current Act."), *GetName());
}

int32 AStage::RegisterProp(AActor* NewProp)
{
	if (!NewProp) return -1;

	// Find the UStagePropComponent on this Actor
	UStagePropComponent* PropComponent = NewProp->FindComponentByClass<UStagePropComponent>();
	if (!PropComponent)
	{
		UE_LOG(LogTemp, Error, TEXT("Stage [%s]: Cannot register Prop '%s' - no UStagePropComponent found!"), *GetName(), *NewProp->GetName());
		return -1;
	}

	// Simple ID generation for Prototype: Find max ID + 1
	int32 NewID = 1;
	if (PropRegistry.Num() > 0)
	{
		int32 MaxID = 0;
		for (const auto& Pair : PropRegistry)
		{
			if (Pair.Key > MaxID) MaxID = Pair.Key;
		}
		NewID = MaxID + 1;
	}

	PropRegistry.Add(NewID, NewProp);
	PropComponent->PropID = NewID; // Sync ID to Prop Component
	PropComponent->OwnerStage = this; // Set owner stage reference

	// Auto-add to Default Act (ID 0)
	FAct* DefaultAct = Acts.FindByPredicate([](const FAct& Act) {
		return Act.ActID.ActID == 0;
	});

	if (DefaultAct)
	{
		// Default state is 0 (Closed/Default)
		DefaultAct->PropStateOverrides.Add(NewID, 0);
	}
	else
	{
		// Should not happen if constructor works, but handle it just in case
		FAct NewDefaultAct;
		NewDefaultAct.ActID.StageID = StageID;
		NewDefaultAct.ActID.ActID = 0;
		NewDefaultAct.DisplayName = TEXT("Default Act");
		NewDefaultAct.PropStateOverrides.Add(NewID, 0);
		Acts.Add(NewDefaultAct);
	}

	UE_LOG(LogTemp, Log, TEXT("Stage [%s]: Registered Prop '%s' with ID %d and added to Default Act"), *GetName(), *NewProp->GetName(), NewID);
	return NewID;
}

void AStage::UnregisterProp(int32 PropID)
{
	PropRegistry.Remove(PropID);
}

AActor* AStage::GetPropByID(int32 PropID) const
{
	if (const TSoftObjectPtr<AActor>* PropPtr = PropRegistry.Find(PropID))
	{
		return PropPtr->Get();
	}
	return nullptr;
}
