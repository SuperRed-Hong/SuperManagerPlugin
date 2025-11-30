#include "EditorLogic/StageEditorController.h"
#include "Actors/Stage.h"
#include "Actors/Prop.h"
#include "DebugHeader.h"
#include "ScopedTransaction.h"
#include "AssetToolsModule.h"
#include "Factories/BlueprintFactory.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "Subsystems/AssetEditorSubsystem.h"
#include "Subsystems/StageManagerSubsystem.h"
#include "Editor.h"
#include "EngineUtils.h"
#include "Components/StagePropComponent.h"
#include "WorldPartition/DataLayer/DataLayerAsset.h"
#include "DataLayer/DataLayerEditorSubsystem.h"
#include "WorldPartition/DataLayer/DataLayerInstance.h"
#include "WorldPartition/DataLayer/DataLayerManager.h"
#include "WorldPartition/WorldPartition.h"
#include "WorldPartition/DataLayer/WorldDataLayers.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "UObject/SavePackage.h"
#include "WorldPartitionEditorModule.h"
#include "Kismet2/SClassPickerDialog.h"
#include "ClassViewerFilter.h"
#include "ClassViewerModule.h"
#include "DataLayerSync/StageDataLayerNameParser.h"
#include "DataLayerSync/DataLayerSyncStatusCache.h"
#include "ObjectTools.h"

#define LOCTEXT_NAMESPACE "FStageEditorController"

// Static member initialization
int32 FStageEditorController::ImportBypassCounter = 0;

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

