#pragma region Imports
#include "Subsystems/StageManagerSubsystem.h"
#include "Actors/Stage.h"
#include "EngineUtils.h"
#pragma endregion Imports

DEFINE_LOG_CATEGORY_STATIC(LogStageManager, Log, All);

#pragma region Lifecycle

void UStageManagerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// Reset ID counter to 1 (0 is reserved for invalid)
	NextStageID = 1;
	StageRegistry.Empty();

	// Scan for existing Stages in the world
	ScanWorldForExistingStages();

	UE_LOG(LogStageManager, Log, TEXT("StageManagerSubsystem initialized. NextStageID: %d"), NextStageID);
}

void UStageManagerSubsystem::Deinitialize()
{
	// Release all overrides before clearing registry
	ReleaseAllStageOverrides();

	UE_LOG(LogStageManager, Log, TEXT("StageManagerSubsystem deinitialized. Total registered Stages: %d"), StageRegistry.Num());

	StageRegistry.Empty();
	OverriddenStageStates.Empty();

	Super::Deinitialize();
}

bool UStageManagerSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	// Create for all worlds that have a valid world context
	UWorld* World = Cast<UWorld>(Outer);
	if (!World)
	{
		return false;
	}

	// Create for Editor, PIE, and Game worlds
	// Skip for preview worlds (e.g., asset editor previews)
	EWorldType::Type WorldType = World->WorldType;
	return WorldType == EWorldType::Editor ||
	       WorldType == EWorldType::PIE ||
	       WorldType == EWorldType::Game ||
	       WorldType == EWorldType::GamePreview;
}

#pragma endregion Lifecycle

#pragma region Stage Registration API

int32 UStageManagerSubsystem::RegisterStage(AStage* Stage)
{
	if (!Stage)
	{
		UE_LOG(LogStageManager, Error, TEXT("StageManagerSubsystem: Cannot register null Stage"));
		return -1;
	}

	// Clean up invalid entries first
	CleanupRegistry();

	int32 ResultStageID = -1;

	// Check if Stage already has a valid ID and is already registered
	if (Stage->GetStageID() > 0)
	{
		// Check if this Stage is already in our registry with this ID
		if (TWeakObjectPtr<AStage>* ExistingPtr = StageRegistry.Find(Stage->GetStageID()))
		{
			if (ExistingPtr->Get() == Stage)
			{
				// Already registered with this ID, nothing to do
				UE_LOG(LogStageManager, Verbose, TEXT("StageManagerSubsystem: Stage '%s' already registered with ID: %d"),
					*Stage->GetName(), Stage->GetStageID());
				ResultStageID = Stage->GetStageID();
			}
			else if (ExistingPtr->IsValid())
			{
				// Different Stage has this ID - conflict! This shouldn't happen in normal usage.
				UE_LOG(LogStageManager, Warning, TEXT("StageManagerSubsystem: ID conflict! Stage '%s' tried to register with ID %d which belongs to '%s'"),
					*Stage->GetName(), Stage->GetStageID(), *ExistingPtr->Get()->GetName());
				// Allocate a new ID for this Stage
				int32 NewStageID = AllocateStageID();
				StageRegistry.Add(NewStageID, Stage);
				UE_LOG(LogStageManager, Log, TEXT("StageManagerSubsystem: Registered Stage '%s' with new ID: %d (resolved conflict)"),
					*Stage->GetName(), NewStageID);
				ResultStageID = NewStageID;
			}
		}

		if (ResultStageID == -1)
		{
			// Stage has an ID but isn't in registry - register it with its existing ID
			StageRegistry.Add(Stage->GetStageID(), Stage);

			// Update NextStageID if necessary to prevent future conflicts
			if (Stage->GetStageID() >= NextStageID)
			{
				NextStageID = Stage->GetStageID() + 1;
			}

			UE_LOG(LogStageManager, Log, TEXT("StageManagerSubsystem: Re-registered Stage '%s' with existing ID: %d"),
				*Stage->GetName(), Stage->GetStageID());
			ResultStageID = Stage->GetStageID();
		}
	}
	else
	{
		// Stage needs a new ID
		int32 NewStageID = AllocateStageID();
		StageRegistry.Add(NewStageID, Stage);

		UE_LOG(LogStageManager, Log, TEXT("StageManagerSubsystem: Registered Stage '%s' with new ID: %d"),
			*Stage->GetName(), NewStageID);
		ResultStageID = NewStageID;
	}

	// Sync Editor watch state to runtime watch list
	if (ResultStageID > 0 && Stage->bEditorWatched)
	{
		WatchedStageIDs.Add(ResultStageID);
		UE_LOG(LogStageManager, Log, TEXT("StageManagerSubsystem: Stage '%s' (ID:%d) auto-added to watch list (bEditorWatched=true)"),
			*Stage->GetName(), ResultStageID);
	}

	return ResultStageID;
}

