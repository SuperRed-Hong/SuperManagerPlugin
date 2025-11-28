#include "Components/TriggerZoneComponentBase.h"
#include "Actors/Stage.h"
#include "GameFramework/Pawn.h"

DEFINE_LOG_CATEGORY_STATIC(LogTriggerZone, Log, All);

UTriggerZoneComponentBase::UTriggerZoneComponentBase()
{
	// Component settings
	PrimaryComponentTick.bCanEverTick = false;
	bWantsOnUpdateTransform = false;

	// Default box extent
	BoxExtent = FVector(200.0f, 200.0f, 100.0f);

	// Collision settings for trigger
	BodyInstance.SetCollisionProfileName(TEXT("Trigger"));
	SetGenerateOverlapEvents(true);

	// Visual settings for editor
	bHiddenInGame = false;
	ShapeColor = FColor::Cyan;
	LineThickness = 5.0f;
}

//----------------------------------------------------------------
// Documentation
//----------------------------------------------------------------

void UTriggerZoneComponentBase::ApplyDescriptionPreset(ETriggerZonePreset Preset)
{
	Description.Preset = Preset;
	Description.ApplyPreset();
}

//----------------------------------------------------------------
// Stage Binding
//----------------------------------------------------------------

bool UTriggerZoneComponentBase::IsBoundToStage() const
{
	return OwnerStage.IsValid() || OwnerStage.ToSoftObjectPath().IsValid();
}

AStage* UTriggerZoneComponentBase::GetOwnerStage() const
{
	return OwnerStage.Get();
}

//----------------------------------------------------------------
// Filtering
//----------------------------------------------------------------

bool UTriggerZoneComponentBase::ShouldTriggerForActor_Implementation(AActor* Actor) const
{
	if (!Actor) return false;

	// If TriggerActorTags is not empty, use tag-based filtering
	if (TriggerActorTags.Num() > 0)
	{
		// Check if actor has ANY of the required tags
		bool bHasAnyTag = false;
		for (const FName& Tag : TriggerActorTags)
		{
			if (Tag != NAME_None && Actor->ActorHasTag(Tag))
			{
				bHasAnyTag = true;
				break;
			}
		}

		if (!bHasAnyTag)
		{
			return false;
		}

		// If bRequirePawn is true, also check if it's a Pawn
		if (bRequirePawn)
		{
			return Actor->IsA(APawn::StaticClass());
		}

		// Tag matched, no Pawn requirement
		return true;
	}

	// Default behavior (no tags set): only trigger for Pawns
	return Actor->IsA(APawn::StaticClass());
}

//----------------------------------------------------------------
// State
//----------------------------------------------------------------

void UTriggerZoneComponentBase::SetZoneEnabled(bool bEnabled)
{
	bZoneEnabled = bEnabled;
	SetGenerateOverlapEvents(bEnabled);

	UE_LOG(LogTriggerZone, Log, TEXT("TriggerZone [%s]: %s"),
		*GetName(), bEnabled ? TEXT("Enabled") : TEXT("Disabled"));
}

//----------------------------------------------------------------
// Lifecycle
//----------------------------------------------------------------

void UTriggerZoneComponentBase::BeginPlay()
{
	Super::BeginPlay();

	// Bind overlap events
	BindOverlapEvents();

	// Register to Stage if bound
	RegisterToStage();

	UE_LOG(LogTriggerZone, Log, TEXT("TriggerZone [%s]: BeginPlay - OwnerStage: %s, Enabled: %s"),
		*GetName(),
		OwnerStage.IsValid() ? *OwnerStage->GetName() : TEXT("None"),
		bZoneEnabled ? TEXT("Yes") : TEXT("No"));
}

void UTriggerZoneComponentBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UnbindOverlapEvents();
	UnregisterFromStage();

	Super::EndPlay(EndPlayReason);
}

#if WITH_EDITOR
void UTriggerZoneComponentBase::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	const FName PropertyName = PropertyChangedEvent.GetPropertyName();

	// Auto-apply preset when changed
	if (PropertyName == GET_MEMBER_NAME_CHECKED(FTriggerZoneDescription, Preset))
	{
		if (Description.Preset != ETriggerZonePreset::Custom)
		{
			Description.ApplyPreset();
		}
	}

	// Update overlap events when enabled state changes
	if (PropertyName == GET_MEMBER_NAME_CHECKED(UTriggerZoneComponentBase, bZoneEnabled))
	{
		SetGenerateOverlapEvents(bZoneEnabled);
	}
}
#endif