int32 FStageEditorController::CreateNewAct()
{
	AStage* Stage = GetActiveStage();
	if (!Stage) return -1;

	const FScopedTransaction Transaction(LOCTEXT("CreateNewAct", "Create New Act"));
	Stage->Modify();

	// Mock ID Generation: Find max Act ID + 1
	int32 NewActID = 1;
	for (const FAct& Act : Stage->Acts)
	{
		if (Act.SUID.ActID >= NewActID)
		{
			NewActID = Act.SUID.ActID + 1;
		}
	}

	FAct NewAct;
	NewAct.SUID.StageID = Stage->GetStageID();
	NewAct.SUID.ActID = NewActID;
	NewAct.DisplayName = FString::Printf(TEXT("Act_%d"), NewActID);

	Stage->Acts.Add(NewAct);

	// Auto-create DataLayer for new Act if World Partition is active
	if (IsWorldPartitionActive())
	{
		CreateDataLayerForAct(NewActID);
	}

	// Invalidate sync status cache for Stage's DataLayer
	if (Stage->StageDataLayerAsset)
	{
		FDataLayerSyncStatusCache::Get().InvalidateCache(Stage->StageDataLayerAsset);
	}

	OnModelChanged.Broadcast();
	DebugHeader::ShowNotifyInfo(TEXT("Created new Act: ") + NewAct.DisplayName);
	return NewActID;
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

		// === Prevent registering Stage actors as Props (nested Stage not allowed) ===
		if (Actor->IsA<AStage>())
		{
			UE_LOG(LogTemp, Error,
				TEXT("Cannot register Stage actor '%s' as a Prop! Stage actors cannot be nested."),
				*Actor->GetActorLabel());

			DebugHeader::ShowMsgDialog(
				EAppMsgType::Ok,
				FString::Printf(TEXT("Cannot register Stage as a Prop!\n\n"
					"Stage: %s\n\n"
					"Stage actors cannot be registered as Props.\n"
					"This is a dangerous nested operation and is not allowed."),
					*Actor->GetActorLabel()),
				true);
			continue;
		}

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
				int32 NewPropID = Stage->RegisterProp(Actor);
				bAnyRegistered = true;

				// NOTE: Props are now registered to Stage only, NOT automatically added to any Act.
				// User can manually add Props to Acts via:
				// 1. Right-click context menu in RegisteredProps folder
				// 2. Drag & drop Props onto Acts
				// This gives users full control over which Props belong to which Acts.
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

/** Class filter that only allows AStage and its subclasses */
class FStageClassFilter : public IClassViewerFilter
{
public:
	virtual bool IsClassAllowed(const FClassViewerInitializationOptions& InInitOptions, const UClass* InClass, TSharedRef<FClassViewerFilterFuncs> InFilterFuncs) override
	{
		return InClass && InClass->IsChildOf(AStage::StaticClass());
	}

	virtual bool IsUnloadedClassAllowed(const FClassViewerInitializationOptions& InInitOptions, const TSharedRef<const IUnloadedBlueprintData> InUnloadedClassData, TSharedRef<FClassViewerFilterFuncs> InFilterFuncs) override
	{
		return InUnloadedClassData->IsChildOf(AStage::StaticClass());
	}
};

void FStageEditorController::CreateStageBlueprint(const FString& DefaultPath)
{
	// Configure class picker to only show Stage and its subclasses
	FClassViewerInitializationOptions Options;
	Options.Mode = EClassViewerMode::ClassPicker;
	Options.DisplayMode = EClassViewerDisplayMode::TreeView;
	Options.bShowUnloadedBlueprints = true;
	Options.bShowNoneOption = false;
	Options.ClassFilters.Add(MakeShared<FStageClassFilter>());

	// Show the class picker dialog
	UClass* SelectedClass = nullptr;
	const bool bPressedOk = SClassPickerDialog::PickClass(
		LOCTEXT("PickStageParentClass", "Pick Parent Class for Stage Blueprint"),
		Options,
		SelectedClass,
		AStage::StaticClass()
	);

	if (bPressedOk && SelectedClass)
	{
		CreateBlueprintAsset(SelectedClass, TEXT("BP_NewStage"), DefaultPath);
	}
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
				if (Act.SUID.ActID == ActID)
				{
					TargetDataLayerAsset = Act.AssociatedDataLayer;
					break;
				}
			}

			// Iterate all Data Layer Instances
			for (UDataLayerInstance* Instance : DataLayerSubsystem->GetAllDataLayers())
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
		return Act.SUID.ActID == ActID;
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
		return Act.SUID.ActID == ActID;
	});

	if (TargetAct && TargetAct->AssociatedDataLayer)
	{
		if (const TSoftObjectPtr<AActor>* PropActorPtr = Stage->PropRegistry.Find(PropID))
		{
			if (AActor* PropActor = PropActorPtr->Get())
			{
				if (UDataLayerEditorSubsystem* DataLayerSubsystem = UDataLayerEditorSubsystem::Get())
				{
					// Find Instance
					UDataLayerInstance* DataLayerInstance = nullptr;
					for (UDataLayerInstance* Instance : DataLayerSubsystem->GetAllDataLayers())
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
		return Act.SUID.ActID == ActID;
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
			for (UDataLayerInstance* Instance : DataLayerSubsystem->GetAllDataLayers())
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
					if (const TSoftObjectPtr<AActor>* PropActorPtr = Stage->PropRegistry.Find(PropID))
					{
						if (AActor* PropActor = PropActorPtr->Get())
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

	// Remove from DataLayers before unregistering
	if (IsWorldPartitionActive())
	{
		// Remove from Stage DataLayer
		RemovePropFromStageDataLayer(PropID);

		// Remove from all Act DataLayers
		for (const FAct& Act : Stage->Acts)
		{
			if (Act.PropStateOverrides.Contains(PropID) && Act.AssociatedDataLayer)
			{
				RemovePropFromActDataLayer(PropID, Act.SUID.ActID);
			}
		}
	}

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

	// Remove all Props from DataLayers before clearing
	if (IsWorldPartitionActive())
	{
		for (const auto& Pair : Stage->PropRegistry)
		{
			int32 PropID = Pair.Key;
			// Remove from Stage DataLayer
			RemovePropFromStageDataLayer(PropID);

			// Remove from all Act DataLayers
			for (const FAct& Act : Stage->Acts)
			{
				if (Act.PropStateOverrides.Contains(PropID) && Act.AssociatedDataLayer)
				{
					RemovePropFromActDataLayer(PropID, Act.SUID.ActID);
				}
			}
		}
	}

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
		return Act.SUID.ActID == ActID;
	});

	if (!TargetAct)
	{
		DebugHeader::ShowNotifyInfo(TEXT("Error: Act not found"));
		return false;
	}

	FString ActName = TargetAct->DisplayName;

	const FScopedTransaction Transaction(LOCTEXT("DeleteAct", "Delete Act"));
	Stage->Modify();

	// Delete Act's DataLayer before removing the Act
	if (IsWorldPartitionActive() && TargetAct->AssociatedDataLayer)
	{
		DeleteDataLayerForAct(ActID);
	}

	Stage->RemoveAct(ActID);

	// Invalidate sync status cache for Stage's DataLayer
	if (Stage->StageDataLayerAsset)
	{
		FDataLayerSyncStatusCache::Get().InvalidateCache(Stage->StageDataLayerAsset);
	}

	OnModelChanged.Broadcast();
	DebugHeader::ShowNotifyInfo(FString::Printf(TEXT("Deleted Act '%s'"), *ActName));
	return true;
}


void FStageEditorController::CreateBlueprintAsset(UClass* ParentClass, const FString& BaseName, const FString& DefaultPath)
{
	IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();

	// Create the factory with proper configuration
	UBlueprintFactory* Factory = NewObject<UBlueprintFactory>();
	Factory->ParentClass = ParentClass;
	Factory->BlueprintType = BPTYPE_Normal;
	Factory->bSkipClassPicker = true;  // Skip the Parent Class selection dialog (class already selected)

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

	// Subscribe to DataLayer changes (for external rename detection)
	if (UDataLayerEditorSubsystem* DataLayerSubsystem = UDataLayerEditorSubsystem::Get())
	{
		DataLayerChangedHandle = DataLayerSubsystem->OnDataLayerChanged().AddRaw(
			this, &FStageEditorController::OnExternalDataLayerChanged);
	}
}

void FStageEditorController::UnbindEditorDelegates()
{
	if (GEngine)
	{
		GEngine->OnLevelActorAdded().RemoveAll(this);
		GEngine->OnLevelActorDeleted().RemoveAll(this);
	}

	// Unsubscribe from DataLayer changes
	if (DataLayerChangedHandle.IsValid())
	{
		if (UDataLayerEditorSubsystem* DataLayerSubsystem = UDataLayerEditorSubsystem::Get())
		{
			DataLayerSubsystem->OnDataLayerChanged().Remove(DataLayerChangedHandle);
		}
		DataLayerChangedHandle.Reset();
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

		// CRITICAL: Skip if we're in the middle of Blueprint reconstruction
		// When a BP is recompiled, OnLevelActorAdded is called for the new instance,
		// but we should NOT treat it as a new Stage (it inherits data from old instance)
		if (GIsReconstructingBlueprintInstances)
		{
			UE_LOG(LogTemp, Log, TEXT("StageEditorController: Skipping OnLevelActorAdded during BP reconstruction for '%s'"),
				*InActor->GetActorLabel());
			return;
		}

		// Additional check: ensure actor has a valid label (not default temporary name)
		FString ActorLabel = InActor->GetActorLabel();
		if (ActorLabel.IsEmpty() || ActorLabel.Contains(TEXT("PLACEHOLDER")))
		{
			return;
		}

		AStage* NewStage = Cast<AStage>(InActor);
		if (NewStage)
		{
			// === Singleton Check (Per Blueprint Class) ===
			// Ensures only one Stage of the SAME Blueprint class exists per level.
			// Different Stage Blueprint classes can coexist (e.g., BP_Stage_Forest + BP_Stage_Castle).
			// Safe to run here because GIsReconstructingBlueprintInstances check above
			// prevents this from triggering during BP recompilation.
			UWorld* World = NewStage->GetWorld();
			UClass* NewStageClass = NewStage->GetClass();
			AStage* ExistingStage = nullptr;

			for (TActorIterator<AStage> It(World); It; ++It)
			{
				AStage* Other = *It;
				if (Other != NewStage && IsValid(Other) && !Other->IsPendingKillPending())
				{
					// Only conflict if same Blueprint class
					if (Other->GetClass() == NewStageClass)
					{
						ExistingStage = Other;
						break;
					}
				}
			}

			if (ExistingStage)
			{
				// Log the error
				UE_LOG(LogTemp, Error,
					TEXT("Stage Singleton Violation: Attempted to place a second instance of '%s' in level. "
					     "Existing instance: '%s'. Only one instance per Stage Blueprint class is allowed."),
					*NewStageClass->GetName(), *ExistingStage->GetActorLabel());

				// Show modal dialog to user
				DebugHeader::ShowMsgDialog(
					EAppMsgType::Ok,
					FString::Printf(TEXT("Only one instance of this Stage Blueprint is allowed per level!\n\n"
						"Blueprint: %s\n"
						"Existing instance: %s\n\n"
						"This Stage will be destroyed."),
						*NewStageClass->GetName(),
						*ExistingStage->GetActorLabel()),
					true);

				// Destroy the duplicate Stage
				World->DestroyActor(NewStage);
				return;
			}

			// Register Stage with Subsystem to get unique ID
			if (UStageManagerSubsystem* Subsystem = GetSubsystem())
			{
				// Check if this specific Stage instance is already registered in the subsystem
				bool bAlreadyRegistered = false;
				if (NewStage->GetStageID() > 0)
				{
					AStage* RegisteredStage = Subsystem->GetStage(NewStage->GetStageID());
					bAlreadyRegistered = (RegisteredStage == NewStage);
				}

				if (!bAlreadyRegistered)
				{
					// This is a new instance - always allocate a fresh unique ID
					// Reset StageID to 0 to ensure Subsystem allocates a new ID
					// (BP assets may have saved non-zero IDs that would cause duplicates)
					NewStage->Modify();
					NewStage->SUID.StageID = 0;
					int32 NewID = Subsystem->RegisterStage(NewStage);
					NewStage->SUID.StageID = NewID;
					UE_LOG(LogTemp, Log, TEXT("StageEditorController: Registered Stage '%s' with new ID %d"),
						*NewStage->GetActorLabel(), NewStage->GetStageID());
				}
			}

			// Auto-create DataLayers if World Partition is active
			// IMPORTANT: Skip if in import bypass mode (DataLayerImporter will assign existing DataLayers)
			if (IsWorldPartitionActive() && !IsImportBypassActive())
			{
				// Create Stage's root DataLayer
				if (!NewStage->StageDataLayerAsset && NewStage->GetStageID() > 0)
				{
					CreateDataLayerForStage(NewStage);
					UE_LOG(LogTemp, Log, TEXT("StageEditorController: Created DataLayer for Stage '%s'"),
						*NewStage->GetActorLabel());
				}

				// Create DataLayer for Default Act (ActID == 1, as defined in Stage constructor)
				for (FAct& Act : NewStage->Acts)
				{
					if (Act.SUID.ActID == 1 && !Act.AssociatedDataLayer)
					{
						// Temporarily set this as active stage to use CreateDataLayerForAct
						TWeakObjectPtr<AStage> PrevActiveStage = ActiveStage;
						ActiveStage = NewStage;
						CreateDataLayerForAct(1);
						ActiveStage = PrevActiveStage;
						UE_LOG(LogTemp, Log, TEXT("StageEditorController: Created DataLayer for Default Act (ID=1)"));
						break;
					}
				}
			}
			else if (IsImportBypassActive())
			{
				UE_LOG(LogTemp, Log, TEXT("StageEditorController: Skipping auto DataLayer creation (import bypass active) for Stage '%s'"),
					*NewStage->GetActorLabel());
			}
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

		// CRITICAL: Skip if we're in the middle of Blueprint reconstruction
		// When a BP is recompiled, OnLevelActorDeleted is called for the old instance,
		// but we should NOT delete DataLayers (they will be inherited by new instance)
		if (GIsReconstructingBlueprintInstances)
		{
			UE_LOG(LogTemp, Log, TEXT("StageEditorController: Skipping OnLevelActorDeleted during BP reconstruction for '%s'"),
				*InActor->GetActorLabel());
			return;
		}

		// Delete all associated DataLayers before the Stage is removed
		AStage* DeletedStage = Cast<AStage>(InActor);
		if (DeletedStage && DeletedStage->GetStageID() > 0 && IsWorldPartitionActive())
		{
			UDataLayerEditorSubsystem* DataLayerSubsystem = UDataLayerEditorSubsystem::Get();
			if (DataLayerSubsystem)
			{
				const TArray<UDataLayerInstance*> AllInstances = DataLayerSubsystem->GetAllDataLayers();

				// First, delete all Act DataLayers
				for (const FAct& Act : DeletedStage->Acts)
				{
					if (Act.AssociatedDataLayer)
					{
						for (UDataLayerInstance* Instance : AllInstances)
						{
							if (Instance && Instance->GetAsset() == Act.AssociatedDataLayer)
							{
								DataLayerSubsystem->DeleteDataLayer(Instance);
								UE_LOG(LogTemp, Log, TEXT("StageEditorController: Deleted Act DataLayer '%s' on Stage deletion"),
									*Act.AssociatedDataLayer->GetName());
								break;
							}
						}
					}
				}

				// Then, delete the Stage's root DataLayer
				if (DeletedStage->StageDataLayerAsset)
				{
					// Re-fetch instances as the list may have changed
					const TArray<UDataLayerInstance*> UpdatedInstances = DataLayerSubsystem->GetAllDataLayers();
					for (UDataLayerInstance* Instance : UpdatedInstances)
					{
						if (Instance && Instance->GetAsset() == DeletedStage->StageDataLayerAsset)
						{
							DataLayerSubsystem->DeleteDataLayer(Instance);
							UE_LOG(LogTemp, Log, TEXT("StageEditorController: Deleted Stage DataLayer '%s' on Stage deletion"),
								*DeletedStage->StageDataLayerAsset->GetName());
							break;
						}
					}
				}
			}
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
		int32 PropID = PropComp->GetPropID();
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
		if (Act.SUID.ActID == ActID)
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

	// Create a new DataLayer asset using our helper function
	FString DataLayerName = FString::Printf(TEXT("DL_Act_%d_%d"), Stage->GetStageID(), ActID);
	UDataLayerAsset* NewDataLayerAsset = CreateDataLayerAsset(DataLayerName, DataLayerAssetFolderPath);

	if (NewDataLayerAsset)
	{
		// Create instance for the asset
		UDataLayerInstance* NewDataLayer = GetOrCreateInstanceForAsset(NewDataLayerAsset);
		if (NewDataLayer)
		{
			TargetAct->AssociatedDataLayer = NewDataLayerAsset;

			// Set Act DataLayer as child of Stage DataLayer
			UDataLayerInstance* StageDataLayerInstance = FindStageDataLayerInstance(Stage);
			if (StageDataLayerInstance)
			{
				DataLayerSubsystem->SetParentDataLayer(NewDataLayer, StageDataLayerInstance);
				UE_LOG(LogTemp, Log, TEXT("Set Act DataLayer '%s' as child of Stage DataLayer '%s'"),
					*DataLayerName, *Stage->StageDataLayerAsset->GetName());
			}

			// Invalidate sync status cache for both Stage and new Act DataLayers
			if (Stage->StageDataLayerAsset)
			{
				FDataLayerSyncStatusCache::Get().InvalidateCache(Stage->StageDataLayerAsset);
			}
			FDataLayerSyncStatusCache::Get().InvalidateCache(NewDataLayerAsset);

			OnModelChanged.Broadcast();
			DebugHeader::ShowNotifyInfo(FString::Printf(TEXT("Created Data Layer '%s' for Act"), *DataLayerName));
			return true;
		}
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
		if (Act.SUID.ActID == ActID)
		{
			TargetAct = &Act;
			break;
		}
	}

	if (!TargetAct) return false;

	const FScopedTransaction Transaction(LOCTEXT("AssignDataLayer", "Assign Data Layer to Act"));
	Stage->Modify();

	TargetAct->AssociatedDataLayer = DataLayer;

	// Invalidate sync status cache for both Stage and Act DataLayers
	if (Stage->StageDataLayerAsset)
	{
		FDataLayerSyncStatusCache::Get().InvalidateCache(Stage->StageDataLayerAsset);
	}
	if (DataLayer)
	{
		FDataLayerSyncStatusCache::Get().InvalidateCache(DataLayer);
	}

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
		if (Act.SUID.ActID == ActID)
		{
			TargetAct = &Act;
			break;
		}
	}

	if (!TargetAct || !TargetAct->AssociatedDataLayer) return false;

	// Find Prop Actor
	AActor* PropActor = nullptr;
	if (const TSoftObjectPtr<AActor>* PropActorPtr = Stage->PropRegistry.Find(PropID))
	{
		PropActor = PropActorPtr->Get();
	}

	if (!PropActor) return false;

	UDataLayerEditorSubsystem* DataLayerSubsystem = UDataLayerEditorSubsystem::Get();
	if (!DataLayerSubsystem) return false;

	// Add actor to data layer
	TArray<AActor*> ActorsToAdd;
	ActorsToAdd.Add(PropActor);
	
	// We need the UDataLayerInstance, not just the Asset
	UDataLayerInstance* DataLayerInstance = nullptr;
	for (UDataLayerInstance* Instance : DataLayerSubsystem->GetAllDataLayers())
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

//----------------------------------------------------------------
// Helper Functions
//----------------------------------------------------------------

UStageManagerSubsystem* FStageEditorController::GetSubsystem() const
{
	if (!GEditor) return nullptr;

	UWorld* World = GEditor->GetEditorWorldContext().World();
	return World ? World->GetSubsystem<UStageManagerSubsystem>() : nullptr;
}

//----------------------------------------------------------------
// DataLayer Asset Management
//----------------------------------------------------------------

UDataLayerAsset* FStageEditorController::CreateDataLayerAsset(const FString& AssetName, const FString& FolderPath)
{
	FString PackagePath = FolderPath / AssetName;
	FString PackageName = FPackageName::ObjectPathToPackageName(PackagePath);

	// Check if asset already exists
	if (UDataLayerAsset* ExistingAsset = LoadObject<UDataLayerAsset>(nullptr, *PackagePath))
	{
		return ExistingAsset;
	}

	// Create the package
	UPackage* Package = CreatePackage(*PackageName);
	if (!Package) return nullptr;

	// Create the DataLayerAsset
	UDataLayerAsset* NewAsset = NewObject<UDataLayerAsset>(Package, *AssetName, RF_Public | RF_Standalone);
	if (!NewAsset) return nullptr;

#if WITH_EDITOR
	NewAsset->OnCreated();
#endif

	Package->MarkPackageDirty();
	FAssetRegistryModule::AssetCreated(NewAsset);

	// Save the package
	FString FilePath = FPackageName::LongPackageNameToFilename(PackageName, FPackageName::GetAssetPackageExtension());
	FSavePackageArgs SaveArgs;
	SaveArgs.TopLevelFlags = RF_Public | RF_Standalone;

	if (UPackage::SavePackage(Package, NewAsset, *FilePath, SaveArgs))
	{
		UE_LOG(LogTemp, Log, TEXT("Created DataLayerAsset: %s"), *PackagePath);
		return NewAsset;
	}

	return nullptr;
}

UDataLayerInstance* FStageEditorController::GetOrCreateInstanceForAsset(UDataLayerAsset* Asset)
{
	if (!Asset) return nullptr;

	UDataLayerEditorSubsystem* DataLayerSubsystem = UDataLayerEditorSubsystem::Get();
	if (!DataLayerSubsystem) return nullptr;

	// Check if instance already exists
	const TArray<UDataLayerInstance*> AllInstances = DataLayerSubsystem->GetAllDataLayers();
	for (UDataLayerInstance* Instance : AllInstances)
	{
		if (Instance && Instance->GetAsset() == Asset)
		{
			return Instance;
		}
	}

	// Create new instance
	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World) return nullptr;

	// Check if level supports External Objects (required for DataLayer creation)
	ULevel* Level = World->PersistentLevel;
	if (!Level || !Level->IsUsingExternalObjects())
	{
		UE_LOG(LogTemp, Error, TEXT("Cannot create DataLayerInstance: Level does not support External Objects."));
		UE_LOG(LogTemp, Error, TEXT("   Please enable 'Use External Actors' in World Settings -> World Partition."));
		DebugHeader::ShowNotifyInfo(TEXT("Error: Level must have 'Use External Actors' enabled for DataLayer creation."));
		return nullptr;
	}

	AWorldDataLayers* WorldDataLayers = World->GetWorldDataLayers();
	if (!WorldDataLayers) return nullptr;

	FDataLayerCreationParameters Params;
	Params.DataLayerAsset = Asset;
	Params.WorldDataLayers = WorldDataLayers;

	UDataLayerInstance* NewInstance = DataLayerSubsystem->CreateDataLayerInstance(Params);
	if (NewInstance)
	{
		UE_LOG(LogTemp, Log, TEXT("Created DataLayerInstance for Asset: %s"), *Asset->GetName());
	}
	return NewInstance;
}

UDataLayerInstance* FStageEditorController::FindStageDataLayerInstance(AStage* Stage)
{
	if (!Stage || !Stage->StageDataLayerAsset) return nullptr;

	UDataLayerEditorSubsystem* DataLayerSubsystem = UDataLayerEditorSubsystem::Get();
	if (!DataLayerSubsystem) return nullptr;

	const TArray<UDataLayerInstance*> AllInstances = DataLayerSubsystem->GetAllDataLayers();
	for (UDataLayerInstance* Instance : AllInstances)
	{
		if (Instance && Instance->GetAsset() == Stage->StageDataLayerAsset)
		{
			return Instance;
		}
	}
	return nullptr;
}

bool FStageEditorController::CreateDataLayerForStage(AStage* Stage)
{
	if (!Stage || Stage->GetStageID() <= 0) return false;
	if (Stage->StageDataLayerAsset) return true; // Already has one

	FString AssetName = FString::Printf(TEXT("DL_Stage_%d"), Stage->GetStageID());
	UDataLayerAsset* StageDataLayerAsset = CreateDataLayerAsset(AssetName, DataLayerAssetFolderPath);
	if (!StageDataLayerAsset) return false;

	UDataLayerInstance* StageDataLayerInstance = GetOrCreateInstanceForAsset(StageDataLayerAsset);
	if (!StageDataLayerInstance) return false;

	const FScopedTransaction Transaction(LOCTEXT("CreateStageDataLayer", "Create Stage DataLayer"));
	Stage->Modify();
	Stage->StageDataLayerAsset = StageDataLayerAsset;
	Stage->StageDataLayerName = AssetName;

	UE_LOG(LogTemp, Log, TEXT("Created Stage DataLayer: %s"), *AssetName);
	return true;
}

bool FStageEditorController::DeleteDataLayerForAct(int32 ActID)
{
	AStage* Stage = GetActiveStage();
	if (!Stage) return false;

	FAct* TargetAct = nullptr;
	for (FAct& Act : Stage->Acts)
	{
		if (Act.SUID.ActID == ActID)
		{
			TargetAct = &Act;
			break;
		}
	}

	if (!TargetAct || !TargetAct->AssociatedDataLayer) return false;

	UDataLayerEditorSubsystem* DataLayerSubsystem = UDataLayerEditorSubsystem::Get();
	if (!DataLayerSubsystem) return false;

	// Find and delete the DataLayer Instance
	const TArray<UDataLayerInstance*> AllInstances = DataLayerSubsystem->GetAllDataLayers();
	for (UDataLayerInstance* Instance : AllInstances)
	{
		if (Instance && Instance->GetAsset() == TargetAct->AssociatedDataLayer)
		{
			DataLayerSubsystem->DeleteDataLayer(Instance);
			break;
		}
	}

	const FScopedTransaction Transaction(LOCTEXT("DeleteDataLayerForAct", "Delete DataLayer for Act"));
	Stage->Modify();
	TargetAct->AssociatedDataLayer = nullptr;

	return true;
}

bool FStageEditorController::AssignPropToStageDataLayer(int32 PropID)
{
	AStage* Stage = GetActiveStage();
	if (!Stage || !Stage->StageDataLayerAsset) return false;

	const TSoftObjectPtr<AActor>* PropActorPtr = Stage->PropRegistry.Find(PropID);
	if (!PropActorPtr || !PropActorPtr->Get()) return false;

	AActor* PropActor = PropActorPtr->Get();

	UDataLayerEditorSubsystem* DataLayerSubsystem = UDataLayerEditorSubsystem::Get();
	if (!DataLayerSubsystem) return false;

	UDataLayerInstance* StageDataLayerInstance = FindStageDataLayerInstance(Stage);
	if (!StageDataLayerInstance) return false;

	TArray<AActor*> ActorsToAdd;
	ActorsToAdd.Add(PropActor);
	return DataLayerSubsystem->AddActorsToDataLayer(ActorsToAdd, StageDataLayerInstance);
}

bool FStageEditorController::RemovePropFromStageDataLayer(int32 PropID)
{
	AStage* Stage = GetActiveStage();
	if (!Stage || !Stage->StageDataLayerAsset) return false;

	const TSoftObjectPtr<AActor>* PropActorPtr = Stage->PropRegistry.Find(PropID);
	if (!PropActorPtr || !PropActorPtr->Get()) return false;

	AActor* PropActor = PropActorPtr->Get();

	UDataLayerEditorSubsystem* DataLayerSubsystem = UDataLayerEditorSubsystem::Get();
	if (!DataLayerSubsystem) return false;

	UDataLayerInstance* StageDataLayerInstance = FindStageDataLayerInstance(Stage);
	if (!StageDataLayerInstance) return false;

	TArray<AActor*> ActorsToRemove;
	ActorsToRemove.Add(PropActor);
	return DataLayerSubsystem->RemoveActorsFromDataLayer(ActorsToRemove, StageDataLayerInstance);
}

bool FStageEditorController::AssignPropToActDataLayer(int32 PropID, int32 ActID)
{
	return SyncPropToDataLayer(PropID, ActID);
}

bool FStageEditorController::RemovePropFromActDataLayer(int32 PropID, int32 ActID)
{
	AStage* Stage = GetActiveStage();
	if (!Stage) return false;

	FAct* TargetAct = nullptr;
	for (FAct& Act : Stage->Acts)
	{
		if (Act.SUID.ActID == ActID)
		{
			TargetAct = &Act;
			break;
		}
	}

	if (!TargetAct || !TargetAct->AssociatedDataLayer) return false;

	const TSoftObjectPtr<AActor>* PropActorPtr = Stage->PropRegistry.Find(PropID);
	if (!PropActorPtr || !PropActorPtr->Get()) return false;

	AActor* PropActor = PropActorPtr->Get();

	UDataLayerEditorSubsystem* DataLayerSubsystem = UDataLayerEditorSubsystem::Get();
	if (!DataLayerSubsystem) return false;

	UDataLayerInstance* DataLayerInstance = nullptr;
	const TArray<UDataLayerInstance*> AllInstances = DataLayerSubsystem->GetAllDataLayers();
	for (UDataLayerInstance* Instance : AllInstances)
	{
		if (Instance && Instance->GetAsset() == TargetAct->AssociatedDataLayer)
		{
			DataLayerInstance = Instance;
			break;
		}
	}

	if (!DataLayerInstance) return false;

	TArray<AActor*> ActorsToRemove;
	ActorsToRemove.Add(PropActor);
	return DataLayerSubsystem->RemoveActorsFromDataLayer(ActorsToRemove, DataLayerInstance);
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
	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World)
	{
		DebugHeader::ShowNotifyInfo(TEXT("Error: No active world found"));
		return;
	}

	// Check if already World Partition
	if (World->GetWorldPartition())
	{
		DebugHeader::ShowNotifyInfo(TEXT("Level is already a World Partition level"));
		return;
	}

	// Use UE native World Partition conversion
	FWorldPartitionEditorModule& WorldPartitionEditorModule =
		FModuleManager::LoadModuleChecked<FWorldPartitionEditorModule>("WorldPartitionEditor");

	FString LongPackageName = World->GetPackage()->GetName();

	// Call native conversion function (opens dialog)
	WorldPartitionEditorModule.ConvertMap(LongPackageName);
}

//----------------------------------------------------------------
// DataLayer Import Integration
//----------------------------------------------------------------

bool FStageEditorController::CanImportDataLayer(UDataLayerAsset* DataLayerAsset, FText& OutReason)
{
	if (!DataLayerAsset)
	{
		OutReason = LOCTEXT("ErrorNullAsset", "DataLayerAsset is null");
		return false;
	}

	UWorld* World = GEditor ? GEditor->GetEditorWorldContext().World() : nullptr;
	if (!World)
	{
		OutReason = LOCTEXT("ErrorNoWorld", "No valid World found");
		return false;
	}

	// Check naming convention
	FDataLayerNameParseResult ParseResult = FStageDataLayerNameParser::Parse(DataLayerAsset->GetName());
	if (!ParseResult.bIsValid)
	{
		OutReason = LOCTEXT("ErrorInvalidNaming", "DataLayer name does not follow naming convention (DL_Stage_*)");
		return false;
	}

	if (!ParseResult.bIsStageLayer)
	{
		OutReason = LOCTEXT("ErrorNotStageLayer", "Only Stage-level DataLayers (DL_Stage_*) can be imported. Select the parent Stage DataLayer.");
		return false;
	}

	// Check if already imported
	if (UStageManagerSubsystem* Subsystem = GetSubsystem())
	{
		if (Subsystem->IsDataLayerImported(DataLayerAsset))
		{
			OutReason = LOCTEXT("ErrorAlreadyImported", "This DataLayer has already been imported");
			return false;
		}
	}

	return true;
}

AStage* FStageEditorController::ImportStageFromDataLayer(UDataLayerAsset* StageDataLayerAsset, UWorld* World)
{
	// Default behavior: use first child DataLayer as DefaultAct (index 0)
	return ImportStageFromDataLayerWithDefaultAct(StageDataLayerAsset, 0, World);
}

AStage* FStageEditorController::ImportStageFromDataLayerWithDefaultAct(UDataLayerAsset* StageDataLayerAsset, int32 SelectedDefaultActIndex, UWorld* World)
{
	// Validate
	FText ErrorMessage;
	if (!CanImportDataLayer(StageDataLayerAsset, ErrorMessage))
	{
		DebugHeader::ShowNotifyInfo(ErrorMessage.ToString());
		return nullptr;
	}

	if (!World)
	{
		World = GEditor ? GEditor->GetEditorWorldContext().World() : nullptr;
	}
	if (!World)
	{
		return nullptr;
	}

	// Parse Stage name
	FDataLayerNameParseResult StageParseResult = FStageDataLayerNameParser::Parse(StageDataLayerAsset->GetName());

	// Begin transaction
	FScopedTransaction Transaction(LOCTEXT("ImportDataLayerAsStage", "Import DataLayer as Stage"));

	// CRITICAL: Suppress automatic DataLayer creation in OnLevelActorAdded
	FScopedImportBypass ImportBypass;

	// 1. Create Stage Actor (constructor creates empty DefaultAct at Acts[0])
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AStage* NewStage = World->SpawnActor<AStage>(AStage::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
	if (!NewStage)
	{
		DebugHeader::ShowNotifyInfo(TEXT("Failed to create Stage actor"));
		return nullptr;
	}

	NewStage->Modify();
	NewStage->StageName = StageParseResult.StageName;
	NewStage->SetActorLabel(StageParseResult.StageName);

	// 2. Assign SUID via Subsystem
	if (UStageManagerSubsystem* Subsystem = GetSubsystem())
	{
		Subsystem->RegisterStage(NewStage);
	}

	// 3. Associate Stage with DataLayer
	NewStage->StageDataLayerAsset = StageDataLayerAsset;
	NewStage->StageDataLayerName = StageDataLayerAsset->GetName();

	// 4. Collect all child DataLayers first
	struct FChildDataLayerInfo
	{
		UDataLayerAsset* Asset;
		FString DisplayName;
	};
	TArray<FChildDataLayerInfo> ChildDataLayers;

	UDataLayerManager* Manager = UDataLayerManager::GetDataLayerManager(World);
	if (Manager)
	{
		Manager->ForEachDataLayerInstance([&](UDataLayerInstance* Instance)
		{
			if (!Instance || !Instance->GetParent())
			{
				return true; // Continue
			}

			if (Instance->GetParent()->GetAsset() != StageDataLayerAsset)
			{
				return true; // Not a child of this Stage
			}

			const UDataLayerAsset* ChildAsset = Instance->GetAsset();
			if (!ChildAsset)
			{
				return true;
			}

			// Parse naming to determine display name
			FDataLayerNameParseResult ActParseResult = FStageDataLayerNameParser::Parse(ChildAsset->GetName());

			FChildDataLayerInfo Info;
			Info.Asset = const_cast<UDataLayerAsset*>(ChildAsset);

			if (!ActParseResult.bIsValid)
			{
				Info.DisplayName = ChildAsset->GetName();
			}
			else if (ActParseResult.bIsStageLayer)
			{
				Info.DisplayName = ActParseResult.StageName;
			}
			else
			{
				Info.DisplayName = ActParseResult.ActName;
			}

			ChildDataLayers.Add(Info);
			return true; // Continue iteration
		});
	}

	// 5. Process DefaultAct and other Acts
	// DefaultAct (ID=1) is already at Acts[0] from constructor
	// We need to either update it with selected DataLayer or keep it empty

	if (ChildDataLayers.Num() > 0 && SelectedDefaultActIndex >= 0)
	{
		// Clamp index to valid range
		SelectedDefaultActIndex = FMath::Clamp(SelectedDefaultActIndex, 0, ChildDataLayers.Num() - 1);

		// Update DefaultAct (Acts[0]) with selected child DataLayer
		FChildDataLayerInfo& SelectedChild = ChildDataLayers[SelectedDefaultActIndex];
		NewStage->Acts[0].DisplayName = SelectedChild.DisplayName;
		NewStage->Acts[0].AssociatedDataLayer = SelectedChild.Asset;
		NewStage->Acts[0].SUID = FSUID::MakeActID(NewStage->SUID.StageID, 1); // ActID=1 for DefaultAct

		// Register Props for DefaultAct
		if (Manager)
		{
			const UDataLayerInstance* DefaultActInstance = Manager->GetDataLayerInstance(SelectedChild.Asset);
			if (DefaultActInstance)
			{
				for (TActorIterator<AActor> It(World); It; ++It)
				{
					AActor* Actor = *It;
					if (Actor && Actor != NewStage && Actor->HasDataLayers())
					{
						TArray<const UDataLayerInstance*> ActorDataLayers = Actor->GetDataLayerInstances();
						if (ActorDataLayers.Contains(DefaultActInstance))
						{
							int32 PropID = NewStage->RegisterProp(Actor);
							if (PropID >= 0)
							{
								NewStage->Acts[0].PropStateOverrides.Add(PropID, 0);
							}
						}
					}
				}
			}
		}

		// Add other child DataLayers as Acts (starting from ActID=2)
		int32 NextActID = 2;
		for (int32 i = 0; i < ChildDataLayers.Num(); ++i)
		{
			if (i == SelectedDefaultActIndex)
			{
				continue; // Skip the one we used for DefaultAct
			}

			FChildDataLayerInfo& ChildInfo = ChildDataLayers[i];

			FAct NewAct;
			NewAct.SUID = FSUID::MakeActID(NewStage->SUID.StageID, NextActID);
			NewAct.DisplayName = ChildInfo.DisplayName;
			NewAct.AssociatedDataLayer = ChildInfo.Asset;

			int32 NewActIndex = NewStage->Acts.Add(NewAct);

			// Register Props for this Act
			if (Manager)
			{
				const UDataLayerInstance* ActInstance = Manager->GetDataLayerInstance(ChildInfo.Asset);
				if (ActInstance)
				{
					for (TActorIterator<AActor> It(World); It; ++It)
					{
						AActor* Actor = *It;
						if (Actor && Actor != NewStage && Actor->HasDataLayers())
						{
							TArray<const UDataLayerInstance*> ActorDataLayers = Actor->GetDataLayerInstances();
							if (ActorDataLayers.Contains(ActInstance))
							{
								int32 PropID = NewStage->RegisterProp(Actor);
								if (PropID >= 0)
								{
									NewStage->Acts[NewActIndex].PropStateOverrides.Add(PropID, 0);
								}
							}
						}
					}
				}
			}

			NextActID++;
		}
	}
	else if (ChildDataLayers.Num() > 0)
	{
		// SelectedDefaultActIndex == -1: Keep empty DefaultAct, add all children starting from ActID=2
		int32 NextActID = 2;
		for (FChildDataLayerInfo& ChildInfo : ChildDataLayers)
		{
			FAct NewAct;
			NewAct.SUID = FSUID::MakeActID(NewStage->SUID.StageID, NextActID);
			NewAct.DisplayName = ChildInfo.DisplayName;
			NewAct.AssociatedDataLayer = ChildInfo.Asset;

			int32 NewActIndex = NewStage->Acts.Add(NewAct);

			// Register Props for this Act
			if (Manager)
			{
				const UDataLayerInstance* ActInstance = Manager->GetDataLayerInstance(ChildInfo.Asset);
				if (ActInstance)
				{
					for (TActorIterator<AActor> It(World); It; ++It)
					{
						AActor* Actor = *It;
						if (Actor && Actor != NewStage && Actor->HasDataLayers())
						{
							TArray<const UDataLayerInstance*> ActorDataLayers = Actor->GetDataLayerInstances();
							if (ActorDataLayers.Contains(ActInstance))
							{
								int32 PropID = NewStage->RegisterProp(Actor);
								if (PropID >= 0)
								{
									NewStage->Acts[NewActIndex].PropStateOverrides.Add(PropID, 0);
								}
							}
						}
					}
				}
			}

			NextActID++;
		}
	}
	// else: No child DataLayers - keep empty DefaultAct from constructor

	// Invalidate sync status cache
	FDataLayerSyncStatusCache::Get().InvalidateCache(StageDataLayerAsset);
	if (Manager)
	{
		Manager->ForEachDataLayerInstance([&](UDataLayerInstance* Instance)
		{
			if (Instance && Instance->GetAsset())
			{
				FDataLayerSyncStatusCache::Get().InvalidateCache(Instance->GetAsset());
			}
			return true;
		});
	}

	// Refresh controller state
	FindStageInWorld();
	OnModelChanged.Broadcast();

	DebugHeader::ShowNotifyInfo(FString::Printf(TEXT("Imported Stage '%s' with %d Acts"),
		*StageParseResult.StageName, NewStage->Acts.Num() - 1)); // -1 because Acts[0] is DefaultAct

	return NewStage;
}

//----------------------------------------------------------------
// DataLayer Rename
//----------------------------------------------------------------

bool FStageEditorController::RenameStageDataLayer(AStage* Stage, const FString& NewAssetName)
{
	if (!Stage || !Stage->StageDataLayerAsset)
	{
		return false;
	}

	UDataLayerAsset* Asset = Stage->StageDataLayerAsset;
	FString OldName = Asset->GetName();

	if (OldName == NewAssetName)
	{
		return true; // No change needed
	}

	// Parse old and new Stage names for cascade update
	FDataLayerNameParseResult OldParseResult = FStageDataLayerNameParser::Parse(OldName);
	FDataLayerNameParseResult NewParseResult = FStageDataLayerNameParser::Parse(NewAssetName);

	FString OldStageName = OldParseResult.bIsValid ? OldParseResult.StageName : TEXT("");
	FString NewStageName = NewParseResult.bIsValid ? NewParseResult.StageName : TEXT("");

	// Perform asset rename for Stage DataLayer
	TArray<FAssetRenameData> RenameData;
	FString PackagePath = FPackageName::GetLongPackagePath(Asset->GetOutermost()->GetName());
	RenameData.Add(FAssetRenameData(Asset, PackagePath, NewAssetName));

	FAssetToolsModule& AssetToolsModule = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools");
	bool bSuccess = AssetToolsModule.Get().RenameAssets(RenameData);

	if (bSuccess)
	{
		// Update Stage's cached name
		const FScopedTransaction Transaction(LOCTEXT("RenameStageDataLayer", "Rename Stage DataLayer"));
		Stage->Modify();
		Stage->StageDataLayerName = NewAssetName;

		// Invalidate cache
		FDataLayerSyncStatusCache::Get().InvalidateCache(Asset);

		// Broadcast
		OnDataLayerRenamed.Broadcast(Asset, NewAssetName);

		UE_LOG(LogTemp, Log, TEXT("Renamed Stage DataLayer from '%s' to '%s'"), *OldName, *NewAssetName);

		// Cascade update: rename all child Act DataLayers if StageName changed
		if (!OldStageName.IsEmpty() && !NewStageName.IsEmpty() && OldStageName != NewStageName)
		{
			int32 CascadeCount = 0;
			for (FAct& Act : Stage->Acts)
			{
				if (!Act.AssociatedDataLayer)
				{
					continue;
				}

				UDataLayerAsset* ActAsset = Act.AssociatedDataLayer;
				FString ActOldName = ActAsset->GetName();

				// Parse Act DataLayer name
				FDataLayerNameParseResult ActParseResult = FStageDataLayerNameParser::Parse(ActOldName);
				if (!ActParseResult.bIsValid || ActParseResult.bIsStageLayer)
				{
					continue; // Skip if not a valid Act DataLayer
				}

				// Check if this Act's StageName matches the old Stage name
				if (ActParseResult.StageName != OldStageName)
				{
					continue; // Skip if StageName doesn't match (shouldn't happen normally)
				}

				// Build new Act DataLayer name: DL_Act_<NewStageName>_<ActName>
				FString ActNewName = FString::Printf(TEXT("DL_Act_%s_%s"), *NewStageName, *ActParseResult.ActName);

				// Rename Act DataLayer
				TArray<FAssetRenameData> ActRenameData;
				FString ActPackagePath = FPackageName::GetLongPackagePath(ActAsset->GetOutermost()->GetName());
				ActRenameData.Add(FAssetRenameData(ActAsset, ActPackagePath, ActNewName));

				if (AssetToolsModule.Get().RenameAssets(ActRenameData))
				{
					FDataLayerSyncStatusCache::Get().InvalidateCache(ActAsset);
					OnDataLayerRenamed.Broadcast(ActAsset, ActNewName);
					CascadeCount++;

					UE_LOG(LogTemp, Log, TEXT("  Cascade renamed Act DataLayer from '%s' to '%s'"), *ActOldName, *ActNewName);
				}
				else
				{
					UE_LOG(LogTemp, Warning, TEXT("  Failed to cascade rename Act DataLayer '%s'"), *ActOldName);
				}
			}

			if (CascadeCount > 0)
			{
				UE_LOG(LogTemp, Log, TEXT("Cascade renamed %d Act DataLayer(s) to use new StageName '%s'"), CascadeCount, *NewStageName);
			}
		}

		OnModelChanged.Broadcast();
	}

	return bSuccess;
}

bool FStageEditorController::RenameActDataLayer(AStage* Stage, int32 ActID, const FString& NewAssetName)
{
	if (!Stage)
	{
		return false;
	}

	// Find the Act
	FAct* TargetAct = nullptr;
	for (FAct& Act : Stage->Acts)
	{
		if (Act.SUID.ActID == ActID)
		{
			TargetAct = &Act;
			break;
		}
	}

	if (!TargetAct || !TargetAct->AssociatedDataLayer)
	{
		return false;
	}

	UDataLayerAsset* Asset = TargetAct->AssociatedDataLayer;
	FString OldName = Asset->GetName();

	if (OldName == NewAssetName)
	{
		return true; // No change needed
	}

	// Perform asset rename
	TArray<FAssetRenameData> RenameData;
	FString PackagePath = FPackageName::GetLongPackagePath(Asset->GetOutermost()->GetName());
	RenameData.Add(FAssetRenameData(Asset, PackagePath, NewAssetName));

	FAssetToolsModule& AssetToolsModule = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools");
	bool bSuccess = AssetToolsModule.Get().RenameAssets(RenameData);

	if (bSuccess)
	{
		// Invalidate cache
		FDataLayerSyncStatusCache::Get().InvalidateCache(Asset);

		// Broadcast
		OnDataLayerRenamed.Broadcast(Asset, NewAssetName);
		OnModelChanged.Broadcast();

		UE_LOG(LogTemp, Log, TEXT("Renamed Act DataLayer from '%s' to '%s'"), *OldName, *NewAssetName);
	}

	return bSuccess;
}

AStage* FStageEditorController::FindStageByDataLayer(UDataLayerAsset* DataLayerAsset)
{
	if (!DataLayerAsset)
	{
		return nullptr;
	}

	// Check all found stages
	for (const TWeakObjectPtr<AStage>& StagePtr : FoundStages)
	{
		AStage* Stage = StagePtr.Get();
		if (!Stage)
		{
			continue;
		}

		// Check Stage's root DataLayer
		if (Stage->StageDataLayerAsset == DataLayerAsset)
		{
			return Stage;
		}

		// Check Act DataLayers
		for (const FAct& Act : Stage->Acts)
		{
			if (Act.AssociatedDataLayer == DataLayerAsset)
			{
				return Stage;
			}
		}
	}

	return nullptr;
}

int32 FStageEditorController::FindActIDByDataLayer(AStage* Stage, UDataLayerAsset* DataLayerAsset)
{
	if (!Stage || !DataLayerAsset)
	{
		return -1;
	}

	for (const FAct& Act : Stage->Acts)
	{
		if (Act.AssociatedDataLayer == DataLayerAsset)
		{
			return Act.SUID.ActID;
		}
	}

	return -1;
}

//----------------------------------------------------------------
// External DataLayer Change Handler (Phase 9.3)
//----------------------------------------------------------------

void FStageEditorController::OnExternalDataLayerChanged(
	const EDataLayerAction Action,
	const TWeakObjectPtr<const UDataLayerInstance>& ChangedDataLayer,
	const FName& ChangedProperty)
{
	// Only handle rename actions
	if (Action != EDataLayerAction::Rename)
	{
		return;
	}

	const UDataLayerInstance* Instance = ChangedDataLayer.Get();
	if (!Instance)
	{
		return;
	}

	UDataLayerAsset* Asset = const_cast<UDataLayerAsset*>(Instance->GetAsset());
	if (!Asset)
	{
		return;
	}

	// Check if this DataLayer belongs to any of our Stages
	AStage* OwnerStage = FindStageByDataLayer(Asset);
	if (!OwnerStage)
	{
		// Not an imported DataLayer, nothing to sync
		return;
	}

	FString NewName = Asset->GetName();

	// Check if it's the Stage's root DataLayer
	if (OwnerStage->StageDataLayerAsset == Asset)
	{
		// Sync StageDataLayerName
		if (OwnerStage->StageDataLayerName != NewName)
		{
			const FScopedTransaction Transaction(LOCTEXT("SyncStageDataLayerName", "Sync Stage DataLayer Name"));
			OwnerStage->Modify();
			OwnerStage->StageDataLayerName = NewName;

			UE_LOG(LogTemp, Log, TEXT("External rename detected: Synced Stage '%s' DataLayer name to '%s'"),
				*OwnerStage->GetActorLabel(), *NewName);
		}
	}

	// Invalidate cache for this DataLayer
	FDataLayerSyncStatusCache::Get().InvalidateCache(Asset);

	// Broadcast changes
	OnDataLayerRenamed.Broadcast(Asset, NewName);
	OnModelChanged.Broadcast();
}

#undef LOCTEXT_NAMESPACE
