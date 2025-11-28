#include "Components/StageTriggerZoneComponent.h"
#include "Actors/Stage.h"
#include "GameFramework/Pawn.h"

DEFINE_LOG_CATEGORY_STATIC(LogStageTriggerZone, Log, All);

UStageTriggerZoneComponent::UStageTriggerZoneComponent()
{
	// Component settings - safe to set in constructor
	PrimaryComponentTick.bCanEverTick = false;
	bWantsOnUpdateTransform = false;

	// Box extent - use direct member access instead of setter for CDO safety
	BoxExtent = FVector(500.0f, 500.0f, 200.0f);

	// Collision settings - use collision profile directly
	// Note: SetCollisionProfileName may cause issues during CDO construction
	BodyInstance.SetCollisionProfileName(TEXT("Trigger"));
	SetGenerateOverlapEvents(true);

	// Visual settings for editor - use direct member access
	bHiddenInGame = false;
	ShapeColor = FColor::Yellow; // Default to LoadZone color
	LineThickness = 10.0f;
}

#if WITH_EDITOR
bool UStageTriggerZoneComponent::CanEditChange(const FProperty* InProperty) const
{
	if (!Super::CanEditChange(InProperty))
	{
		return false;
	}

	// ZoneType should be read-only for built-in zones (created via CreateDefaultSubobject)
	// but editable for user-placed external zones
	if (InProperty && InProperty->GetFName() == GET_MEMBER_NAME_CHECKED(UStageTriggerZoneComponent, ZoneType))
	{
		// DefaultSubobject components are created by CreateDefaultSubobject in owner's constructor
		// These are the built-in zones that should have fixed ZoneType
		return !IsDefaultSubobject();
	}

	return true;
}
#endif

void UStageTriggerZoneComponent::BeginPlay()
{
	Super::BeginPlay();

	// Bind overlap events
	BindOverlapEvents();

	// Update visual color based on zone type
	if (ZoneType == EStageTriggerZoneType::LoadZone)
	{
		ShapeColor = FColor::Yellow;
	}
	else
	{
		ShapeColor = FColor::Green;
	}
}

void UStageTriggerZoneComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UnbindOverlapEvents();
	UnbindFromStage();

	Super::EndPlay(EndPlayReason);
}

//----------------------------------------------------------------
// Binding
//----------------------------------------------------------------

void UStageTriggerZoneComponent::BindToStage(AStage* Stage)
{
	if (!Stage)
	{
		UE_LOG(LogStageTriggerZone, Warning, TEXT("TriggerZone [%s]: Cannot bind to null Stage"), *GetName());
		return;
	}

	if (OwnerStage == Stage)
	{
		// Already bound to this Stage
		return;
	}

	// Unbind from previous Stage if any
	if (OwnerStage)
	{
		UnbindFromStage();
	}

	OwnerStage = Stage;

	UE_LOG(LogStageTriggerZone, Log, TEXT("TriggerZone [%s] (%s): Bound to Stage '%s'"),
		*GetName(),
		ZoneType == EStageTriggerZoneType::LoadZone ? TEXT("LoadZone") : TEXT("ActivateZone"),
		*Stage->GetName());
}

void UStageTriggerZoneComponent::UnbindFromStage()
{
	if (OwnerStage)
	{
		UE_LOG(LogStageTriggerZone, Log, TEXT("TriggerZone [%s]: Unbound from Stage '%s'"),
			*GetName(), *OwnerStage->GetName());
		OwnerStage = nullptr;
	}
}

//----------------------------------------------------------------
// Filtering
//----------------------------------------------------------------

bool UStageTriggerZoneComponent::ShouldTriggerForActor_Implementation(AActor* Actor) const
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

		// If bRequirePawnWithTag is true, also check if it's a Pawn
		if (bRequirePawnWithTag)
		{
			return Actor->IsA(APawn::StaticClass());
		}

		// Tag matched, no Pawn requirement
		return true;
	}

	// Default behavior (no tags set): only trigger for Pawns (players, AI characters)
	return Actor->IsA(APawn::StaticClass());
}

//----------------------------------------------------------------
// Overlap Events
//----------------------------------------------------------------

void UStageTriggerZoneComponent::BindOverlapEvents()
{
	if (bOverlapEventsBound) return;

	OnComponentBeginOverlap.AddDynamic(this, &UStageTriggerZoneComponent::OnZoneBeginOverlap);
	OnComponentEndOverlap.AddDynamic(this, &UStageTriggerZoneComponent::OnZoneEndOverlap);

	bOverlapEventsBound = true;
}

void UStageTriggerZoneComponent::UnbindOverlapEvents()
{
	if (!bOverlapEventsBound) return;

	OnComponentBeginOverlap.RemoveDynamic(this, &UStageTriggerZoneComponent::OnZoneBeginOverlap);
	OnComponentEndOverlap.RemoveDynamic(this, &UStageTriggerZoneComponent::OnZoneEndOverlap);

	bOverlapEventsBound = false;
}

void UStageTriggerZoneComponent::OnZoneBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// Filter check
	if (!ShouldTriggerForActor(OtherActor))
	{
		return;
	}

	UE_LOG(LogStageTriggerZone, Verbose, TEXT("TriggerZone [%s]: Actor '%s' entered (%s)"),
		*GetName(), *OtherActor->GetName(),
		ZoneType == EStageTriggerZoneType::LoadZone ? TEXT("LoadZone") : TEXT("ActivateZone"));

	// Forward to Stage if bound
	if (OwnerStage)
	{
		OwnerStage->HandleZoneBeginOverlap(this, OtherActor);
	}
}

void UStageTriggerZoneComponent::OnZoneEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	// Filter check
	if (!ShouldTriggerForActor(OtherActor))
	{
		return;
	}

	UE_LOG(LogStageTriggerZone, Verbose, TEXT("TriggerZone [%s]: Actor '%s' exited (%s)"),
		*GetName(), *OtherActor->GetName(),
		ZoneType == EStageTriggerZoneType::LoadZone ? TEXT("LoadZone") : TEXT("ActivateZone"));

	// Forward to Stage if bound
	if (OwnerStage)
	{
		OwnerStage->HandleZoneEndOverlap(this, OtherActor);
	}
}
