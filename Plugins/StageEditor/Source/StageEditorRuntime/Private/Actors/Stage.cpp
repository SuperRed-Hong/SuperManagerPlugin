#include "Actors/Stage.h"
#include "Components/StagePropComponent.h"
#include "Components/StageTriggerZoneComponent.h"
#include "Components/TriggerZoneComponentBase.h"
#include "Components/BoxComponent.h"
#include "GameFramework/Pawn.h"
#include "WorldPartition/DataLayer/DataLayerManager.h"
#include "WorldPartition/DataLayer/DataLayerAsset.h"
#include "Subsystems/StageManagerSubsystem.h"

DEFINE_LOG_CATEGORY_STATIC(LogStage, Log, All);

AStage::AStage()
{
	PrimaryActorTick.bCanEverTick = false;

	// Create root component if needed
	USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
	SetRootComponent(Root);

	// Create built-in LoadZone component (outer trigger)
	BuiltInLoadZone = CreateDefaultSubobject<UStageTriggerZoneComponent>(TEXT("BuiltInLoadZone"));
	BuiltInLoadZone->SetupAttachment(Root);
	BuiltInLoadZone->ZoneType = EStageTriggerZoneType::LoadZone;
	BuiltInLoadZone->SetBoxExtent(LoadZoneExtent);
	BuiltInLoadZone->ShapeColor = FColor::Yellow;

	// Create built-in ActivateZone component (inner trigger)
	BuiltInActivateZone = CreateDefaultSubobject<UStageTriggerZoneComponent>(TEXT("BuiltInActivateZone"));
	BuiltInActivateZone->SetupAttachment(Root);
	BuiltInActivateZone->ZoneType = EStageTriggerZoneType::ActivateZone;
	BuiltInActivateZone->SetBoxExtent(ActivateZoneExtent);
	BuiltInActivateZone->ShapeColor = FColor::Green;

	// Ensure Default Act exists (ID 1 is reserved for Default Act)
	// ActID starts from 1 to avoid SUID collision with Stage (Stage is X.0.0, Default Act is X.1.0)
	FAct DefaultAct;
	DefaultAct.SUID.StageID = SUID.StageID; // Note: StageID might be 0 at this point
	DefaultAct.SUID.ActID = 1; // Default Act is always ID 1
	DefaultAct.DisplayName = TEXT("Default Act");
	DefaultAct.bFollowStageState = true; // Default Act follows Stage state by default
	DefaultAct.InitialDataLayerState = EDataLayerRuntimeState::Activated; // Fallback if bFollowStageState is disabled
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

	// Note: StageID assignment, Subsystem registration, and Singleton validation
	// are all handled by StageEditorController::OnLevelActorAdded
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

	const FName PropertyName = PropertyChangedEvent.GetPropertyName();

	// For struct properties (like FVector), we need to check MemberProperty
	// GetPropertyName() returns "X"/"Y"/"Z", but MemberProperty gives us "LoadZoneExtent"
	const FName MemberPropertyName = PropertyChangedEvent.MemberProperty
		? PropertyChangedEvent.MemberProperty->GetFName()
		: PropertyName;

	// If StageDataLayerAsset changes, sync the display name
	if (PropertyName == GET_MEMBER_NAME_CHECKED(AStage, StageDataLayerAsset))
	{
		if (StageDataLayerAsset)
		{
			StageDataLayerName = StageDataLayerAsset->GetName();
		}
	}

	// If bDisableBuiltInZones changes, update zone visibility
	if (PropertyName == GET_MEMBER_NAME_CHECKED(AStage, bDisableBuiltInZones))
	{
		UpdateBuiltInZoneVisibility();
	}

	// If LoadZoneExtent changes, sync to built-in LoadZone component
	if (MemberPropertyName == GET_MEMBER_NAME_CHECKED(AStage, LoadZoneExtent))
	{
		if (BuiltInLoadZone)
		{
			BuiltInLoadZone->SetBoxExtent(LoadZoneExtent);
		}
	}

	// If ActivateZoneExtent changes, sync to built-in ActivateZone component
	if (MemberPropertyName == GET_MEMBER_NAME_CHECKED(AStage, ActivateZoneExtent))
	{
		if (BuiltInActivateZone)
		{
			BuiltInActivateZone->SetBoxExtent(ActivateZoneExtent);
		}
	}
}

