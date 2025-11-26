#include "Actors/Stage.h"
#include "Components/StagePropComponent.h"
#include "WorldPartition/DataLayer/DataLayerManager.h"
#include "WorldPartition/DataLayer/DataLayerAsset.h"

AStage::AStage()
{
	PrimaryActorTick.bCanEverTick = false;

	// Ensure Default Act exists (ID 0 is reserved for Default Act)
	FAct DefaultAct;
	DefaultAct.SUID.StageID = SUID.StageID; // Note: StageID might be 0 at this point
	DefaultAct.SUID.ActID = 0; // Default Act is always ID 0
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

//----------------------------------------------------------------
// Multi-Act Activation Control API Implementation
//----------------------------------------------------------------

void AStage::ActivateAct(int32 ActID)
{
	// 1. Verify Act exists
	if (!DoesActExist(ActID))
	{
		UE_LOG(LogTemp, Warning, TEXT("Stage [%s]: Failed to activate Act %d. Act not found."), *GetName(), ActID);
		return;
	}

	// Find the Act for logging and DataLayer access
	const FAct* TargetAct = Acts.FindByPredicate([ActID](const FAct& Act) {
		return Act.SUID.ActID == ActID;
	});

	UE_LOG(LogTemp, Log, TEXT("Stage [%s]: Activating Act '%s' (ID:%d)"), *GetName(), *TargetAct->DisplayName, ActID);

	// 2. If already active, remove first (will be added to end for highest priority)
	ActiveActIDs.Remove(ActID);

	// 3. Add to end (highest priority)
	ActiveActIDs.Add(ActID);

	// 4. Update RecentActivatedActID
	RecentActivatedActID = ActID;

	// 5. Activate DataLayer (Activated state, not just Loaded)
	if (TargetAct->AssociatedDataLayer)
	{
		SetActDataLayerState(ActID, EDataLayerRuntimeState::Activated);
	}

	// 6. Apply this Act's PropState overrides
	ApplyActPropStatesOnly(ActID);

	// 7. Update CurrentDataLayer
	CurrentDataLayer = TargetAct->AssociatedDataLayer;

	// 8. Broadcast events
	OnActActivated.Broadcast(ActID);
	OnActiveActsChanged.Broadcast();
}

void AStage::DeactivateAct(int32 ActID)
{
	// 1. Check if Act is in the active list
	if (!ActiveActIDs.Contains(ActID))
	{
		UE_LOG(LogTemp, Warning, TEXT("Stage [%s]: Act %d is not active, cannot deactivate."), *GetName(), ActID);
		return;
	}

	// Find Act for logging
	const FAct* TargetAct = Acts.FindByPredicate([ActID](const FAct& Act) {
		return Act.SUID.ActID == ActID;
	});

	FString ActName = TargetAct ? TargetAct->DisplayName : FString::Printf(TEXT("Act_%d"), ActID);
	UE_LOG(LogTemp, Log, TEXT("Stage [%s]: Deactivating Act '%s' (ID:%d)"), *GetName(), *ActName, ActID);

	// 2. Remove from active list
	ActiveActIDs.Remove(ActID);

	// 3. Unload DataLayer
	SetActDataLayerState(ActID, EDataLayerRuntimeState::Unloaded);

	// 4. Prop states are NOT reverted (by design)

	// 5. Update CurrentDataLayer
	if (ActiveActIDs.Num() > 0)
	{
		int32 HighestPriorityActID = ActiveActIDs.Last();
		const FAct* HighestAct = Acts.FindByPredicate([HighestPriorityActID](const FAct& Act) {
			return Act.SUID.ActID == HighestPriorityActID;
		});
		CurrentDataLayer = HighestAct ? HighestAct->AssociatedDataLayer : nullptr;
	}
	else
	{
		CurrentDataLayer = nullptr;
	}

	// 6. Broadcast events
	OnActDeactivated.Broadcast(ActID);
	OnActiveActsChanged.Broadcast();
}

void AStage::ActivateActs(const TArray<int32>& ActIDs)
{
	for (int32 ActID : ActIDs)
	{
		ActivateAct(ActID);
	}
}

void AStage::DeactivateAllActs()
{
	// Copy the array since we'll be modifying it
	TArray<int32> ActsToDeactivate = ActiveActIDs;

	for (int32 ActID : ActsToDeactivate)
	{
		DeactivateAct(ActID);
	}
}

//----------------------------------------------------------------
// Multi-Act Query API Implementation
//----------------------------------------------------------------

bool AStage::IsActActive(int32 ActID) const
{
	return ActiveActIDs.Contains(ActID);
}

int32 AStage::GetHighestPriorityActID() const
{
	return ActiveActIDs.Num() > 0 ? ActiveActIDs.Last() : -1;
}

//----------------------------------------------------------------
// Prop Effective State API Implementation
//----------------------------------------------------------------

int32 AStage::GetEffectivePropState(int32 PropID) const
{
	// Iterate from highest priority (end) to lowest (beginning)
	for (int32 i = ActiveActIDs.Num() - 1; i >= 0; --i)
	{
		int32 ActID = ActiveActIDs[i];
		const FAct* Act = Acts.FindByPredicate([ActID](const FAct& A) {
			return A.SUID.ActID == ActID;
		});

		if (Act)
		{
			if (const int32* State = Act->PropStateOverrides.Find(PropID))
			{
				return *State;
			}
		}
	}

	// Fallback: return current actual Prop state
	return GetPropStateByID(PropID);
}

int32 AStage::GetControllingActForProp(int32 PropID) const
{
	// Iterate from highest priority (end) to lowest (beginning)
	for (int32 i = ActiveActIDs.Num() - 1; i >= 0; --i)
	{
		int32 ActID = ActiveActIDs[i];
		const FAct* Act = Acts.FindByPredicate([ActID](const FAct& A) {
			return A.SUID.ActID == ActID;
		});

		if (Act && Act->PropStateOverrides.Contains(PropID))
		{
			return ActID;
		}
	}

	return -1;
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
	PropComponent->SUID.StageID = SUID.StageID; // Sync Stage ID to Prop Component
	PropComponent->SUID.PropID = NewID; // Sync Prop ID to Prop Component
	PropComponent->OwnerStage = this; // Set owner stage reference

	// Auto-add to Default Act (ID 0)
	FAct* DefaultAct = Acts.FindByPredicate([](const FAct& Act) {
		return Act.SUID.ActID == 0;
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
		NewDefaultAct.SUID.StageID = SUID.StageID;
		NewDefaultAct.SUID.ActID = 0;
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
		return Act.SUID.ActID == ActID;
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
		return Act.SUID.ActID == ActID;
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

//----------------------------------------------------------------
// DataLayer Runtime Control API Implementation
//----------------------------------------------------------------

bool AStage::SetActDataLayerState(int32 ActID, EDataLayerRuntimeState NewState)
{
	// Find the Act
	const FAct* TargetAct = Acts.FindByPredicate([ActID](const FAct& Act) {
		return Act.SUID.ActID == ActID;
	});

	if (!TargetAct)
	{
		UE_LOG(LogTemp, Warning, TEXT("Stage [%s]: SetActDataLayerState - Act %d not found."), *GetName(), ActID);
		return false;
	}

	if (!TargetAct->AssociatedDataLayer)
	{
		UE_LOG(LogTemp, Warning, TEXT("Stage [%s]: SetActDataLayerState - Act '%s' has no associated DataLayer."), *GetName(), *TargetAct->DisplayName);
		return false;
	}

	UDataLayerManager* DataLayerManager = UDataLayerManager::GetDataLayerManager(GetWorld());
	if (!DataLayerManager)
	{
		UE_LOG(LogTemp, Warning, TEXT("Stage [%s]: SetActDataLayerState - DataLayerManager not available."), *GetName());
		return false;
	}

	bool bSuccess = DataLayerManager->SetDataLayerRuntimeState(TargetAct->AssociatedDataLayer, NewState);

	if (bSuccess)
	{
		UE_LOG(LogTemp, Log, TEXT("Stage [%s]: Set Act '%s' DataLayer '%s' to state %d"),
			*GetName(), *TargetAct->DisplayName, *TargetAct->AssociatedDataLayer->GetName(), (int32)NewState);
	}

	return bSuccess;
}

EDataLayerRuntimeState AStage::GetActDataLayerState(int32 ActID) const
{
	// Find the Act
	const FAct* TargetAct = Acts.FindByPredicate([ActID](const FAct& Act) {
		return Act.SUID.ActID == ActID;
	});

	if (!TargetAct || !TargetAct->AssociatedDataLayer)
	{
		return EDataLayerRuntimeState::Unloaded;
	}

	UDataLayerManager* DataLayerManager = UDataLayerManager::GetDataLayerManager(GetWorld());
	if (!DataLayerManager)
	{
		return EDataLayerRuntimeState::Unloaded;
	}

	// Get instance from asset, then query runtime state
	const UDataLayerInstance* Instance = DataLayerManager->GetDataLayerInstanceFromAsset(TargetAct->AssociatedDataLayer);
	if (!Instance)
	{
		return EDataLayerRuntimeState::Unloaded;
	}

	return DataLayerManager->GetDataLayerInstanceRuntimeState(Instance);
}

bool AStage::IsActDataLayerLoaded(int32 ActID) const
{
	EDataLayerRuntimeState State = GetActDataLayerState(ActID);
	return State == EDataLayerRuntimeState::Loaded || State == EDataLayerRuntimeState::Activated;
}

bool AStage::LoadActDataLayer(int32 ActID)
{
	return SetActDataLayerState(ActID, EDataLayerRuntimeState::Activated);
}

bool AStage::UnloadActDataLayer(int32 ActID)
{
	return SetActDataLayerState(ActID, EDataLayerRuntimeState::Unloaded);
}

bool AStage::SetStageDataLayerState(EDataLayerRuntimeState NewState)
{
	if (!StageDataLayerAsset)
	{
		UE_LOG(LogTemp, Warning, TEXT("Stage [%s]: SetStageDataLayerState - No Stage DataLayer Asset assigned."), *GetName());
		return false;
	}

	UDataLayerManager* DataLayerManager = UDataLayerManager::GetDataLayerManager(GetWorld());
	if (!DataLayerManager)
	{
		UE_LOG(LogTemp, Warning, TEXT("Stage [%s]: SetStageDataLayerState - DataLayerManager not available."), *GetName());
		return false;
	}

	bool bSuccess = DataLayerManager->SetDataLayerRuntimeState(StageDataLayerAsset, NewState);

	if (bSuccess)
	{
		UE_LOG(LogTemp, Log, TEXT("Stage [%s]: Set Stage DataLayer '%s' to state %d"),
			*GetName(), *StageDataLayerAsset->GetName(), (int32)NewState);
	}

	return bSuccess;
}

EDataLayerRuntimeState AStage::GetStageDataLayerState() const
{
	if (!StageDataLayerAsset)
	{
		return EDataLayerRuntimeState::Unloaded;
	}

	UDataLayerManager* DataLayerManager = UDataLayerManager::GetDataLayerManager(GetWorld());
	if (!DataLayerManager)
	{
		return EDataLayerRuntimeState::Unloaded;
	}

	// Get instance from asset, then query runtime state
	const UDataLayerInstance* Instance = DataLayerManager->GetDataLayerInstanceFromAsset(StageDataLayerAsset);
	if (!Instance)
	{
		return EDataLayerRuntimeState::Unloaded;
	}

	return DataLayerManager->GetDataLayerInstanceRuntimeState(Instance);
}

UDataLayerAsset* AStage::GetActDataLayerAsset(int32 ActID) const
{
	const FAct* TargetAct = Acts.FindByPredicate([ActID](const FAct& Act) {
		return Act.SUID.ActID == ActID;
	});

	if (TargetAct)
	{
		return TargetAct->AssociatedDataLayer;
	}

	return nullptr;
}

//----------------------------------------------------------------
// Prop State Control API Implementation
//----------------------------------------------------------------

bool AStage::ApplyActPropStatesOnly(int32 ActID)
{
	const FAct* TargetAct = Acts.FindByPredicate([ActID](const FAct& Act) {
		return Act.SUID.ActID == ActID;
	});

	if (!TargetAct)
	{
		UE_LOG(LogTemp, Warning, TEXT("Stage [%s]: ApplyActPropStatesOnly - Act %d not found."), *GetName(), ActID);
		return false;
	}

	// Apply Prop States only (no DataLayer changes)
	for (const auto& Pair : TargetAct->PropStateOverrides)
	{
		SetPropStateByID(Pair.Key, Pair.Value);
	}

	UE_LOG(LogTemp, Log, TEXT("Stage [%s]: Applied PropStates from Act '%s' (ID:%d) without DataLayer change."),
		*GetName(), *TargetAct->DisplayName, ActID);

	return true;
}

bool AStage::SetPropStateByID(int32 PropID, int32 NewState, bool bForce)
{
	AActor* PropActor = GetPropByID(PropID);
	if (!PropActor)
	{
		UE_LOG(LogTemp, Warning, TEXT("Stage [%s]: SetPropStateByID - Prop ID %d not found."), *GetName(), PropID);
		return false;
	}

	UStagePropComponent* PropComp = PropActor->FindComponentByClass<UStagePropComponent>();
	if (!PropComp)
	{
		UE_LOG(LogTemp, Warning, TEXT("Stage [%s]: SetPropStateByID - Prop '%s' has no UStagePropComponent."), *GetName(), *PropActor->GetName());
		return false;
	}

	const int32 OldState = PropComp->PropState;
	PropComp->SetPropState(NewState, bForce);

	// Broadcast Stage-level event if state changed
	if (OldState != NewState || bForce)
	{
		OnStagePropStateChanged.Broadcast(PropID, OldState, NewState);
	}

	return true;
}

int32 AStage::GetPropStateByID(int32 PropID) const
{
	AActor* PropActor = GetPropByID(PropID);
	if (!PropActor)
	{
		return -1;
	}

	UStagePropComponent* PropComp = PropActor->FindComponentByClass<UStagePropComponent>();
	if (!PropComp)
	{
		return -1;
	}

	return PropComp->PropState;
}

void AStage::SetMultiplePropStates(const TMap<int32, int32>& PropStates)
{
	for (const auto& Pair : PropStates)
	{
		SetPropStateByID(Pair.Key, Pair.Value);
	}
}

//----------------------------------------------------------------
// Prop Query API Implementation
//----------------------------------------------------------------

AActor* AStage::GetPropActorByID(int32 PropID) const
{
	return GetPropByID(PropID);
}

UStagePropComponent* AStage::GetPropComponentByID(int32 PropID) const
{
	AActor* PropActor = GetPropByID(PropID);
	if (!PropActor)
	{
		return nullptr;
	}
	return PropActor->FindComponentByClass<UStagePropComponent>();
}

TArray<int32> AStage::GetAllPropIDs() const
{
	TArray<int32> PropIDs;
	PropRegistry.GetKeys(PropIDs);
	return PropIDs;
}

TArray<AActor*> AStage::GetAllPropActors() const
{
	TArray<AActor*> PropActors;
	for (const auto& Pair : PropRegistry)
	{
		if (AActor* Actor = Pair.Value.Get())
		{
			PropActors.Add(Actor);
		}
	}
	return PropActors;
}

int32 AStage::GetPropCount() const
{
	return PropRegistry.Num();
}

bool AStage::DoesPropExist(int32 PropID) const
{
	return PropRegistry.Contains(PropID) && PropRegistry[PropID].Get() != nullptr;
}

//----------------------------------------------------------------
// Act Query API Implementation
//----------------------------------------------------------------

FString AStage::GetActDisplayName(int32 ActID) const
{
	const FAct* TargetAct = Acts.FindByPredicate([ActID](const FAct& Act) {
		return Act.SUID.ActID == ActID;
	});

	if (TargetAct)
	{
		return TargetAct->DisplayName;
	}

	return FString();
}

TMap<int32, int32> AStage::GetActPropStates(int32 ActID) const
{
	const FAct* TargetAct = Acts.FindByPredicate([ActID](const FAct& Act) {
		return Act.SUID.ActID == ActID;
	});

	if (TargetAct)
	{
		return TargetAct->PropStateOverrides;
	}

	return TMap<int32, int32>();
}

TArray<int32> AStage::GetAllActIDs() const
{
	TArray<int32> ActIDs;
	for (const FAct& Act : Acts)
	{
		ActIDs.Add(Act.SUID.ActID);
	}
	return ActIDs;
}

bool AStage::DoesActExist(int32 ActID) const
{
	return Acts.ContainsByPredicate([ActID](const FAct& Act) {
		return Act.SUID.ActID == ActID;
	});
}