void UStageManagerSubsystem::UnregisterStage(int32 StageID)
{
	if (StageID <= 0)
	{
		UE_LOG(LogStageManager, Warning, TEXT("StageManagerSubsystem: Invalid StageID for unregistration: %d"), StageID);
		return;
	}

	// Remove any override tracking for this Stage
	OverriddenStageStates.Remove(StageID);

	if (StageRegistry.Remove(StageID) > 0)
	{
		UE_LOG(LogStageManager, Log, TEXT("StageManagerSubsystem: Unregistered Stage ID: %d"), StageID);
	}
	else
	{
		UE_LOG(LogStageManager, Verbose, TEXT("StageManagerSubsystem: Attempted to unregister non-existent Stage ID: %d"), StageID);
	}

	// Note: We do NOT decrement NextStageID - IDs are never reused
}

AStage* UStageManagerSubsystem::GetStage(int32 StageID) const
{
	if (StageID <= 0) return nullptr;

	const TWeakObjectPtr<AStage>* StagePtr = StageRegistry.Find(StageID);
	if (StagePtr && StagePtr->IsValid())
	{
		return StagePtr->Get();
	}
	return nullptr;
}

TArray<AStage*> UStageManagerSubsystem::GetAllStages() const
{
	TArray<AStage*> Stages;

	for (const TPair<int32, TWeakObjectPtr<AStage>>& Pair : StageRegistry)
	{
		if (Pair.Value.IsValid())
		{
			Stages.Add(Pair.Value.Get());
		}
	}

	return Stages;
}

bool UStageManagerSubsystem::IsStageIDRegistered(int32 StageID) const
{
	if (StageID <= 0) return false;

	const TWeakObjectPtr<AStage>* StagePtr = StageRegistry.Find(StageID);
	return StagePtr && StagePtr->IsValid();
}

int32 UStageManagerSubsystem::GetRegisteredStageCount() const
{
	int32 Count = 0;
	for (const TPair<int32, TWeakObjectPtr<AStage>>& Pair : StageRegistry)
	{
		if (Pair.Value.IsValid())
		{
			Count++;
		}
	}
	return Count;
}

#pragma endregion Stage Registration API

#pragma region Internal Methods

int32 UStageManagerSubsystem::AllocateStageID()
{
	int32 AllocatedID = NextStageID;
	NextStageID++;

	UE_LOG(LogStageManager, Verbose, TEXT("StageManagerSubsystem: Allocated StageID: %d (NextStageID now: %d)"),
		AllocatedID, NextStageID);

	return AllocatedID;
}

void UStageManagerSubsystem::CleanupRegistry()
{
	TArray<int32> InvalidIDs;

	for (const TPair<int32, TWeakObjectPtr<AStage>>& Pair : StageRegistry)
	{
		if (!Pair.Value.IsValid())
		{
			InvalidIDs.Add(Pair.Key);
		}
	}

	for (int32 ID : InvalidIDs)
	{
		StageRegistry.Remove(ID);
		UE_LOG(LogStageManager, Verbose, TEXT("StageManagerSubsystem: Cleaned up invalid Stage ID: %d"), ID);
	}

	if (InvalidIDs.Num() > 0)
	{
		UE_LOG(LogStageManager, Log, TEXT("StageManagerSubsystem: Cleaned up %d invalid Stage entries"), InvalidIDs.Num());
	}
}