void AStage::BeginDestroy()
{
	// Note: Subsystem unregistration is handled by StageEditorController::OnLevelActorDeleted
	Super::BeginDestroy();
}
#endif

void AStage::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	// Apply built-in zone visibility based on bDisableBuiltInZones
	UpdateBuiltInZoneVisibility();
}

void AStage::UpdateBuiltInZoneVisibility()
{
	// When bDisableBuiltInZones is true, completely hide the built-in zones
	const bool bShouldBeVisible = !bDisableBuiltInZones;

	if (BuiltInLoadZone)
	{
		BuiltInLoadZone->SetVisibility(bShouldBeVisible);
		BuiltInLoadZone->SetHiddenInGame(!bShouldBeVisible);
		// Also disable collision when hidden
		BuiltInLoadZone->SetCollisionEnabled(bShouldBeVisible ? ECollisionEnabled::QueryOnly : ECollisionEnabled::NoCollision);
	}

	if (BuiltInActivateZone)
	{
		BuiltInActivateZone->SetVisibility(bShouldBeVisible);
		BuiltInActivateZone->SetHiddenInGame(!bShouldBeVisible);
		BuiltInActivateZone->SetCollisionEnabled(bShouldBeVisible ? ECollisionEnabled::QueryOnly : ECollisionEnabled::NoCollision);
	}
}

void AStage::BeginPlay()
{
	Super::BeginPlay();

	// Register with StageManagerSubsystem
	// This ensures Stage is registered even in WorldPartition where it may load after Subsystem init
	if (UWorld* World = GetWorld())
	{
		if (UStageManagerSubsystem* Subsystem = World->GetSubsystem<UStageManagerSubsystem>())
		{
			Subsystem->RegisterStage(this);
			UE_LOG(LogStage, Log, TEXT("Stage [%s]: Registered with StageManagerSubsystem (ID: %d)"),
				*GetName(), GetStageID());
		}
	}

	// Initialize TriggerZones (binds built-in and external zones)
	InitializeTriggerZones();

	// Handle initial state based on configuration
	if (bDisableBuiltInZones)
	{
		// Built-in zones disabled - apply InitialStageState directly
		UE_LOG(LogStage, Log, TEXT("Stage [%s]: Built-in zones disabled, applying InitialStageState: %d"),
			*GetName(), (int32)InitialStageState);

		if (InitialStageState != EStageRuntimeState::Unloaded)
		{
			// Force state transition to desired initial state
			InternalGotoState(InitialStageState);
		}
		// If Unloaded, stay in default state and wait for external trigger
	}
	else
	{
		// Built-in zones enabled - check for actors already inside TriggerZones at spawn
		// (fixes player spawning inside LoadZone not triggering state change)
		// Delay by one frame to let physics system update overlap data
		GetWorld()->GetTimerManager().SetTimerForNextTick(this, &AStage::CheckInitialOverlaps);
	}

	// Note: Act InitialDataLayerState is applied in OnEnterState(Active)
	// when Stage transitions to Active state (via TriggerZone or InitialStageState)
}

//----------------------------------------------------------------
// Stage State Machine Implementation
//----------------------------------------------------------------

void AStage::InternalGotoState(EStageRuntimeState NewState)
{
	// Skip if already in target state
	if (CurrentStageState == NewState)
	{
		return;
	}

	// Check state lock (H-004.6)
	// If locked, only allow transition to the locked state (for ForceStageStateOverride)
	if (bIsStageStateLocked && NewState != LockedStageState)
	{
		UE_LOG(LogStage, Warning, TEXT("Stage [%s]: State transition blocked by lock. Locked to state %d, requested %d"),
			*GetName(), (int32)LockedStageState, (int32)NewState);
		return;
	}

	UE_LOG(LogStage, Log, TEXT("Stage [%s]: State transition %d -> %d"),
		*GetName(), (int32)CurrentStageState, (int32)NewState);

	// Exit current state
	OnExitState(CurrentStageState);

	// Update state
	EStageRuntimeState OldState = CurrentStageState;
	CurrentStageState = NewState;

	// Enter new state
	OnEnterState(NewState);
}

