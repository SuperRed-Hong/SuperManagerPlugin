#include "EditorLogic/StageEditorController.h"
#include "Actors/Stage.h"
#include "Actors/Prop.h"
#include "DebugHeader.h"
#include "ScopedTransaction.h"
#include "AssetToolsModule.h"
#include "Factories/BlueprintFactory.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "Subsystems/AssetEditorSubsystem.h"
#include "Editor.h"
#include "EngineUtils.h"
#include "Components/StagePropComponent.h"
#include "WorldPartition/DataLayer/DataLayerAsset.h"
#include "DataLayer/DataLayerEditorSubsystem.h"
#include "WorldPartition/DataLayer/DataLayerInstance.h"
#include "WorldPartition/WorldPartitionConverter.h"

#define LOCTEXT_NAMESPACE "FStageEditorController"

FStageEditorController::FStageEditorController()
{
}

void FStageEditorController::Initialize()
{
	BindEditorDelegates();
	FindStageInWorld();
}

void FStageEditorController::FindStageInWorld()
{
	if (!GEditor) return;

	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World) return;

	FoundStages.Empty();
	ActiveStage.Reset();

	for (TActorIterator<AStage> It(World); It; ++It)
	{
		AStage* FoundStage = *It;
		if (FoundStage)
		{
			FoundStages.Add(FoundStage);
			UE_LOG(LogTemp, Log, TEXT("StageEditor: Found Stage: %s"), *FoundStage->GetActorLabel());
		}
	}
	UE_LOG(LogTemp, Log, TEXT("StageEditor: Total Stages Found: %d"), FoundStages.Num());

	if (FoundStages.Num() > 0)
	{
		// Default to the first one as active
		SetActiveStage(FoundStages[0].Get());
		DebugHeader::ShowNotifyInfo(TEXT("Found ") + FString::FromInt(FoundStages.Num()) + TEXT(" Stages in Level."));
	}
	else
	{
		OnModelChanged.Broadcast(); // Clear UI if no stages found
	}
}

FStageEditorController::~FStageEditorController()
{
	UnbindEditorDelegates();
}

void FStageEditorController::SetActiveStage(AStage* NewStage)
{
	ActiveStage = NewStage;
	OnModelChanged.Broadcast();
}

AStage* FStageEditorController::GetActiveStage() const
{
	return ActiveStage.Get();
}

bool FStageEditorController::CreateNewAct()
{
	AStage* Stage = GetActiveStage();
	if (!Stage) return false;

	const FScopedTransaction Transaction(LOCTEXT("CreateNewAct", "Create New Act"));
	Stage->Modify();

	// Mock ID Generation: Find max Act ID + 1
	int32 NewActID = 1;
	for (const FAct& Act : Stage->Acts)
	{
		if (Act.ActID.ActID >= NewActID)
		{
			NewActID = Act.ActID.ActID + 1;
		}
	}

	FAct NewAct;
	NewAct.ActID.StageID = Stage->StageID;
	NewAct.ActID.ActID = NewActID;
	NewAct.DisplayName = FString::Printf(TEXT("Act_%d"), NewActID);

	Stage->Acts.Add(NewAct);

	OnModelChanged.Broadcast();
	DebugHeader::ShowNotifyInfo(TEXT("Created new Act: ") + NewAct.DisplayName);
	return true;
}

