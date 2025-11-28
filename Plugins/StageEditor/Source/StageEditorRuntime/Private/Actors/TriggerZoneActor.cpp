#include "Actors/TriggerZoneActor.h"
#include "Components/TriggerZoneComponentBase.h"
#include "Components/StagePropComponent.h"

DEFINE_LOG_CATEGORY_STATIC(LogTriggerZoneActor, Log, All);

ATriggerZoneActor::ATriggerZoneActor()
{
	// Create the trigger zone component
	TriggerZoneComponent = CreateDefaultSubobject<UTriggerZoneComponentBase>(TEXT("TriggerZone"));
	RootComponent = TriggerZoneComponent;

	// Default PropState to 1 (enabled)
	// This will be set properly after PropComponent is created in AProp constructor
}

void ATriggerZoneActor::BeginPlay()
{
	Super::BeginPlay();

	// Bind to PropState changes
	if (PropComponent)
	{
		PropComponent->OnPropStateChanged.AddDynamic(this, &ATriggerZoneActor::OnPropStateChangedHandler);

		// Set default PropState to 1 (enabled) if still at 0
		// This ensures the zone is enabled by default
		if (PropComponent->PropState == 0)
		{
			PropComponent->SetPropState(1, false);
		}
	}

	// Initial sync
	SyncZoneEnabledState();

	UE_LOG(LogTriggerZoneActor, Log, TEXT("TriggerZoneActor [%s]: BeginPlay - PropState: %d, ZoneEnabled: %s"),
		*GetName(),
		PropComponent ? PropComponent->PropState : -1,
		IsZoneEnabled() ? TEXT("Yes") : TEXT("No"));
}

void ATriggerZoneActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// Unbind from PropState changes
	if (PropComponent)
	{
		PropComponent->OnPropStateChanged.RemoveDynamic(this, &ATriggerZoneActor::OnPropStateChangedHandler);
	}

	Super::EndPlay(EndPlayReason);
}

//----------------------------------------------------------------
// Convenience Accessors
//----------------------------------------------------------------

bool ATriggerZoneActor::IsZoneEnabled() const
{
	if (TriggerZoneComponent)
	{
		return TriggerZoneComponent->IsZoneEnabled();
	}
	return false;
}

void ATriggerZoneActor::SetZoneEnabled(bool bEnabled)
{
	// Set PropState which will trigger OnPropStateChangedHandler
	if (PropComponent)
	{
		PropComponent->SetPropState(bEnabled ? 1 : 0, false);
	}
}

//----------------------------------------------------------------
// PropState Sync
//----------------------------------------------------------------

void ATriggerZoneActor::OnPropStateChangedHandler(int32 NewState, int32 OldState)
{
	UE_LOG(LogTriggerZoneActor, Verbose, TEXT("TriggerZoneActor [%s]: PropState changed %d → %d"),
		*GetName(), OldState, NewState);

	SyncZoneEnabledState();
}

void ATriggerZoneActor::SyncZoneEnabledState()
{
	if (!TriggerZoneComponent || !PropComponent)
	{
		return;
	}

	// PropState == 0 → Disabled, PropState != 0 → Enabled
	const bool bShouldBeEnabled = PropComponent->PropState != 0;
	const bool bCurrentlyEnabled = TriggerZoneComponent->IsZoneEnabled();

	if (bShouldBeEnabled != bCurrentlyEnabled)
	{
		TriggerZoneComponent->SetZoneEnabled(bShouldBeEnabled);

		UE_LOG(LogTriggerZoneActor, Log, TEXT("TriggerZoneActor [%s]: Zone %s (PropState: %d)"),
			*GetName(),
			bShouldBeEnabled ? TEXT("ENABLED") : TEXT("DISABLED"),
			PropComponent->PropState);
	}
}
