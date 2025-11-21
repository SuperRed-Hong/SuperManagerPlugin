#pragma once

#include "CoreMinimal.h"
#include "StageCoreTypes.generated.h"

/**
 * @brief Represents the runtime state of a Stage.
 */
UENUM(BlueprintType)
enum class EStageRuntimeState : uint8
{
	/** The stage is unloaded or inactive. */
	Unloaded UMETA(DisplayName = "Unloaded"),
	
	/** The stage is loading its resources. */
	Loading UMETA(DisplayName = "Loading"),
	
	/** The stage is fully active and running logic. */
	Active UMETA(DisplayName = "Active"),
};

/**
 * @brief A hierarchical ID structure to uniquely identify a Prop within a Stage and Act context.
 * Structure: StageID -> ActID -> PropID
 */
USTRUCT(BlueprintType)
struct STAGEEDITORRUNTIME_API FStageHierarchicalId
{
	GENERATED_BODY()

	/** The globally unique ID of the Stage (assigned by central server). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stage ID")
	int32 StageID = 0;

	/** The local ID of the Act within the Stage. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stage ID")
	int32 ActID = 0;

	/** The local ID of the Prop within the Stage. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stage ID")
	int32 PropID = 0;

	FStageHierarchicalId() {}
	FStageHierarchicalId(int32 InStageID, int32 InActID, int32 InPropID)
		: StageID(InStageID), ActID(InActID), PropID(InPropID) {}

	bool operator==(const FStageHierarchicalId& Other) const
	{
		return StageID == Other.StageID && ActID == Other.ActID && PropID == Other.PropID;
	}

	friend uint32 GetTypeHash(const FStageHierarchicalId& Id)
	{
		return HashCombine(HashCombine(::GetTypeHash(Id.StageID), ::GetTypeHash(Id.ActID)), ::GetTypeHash(Id.PropID));
	}

	FString ToString() const
	{
		return FString::Printf(TEXT("%d.%d.%d"), StageID, ActID, PropID);
	}
};

/**
 * @brief Defines a "Scene" or "State" of the Stage.
 * Contains the target state for a set of Props.
 */
USTRUCT(BlueprintType)
struct STAGEEDITORRUNTIME_API FAct
{
	GENERATED_BODY()

	/** The unique ID of this Act. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Act")
	FStageHierarchicalId ActID;

	/** Display name for the Editor. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Act")
	FString DisplayName;

	/** 
	 * Map of PropID -> Target PropState Value.
	 * Defines what state each Prop should be in when this Act is active.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Act")
	TMap<int32, int32> PropStateOverrides;

	/** 
	 * The Data Layer associated with this Act.
	 * When this Act is active, this Data Layer will be activated.
	 */
	UPROPERTY(EditAnywhere, Category = "Act")
	TObjectPtr<class UDataLayerAsset> AssociatedDataLayer;
};