bool FStageEditorController::RegisterProps(const TArray<AActor*>& ActorsToRegister, AStage* TargetStage)
{
	AStage* Stage = TargetStage ? TargetStage : GetActiveStage();
	if (!Stage) return false;
	if (ActorsToRegister.Num() == 0) return false;

	const FScopedTransaction Transaction(LOCTEXT("RegisterProps", "Register Props"));
	Stage->Modify();

	bool bAnyRegistered = false;
	for (AActor* Actor : ActorsToRegister)
	{
		if (!Actor) continue;

		// Check if Actor has UStagePropComponent
		// Check if Actor has UStagePropComponent
		UStagePropComponent* PropComp = Actor->FindComponentByClass<UStagePropComponent>();
		
		// Auto-add component if missing
		if (!PropComp)
		{
			Actor->Modify();
			PropComp = NewObject<UStagePropComponent>(Actor, UStagePropComponent::StaticClass(), TEXT("StagePropComponent"));
			if (PropComp)
			{
				Actor->AddInstanceComponent(PropComp);
				PropComp->RegisterComponent();
				Actor->RerunConstructionScripts();
			}
		}

		if (PropComp)
		{
			// Check if already registered (O(N) check for prototype)
			bool bAlreadyRegistered = false;
			for (const auto& Pair : Stage->PropRegistry)
			{
				if (Pair.Value.Get() == Actor)
				{
					bAlreadyRegistered = true;
					break;
				}
			}

			if (!bAlreadyRegistered)
			{
				Stage->RegisterProp(Actor);
				bAnyRegistered = true;
			}
		}
	}

	if (bAnyRegistered)
	{
		OnModelChanged.Broadcast();
		DebugHeader::ShowNotifyInfo(TEXT("Registered ") + FString::FromInt(ActorsToRegister.Num()) + TEXT(" Props"));
	}

	return bAnyRegistered;
}

void FStageEditorController::CreateStageBlueprint(const FString& DefaultPath)
{
	CreateBlueprintAsset(AStage::StaticClass(), TEXT("BP_NewStage"), DefaultPath);
}

void FStageEditorController::CreatePropBlueprint(const FString& DefaultPath)
{
	CreateBlueprintAsset(AProp::StaticClass(), TEXT("BP_NewProp"), DefaultPath);
}


void FStageEditorController::PreviewAct(int32 ActID)
{
	if (AStage* Stage = GetActiveStage())
	{
		// Just call the runtime ActivateAct, which now triggers editor updates via PropComponent
		Stage->ActivateAct(ActID);

		// Data Layer Logic: Activate target Act's Data Layer, deactivate others
		if (UDataLayerEditorSubsystem* DataLayerSubsystem = UDataLayerEditorSubsystem::Get())
		{
			UWorld* World = Stage->GetWorld();
			
			// Find the target Act's Data Layer Asset
			UDataLayerAsset* TargetDataLayerAsset = nullptr;
			for (const FAct& Act : Stage->Acts)
			{
				if (Act.ActID.ActID == ActID)
				{
					TargetDataLayerAsset = Act.AssociatedDataLayer;
					break;
				}
			}

			// Iterate all Data Layer Instances
			for (UDataLayerInstance* Instance : DataLayerSubsystem->GetDataLayerInstances(World))
			{
				if (!Instance) continue;

				// Check if this instance belongs to ANY Act in the Stage
				bool bIsStageDataLayer = false;
				for (const FAct& Act : Stage->Acts)
				{
					if (Act.AssociatedDataLayer == Instance->GetAsset())
					{
						bIsStageDataLayer = true;
						break;
					}
				}

				if (bIsStageDataLayer)
				{
					// If it's the target, activate it. Otherwise, deactivate it.
					bool bShouldBeActive = (Instance->GetAsset() == TargetDataLayerAsset);
					DataLayerSubsystem->SetDataLayerVisibility(Instance, bShouldBeActive);
				}
			}
		}
	}
}

//----------------------------------------------------------------
// Prop Management
//----------------------------------------------------------------

bool FStageEditorController::SetPropStateInAct(int32 PropID, int32 ActID, int32 NewState)
{
	AStage* Stage = GetActiveStage();
	if (!Stage) return false;

	// Find the Act
	FAct* TargetAct = Stage->Acts.FindByPredicate([ActID](const FAct& Act) {
		return Act.ActID.ActID == ActID;
	});

	if (!TargetAct)
	{
		DebugHeader::ShowNotifyInfo(TEXT("Error: Act not found"));
		return false;
	}

	// Check if Prop exists in the Act, if not, add it
	bool bIsNewProp = !TargetAct->PropStateOverrides.Contains(PropID);
	
	const FScopedTransaction Transaction(LOCTEXT("SetPropState", "Set Prop State"));
	Stage->Modify();

	// Update the state
	TargetAct->PropStateOverrides.Add(PropID, NewState);

	// Sync with Data Layer
	SyncPropToDataLayer(PropID, ActID);

	OnModelChanged.Broadcast();
	if (bIsNewProp)
	{
		DebugHeader::ShowNotifyInfo(FString::Printf(TEXT("Added Prop %d to Act with State %d"), PropID, NewState));
	}
	else
	{
		DebugHeader::ShowNotifyInfo(FString::Printf(TEXT("Set Prop %d state to %d"), PropID, NewState));
	}
	return true;
}

