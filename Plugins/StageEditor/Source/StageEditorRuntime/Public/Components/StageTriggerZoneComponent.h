#pragma once

#include "CoreMinimal.h"
#include "Components/BoxComponent.h"
#include "StageTriggerZoneComponent.generated.h"

// Forward declarations
class AStage;

/**
 * @brief Defines the type/purpose of a Stage Trigger Zone.
 */
UENUM(BlueprintType)
enum class EStageTriggerZoneType : uint8
{
	/** Outer zone: Triggers Stage DataLayer loading (Unloaded → Preloading → Loaded) */
	LoadZone UMETA(DisplayName = "Load Zone"),

	/** Inner zone: Triggers Stage activation (Loaded → Active) */
	ActivateZone UMETA(DisplayName = "Activate Zone"),
};

/**
 * @brief A trigger zone component for Stage state management.
 *
 * This component inherits from UBoxComponent and provides:
 * - Zone type configuration (LoadZone / ActivateZone)
 * - Binding to a Stage actor for event forwarding
 * - Actor filtering for trigger events
 *
 * Usage:
 * - Built-in zones are created by Stage actor in constructor
 * - External zones can be placed separately and referenced by Stage
 * - Stage calls InitializeTriggerZones() to bind all zones
 *
 * Event Flow:
 * 1. Actor overlaps this component
 * 2. OnZoneBeginOverlap/OnZoneEndOverlap called
 * 3. If bound to Stage, forwards to Stage->HandleZoneBeginOverlap/EndOverlap
 * 4. Stage updates its actor tracking sets and triggers state transitions
 *
 * @see AStage
 */
UCLASS(ClassGroup = (StageEditor), meta = (BlueprintSpawnableComponent, DisplayName = "Stage Trigger Zone"))
class STAGEEDITORRUNTIME_API UStageTriggerZoneComponent : public UBoxComponent
{
	GENERATED_BODY()

public:
	UStageTriggerZoneComponent();

	//----------------------------------------------------------------
	// Configuration
	//----------------------------------------------------------------

	/**
	 * The type of this trigger zone.
	 * - LoadZone: Outer zone for preloading Stage DataLayer
	 * - ActivateZone: Inner zone for activating Stage
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TriggerZone")
	EStageTriggerZoneType ZoneType = EStageTriggerZoneType::LoadZone;

	//----------------------------------------------------------------
	// Runtime Binding
	//----------------------------------------------------------------

	/**
	 * The Stage this zone is bound to. Set by Stage::InitializeTriggerZones().
	 * When bound, overlap events are forwarded to the Stage.
	 */
	UPROPERTY(Transient, BlueprintReadOnly, Category = "TriggerZone")
	TObjectPtr<AStage> OwnerStage;

	/**
	 * @brief Binds this zone to a Stage actor.
	 * Sets up overlap event forwarding to the Stage.
	 * @param Stage The Stage to bind to.
	 */
	UFUNCTION(BlueprintCallable, Category = "TriggerZone")
	void BindToStage(AStage* Stage);

	/**
	 * @brief Unbinds this zone from its current Stage.
	 * Stops forwarding overlap events.
	 */
	UFUNCTION(BlueprintCallable, Category = "TriggerZone")
	void UnbindFromStage();

	/**
	 * @brief Checks if this zone is currently bound to a Stage.
	 * @return True if OwnerStage is valid.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "TriggerZone")
	bool IsBoundToStage() const { return OwnerStage != nullptr; }

	//----------------------------------------------------------------
	// Filtering
	//----------------------------------------------------------------

	/**
	 * @brief Tag filter for triggering actors.
	 * If set (not NAME_None), only actors with this tag will trigger zone events.
	 * If empty, defaults to allowing only Pawn class actors.
	 *
	 * Common usage:
	 * - Empty (default): Only Pawns trigger (players, AI)
	 * - "Player": Only actors tagged "Player" trigger
	 * - "TriggerActor": Custom tag for specific triggering actors
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TriggerZone|Filtering")
	FName TriggerActorTag;

	/**
	 * @brief If true, requires the actor to be a Pawn in addition to having the tag.
	 * Only relevant when TriggerActorTag is set.
	 * Default: false (tag check only when tag is set)
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TriggerZone|Filtering",
		meta = (EditCondition = "TriggerActorTag != NAME_None"))
	bool bRequirePawnWithTag = false;

	/**
	 * @brief Determines if an actor should trigger zone events.
	 *
	 * Filtering logic:
	 * 1. If TriggerActorTag is set:
	 *    - Check if actor has the specified tag
	 *    - If bRequirePawnWithTag is true, also require actor to be a Pawn
	 * 2. If TriggerActorTag is empty (default):
	 *    - Only Pawns trigger events (players, AI characters)
	 *
	 * Can be overridden in Blueprint or C++ for custom filtering.
	 * @param Actor The actor to check.
	 * @return True if the actor should trigger events.
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "TriggerZone")
	bool ShouldTriggerForActor(AActor* Actor) const;
	virtual bool ShouldTriggerForActor_Implementation(AActor* Actor) const;

protected:
	//----------------------------------------------------------------
	// Overlap Event Handlers
	//----------------------------------------------------------------

	/** Called when an actor begins overlapping this zone. */
	UFUNCTION()
	void OnZoneBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	/** Called when an actor stops overlapping this zone. */
	UFUNCTION()
	void OnZoneEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	/** Whether overlap events are currently bound. */
	bool bOverlapEventsBound = false;

	/** Binds overlap events to this component. */
	void BindOverlapEvents();

	/** Unbinds overlap events from this component. */
	void UnbindOverlapEvents();
};