void AStage::OnEnterState(EStageRuntimeState State)
{
	switch (State)
	{
	case EStageRuntimeState::Unloaded:
		UE_LOG(LogTemp, Log, TEXT("Stage [%s]: Entered Unloaded state"), *GetName());
		break;

	case EStageRuntimeState::Preloading:
		UE_LOG(LogTemp, Log, TEXT("Stage [%s]: Entered Preloading state - requesting DataLayer load"), *GetName());
		// Request Stage DataLayer to load
		if (StageDataLayerAsset)
		{
			SetStageDataLayerState(EDataLayerRuntimeState::Loaded);
			// For now, assume immediate completion. TODO: Add async callback support
			OnStageDataLayerLoaded();
		}
		else
		{
			// No DataLayer, skip to Loaded
			OnStageDataLayerLoaded();
		}
		break;

	case EStageRuntimeState::Loaded:
		UE_LOG(LogTemp, Log, TEXT("Stage [%s]: Entered Loaded state - preload buffer"), *GetName());
		// Apply Loaded state to Acts that follow Stage state
		ApplyFollowingActStates(EDataLayerRuntimeState::Loaded);
		// Check if we should immediately activate (player already in ActivateZone)
		if (OverlappingActivateZoneActors.Num() > 0)
		{
			InternalGotoState(EStageRuntimeState::Active);
		}
		break;

	case EStageRuntimeState::Active:
		UE_LOG(LogTemp, Log, TEXT("Stage [%s]: Entered Active state - fully interactive"), *GetName());
		// Activate Stage DataLayer
		if (StageDataLayerAsset)
		{
			SetStageDataLayerState(EDataLayerRuntimeState::Activated);
		}
		// Apply Activated state to Acts that follow Stage state
		ApplyFollowingActStates(EDataLayerRuntimeState::Activated);
		// Apply InitialDataLayerState for Acts that don't follow Stage state
		ApplyInitialActDataLayerStates();
		break;

	case EStageRuntimeState::Unloading:
		UE_LOG(LogTemp, Log, TEXT("Stage [%s]: Entered Unloading state - requesting DataLayer unload"), *GetName());
		// Unload ALL Act DataLayers (not just following ones)
		// This is necessary because child DataLayers "remember" their state when parent unloads,
		// and will restore to that state when parent reloads.
		UnloadAllActDataLayers();
		// Request Stage DataLayer to unload
		if (StageDataLayerAsset)
		{
			SetStageDataLayerState(EDataLayerRuntimeState::Unloaded);
			// For now, assume immediate completion. TODO: Add async callback support
			OnStageDataLayerUnloaded();
		}
		else
		{
			OnStageDataLayerUnloaded();
		}
		break;
	}
}

void AStage::OnExitState(EStageRuntimeState State)
{
	// Currently no cleanup needed on exit
	// Reserved for future use
}

void AStage::OnStageDataLayerLoaded()
{
	if (CurrentStageState == EStageRuntimeState::Preloading)
	{
		InternalGotoState(EStageRuntimeState::Loaded);
	}
}

void AStage::OnStageDataLayerUnloaded()
{
	if (CurrentStageState == EStageRuntimeState::Unloading)
	{
		InternalGotoState(EStageRuntimeState::Unloaded);
	}
}

void AStage::ApplyFollowingActStates(EDataLayerRuntimeState TargetState)
{
	UE_LOG(LogStage, Log, TEXT("Stage [%s]: Applying FollowStageState for Acts (TargetState=%d)"), *GetName(), (int32)TargetState);

	for (const FAct& Act : Acts)
	{
		// Only process Acts that follow Stage state
		if (!Act.bFollowStageState)
		{
			continue;
		}

		int32 ActID = Act.SUID.ActID;

		switch (TargetState)
		{
		case EDataLayerRuntimeState::Activated:
			// Fully activate this Act
			if (!IsActActive(ActID))
			{
				ActivateAct(ActID);
				UE_LOG(LogStage, Log, TEXT("Stage [%s]: Act '%s' (ID:%d) activated (FollowStageState)"),
					*GetName(), *Act.DisplayName, ActID);
			}
			break;

		case EDataLayerRuntimeState::Loaded:
			// Preload DataLayer only (don't add to ActiveActIDs, don't apply PropStates)
			if (Act.AssociatedDataLayer)
			{
				SetActDataLayerState(ActID, EDataLayerRuntimeState::Loaded);
				UE_LOG(LogStage, Log, TEXT("Stage [%s]: Act '%s' (ID:%d) DataLayer preloaded (FollowStageState)"),
					*GetName(), *Act.DisplayName, ActID);
			}
			break;

		case EDataLayerRuntimeState::Unloaded:
		default:
			// Deactivate Act and unload DataLayer
			if (IsActActive(ActID))
			{
				DeactivateAct(ActID);
			}
			if (Act.AssociatedDataLayer)
			{
				SetActDataLayerState(ActID, EDataLayerRuntimeState::Unloaded);
			}
			UE_LOG(LogStage, Log, TEXT("Stage [%s]: Act '%s' (ID:%d) unloaded (FollowStageState)"),
				*GetName(), *Act.DisplayName, ActID);
			break;
		}
	}
}

