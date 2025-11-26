#pragma region Imports
#include "Subsystems/StageEditorSubsystem.h"
#include "Actors/Stage.h"
#include "DebugHeader.h"
#include "Editor.h"
#include "EngineUtils.h"
#pragma endregion Imports

#pragma region Lifecycle

void UStageEditorSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// Reset ID counter to 1 (0 is reserved for invalid)
	NextStageID = 1;
	StageRegistry.Empty();

	// Scan for existing Stages in the world
	ScanWorldForExistingStages();

	UE_LOG(LogTemp, Log, TEXT("StageEditorSubsystem initialized. NextStageID: %d"), NextStageID);
}

void UStageEditorSubsystem::Deinitialize()
{
	UE_LOG(LogTemp, Log, TEXT("StageEditorSubsystem deinitialized. Total registered Stages: %d"), StageRegistry.Num());

	StageRegistry.Empty();

	Super::Deinitialize();
}

#pragma endregion Lifecycle

#pragma region Stage Registration API

int32 UStageEditorSubsystem::RegisterStage(AStage* Stage)
{
	if (!Stage)
	{
		UE_LOG(LogTemp, Error, TEXT("StageEditorSubsystem: Cannot register null Stage"));
		return -1;
	}

	// Clean up invalid entries first
	CleanupRegistry();

	// Check if Stage already has a valid ID and is already registered
	if (Stage->StageID > 0)
	{
		// Check if this Stage is already in our registry with this ID
		if (TWeakObjectPtr<AStage>* ExistingPtr = StageRegistry.Find(Stage->StageID))
		{
			if (ExistingPtr->Get() == Stage)
			{
				// Already registered with this ID, nothing to do
				UE_LOG(LogTemp, Verbose, TEXT("StageEditorSubsystem: Stage '%s' already registered with ID: %d"),
					*Stage->GetName(), Stage->StageID);
				return Stage->StageID;
			}
			else if (ExistingPtr->IsValid())
			{
				// Different Stage has this ID - conflict! This shouldn't happen in normal usage.
				UE_LOG(LogTemp, Warning, TEXT("StageEditorSubsystem: ID conflict! Stage '%s' tried to register with ID %d which belongs to '%s'"),
					*Stage->GetName(), Stage->StageID, *ExistingPtr->Get()->GetName());
				// Allocate a new ID for this Stage
				int32 NewStageID = AllocateStageID();
				StageRegistry.Add(NewStageID, Stage);
				UE_LOG(LogTemp, Log, TEXT("StageEditorSubsystem: Registered Stage '%s' with new ID: %d (resolved conflict)"),
					*Stage->GetName(), NewStageID);
				return NewStageID;
			}
		}

		// Stage has an ID but isn't in registry - register it with its existing ID
		StageRegistry.Add(Stage->StageID, Stage);

		// Update NextStageID if necessary to prevent future conflicts
		if (Stage->StageID >= NextStageID)
		{
			NextStageID = Stage->StageID + 1;
		}

		UE_LOG(LogTemp, Log, TEXT("StageEditorSubsystem: Re-registered Stage '%s' with existing ID: %d"),
			*Stage->GetName(), Stage->StageID);
		return Stage->StageID;
	}

	// Stage needs a new ID
	int32 NewStageID = AllocateStageID();
	StageRegistry.Add(NewStageID, Stage);

	UE_LOG(LogTemp, Log, TEXT("StageEditorSubsystem: Registered Stage '%s' with new ID: %d"),
		*Stage->GetName(), NewStageID);

	return NewStageID;
}

void UStageEditorSubsystem::UnregisterStage(int32 StageID)
{
	if (StageID <= 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("StageEditorSubsystem: Invalid StageID for unregistration: %d"), StageID);
		return;
	}

	if (StageRegistry.Remove(StageID) > 0)
	{
		UE_LOG(LogTemp, Log, TEXT("StageEditorSubsystem: Unregistered Stage ID: %d"), StageID);
	}
	else
	{
		UE_LOG(LogTemp, Verbose, TEXT("StageEditorSubsystem: Attempted to unregister non-existent Stage ID: %d"), StageID);
	}

	// Note: We do NOT decrement NextStageID - IDs are never reused
}

AStage* UStageEditorSubsystem::GetStage(int32 StageID) const
{
	if (StageID <= 0) return nullptr;

	const TWeakObjectPtr<AStage>* StagePtr = StageRegistry.Find(StageID);
	if (StagePtr && StagePtr->IsValid())
	{
		return StagePtr->Get();
	}
	return nullptr;
}

TArray<AStage*> UStageEditorSubsystem::GetAllStages() const
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

bool UStageEditorSubsystem::IsStageIDRegistered(int32 StageID) const
{
	if (StageID <= 0) return false;

	const TWeakObjectPtr<AStage>* StagePtr = StageRegistry.Find(StageID);
	return StagePtr && StagePtr->IsValid();
}

#pragma endregion Stage Registration API

#pragma region Internal Methods

int32 UStageEditorSubsystem::AllocateStageID()
{
	int32 AllocatedID = NextStageID;
	NextStageID++;

	UE_LOG(LogTemp, Verbose, TEXT("StageEditorSubsystem: Allocated StageID: %d (NextStageID now: %d)"),
		AllocatedID, NextStageID);

	return AllocatedID;
}

void UStageEditorSubsystem::CleanupRegistry()
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
		UE_LOG(LogTemp, Verbose, TEXT("StageEditorSubsystem: Cleaned up invalid Stage ID: %d"), ID);
	}

	if (InvalidIDs.Num() > 0)
	{
		UE_LOG(LogTemp, Log, TEXT("StageEditorSubsystem: Cleaned up %d invalid Stage entries"), InvalidIDs.Num());
	}
}

void UStageEditorSubsystem::ScanWorldForExistingStages()
{
	if (!GEditor) return;

	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World) return;

	int32 FoundCount = 0;
	int32 MaxExistingID = 0;

	for (TActorIterator<AStage> It(World); It; ++It)
	{
		AStage* Stage = *It;
		if (Stage)
		{
			if (Stage->StageID > 0)
			{
				// Stage already has an ID - register it
				StageRegistry.Add(Stage->StageID, Stage);

				if (Stage->StageID > MaxExistingID)
				{
					MaxExistingID = Stage->StageID;
				}

				UE_LOG(LogTemp, Log, TEXT("StageEditorSubsystem: Found existing Stage '%s' with ID: %d"),
					*Stage->GetName(), Stage->StageID);
			}
			else
			{
				// Stage without ID - will be assigned during PostActorCreated or explicit registration
				UE_LOG(LogTemp, Warning, TEXT("StageEditorSubsystem: Found Stage '%s' without ID - needs registration"),
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

	UE_LOG(LogTemp, Log, TEXT("StageEditorSubsystem: World scan complete. Found %d Stages. NextStageID set to: %d"),
		FoundCount, NextStageID);
}

#pragma endregion Internal Methods
