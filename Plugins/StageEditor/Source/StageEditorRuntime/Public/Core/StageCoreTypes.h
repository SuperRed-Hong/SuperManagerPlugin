#pragma once

#include "CoreMinimal.h"
#include "StageCoreTypes.generated.h"

/**
 * @brief Represents the runtime state of a Stage's DataLayer lifecycle.
 *
 * State Flow:
 *   Unloaded → Preloading → Loaded → Active
 *                  ↑                    ↓
 *                  └── Unloading ←──────┘
 *
 * Trigger Zones:
 *   - Enter LoadZone: Unloaded → Preloading → Loaded
 *   - Enter ActivateZone: Loaded → Active
 *   - Leave ActivateZone (still in LoadZone): Stay Active
 *   - Leave LoadZone: Active/Loaded → Unloading → Unloaded
 */
UENUM(BlueprintType)
enum class EStageRuntimeState : uint8
{
	/** Stage DataLayer is completely unloaded. No resources loaded. */
	Unloaded UMETA(DisplayName = "Unloaded"),

	/** Stage DataLayer is being loaded. Transition state - blocks repeated triggers. */
	Preloading UMETA(DisplayName = "Preloading"),

	/** Stage DataLayer is loaded but not activated. Preload buffer zone. */
	Loaded UMETA(DisplayName = "Loaded"),

	/** Stage DataLayer is fully activated. Stage is interactive. */
	Active UMETA(DisplayName = "Active"),

	/** Stage DataLayer is being unloaded. Transition state - blocks repeated triggers. */
	Unloading UMETA(DisplayName = "Unloading"),
};

/**
 * @brief Stage Unique ID - A hierarchical ID structure to uniquely identify entities within the Stage system.
 * Structure: StageID.ActID.PropID
 * All IDs are system-assigned and read-only in the editor.
 *
 * Usage:
 * - Stage level: Only StageID is set (ActID=0, PropID=0) - identifies the Stage itself (e.g., 1.0.0)
 * - Act level: StageID and ActID are set (PropID=0) - ActID starts from 1 (e.g., 1.1.0 for Default Act)
 * - Prop level: StageID and PropID are set (ActID=0) - PropID starts from 1 (e.g., 1.0.1)
 *
 * Note: ActID=1 is reserved for the Default Act, which is created automatically with each Stage.
 *       ActID starts from 1 (not 0) to ensure unique SUID for each entity type.
 */
USTRUCT(BlueprintType)
struct STAGEEDITORRUNTIME_API FSUID
{
	GENERATED_BODY()

	/** The globally unique ID of the Stage (assigned by StageManagerSubsystem). */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SUID")
	int32 StageID = 0;

	/** The local ID of the Act within the Stage (assigned by StageEditorController). */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SUID")
	int32 ActID = 0;

	/** The local ID of the Prop within the Stage (assigned by Stage::RegisterProp). */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SUID")
	int32 PropID = 0;

	FSUID() {}
	FSUID(int32 InStageID, int32 InActID, int32 InPropID)
		: StageID(InStageID), ActID(InActID), PropID(InPropID) {}

	// === Factory Methods ===
	static FSUID MakeStageID(int32 InStageID)
	{
		return FSUID(InStageID, 0, 0);
	}

	static FSUID MakeActID(int32 InStageID, int32 InActID)
	{
		return FSUID(InStageID, InActID, 0);
	}

	static FSUID MakePropID(int32 InStageID, int32 InPropID)
	{
		return FSUID(InStageID, 0, InPropID);
	}

	// === Comparison ===
	bool operator==(const FSUID& Other) const
	{
		return StageID == Other.StageID && ActID == Other.ActID && PropID == Other.PropID;
	}

	bool operator!=(const FSUID& Other) const
	{
		return !(*this == Other);
	}

	friend uint32 GetTypeHash(const FSUID& Id)
	{
		return HashCombine(HashCombine(::GetTypeHash(Id.StageID), ::GetTypeHash(Id.ActID)), ::GetTypeHash(Id.PropID));
	}

	// === Utility ===
	FString ToString() const
	{
		return FString::Printf(TEXT("%d.%d.%d"), StageID, ActID, PropID);
	}

	bool IsValid() const
	{
		return StageID > 0;
	}

	bool IsStageLevel() const { return StageID > 0 && ActID == 0 && PropID == 0; }
	bool IsActLevel() const { return StageID > 0 && ActID > 0; }
	bool IsPropLevel() const { return StageID > 0 && PropID > 0; }
};

/**
 * @brief Defines a "Scene" or "State" of the Stage.
 * Contains the target state for a set of Props.
 * All fields except DisplayName are managed by StageEditorController.
 */
USTRUCT(BlueprintType)
struct STAGEEDITORRUNTIME_API FAct
{
	GENERATED_BODY()

	/** The unique ID of this Act. Assigned by StageEditorController. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Act")
	FSUID SUID;

	/** Display name for the Editor. User-editable. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Act")
	FString DisplayName;

	/**
	 * Map of PropID -> Target PropState Value.
	 * Defines what state each Prop should be in when this Act is active.
	 * Managed via StageEditorController - do not edit directly.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Act")
	TMap<int32, int32> PropStateOverrides;

	/**
	 * The Data Layer associated with this Act.
	 * When this Act is active, this Data Layer will be activated.
	 * Auto-created by StageEditorController.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Act")
	TObjectPtr<class UDataLayerAsset> AssociatedDataLayer;
};