void AStage::ApplyInitialActDataLayerStates()
{
	UE_LOG(LogStage, Log, TEXT("Stage [%s]: Applying InitialDataLayerState for non-following Acts"), *GetName());

	for (const FAct& Act : Acts)
	{
		// Skip Acts that follow Stage state (they are handled by ApplyFollowingActStates)
		if (Act.bFollowStageState)
		{
			continue;
		}

		int32 ActID = Act.SUID.ActID;

		switch (Act.InitialDataLayerState)
		{
		case EDataLayerRuntimeState::Activated:
			// Fully activate this Act (add to ActiveActIDs + activate DataLayer + apply PropStates)
			if (!IsActActive(ActID))
			{
				ActivateAct(ActID);
				UE_LOG(LogStage, Log, TEXT("Stage [%s]: Act '%s' (ID:%d) activated (InitialDataLayerState=Activated)"),
					*GetName(), *Act.DisplayName, ActID);
			}
			break;

		case EDataLayerRuntimeState::Loaded:
			// Preload DataLayer only (don't add to ActiveActIDs, don't apply PropStates)
			if (Act.AssociatedDataLayer)
			{
				SetActDataLayerState(ActID, EDataLayerRuntimeState::Loaded);
				UE_LOG(LogStage, Log, TEXT("Stage [%s]: Act '%s' (ID:%d) DataLayer preloaded (InitialDataLayerState=Loaded)"),
					*GetName(), *Act.DisplayName, ActID);
			}
			break;

		case EDataLayerRuntimeState::Unloaded:
		default:
			// Keep unloaded, wait for explicit ActivateAct() call
			UE_LOG(LogStage, Verbose, TEXT("Stage [%s]: Act '%s' (ID:%d) remains unloaded (InitialDataLayerState=Unloaded)"),
				*GetName(), *Act.DisplayName, ActID);
			break;
		}
	}
}

void AStage::UnloadAllActDataLayers()
{
	UE_LOG(LogStage, Log, TEXT("Stage [%s]: Unloading ALL Act DataLayers"), *GetName());

	for (const FAct& Act : Acts)
	{
		int32 ActID = Act.SUID.ActID;

		// Deactivate Act if active
		if (IsActActive(ActID))
		{
			DeactivateAct(ActID);
		}

		// Unload DataLayer
		if (Act.AssociatedDataLayer)
		{
			SetActDataLayerState(ActID, EDataLayerRuntimeState::Unloaded);
			UE_LOG(LogStage, Log, TEXT("Stage [%s]: Act '%s' (ID:%d) DataLayer unloaded"),
				*GetName(), *Act.DisplayName, ActID);
		}
	}
}

bool AStage::IsInTransitionState() const
{
	return CurrentStageState == EStageRuntimeState::Preloading ||
	       CurrentStageState == EStageRuntimeState::Unloading;
}

bool AStage::IsStageLoaded() const
{
	return CurrentStageState == EStageRuntimeState::Loaded ||
	       CurrentStageState == EStageRuntimeState::Active;
}

//----------------------------------------------------------------
// State Lock API Implementation (H-004.6)
//----------------------------------------------------------------

void AStage::ForceStageStateOverride(EStageRuntimeState NewState, bool bLockState)
{
	UE_LOG(LogStage, Log, TEXT("Stage [%s]: ForceStageStateOverride to %d (Lock: %s)"),
		*GetName(), (int32)NewState, bLockState ? TEXT("true") : TEXT("false"));

	// If we're going to lock to this state, set the lock first
	if (bLockState)
	{
		LockedStageState = NewState;
		bIsStageStateLocked = true;
	}

	// Force the state transition (bypasses normal lock check because LockedStageState == NewState)
	if (CurrentStageState != NewState)
	{
		// Temporarily allow this transition by setting locked state to target
		EStageRuntimeState PreviousLockedState = LockedStageState;
		bool bWasLocked = bIsStageStateLocked;

		LockedStageState = NewState;

		// Exit current state
		OnExitState(CurrentStageState);

		// Update state
		CurrentStageState = NewState;

		// Enter new state
		OnEnterState(NewState);

		// Restore lock state if we weren't supposed to lock
		if (!bLockState)
		{
			LockedStageState = PreviousLockedState;
			bIsStageStateLocked = bWasLocked;
		}
	}
}