bool FStageEditorController::RemovePropFromAct(int32 PropID, int32 ActID)
{
	AStage* Stage = GetActiveStage();
	if (!Stage) return false;

	const FScopedTransaction Transaction(LOCTEXT("RemovePropFromAct", "Remove Prop from Act"));
	Stage->Modify();

	// Remove from Data Layer if applicable
	FAct* TargetAct = Stage->Acts.FindByPredicate([ActID](const FAct& Act) {
		return Act.ActID.ActID == ActID;
	});

	if (TargetAct && TargetAct->AssociatedDataLayer)
	{
		if (UStagePropComponent* PropComp = Stage->PropRegistry.FindRef(PropID))
		{
			if (AActor* PropActor = PropComp->GetOwner())
			{
				if (UDataLayerEditorSubsystem* DataLayerSubsystem = UDataLayerEditorSubsystem::Get())
				{
					// Find Instance
					UDataLayerInstance* DataLayerInstance = nullptr;
					for (UDataLayerInstance* Instance : DataLayerSubsystem->GetDataLayerInstances(Stage->GetWorld()))
					{
						if (Instance->GetAsset() == TargetAct->AssociatedDataLayer)
						{
							DataLayerInstance = Instance;
							break;
						}
					}

					if (DataLayerInstance)
					{
						TArray<AActor*> ActorsToRemove;
						ActorsToRemove.Add(PropActor);
						DataLayerSubsystem->RemoveActorsFromDataLayer(ActorsToRemove, DataLayerInstance);
					}
				}
			}
		}
	}

	Stage->RemovePropFromAct(PropID, ActID);

	OnModelChanged.Broadcast();
	DebugHeader::ShowNotifyInfo(FString::Printf(TEXT("Removed Prop %d from Act %d"), PropID, ActID));
	return true;
}

bool FStageEditorController::RemoveAllPropsFromAct(int32 ActID)
{
	AStage* Stage = GetActiveStage();
	if (!Stage) return false;

	// Find the Act
	FAct* TargetAct = Stage->Acts.FindByPredicate([ActID](const FAct& Act) {
		return Act.ActID.ActID == ActID;
	});

	if (!TargetAct)
	{
		DebugHeader::ShowNotifyInfo(TEXT("Error: Act not found"));
		return false;
	}

	int32 PropCount = TargetAct->PropStateOverrides.Num();
	if (PropCount == 0)
	{
		DebugHeader::ShowNotifyInfo(TEXT("Act has no Props to remove"));
		return false;
	}

	const FScopedTransaction Transaction(LOCTEXT("RemoveAllPropsFromAct", "Remove All Props from Act"));
	Stage->Modify();

	// Remove from Data Layer if applicable
	if (TargetAct->AssociatedDataLayer)
	{
		if (UDataLayerEditorSubsystem* DataLayerSubsystem = UDataLayerEditorSubsystem::Get())
		{
			// Find Instance
			UDataLayerInstance* DataLayerInstance = nullptr;
			for (UDataLayerInstance* Instance : DataLayerSubsystem->GetDataLayerInstances(Stage->GetWorld()))
			{
				if (Instance->GetAsset() == TargetAct->AssociatedDataLayer)
				{
					DataLayerInstance = Instance;
					break;
				}
			}

			if (DataLayerInstance)
			{
				TArray<AActor*> ActorsToRemove;
				for (const auto& Pair : TargetAct->PropStateOverrides)
				{
					int32 PropID = Pair.Key;
					if (UStagePropComponent* PropComp = Stage->PropRegistry.FindRef(PropID))
					{
						if (AActor* PropActor = PropComp->GetOwner())
						{
							ActorsToRemove.Add(PropActor);
						}
					}
				}
				
				if (ActorsToRemove.Num() > 0)
				{
					DataLayerSubsystem->RemoveActorsFromDataLayer(ActorsToRemove, DataLayerInstance);
				}
			}
		}
	}

	TargetAct->PropStateOverrides.Empty();

	OnModelChanged.Broadcast();
	DebugHeader::ShowNotifyInfo(FString::Printf(TEXT("Removed %d Props from Act '%s'"), PropCount, *TargetAct->DisplayName));
	return true;
}

