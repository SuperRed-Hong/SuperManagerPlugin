#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Core/StageCoreTypes.h"
#include "Actors/Prop.h"
#include "Stage.generated.h"

/**
 * @brief The "Director" of the stage.
 * Manages a list of Acts and a registry of Props.
 * Responsible for applying Act states to Props.
 */
UCLASS()
class STAGEEDITORRUNTIME_API AStage : public AActor
{
	GENERATED_BODY()
	
public:	
	AStage();

protected:
	virtual void BeginPlay() override;

public:	
	//----------------------------------------------------------------
	// Core Data
	//----------------------------------------------------------------

	/** Globally unique ID for this Stage. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stage")
	int32 StageID = 0;

	/** List of all defined Acts for this Stage. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stage")
	TArray<FAct> Acts;

	/** 
	 * Registry of Props managed by this Stage.
	 * Key: PropID, Value: Soft reference to the Prop Actor.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stage")
	TMap<int32, TSoftObjectPtr<AActor>> PropRegistry;

	//----------------------------------------------------------------
	// Runtime Logic
	//----------------------------------------------------------------

	/**
	 * @brief Activates a specific Act by its local ID.
	 * Applies the PropState overrides defined in the Act to the registered Props.
	 */
	UFUNCTION(BlueprintCallable, Category = "Stage")
	void ActivateAct(int32 ActID);

	/**
	 * @brief Deactivates the current Act (optional logic, e.g., reset props).
	 */
	UFUNCTION(BlueprintCallable, Category = "Stage")
	void DeactivateAct();

	//----------------------------------------------------------------
	// Editor/Setup API
	//----------------------------------------------------------------

	/** Registers a Prop to this Stage. Returns the assigned PropID. */
	int32 RegisterProp(AActor* NewProp);

	/** Unregisters a Prop by ID. Removes from PropRegistry and all Acts. */
	void UnregisterProp(int32 PropID);

	/** Removes a Prop from a specific Act's PropStateOverrides. */
	void RemovePropFromAct(int32 PropID, int32 ActID);

	/** Removes an Act by its ActID. */
	void RemoveAct(int32 ActID);

	/** Finds a Prop by ID. */
	AActor* GetPropByID(int32 PropID) const;
};