void AStage::ReleaseStageStateOverride()
{
	if (!bIsStageStateLocked)
	{
		UE_LOG(LogStage, Log, TEXT("Stage [%s]: ReleaseStageStateOverride - Stage was not locked"), *GetName());
		return;
	}

	UE_LOG(LogStage, Log, TEXT("Stage [%s]: ReleaseStageStateOverride - releasing lock"), *GetName());

	bIsStageStateLocked = false;
	LockedStageState = EStageRuntimeState::Unloaded;

	// Re-evaluate state based on current TriggerZone overlaps
	// This allows the Stage to transition to the correct state based on reality
	if (OverlappingLoadZoneActors.Num() == 0)
	{
		// No one in LoadZone -> should be Unloaded or Unloading
		if (CurrentStageState == EStageRuntimeState::Loaded ||
		    CurrentStageState == EStageRuntimeState::Active)
		{
			InternalGotoState(EStageRuntimeState::Unloading);
		}
	}
	else if (OverlappingActivateZoneActors.Num() > 0)
	{
		// Someone in ActivateZone -> should be Active
		if (CurrentStageState == EStageRuntimeState::Loaded)
		{
			InternalGotoState(EStageRuntimeState::Active);
		}
		else if (CurrentStageState == EStageRuntimeState::Unloaded)
		{
			InternalGotoState(EStageRuntimeState::Preloading);
		}
	}
	else
	{
		// In LoadZone but not ActivateZone -> should be Loaded
		if (CurrentStageState == EStageRuntimeState::Unloaded)
		{
			InternalGotoState(EStageRuntimeState::Preloading);
		}
	}
}

void AStage::LockAct(int32 ActID)
{
	if (!DoesActExist(ActID))
	{
		UE_LOG(LogStage, Warning, TEXT("Stage [%s]: LockAct - Act %d not found"), *GetName(), ActID);
		return;
	}

	LockedActIDs.Add(ActID);
	UE_LOG(LogStage, Log, TEXT("Stage [%s]: Locked Act %d"), *GetName(), ActID);
}

void AStage::UnlockAct(int32 ActID)
{
	if (LockedActIDs.Remove(ActID) > 0)
	{
		UE_LOG(LogStage, Log, TEXT("Stage [%s]: Unlocked Act %d"), *GetName(), ActID);
	}
}

//----------------------------------------------------------------
// TriggerZone Management
//----------------------------------------------------------------

void AStage::InitializeTriggerZones()
{
	// Clear previous registrations
	RegisteredLoadZones.Empty();
	RegisteredActivateZones.Empty();

	// Helper lambda to sync shared filtering settings to a zone
	auto SyncSharedFilteringToZone = [this](UStageTriggerZoneComponent* Zone)
	{
		if (Zone && bUseSharedFiltering)
		{
			Zone->TriggerActorTags = SharedTriggerActorTags;
			Zone->bMustBePawn = bSharedRequirePawnWithTag;
		}
	};

	// 1. Process built-in zones (if not disabled)
	if (!bDisableBuiltInZones)
	{
		if (BuiltInLoadZone)
		{
			SyncSharedFilteringToZone(BuiltInLoadZone);
			BuiltInLoadZone->BindToStage(this);
			RegisteredLoadZones.Add(BuiltInLoadZone);
		}

		if (BuiltInActivateZone)
		{
			SyncSharedFilteringToZone(BuiltInActivateZone);
			BuiltInActivateZone->BindToStage(this);
			RegisteredActivateZones.Add(BuiltInActivateZone);
		}
	}
	else
	{
		UE_LOG(LogStage, Warning, TEXT("Stage [%s]: Built-in TriggerZones disabled. Make sure external zones are configured!"), *GetName());
	}

	// 2. Process external zones
	for (const TSoftObjectPtr<UStageTriggerZoneComponent>& ZoneRef : ExternalTriggerZones)
	{
		if (UStageTriggerZoneComponent* Zone = ZoneRef.Get())
		{
			Zone->BindToStage(this);

			if (Zone->ZoneType == EStageTriggerZoneType::LoadZone)
			{
				RegisteredLoadZones.Add(Zone);
			}
			else
			{
				RegisteredActivateZones.Add(Zone);
			}

			UE_LOG(LogStage, Log, TEXT("Stage [%s]: Bound external %s '%s'"),
				*GetName(),
				Zone->ZoneType == EStageTriggerZoneType::LoadZone ? TEXT("LoadZone") : TEXT("ActivateZone"),
				*Zone->GetName());
		}
	}

	// 3. Validation
	if (RegisteredLoadZones.Num() == 0)
	{
		UE_LOG(LogStage, Error, TEXT("Stage [%s]: No LoadZone registered! State machine will not work properly."), *GetName());
	}

	if (RegisteredActivateZones.Num() == 0)
	{
		UE_LOG(LogStage, Warning, TEXT("Stage [%s]: No ActivateZone registered. Stage will only transition to Loaded state."), *GetName());
	}

	UE_LOG(LogStage, Log, TEXT("Stage [%s]: TriggerZones initialized. LoadZones: %d, ActivateZones: %d"),
		*GetName(), RegisteredLoadZones.Num(), RegisteredActivateZones.Num());
}