void UStageManagerSubsystem::ScanWorldForExistingStages()
{
	UWorld* World = GetWorld();
	if (!World) return;

	int32 FoundCount = 0;
	int32 MaxExistingID = 0;
	int32 EditorWatchedCount = 0;

	for (TActorIterator<AStage> It(World); It; ++It)
	{
		AStage* Stage = *It;
		if (Stage)
		{
			if (Stage->GetStageID() > 0)
			{
				// Stage already has an ID - register it
				StageRegistry.Add(Stage->GetStageID(), Stage);

				if (Stage->GetStageID() > MaxExistingID)
				{
					MaxExistingID = Stage->GetStageID();
				}

				// Sync Editor watch state to runtime watch list
				if (Stage->bEditorWatched)
				{
					WatchedStageIDs.Add(Stage->GetStageID());
					EditorWatchedCount++;
				}

				UE_LOG(LogStageManager, Log, TEXT("StageManagerSubsystem: Found existing Stage '%s' with ID: %d%s"),
					*Stage->GetName(), Stage->GetStageID(),
					Stage->bEditorWatched ? TEXT(" (watched)") : TEXT(""));
			}
			else
			{
				// Stage without ID - will be assigned during PostActorCreated or explicit registration
				UE_LOG(LogStageManager, Warning, TEXT("StageManagerSubsystem: Found Stage '%s' without ID - needs registration"),
					*Stage->GetName());
			}
			FoundCount++;
		}
	}

	// Update NextStageID to be higher than any existing ID
	if (MaxExistingID >= NextStageID)
	{
		NextStageID = MaxExistingID + 1;
	}

	UE_LOG(LogStageManager, Log, TEXT("StageManagerSubsystem: World scan complete. Found %d Stages, %d pre-watched. NextStageID: %d"),
		FoundCount, EditorWatchedCount, NextStageID);
}

#pragma endregion Internal Methods

#pragma region Cross-Stage Communication API

void UStageManagerSubsystem::ForceActivateStage(int32 StageID, bool bLockState)
{
	ForceStageState(StageID, EStageRuntimeState::Active, bLockState);
}

void UStageManagerSubsystem::ForceUnloadStage(int32 StageID, bool bLockState)
{
	ForceStageState(StageID, EStageRuntimeState::Unloaded, bLockState);
}

void UStageManagerSubsystem::ForceStageState(int32 StageID, EStageRuntimeState NewState, bool bLockState)
{
	// 1. Validate StageID
	if (StageID <= 0)
	{
		UE_LOG(LogStageManager, Warning, TEXT("ForceStageState: Invalid StageID: %d"), StageID);
		return;
	}

	// 2. Find the Stage
	AStage* Stage = GetStage(StageID);
	if (!Stage)
	{
		UE_LOG(LogStageManager, Warning, TEXT("ForceStageState: Stage %d not found or invalid"), StageID);
		return;
	}

	// 3. Call Stage's ForceStageStateOverride
	Stage->ForceStageStateOverride(NewState, bLockState);

	// 4. Update our tracking based on bLockState
	if (bLockState)
	{
		OverriddenStageStates.Add(StageID, NewState);
		UE_LOG(LogStageManager, Log, TEXT("ForceStageState: Stage '%s' (ID:%d) forced to state %d and locked"),
			*Stage->GetName(), StageID, (int32)NewState);
	}
	else
	{
		// Only a one-time force, not a persistent override from Subsystem
		OverriddenStageStates.Remove(StageID);
		UE_LOG(LogStageManager, Log, TEXT("ForceStageState: Stage '%s' (ID:%d) forced to state %d (not locked)"),
			*Stage->GetName(), StageID, (int32)NewState);
	}
}

void UStageManagerSubsystem::ReleaseStageOverride(int32 StageID)
{
	// 1. Validate StageID
	if (StageID <= 0)
	{
		UE_LOG(LogStageManager, Warning, TEXT("ReleaseStageOverride: Invalid StageID: %d"), StageID);
		return;
	}

	// 2. Remove from our tracking
	bool bWasOverridden = OverriddenStageStates.Remove(StageID) > 0;

	// 3. Find the Stage and release its override
	AStage* Stage = GetStage(StageID);
	if (Stage)
	{
		Stage->ReleaseStageStateOverride();
		UE_LOG(LogStageManager, Log, TEXT("ReleaseStageOverride: Released override on Stage '%s' (ID:%d)"),
			*Stage->GetName(), StageID);
	}
	else if (bWasOverridden)
	{
		UE_LOG(LogStageManager, Warning, TEXT("ReleaseStageOverride: Stage %d was tracked but no longer valid"), StageID);
	}
}

bool UStageManagerSubsystem::IsStageOverridden(int32 StageID) const
{
	return OverriddenStageStates.Contains(StageID);
}

bool UStageManagerSubsystem::GetStageOverrideState(int32 StageID, EStageRuntimeState& OutState) const
{
	const EStageRuntimeState* FoundState = OverriddenStageStates.Find(StageID);
	if (FoundState)
	{
		OutState = *FoundState;
		return true;
	}
	return false;
}

