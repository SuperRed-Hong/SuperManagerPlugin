#pragma once

#include "CoreMinimal.h"
#include "Components/BoxComponent.h"
#include "Core/StageCoreTypes.h"
#include "TriggerZoneComponentBase.generated.h"

// Forward declarations
class AStage;

//----------------------------------------------------------------
// Delegate Declarations
//----------------------------------------------------------------

/** Broadcast when a valid actor enters the trigger zone. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTriggerZoneActorEnter, UTriggerZoneComponentBase*, Zone, AActor*, Actor);

/** Broadcast when a valid actor exits the trigger zone. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTriggerZoneActorExit, UTriggerZoneComponentBase*, Zone, AActor*, Actor);

/**
 * @brief Base class for all TriggerZone components in the StageEditor system.
 *
 * This component provides:
 * - Overlap detection with configurable filtering
 * - Blueprint events for custom trigger logic
 * - Structured description for documentation and debugging
 * - Auto-registration to owning Stage
 *
 * Usage:
 * - Attach to any Actor for trigger zone functionality
 * - Set OwnerStage to bind to a Stage
 * - Bind OnActorEnter/OnActorExit in Blueprint to implement custom logic
 * - Fill in Description for debugging and flow visualization
 *
 * Design Principle: This component only provides events.
 * All trigger logic (what action to perform) is defined by the user in Blueprint.
 *
 * @see ATriggerZoneActor for a standalone Actor that uses this component
 * @see UStageTriggerZoneComponent for built-in Stage zones with hardcoded logic
 */
UCLASS(ClassGroup = (StageEditor), Blueprintable, meta = (BlueprintSpawnableComponent, DisplayName = "Trigger Zone (Base)"))
class STAGEEDITORRUNTIME_API UTriggerZoneComponentBase : public UBoxComponent
{
	GENERATED_BODY()

public:
	UTriggerZoneComponentBase();

	//----------------------------------------------------------------
	// Documentation
	//----------------------------------------------------------------

	/**
	 * Description of this TriggerZone's purpose.
	 * Fill in the template to document what this zone does.
	 * This information is used by debug tools to visualize zone flow.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TriggerZone|Documentation",
		meta = (DisplayName = "Zone Description", ShowOnlyInnerProperties))
	FTriggerZoneDescription Description;

	/** Get readable description text (for debug display). */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "TriggerZone|Documentation")
	FString GetDescriptionText() const { return Description.ToReadableString(); }

	/** Get the description struct for detailed parsing. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "TriggerZone|Documentation")
	const FTriggerZoneDescription& GetDescription() const { return Description; }

	/** Apply a preset template to quickly fill description. */
	UFUNCTION(BlueprintCallable, Category = "TriggerZone|Documentation")
	void ApplyDescriptionPreset(ETriggerZonePreset Preset);

	//----------------------------------------------------------------
	// Stage Binding
	//----------------------------------------------------------------

	/**
	 * The Stage this zone is bound to.
	 * Zone will auto-register to this Stage at BeginPlay.
	 * Set this to enable Stage to track and query this zone.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TriggerZone|Binding",
		meta = (DisplayName = "Owner Stage"))
	TSoftObjectPtr<AStage> OwnerStage;

	/**
	 * @brief Checks if this zone is bound to a Stage.
	 * @return True if OwnerStage is valid.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "TriggerZone|Binding")
	bool IsBoundToStage() const;

	/**
	 * @brief Gets the owning Stage (resolved from soft pointer).
	 * @return The Stage actor, or nullptr if not bound or invalid.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "TriggerZone|Binding")
	AStage* GetOwnerStage() const;

	//----------------------------------------------------------------
	// Filtering
	//----------------------------------------------------------------

	/**
	 * Tag filters for triggering actors.
	 * If not empty, only actors with ANY of these tags will trigger zone events.
	 * If empty, defaults to allowing only Pawn class actors.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TriggerZone|Filtering",
		meta = (DisplayName = "Trigger Actor Tags"))
	TArray<FName> TriggerActorTags;

	/**
	 * If true, the triggering actor must be a Pawn.
	 * When TriggerActorTags is empty, this is always true (default behavior).
	 * When TriggerActorTags is set, this adds an additional Pawn requirement.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TriggerZone|Filtering",
		meta = (DisplayName = "Must Be Pawn", EditCondition = "TriggerActorTags.Num() > 0"))
	bool bMustBePawn = false;

	/**
	 * @brief Determines if an actor should trigger zone events.
	 * Can be overridden in Blueprint or C++ for custom filtering.
	 * @param Actor The actor to check.
	 * @return True if the actor should trigger events.
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "TriggerZone|Filtering")
	bool ShouldTriggerForActor(AActor* Actor) const;
	virtual bool ShouldTriggerForActor_Implementation(AActor* Actor) const;

	//----------------------------------------------------------------
	// Blueprint Events
	//----------------------------------------------------------------

	/**
	 * Called when a valid actor enters this zone.
	 * Bind this event in Blueprint to implement custom trigger logic.
	 *
	 * Example usage:
	 * - Check additional conditions
	 * - Call Stage API (LoadStage, ActivateAct, etc.)
	 * - Play effects, sounds, etc.
	 */
	UPROPERTY(BlueprintAssignable, Category = "TriggerZone|Events")
	FOnTriggerZoneActorEnter OnActorEnter;

	/**
	 * Called when a valid actor exits this zone.
	 * Bind this event in Blueprint to implement custom exit logic.
	 */
	UPROPERTY(BlueprintAssignable, Category = "TriggerZone|Events")
	FOnTriggerZoneActorExit OnActorExit;

	//----------------------------------------------------------------
	// State
	//----------------------------------------------------------------

	/**
	 * If false, the zone will not trigger events.
	 * Use this to temporarily disable the zone without removing it.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TriggerZone",
		meta = (DisplayName = "Enabled"))
	bool bZoneEnabled = true;

	/** Set zone enabled state. */
	UFUNCTION(BlueprintCallable, Category = "TriggerZone")
	void SetZoneEnabled(bool bEnabled);

	/** Check if zone is enabled. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "TriggerZone")
	bool IsZoneEnabled() const { return bZoneEnabled; }

protected:
	//----------------------------------------------------------------
	// Lifecycle
	//----------------------------------------------------------------

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

	//----------------------------------------------------------------
	// Overlap Handling
	//----------------------------------------------------------------

	/** Internal overlap event handlers. */
	UFUNCTION()
	void OnZoneBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnZoneEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	/**
	 * @brief Called after filtering passes and before delegate broadcast.
	 * Override in derived classes to add custom behavior (e.g., Stage forwarding).
	 * @param Actor The actor that entered the zone.
	 */
	virtual void HandleActorEnter(AActor* Actor);

	/**
	 * @brief Called after filtering passes and before delegate broadcast.
	 * Override in derived classes to add custom behavior (e.g., Stage forwarding).
	 * @param Actor The actor that exited the zone.
	 */
	virtual void HandleActorExit(AActor* Actor);

	/** Whether overlap events are currently bound. */
	bool bOverlapEventsBound = false;

	/** Binds overlap events to this component. */
	void BindOverlapEvents();

	/** Unbinds overlap events from this component. */
	void UnbindOverlapEvents();

	//----------------------------------------------------------------
	// Stage Registration
	//----------------------------------------------------------------

	/** Whether this zone is registered to a Stage. */
	bool bRegisteredToStage = false;

	/** Register this zone to OwnerStage. */
	void RegisterToStage();

	/** Unregister this zone from OwnerStage. */
	void UnregisterFromStage();
};