void AStage::CheckInitialOverlaps()
{
	// Fix for actors already inside TriggerZones at BeginPlay
	// Delayed by one frame (SetTimerForNextTick) to ensure physics overlap data is updated
	// This handles player spawning inside zones correctly

	UE_LOG(LogStage, Log, TEXT("Stage [%s]: CheckInitialOverlaps - checking existing overlapping actors"), *GetName());

	TArray<AActor*> OverlappingActors;

	// Check LoadZones for existing overlapping actors
	for (UStageTriggerZoneComponent* LoadZone : RegisteredLoadZones)
	{
		if (!LoadZone) continue;

		OverlappingActors.Reset();
		LoadZone->GetOverlappingActors(OverlappingActors);

		for (AActor* Actor : OverlappingActors)
		{
			if (Actor && LoadZone->ShouldTriggerForActor(Actor))
			{
				HandleZoneBeginOverlap(LoadZone, Actor);
			}
		}
	}

	// Check ActivateZones for existing overlapping actors
	for (UStageTriggerZoneComponent* ActivateZone : RegisteredActivateZones)
	{
		if (!ActivateZone) continue;

		OverlappingActors.Reset();
		ActivateZone->GetOverlappingActors(OverlappingActors);

		for (AActor* Actor : OverlappingActors)
		{
			if (Actor && ActivateZone->ShouldTriggerForActor(Actor))
			{
				HandleZoneBeginOverlap(ActivateZone, Actor);
			}
		}
	}

	UE_LOG(LogStage, Log, TEXT("Stage [%s]: CheckInitialOverlaps complete. State: %d, LoadZoneActors: %d, ActivateZoneActors: %d"),
		*GetName(), (int32)CurrentStageState, OverlappingLoadZoneActors.Num(), OverlappingActivateZoneActors.Num());
}

void AStage::HandleZoneBeginOverlap(UStageTriggerZoneComponent* Zone, AActor* OtherActor)
{
	if (!Zone || !OtherActor) return;

	if (Zone->ZoneType == EStageTriggerZoneType::LoadZone)
	{
		// Add to LoadZone tracking set
		OverlappingLoadZoneActors.Add(OtherActor);

		UE_LOG(LogStage, Log, TEXT("Stage [%s]: Actor '%s' entered LoadZone (count: %d)"),
			*GetName(), *OtherActor->GetName(), OverlappingLoadZoneActors.Num());

		// First actor entering LoadZone triggers loading
		if (OverlappingLoadZoneActors.Num() == 1)
		{
			if (CurrentStageState == EStageRuntimeState::Unloaded)
			{
				InternalGotoState(EStageRuntimeState::Preloading);
			}
		}
	}
	else // ActivateZone
	{
		// Add to ActivateZone tracking set
		OverlappingActivateZoneActors.Add(OtherActor);

		UE_LOG(LogStage, Log, TEXT("Stage [%s]: Actor '%s' entered ActivateZone (count: %d)"),
			*GetName(), *OtherActor->GetName(), OverlappingActivateZoneActors.Num());

		// First actor entering ActivateZone triggers activation
		if (OverlappingActivateZoneActors.Num() == 1)
		{
			if (CurrentStageState == EStageRuntimeState::Loaded)
			{
				InternalGotoState(EStageRuntimeState::Active);
			}
		}
	}
}