bool FStageEditorController::UnregisterProp(int32 PropID)
{
	AStage* Stage = GetActiveStage();
	if (!Stage) return false;

	// Verify Prop exists
	if (!Stage->PropRegistry.Contains(PropID))
	{
		DebugHeader::ShowNotifyInfo(TEXT("Error: Prop not registered to Stage"));
		return false;
	}

	const FScopedTransaction Transaction(LOCTEXT("UnregisterProp", "Unregister Prop"));
	Stage->Modify();

	Stage->UnregisterProp(PropID);

	OnModelChanged.Broadcast();
	DebugHeader::ShowNotifyInfo(FString::Printf(TEXT("Unregistered Prop %d from Stage"), PropID));
	return true;
}

bool FStageEditorController::UnregisterAllProps()
{
	AStage* Stage = GetActiveStage();
	if (!Stage) return false;

	int32 PropCount = Stage->PropRegistry.Num();
	if (PropCount == 0)
	{
		DebugHeader::ShowNotifyInfo(TEXT("Stage has no registered Props"));
		return false;
	}

	const FScopedTransaction Transaction(LOCTEXT("UnregisterAllProps", "Unregister All Props"));
	Stage->Modify();

	// Clear PropRegistry
	Stage->PropRegistry.Empty();

	// Clear all Props from all Acts
	for (FAct& Act : Stage->Acts)
	{
		Act.PropStateOverrides.Empty();
	}

	OnModelChanged.Broadcast();
	DebugHeader::ShowNotifyInfo(FString::Printf(TEXT("Unregistered %d Props from Stage"), PropCount));
	return true;
}

//----------------------------------------------------------------
// Act Management
//----------------------------------------------------------------

bool FStageEditorController::DeleteAct(int32 ActID)
{
	AStage* Stage = GetActiveStage();
	if (!Stage) return false;

	// Prevent deleting Default Act (ID 0)
	if (ActID == 0)
	{
		DebugHeader::ShowNotifyInfo(TEXT("Cannot delete Default Act"));
		return false;
	}

	// Find the Act to get its name for notification
	FAct* TargetAct = Stage->Acts.FindByPredicate([ActID](const FAct& Act) {
		return Act.ActID.ActID == ActID;
	});

	if (!TargetAct)
	{
		DebugHeader::ShowNotifyInfo(TEXT("Error: Act not found"));
		return false;
	}

	FString ActName = TargetAct->DisplayName;

	const FScopedTransaction Transaction(LOCTEXT("DeleteAct", "Delete Act"));
	Stage->Modify();

	Stage->RemoveAct(ActID);

	OnModelChanged.Broadcast();
	DebugHeader::ShowNotifyInfo(FString::Printf(TEXT("Deleted Act '%s'"), *ActName));
	return true;
}


void FStageEditorController::CreateBlueprintAsset(UClass* ParentClass, const FString& BaseName, const FString& DefaultPath)
{
	// Debug: Show what path we're using
	FString DebugMsg = FString::Printf(TEXT("Creating BP with DefaultPath: %s"), *DefaultPath);
	DebugHeader::ShowNotifyInfo(DebugMsg);
	UE_LOG(LogTemp, Warning, TEXT("%s"), *DebugMsg);
	
	IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
	
	// Create the factory with proper configuration to skip Parent Class picker
	UBlueprintFactory* Factory = NewObject<UBlueprintFactory>();
	Factory->ParentClass = ParentClass;
	Factory->BlueprintType = BPTYPE_Normal;
	Factory->bSkipClassPicker = false;  // This skips the Parent Class selection dialog
	
	// Open the "Save Asset As" dialog
	UObject* NewAsset = AssetTools.CreateAssetWithDialog(BaseName, DefaultPath, UBlueprint::StaticClass(), Factory);
	
	if (NewAsset)
	{
		DebugHeader::ShowNotifyInfo(TEXT("Created new Blueprint: ") + NewAsset->GetName());
		// Open the editor for the new asset
		GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()->OpenEditorForAsset(NewAsset);
	}
}