void UStageManagerSubsystem::ReleaseAllStageOverrides()
{
	if (OverriddenStageStates.Num() == 0)
	{
		return;
	}

	UE_LOG(LogStageManager, Log, TEXT("ReleaseAllStageOverrides: Releasing %d Stage overrides"), OverriddenStageStates.Num());

	// Collect IDs first since we'll be modifying the map
	TArray<int32> OverriddenIDs;
	OverriddenStageStates.GetKeys(OverriddenIDs);

	for (int32 StageID : OverriddenIDs)
	{
		AStage* Stage = GetStage(StageID);
		if (Stage)
		{
			Stage->ReleaseStageStateOverride();
			UE_LOG(LogStageManager, Verbose, TEXT("ReleaseAllStageOverrides: Released Stage '%s' (ID:%d)"),
				*Stage->GetName(), StageID);
		}
	}

	OverriddenStageStates.Empty();
}

#pragma endregion Cross-Stage Communication API

#pragma region Debug Watch API

bool UStageManagerSubsystem::WatchStage(int32 StageID)
{
	if (StageID <= 0)
	{
		UE_LOG(LogStageManager, Warning, TEXT("WatchStage: Invalid StageID: %d"), StageID);
		return false;
	}

	// Verify Stage exists
	AStage* Stage = GetStage(StageID);
	if (!Stage)
	{
		UE_LOG(LogStageManager, Warning, TEXT("WatchStage: Stage %d not found"), StageID);
		return false;
	}

	bool bAlreadyInSet = false;
	WatchedStageIDs.Add(StageID, &bAlreadyInSet);

	if (bAlreadyInSet)
	{
		UE_LOG(LogStageManager, Log, TEXT("WatchStage: Stage '%s' (ID:%d) already being watched"),
			*Stage->GetName(), StageID);
	}
	else
	{
		UE_LOG(LogStageManager, Log, TEXT("WatchStage: Now watching Stage '%s' (ID:%d). Total watched: %d"),
			*Stage->GetName(), StageID, WatchedStageIDs.Num());
	}

	return true;
}

bool UStageManagerSubsystem::UnwatchStage(int32 StageID)
{
	if (StageID <= 0)
	{
		UE_LOG(LogStageManager, Warning, TEXT("UnwatchStage: Invalid StageID: %d"), StageID);
		return false;
	}

	int32 Removed = WatchedStageIDs.Remove(StageID);
	if (Removed > 0)
	{
		AStage* Stage = GetStage(StageID);
		FString StageName = Stage ? Stage->GetName() : TEXT("(deleted)");
		UE_LOG(LogStageManager, Log, TEXT("UnwatchStage: Stopped watching Stage '%s' (ID:%d). Total watched: %d"),
			*StageName, StageID, WatchedStageIDs.Num());
		return true;
	}

	UE_LOG(LogStageManager, Warning, TEXT("UnwatchStage: Stage %d was not being watched"), StageID);
	return false;
}

void UStageManagerSubsystem::ClearWatchList()
{
	int32 PreviousCount = WatchedStageIDs.Num();
	WatchedStageIDs.Empty();
	UE_LOG(LogStageManager, Log, TEXT("ClearWatchList: Cleared %d watched Stages"), PreviousCount);
}

void UStageManagerSubsystem::WatchAllStages()
{
	WatchedStageIDs.Empty();

	for (const TPair<int32, TWeakObjectPtr<AStage>>& Pair : StageRegistry)
	{
		if (Pair.Value.IsValid())
		{
			WatchedStageIDs.Add(Pair.Key);
		}
	}

	UE_LOG(LogStageManager, Log, TEXT("WatchAllStages: Now watching %d Stages"), WatchedStageIDs.Num());
}

bool UStageManagerSubsystem::WatchOnlyStage(int32 StageID)
{
	if (StageID <= 0)
	{
		UE_LOG(LogStageManager, Warning, TEXT("WatchOnlyStage: Invalid StageID: %d"), StageID);
		return false;
	}

	AStage* Stage = GetStage(StageID);
	if (!Stage)
	{
		UE_LOG(LogStageManager, Warning, TEXT("WatchOnlyStage: Stage %d not found"), StageID);
		return false;
	}

	WatchedStageIDs.Empty();
	WatchedStageIDs.Add(StageID);

	UE_LOG(LogStageManager, Log, TEXT("WatchOnlyStage: Now watching only Stage '%s' (ID:%d)"),
		*Stage->GetName(), StageID);
	return true;
}

bool UStageManagerSubsystem::IsStageWatched(int32 StageID) const
{
	return WatchedStageIDs.Contains(StageID);
}

TArray<int32> UStageManagerSubsystem::GetWatchedStageIDs() const
{
	return WatchedStageIDs.Array();
}

#pragma endregion Debug Watch API