void AStage::HandleZoneEndOverlap(UStageTriggerZoneComponent* Zone, AActor* OtherActor)
{
	if (!Zone || !OtherActor) return;

	if (Zone->ZoneType == EStageTriggerZoneType::LoadZone)
	{
		// Remove from LoadZone tracking set
		OverlappingLoadZoneActors.Remove(OtherActor);
		// Also remove from ActivateZone tracking (leaving LoadZone means also leaving ActivateZone)
		OverlappingActivateZoneActors.Remove(OtherActor);

		UE_LOG(LogStage, Log, TEXT("Stage [%s]: Actor '%s' left LoadZone (count: %d)"),
			*GetName(), *OtherActor->GetName(), OverlappingLoadZoneActors.Num());

		// Last actor leaving LoadZone triggers unloading
		if (OverlappingLoadZoneActors.Num() == 0)
		{
			if (CurrentStageState == EStageRuntimeState::Loaded ||
			    CurrentStageState == EStageRuntimeState::Active)
			{
				InternalGotoState(EStageRuntimeState::Unloading);
			}
		}
	}
	else // ActivateZone
	{
		// Remove from ActivateZone tracking set
		OverlappingActivateZoneActors.Remove(OtherActor);

		UE_LOG(LogStage, Log, TEXT("Stage [%s]: Actor '%s' left ActivateZone (count: %d)"),
			*GetName(), *OtherActor->GetName(), OverlappingActivateZoneActors.Num());

		// Design decision: Stay Active even when leaving ActivateZone (until leaving LoadZone)
		// This prevents flickering when player is on the ActivateZone boundary
		// Unloading only happens when leaving LoadZone entirely
	}
}

//----------------------------------------------------------------
// Multi-Act Activation Control API Implementation
//----------------------------------------------------------------