void FStageEditorController::BindEditorDelegates()
{
	if (GEngine)
	{
		GEngine->OnLevelActorAdded().AddRaw(this, &FStageEditorController::OnLevelActorAdded);
		GEngine->OnLevelActorDeleted().AddRaw(this, &FStageEditorController::OnLevelActorDeleted);
	}
}

void FStageEditorController::UnbindEditorDelegates()
{
	if (GEngine)
	{
		GEngine->OnLevelActorAdded().RemoveAll(this);
		GEngine->OnLevelActorDeleted().RemoveAll(this);
	}
}

void FStageEditorController::OnLevelActorAdded(AActor* InActor)
{
	if (InActor && InActor->IsA<AStage>())
	{
		// Filter out temporary actors during drag-and-drop preview
		// Only refresh when actor is fully placed and committed
		if (InActor->HasAnyFlags(RF_Transient) || 
			InActor->IsActorBeingDestroyed() ||
			!InActor->GetWorld() ||
			InActor->GetWorld()->WorldType != EWorldType::Editor)
		{
			return;
		}
		
		// Additional check: ensure actor has a valid label (not default temporary name)
		FString ActorLabel = InActor->GetActorLabel();
		if (ActorLabel.IsEmpty() || ActorLabel.Contains(TEXT("PLACEHOLDER")))
		{
			return;
		}
		
		FindStageInWorld();
	}
}

void FStageEditorController::OnLevelActorDeleted(AActor* InActor)
{
	if (!InActor) return;

	// Check if it's a Stage actor
	if (InActor->IsA<AStage>())
	{
		// Filter out temporary actors (similar to OnLevelActorAdded)
		if (InActor->HasAnyFlags(RF_Transient) || 
			!InActor->GetWorld() ||
			InActor->GetWorld()->WorldType != EWorldType::Editor)
		{
			return;
		}
		
		// Delay slightly to ensure actor is fully removed from iterator
		// TActorIterator should skip pending kill actors automatically
		FindStageInWorld();
		return;
	}

	// Check if it's a Prop actor (has StagePropComponent)
	if (UStagePropComponent* PropComp = InActor->FindComponentByClass<UStagePropComponent>())
	{
		// Filter out temporary actors
		if (InActor->HasAnyFlags(RF_Transient) || 
			!InActor->GetWorld() ||
			InActor->GetWorld()->WorldType != EWorldType::Editor)
		{
			return;
		}

		// Auto-unregister this Prop from all Stages
		int32 PropID = PropComp->PropID;
		AStage* OwnerStage = PropComp->OwnerStage.Get();

		if (OwnerStage && PropID >= 0)
		{
			const FScopedTransaction Transaction(LOCTEXT("AutoUnregisterProp", "Auto Unregister Prop"));
			OwnerStage->Modify();
			OwnerStage->UnregisterProp(PropID);
			
			// Refresh UI if this is the active stage
			if (OwnerStage == GetActiveStage())
			{
				OnModelChanged.Broadcast();
			}
			
			UE_LOG(LogTemp, Log, TEXT("StageEditor: Auto-unregistered Prop '%s' (ID:%d) from Stage '%s' due to level deletion"), 
				*InActor->GetName(), PropID, *OwnerStage->GetName());
		}
	}
}

//----------------------------------------------------------------
// Data Layer Integration
//----------------------------------------------------------------

