#pragma once

#include "CoreMinimal.h"
#include "Actors/Prop.h"
#include "TriggerZoneActor.generated.h"

class UTriggerZoneComponentBase;

/**
 * @brief Convenience Actor for placing TriggerZones in the level.
 *
 * This Actor combines:
 * - AProp functionality (Prop registration, SUID, Act management via DataLayers)
 * - UTriggerZoneComponentBase (trigger detection, Blueprint events)
 *
 * PropState controls zone enabled state:
 * - PropState == 0: Zone disabled (no overlap events)
 * - PropState != 0: Zone enabled (overlap events fire)
 *
 * This allows Acts to control zone activation:
 * - Place zone in Act's DataLayer → Zone loads/unloads with Act
 * - Use PropStateOverrides → Act can enable/disable zone
 *
 * Usage:
 * 1. Place TriggerZoneActor in level (or create Blueprint subclass)
 * 2. Set OwnerStage on TriggerZoneComponent (for tracking)
 * 3. Fill in Description (for debugging)
 * 4. Create Blueprint subclass to bind OnActorEnter/Exit events
 * 5. Implement custom trigger logic in Blueprint
 *
 * @see UTriggerZoneComponentBase for the core trigger logic
 * @see AProp for Prop functionality
 */
UCLASS(Blueprintable, meta = (DisplayName = "Trigger Zone Actor"))
class STAGEEDITORRUNTIME_API ATriggerZoneActor : public AProp
{
	GENERATED_BODY()

public:
	ATriggerZoneActor();

	//----------------------------------------------------------------
	// Components
	//----------------------------------------------------------------

	/**
	 * The trigger zone component that handles overlap detection and events.
	 * Configure filtering, description, and bind events on this component.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Trigger Zone")
	TObjectPtr<UTriggerZoneComponentBase> TriggerZoneComponent;

	//----------------------------------------------------------------
	// Convenience Accessors
	//----------------------------------------------------------------

	/**
	 * @brief Gets the TriggerZone component.
	 * @return The TriggerZoneComponentBase, or nullptr if not set.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Trigger Zone")
	UTriggerZoneComponentBase* GetTriggerZone() const { return TriggerZoneComponent; }

	/**
	 * @brief Checks if the trigger zone is enabled.
	 * Zone is enabled when PropState != 0.
	 * @return True if zone is enabled.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Trigger Zone")
	bool IsZoneEnabled() const;

	/**
	 * @brief Enables or disables the trigger zone via PropState.
	 * Sets PropState to 1 (enabled) or 0 (disabled).
	 * @param bEnabled True to enable, false to disable.
	 */
	UFUNCTION(BlueprintCallable, Category = "Trigger Zone")
	void SetZoneEnabled(bool bEnabled);

protected:
	//----------------------------------------------------------------
	// Lifecycle
	//----------------------------------------------------------------

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	//----------------------------------------------------------------
	// PropState Sync
	//----------------------------------------------------------------

	/**
	 * Called when PropState changes.
	 * Syncs the TriggerZoneComponent enabled state with PropState.
	 */
	UFUNCTION()
	void OnPropStateChangedHandler(int32 NewState, int32 OldState);

	/**
	 * Syncs TriggerZoneComponent enabled state with current PropState.
	 * PropState == 0 → Disabled, PropState != 0 → Enabled
	 */
	void SyncZoneEnabledState();
};
