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

bool FStageEditorController::RegisterProps(const TArray<AActor*>& ActorsToRegister)
{
	AStage* Stage = GetActiveStage();
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
	}
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
	if (InActor && InActor->IsA<AStage>())
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
	}
}

#undef LOCTEXT_NAMESPACE