void AStage::ActivateAct(int32 ActID)
{
	// 1. Verify Act exists
	if (!DoesActExist(ActID))
	{
		UE_LOG(LogStage, Warning, TEXT("Stage [%s]: Failed to activate Act %d. Act not found."), *GetName(), ActID);
		return;
	}

	// Find the Act for logging and DataLayer access
	const FAct* TargetAct = Acts.FindByPredicate([ActID](const FAct& Act) {
		return Act.SUID.ActID == ActID;
	});

	UE_LOG(LogStage, Log, TEXT("Stage [%s]: Activating Act '%s' (ID:%d)"), *GetName(), *TargetAct->DisplayName, ActID);

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
		UE_LOG(LogStage, Warning, TEXT("Stage [%s]: Act %d is not active, cannot deactivate."), *GetName(), ActID);
		return;
	}

	// 2. Check Act lock (H-004.6)
	if (LockedActIDs.Contains(ActID))
	{
		UE_LOG(LogStage, Warning, TEXT("Stage [%s]: Act %d is locked, cannot deactivate."), *GetName(), ActID);
		return;
	}

	// Find Act for logging
	const FAct* TargetAct = Acts.FindByPredicate([ActID](const FAct& Act) {
		return Act.SUID.ActID == ActID;
	});

	FString ActName = TargetAct ? TargetAct->DisplayName : FString::Printf(TEXT("Act_%d"), ActID);
	UE_LOG(LogStage, Log, TEXT("Stage [%s]: Deactivating Act '%s' (ID:%d)"), *GetName(), *ActName, ActID);

	// 3. Remove from active list
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
		// DeactivateAct will check for locks internally
		DeactivateAct(ActID);
	}

	// Log if any Acts remained active due to locks
	if (ActiveActIDs.Num() > 0)
	{
		UE_LOG(LogStage, Log, TEXT("Stage [%s]: DeactivateAllActs - %d Acts remain active due to locks"),
			*GetName(), ActiveActIDs.Num());
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

	// === Prevent registering Stage actors as Props (nested Stage not allowed) ===
	if (NewProp->IsA<AStage>())
	{
		UE_LOG(LogTemp, Error,
			TEXT("Stage [%s]: Cannot register Stage actor '%s' as a Prop! "
			     "Stage actors cannot be nested. This is a dangerous operation."),
			*GetName(), *NewProp->GetName());
		return -1;
	}

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

	// Auto-add to Default Act (ID 1)
	FAct* DefaultAct = Acts.FindByPredicate([](const FAct& Act) {
		return Act.SUID.ActID == 1;
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
		NewDefaultAct.SUID.ActID = 1;
		NewDefaultAct.DisplayName = TEXT("Default Act");
		NewDefaultAct.bFollowStageState = true; // Default Act follows Stage state
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

//----------------------------------------------------------------
// Generic TriggerZone Registration (H-006)
//----------------------------------------------------------------

void AStage::RegisterTriggerZone(UTriggerZoneComponentBase* Zone)
{
	if (!Zone)
	{
		return;
	}

	if (RegisteredTriggerZones.Contains(Zone))
	{
		UE_LOG(LogStage, Verbose, TEXT("Stage [%s]: TriggerZone [%s] already registered"),
			*GetName(), *Zone->GetName());
		return;
	}

	RegisteredTriggerZones.Add(Zone);

	UE_LOG(LogStage, Log, TEXT("Stage [%s]: Registered TriggerZone [%s] (Total: %d)"),
		*GetName(), *Zone->GetName(), RegisteredTriggerZones.Num());
}

void AStage::UnregisterTriggerZone(UTriggerZoneComponentBase* Zone)
{
	if (!Zone)
	{
		return;
	}

	if (RegisteredTriggerZones.Remove(Zone) > 0)
	{
		UE_LOG(LogStage, Log, TEXT("Stage [%s]: Unregistered TriggerZone [%s] (Remaining: %d)"),
			*GetName(), *Zone->GetName(), RegisteredTriggerZones.Num());
	}
}

TArray<UTriggerZoneComponentBase*> AStage::GetAllTriggerZones() const
{
	TArray<UTriggerZoneComponentBase*> Result;
	for (const TObjectPtr<UTriggerZoneComponentBase>& Zone : RegisteredTriggerZones)
	{
		if (Zone)
		{
			Result.Add(Zone);
		}
	}
	return Result;
}

//----------------------------------------------------------------
// Simplified User API (3-State) - H-006
//----------------------------------------------------------------

EStageState AStage::GetStageState() const
{
	switch (CurrentStageState)
	{
	case EStageRuntimeState::Unloaded:
	case EStageRuntimeState::Unloading:
		return EStageState::Unloaded;

	case EStageRuntimeState::Preloading:
	case EStageRuntimeState::Loaded:
		return EStageState::Loaded;

	case EStageRuntimeState::Active:
		return EStageState::Active;

	default:
		return EStageState::Unloaded;
	}
}

void AStage::GotoState(EStageState TargetState)
{
	UE_LOG(LogStage, Log, TEXT("Stage [%s]: GotoState(%s) requested"),
		*GetName(),
		TargetState == EStageState::Unloaded ? TEXT("Unloaded") :
		TargetState == EStageState::Loaded ? TEXT("Loaded") :
		TargetState == EStageState::Active ? TEXT("Active") : TEXT("Unknown"));

	switch (TargetState)
	{
	case EStageState::Unloaded:
		// Request unload
		if (CurrentStageState != EStageRuntimeState::Unloaded &&
			CurrentStageState != EStageRuntimeState::Unloading)
		{
			InternalGotoState(EStageRuntimeState::Unloading);
		}
		break;

	case EStageState::Loaded:
		// Request load (Unloaded → Preloading → Loaded)
		if (CurrentStageState == EStageRuntimeState::Unloaded)
		{
			InternalGotoState(EStageRuntimeState::Preloading);
		}
		// If Active, could go to Loaded (deactivate but stay loaded)
		// For now, ignore - user can call GotoState(Unloaded) then GotoState(Loaded)
		break;

	case EStageState::Active:
		// Request full activation
		switch (CurrentStageState)
		{
		case EStageRuntimeState::Unloaded:
			// Need to load first, then activate
			InternalGotoState(EStageRuntimeState::Preloading);
			InternalGotoState(EStageRuntimeState::Active);
			break;

		case EStageRuntimeState::Preloading:
			// Already loading, wait for completion
			UE_LOG(LogStage, Verbose, TEXT("Stage [%s]: GotoState(Active) - still preloading"),
				*GetName());
			break;

		case EStageRuntimeState::Loaded:
			// Ready to activate
			InternalGotoState(EStageRuntimeState::Active);
			break;

		case EStageRuntimeState::Active:
			// Already active
			break;

		case EStageRuntimeState::Unloading:
			UE_LOG(LogStage, Warning, TEXT("Stage [%s]: GotoState(Active) called during unloading - ignored"),
				*GetName());
			break;
		}
		break;
	}
}