bool FStageEditorController::CreateDataLayerForAct(int32 ActID)
{
	AStage* Stage = GetActiveStage();
	if (!Stage) return false;

	FAct* TargetAct = nullptr;
	for (FAct& Act : Stage->Acts)
	{
		if (Act.ActID.ActID == ActID)
		{
			TargetAct = &Act;
			break;
		}
	}

	if (!TargetAct) return false;

	UDataLayerEditorSubsystem* DataLayerSubsystem = UDataLayerEditorSubsystem::Get();
	if (!DataLayerSubsystem) return false;

	const FScopedTransaction Transaction(LOCTEXT("CreateDataLayer", "Create Data Layer for Act"));
	Stage->Modify();

	FDataLayerCreationParameters Params;
	Params.DataLayerAsset = nullptr; // Create new asset
	Params.World = Stage->GetWorld();
	
	UDataLayerInstance* NewDataLayer = DataLayerSubsystem->CreateDataLayerInstance(Params);
	if (NewDataLayer)
	{
		TargetAct->AssociatedDataLayer = NewDataLayer->GetAsset();
		
		// Try to rename the instance to match Act
		FString DataLayerName = FString::Printf(TEXT("DL_%s_%s"), *Stage->GetActorLabel(), *TargetAct->DisplayName);
		NewDataLayer->SetActorLabel(DataLayerName);

		OnModelChanged.Broadcast();
		DebugHeader::ShowNotifyInfo(FString::Printf(TEXT("Created Data Layer '%s' for Act"), *DataLayerName));
		return true;
	}

	return false;
}

bool FStageEditorController::AssignDataLayerToAct(int32 ActID, UDataLayerAsset* DataLayer)
{
	AStage* Stage = GetActiveStage();
	if (!Stage) return false;

	FAct* TargetAct = nullptr;
	for (FAct& Act : Stage->Acts)
	{
		if (Act.ActID.ActID == ActID)
		{
			TargetAct = &Act;
			break;
		}
	}

	if (!TargetAct) return false;

	const FScopedTransaction Transaction(LOCTEXT("AssignDataLayer", "Assign Data Layer to Act"));
	Stage->Modify();

	TargetAct->AssociatedDataLayer = DataLayer;
	OnModelChanged.Broadcast();
	
	return true;
}

bool FStageEditorController::SyncPropToDataLayer(int32 PropID, int32 ActID)
{
	AStage* Stage = GetActiveStage();
	if (!Stage) return false;

	// Find Act
	FAct* TargetAct = nullptr;
	for (FAct& Act : Stage->Acts)
	{
		if (Act.ActID.ActID == ActID)
		{
			TargetAct = &Act;
			break;
		}
	}

	if (!TargetAct || !TargetAct->AssociatedDataLayer) return false;

	// Find Prop Actor
	AActor* PropActor = nullptr;
	if (Stage->PropRegistry.Contains(PropID))
	{
		if (UStagePropComponent* PropComp = Stage->PropRegistry[PropID])
		{
			PropActor = PropComp->GetOwner();
		}
	}

	if (!PropActor) return false;

	UDataLayerEditorSubsystem* DataLayerSubsystem = UDataLayerEditorSubsystem::Get();
	if (!DataLayerSubsystem) return false;

	// Add actor to data layer
	TArray<AActor*> ActorsToAdd;
	ActorsToAdd.Add(PropActor);
	
	// We need the UDataLayerInstance, not just the Asset
	UDataLayerInstance* DataLayerInstance = nullptr;
	for (UDataLayerInstance* Instance : DataLayerSubsystem->GetDataLayerInstances(Stage->GetWorld()))
	{
		if (Instance->GetAsset() == TargetAct->AssociatedDataLayer)
		{
			DataLayerInstance = Instance;
			break;
		}
	}

	if (DataLayerInstance)
	{
		return DataLayerSubsystem->AddActorsToDataLayer(ActorsToAdd, DataLayerInstance);
	}

	return false;
}

	return false;
}

//----------------------------------------------------------------
// World Partition Support
//----------------------------------------------------------------

bool FStageEditorController::IsWorldPartitionActive() const
{
	if (UWorld* World = GEditor->GetEditorWorldContext().World())
	{
		return World->GetWorldPartition() != nullptr;
	}
	return false;
}

void FStageEditorController::ConvertToWorldPartition()
{
	if (UWorld* World = GEditor->GetEditorWorldContext().World())
	{
		FWorldPartitionConverter::ConvertWorld(World);
		// After conversion, we might need to refresh
		Initialize();
	}
}

#undef LOCTEXT_NAMESPACE