//----------------------------------------------------------------
// Overlap Handling
//----------------------------------------------------------------

void UTriggerZoneComponentBase::BindOverlapEvents()
{
	if (bOverlapEventsBound) return;

	OnComponentBeginOverlap.AddDynamic(this, &UTriggerZoneComponentBase::OnZoneBeginOverlap);
	OnComponentEndOverlap.AddDynamic(this, &UTriggerZoneComponentBase::OnZoneEndOverlap);

	bOverlapEventsBound = true;
}

void UTriggerZoneComponentBase::UnbindOverlapEvents()
{
	if (!bOverlapEventsBound) return;

	OnComponentBeginOverlap.RemoveDynamic(this, &UTriggerZoneComponentBase::OnZoneBeginOverlap);
	OnComponentEndOverlap.RemoveDynamic(this, &UTriggerZoneComponentBase::OnZoneEndOverlap);

	bOverlapEventsBound = false;
}

void UTriggerZoneComponentBase::OnZoneBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// Check if zone is enabled
	if (!bZoneEnabled)
	{
		return;
	}

	// Filter check
	if (!ShouldTriggerForActor(OtherActor))
	{
		return;
	}

	UE_LOG(LogTriggerZone, Verbose, TEXT("TriggerZone [%s]: Actor '%s' entered"),
		*GetName(), *OtherActor->GetName());

	// Execute preset action (if not Custom)
	ExecutePresetAction(OnEnterAction);

	// Broadcast Blueprint event (always, for custom logic or additional handling)
	OnActorEnter.Broadcast(this, OtherActor);
}

void UTriggerZoneComponentBase::OnZoneEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	// Check if zone is enabled
	if (!bZoneEnabled)
	{
		return;
	}

	// Filter check
	if (!ShouldTriggerForActor(OtherActor))
	{
		return;
	}

	UE_LOG(LogTriggerZone, Verbose, TEXT("TriggerZone [%s]: Actor '%s' exited"),
		*GetName(), *OtherActor->GetName());

	// Execute preset action (if not Custom)
	ExecutePresetAction(OnExitAction);

	// Broadcast Blueprint event (always, for custom logic or additional handling)
	OnActorExit.Broadcast(this, OtherActor);
}

//----------------------------------------------------------------
// Stage Registration
//----------------------------------------------------------------

void UTriggerZoneComponentBase::RegisterToStage()
{
	if (bRegisteredToStage) return;

	AStage* Stage = GetOwnerStage();
	if (!Stage)
	{
		return;
	}

	Stage->RegisterTriggerZone(this);
	bRegisteredToStage = true;
}

void UTriggerZoneComponentBase::UnregisterFromStage()
{
	if (!bRegisteredToStage) return;

	AStage* Stage = GetOwnerStage();
	if (Stage)
	{
		Stage->UnregisterTriggerZone(this);
	}

	bRegisteredToStage = false;
}

//----------------------------------------------------------------
// Preset Action Execution
//----------------------------------------------------------------

void UTriggerZoneComponentBase::ExecutePresetAction(ETriggerZoneDefaultAction Action)
{
	// Skip if Custom (user handles via Blueprint event)
	if (Action == ETriggerZoneDefaultAction::Custom)
	{
		return;
	}

	// Get owner Stage
	AStage* Stage = GetOwnerStage();
	if (!Stage)
	{
		UE_LOG(LogTriggerZone, Warning, TEXT("TriggerZone [%s]: Cannot execute preset action - no OwnerStage bound"),
			*GetName());
		return;
	}

	// Execute the appropriate action
	switch (Action)
	{
	case ETriggerZoneDefaultAction::LoadStage:
		UE_LOG(LogTriggerZone, Log, TEXT("TriggerZone [%s]: Executing preset action -> LoadStage()"),
			*GetName());
		Stage->LoadStage();
		break;

	case ETriggerZoneDefaultAction::ActivateStage:
		UE_LOG(LogTriggerZone, Log, TEXT("TriggerZone [%s]: Executing preset action -> ActivateStage()"),
			*GetName());
		Stage->ActivateStage();
		break;

	case ETriggerZoneDefaultAction::UnloadStage:
		UE_LOG(LogTriggerZone, Log, TEXT("TriggerZone [%s]: Executing preset action -> UnloadStage()"),
			*GetName());
		Stage->UnloadStage();
		break;

	default:
		break;
	}
}
