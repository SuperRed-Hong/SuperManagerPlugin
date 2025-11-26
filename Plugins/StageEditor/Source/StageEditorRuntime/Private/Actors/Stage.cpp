#include "Actors/Stage.h"
#include "Components/StagePropComponent.h"
#include "WorldPartition/DataLayer/DataLayerSubsystem.h"
#include "WorldPartition/DataLayer/DataLayerAsset.h"

AStage::AStage()
{
	PrimaryActorTick.bCanEverTick = false;

	// Ensure Default Act exists
	FAct DefaultAct;
	DefaultAct.ActID.StageID = StageID; // Note: StageID might be 0 at this point
	DefaultAct.ActID.ActID = 0; // 0 is reserved for Default Act
	DefaultAct.DisplayName = TEXT("Default Act");
	Acts.Add(DefaultAct);

	// Initialize DataLayer name
	StageDataLayerName = FString::Printf(TEXT("Stage_%s"), *GetName());
}

#if WITH_EDITOR
void AStage::PostActorCreated()
{
	Super::PostActorCreated();

	// Set DataLayer name based on Actor name (fallback display name)
	StageDataLayerName = FString::Printf(TEXT("Stage_%s"), *GetName());

	// Note: StageID assignment and Subsystem registration is handled by
	// StageEditorController::OnLevelActorAdded to avoid circular dependencies
}

void AStage::EnsureStageDataLayer()
{
	// This function is called by StageEditorController to ensure
	// the DataLayer name is set correctly
	if (StageDataLayerName.IsEmpty() || StageDataLayerName == TEXT("Stage_None"))
	{
		StageDataLayerName = FString::Printf(TEXT("Stage_%s"), *GetName());
	}
}

void AStage::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	// If StageDataLayerAsset changes, sync the display name
	if (PropertyChangedEvent.GetPropertyName() == GET_MEMBER_NAME_CHECKED(AStage, StageDataLayerAsset))
	{
		if (StageDataLayerAsset)
		{
			StageDataLayerName = StageDataLayerAsset->GetName();
		}
	}
}

void AStage::BeginDestroy()
{
	// Note: Subsystem unregistration is handled by StageEditorController::OnLevelActorDeleted
	Super::BeginDestroy();
}
#endif

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

	// Handle Data Layer Activation
	UDataLayerSubsystem* DataLayerSubsystem = GetWorld()->GetSubsystem<UDataLayerSubsystem>();
	if (DataLayerSubsystem)
	{
		// Deactivate previous Data Layer if it's different
		if (CurrentDataLayer && CurrentDataLayer != TargetAct->AssociatedDataLayer)
		{
			if (const UDataLayerInstance* Instance = DataLayerSubsystem->GetDataLayerInstance(CurrentDataLayer))
			{
				DataLayerSubsystem->SetDataLayerRuntimeState(Instance, EDataLayerRuntimeState::Unloaded);
				UE_LOG(LogTemp, Log, TEXT("Stage [%s]: Unloaded DataLayer '%s'"), *GetName(), *CurrentDataLayer->GetName());
			}
		}

		// Activate new Data Layer
		if (TargetAct->AssociatedDataLayer)
		{
			if (const UDataLayerInstance* Instance = DataLayerSubsystem->GetDataLayerInstance(TargetAct->AssociatedDataLayer))
			{
				DataLayerSubsystem->SetDataLayerRuntimeState(Instance, EDataLayerRuntimeState::Activated);
				UE_LOG(LogTemp, Log, TEXT("Stage [%s]: Activated DataLayer '%s'"), *GetName(), *TargetAct->AssociatedDataLayer->GetName());
			}
		}
	}

	// Update Runtime State
	CurrentActID = ActID;
	CurrentDataLayer = TargetAct->AssociatedDataLayer;
}

void AStage::DeactivateAct()
{
	// Logic to reset props or unload resources could go here.
	UE_LOG(LogTemp, Log, TEXT("Stage [%s]: Deactivating current Act."), *GetName());

	// Handle Data Layer Deactivation
	if (CurrentDataLayer)
	{
		if (UDataLayerSubsystem* DataLayerSubsystem = GetWorld()->GetSubsystem<UDataLayerSubsystem>())
		{
			if (const UDataLayerInstance* Instance = DataLayerSubsystem->GetDataLayerInstance(CurrentDataLayer))
			{
				DataLayerSubsystem->SetDataLayerRuntimeState(Instance, EDataLayerRuntimeState::Unloaded);
				UE_LOG(LogTemp, Log, TEXT("Stage [%s]: Unloaded DataLayer '%s'"), *GetName(), *CurrentDataLayer->GetName());
			}
		}
		CurrentDataLayer = nullptr;
	}

	CurrentActID = -1;
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
	// Remove from PropRegistry
	PropRegistry.Remove(PropID);
	
	// Clean up PropStateOverrides from ALL Acts
	for (FAct& Act : Acts)
	{
		Act.PropStateOverrides.Remove(PropID);
	}
	
	UE_LOG(LogTemp, Log, TEXT("Stage [%s]: Unregistered Prop ID %d from Stage and all Acts"), *GetName(), PropID);
}

void AStage::RemovePropFromAct(int32 PropID, int32 ActID)
{
	// Find the Act
	FAct* TargetAct = Acts.FindByPredicate([ActID](const FAct& Act) {
		return Act.ActID.ActID == ActID;
	});
	
	if (TargetAct)
	{
		if (TargetAct->PropStateOverrides.Remove(PropID) > 0)
		{
			UE_LOG(LogTemp, Log, TEXT("Stage [%s]: Removed Prop ID %d from Act '%s'"), *GetName(), PropID, *TargetAct->DisplayName);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Stage [%s]: Prop ID %d not found in Act '%s'"), *GetName(), PropID, *TargetAct->DisplayName);
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Stage [%s]: Act ID %d not found"), *GetName(), ActID);
	}
}

void AStage::RemoveAct(int32 ActID)
{
	// Find and remove the Act
	int32 RemovedCount = Acts.RemoveAll([ActID](const FAct& Act) {
		return Act.ActID.ActID == ActID;
	});
	
	if (RemovedCount > 0)
	{
		UE_LOG(LogTemp, Log, TEXT("Stage [%s]: Removed Act ID %d"), *GetName(), ActID);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Stage [%s]: Act ID %d not found"), *GetName(), ActID);
	}
}

AActor* AStage::GetPropByID(int32 PropID) const
{
	if (const TSoftObjectPtr<AActor>* PropPtr = PropRegistry.Find(PropID))
	{
		return PropPtr->Get();
	}
	return nullptr;
}
