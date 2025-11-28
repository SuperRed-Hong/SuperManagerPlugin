#pragma once

#include "CoreMinimal.h"
#include "WorldPartition/DataLayer/DataLayerInstance.h" // For EDataLayerRuntimeState
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

//----------------------------------------------------------------
// User-Facing State (Simplified 3-State)
//----------------------------------------------------------------

/**
 * @brief Simplified Stage state for user-facing API.
 * Maps to internal EStageRuntimeState for implementation.
 *
 * Use this enum with user API: LoadStage(), ActivateStage(), UnloadStage()
 * For debugging, use EStageRuntimeState which shows transition states.
 */
UENUM(BlueprintType)
enum class EStageState : uint8
{
	/** Stage is not loaded. */
	Unloaded UMETA(DisplayName = "Unloaded"),

	/** Stage is loaded but not active. */
	Loaded UMETA(DisplayName = "Loaded"),

	/** Stage is fully active and interactive. */
	Active UMETA(DisplayName = "Active"),
};

//----------------------------------------------------------------
// TriggerZone Types
//----------------------------------------------------------------

/**
 * @brief Default action to perform when actor enters/exits a TriggerZone.
 * Simplifies common use cases without requiring Blueprint logic.
 *
 * For simple scenarios (80%), select a preset action.
 * For complex logic, use Custom and implement in Blueprint.
 */
UENUM(BlueprintType)
enum class ETriggerZoneDefaultAction : uint8
{
	/** No automatic action. Use Blueprint OnActorEnter/OnActorExit events. */
	Custom UMETA(DisplayName = "Custom (Blueprint)"),

	/** Automatically calls Stage->LoadStage() when actor enters. */
	LoadStage UMETA(DisplayName = "Load Stage"),

	/** Automatically calls Stage->ActivateStage() when actor enters. */
	ActivateStage UMETA(DisplayName = "Activate Stage"),

	/** Automatically calls Stage->UnloadStage() when actor enters/exits. */
	UnloadStage UMETA(DisplayName = "Unload Stage"),
};

/**
 * @brief Preset templates for quick TriggerZone description setup.
 * Select a template to auto-fill common patterns, then customize as needed.
 */
UENUM(BlueprintType)
enum class ETriggerZonePreset : uint8
{
	/** No preset - fill all fields manually. */
	Custom UMETA(DisplayName = "Custom"),

	/** Stage loading trigger: Player enters → Stage preloads. */
	StageLoad UMETA(DisplayName = "Stage Load"),

	/** Stage activation trigger: Player enters → Stage activates. */
	StageActivate UMETA(DisplayName = "Stage Activate"),

	/** Act activation trigger: Player enters → Act activates. */
	ActActivate UMETA(DisplayName = "Act Activate"),

	/** Act deactivation trigger: Player enters → Act deactivates. */
	ActDeactivate UMETA(DisplayName = "Act Deactivate"),

	/** Prop state change trigger: Player enters → Prop state changes. */
	PropStateChange UMETA(DisplayName = "Prop State Change"),

	/** Conditional trigger: Player enters + conditions → custom action. */
	ConditionalTrigger UMETA(DisplayName = "Conditional Trigger"),
};

/**
 * @brief Structured description for TriggerZone documentation and debugging.
 * Helps users document the purpose of each zone in a consistent format.
 * This information is used by debug tools to visualize zone flow.
 *
 * Template Format:
 * "When [Trigger] enters this zone, and [Condition], execute [Action] on [Target] to [Effect]."
 */
USTRUCT(BlueprintType)
struct STAGEEDITORRUNTIME_API FTriggerZoneDescription
{
	GENERATED_BODY()

	//----------------------------------------------------------------
	// Preset Template
	//----------------------------------------------------------------

	/**
	 * Quick-fill template selection.
	 * Choose a preset to auto-populate fields with common patterns.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Template",
		meta = (DisplayName = "Preset Template"))
	ETriggerZonePreset Preset = ETriggerZonePreset::Custom;

	//----------------------------------------------------------------
	// Description Fields
	//----------------------------------------------------------------

	/**
	 * WHO/WHAT triggers this zone?
	 * Examples: "Player", "Any Pawn", "Actor with tag 'Enemy'", "Projectile"
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Description",
		meta = (DisplayName = "Trigger",
			ToolTip = "WHO/WHAT triggers this zone?\n\nExamples:\n- Player\n- Any Pawn\n- Actor with tag 'Enemy'\n- Projectile"))
	FString Trigger = TEXT("Player");

	/**
	 * WHEN does the action execute? (Optional pre-conditions)
	 * Examples: "Has key item", "Quest completed", "Health > 50%", "" (always)
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Description",
		meta = (DisplayName = "Condition (Optional)",
			ToolTip = "WHEN does the action execute? (Pre-conditions checked in Blueprint)\n\nExamples:\n- Has key item\n- Quest 'Rescue' completed\n- Player health > 50%\n- (empty) = Always trigger"))
	FString Condition;

	/**
	 * WHAT is affected by this trigger?
	 * Examples: "Stage_BossRoom", "Act_Battle (ID:2)", "Prop_Door_01"
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Description",
		meta = (DisplayName = "Target",
			ToolTip = "WHAT is affected by this trigger?\n\nExamples:\n- Stage_BossRoom\n- Act_Battle (ID: 2)\n- Prop_Door_01 (PropID: 5)\n- Multiple: Stage + Act"))
	FString Target;

	/**
	 * WHAT ACTION is performed?
	 * Examples: "LoadStage()", "ActivateAct(2)", "SetPropState(1)", "Custom BP Event"
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Description",
		meta = (DisplayName = "Action",
			ToolTip = "WHAT ACTION is performed?\n\nExamples:\n- LoadStage()\n- ActivateAct(2)\n- SetPropState(1) - Open\n- Play Sound + Spawn VFX"))
	FString Action;

	/**
	 * WHY - What is the intended gameplay effect?
	 * Examples: "Preload boss room for seamless entry", "Start battle sequence"
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Description",
		meta = (DisplayName = "Effect (Purpose)",
			ToolTip = "WHY - What is the intended gameplay effect?\n\nExamples:\n- Preload boss room for seamless entry\n- Start battle sequence when player is ready\n- Open door to reveal secret area"))
	FString Effect;

	//----------------------------------------------------------------
	// Methods
	//----------------------------------------------------------------

	/**
	 * Generates a human-readable description string.
	 * Format: "When [Trigger] enters, if [Condition], then [Action] on [Target] to [Effect]."
	 */
	FString ToReadableString() const
	{
		FString Result = FString::Printf(TEXT("When [%s] enters this zone"), *Trigger);

		if (!Condition.IsEmpty())
		{
			Result += FString::Printf(TEXT(", and [%s]"), *Condition);
		}

		if (!Target.IsEmpty() && !Action.IsEmpty())
		{
			Result += FString::Printf(TEXT(", execute [%s] on [%s]"), *Action, *Target);
		}
		else if (!Action.IsEmpty())
		{
			Result += FString::Printf(TEXT(", execute [%s]"), *Action);
		}

		if (!Effect.IsEmpty())
		{
			Result += FString::Printf(TEXT(" to [%s]"), *Effect);
		}

		Result += TEXT(".");
		return Result;
	}

	/**
	 * Apply preset template values.
	 * Call this when Preset changes to auto-fill fields.
	 */
	void ApplyPreset()
	{
		switch (Preset)
		{
		case ETriggerZonePreset::StageLoad:
			Trigger = TEXT("Player");
			Condition = TEXT("");
			Target = TEXT("Stage_???");
			Action = TEXT("LoadStage()");
			Effect = TEXT("Preload stage resources before player arrives");
			break;

		case ETriggerZonePreset::StageActivate:
			Trigger = TEXT("Player");
			Condition = TEXT("");
			Target = TEXT("Stage_???");
			Action = TEXT("ActivateStage()");
			Effect = TEXT("Fully activate stage when player enters");
			break;

		case ETriggerZonePreset::ActActivate:
			Trigger = TEXT("Player");
			Condition = TEXT("");
			Target = TEXT("Stage_??? / Act ID: ?");
			Action = TEXT("ActivateAct(?)");
			Effect = TEXT("Activate scene configuration");
			break;

		case ETriggerZonePreset::ActDeactivate:
			Trigger = TEXT("Player");
			Condition = TEXT("");
			Target = TEXT("Stage_??? / Act ID: ?");
			Action = TEXT("DeactivateAct(?)");
			Effect = TEXT("Deactivate scene configuration");
			break;

		case ETriggerZonePreset::PropStateChange:
			Trigger = TEXT("Player");
			Condition = TEXT("");
			Target = TEXT("Prop_??? (PropID: ?)");
			Action = TEXT("SetPropState(?)");
			Effect = TEXT("Change prop visual/behavior state");
			break;

		case ETriggerZonePreset::ConditionalTrigger:
			Trigger = TEXT("Player");
			Condition = TEXT("??? (check in Blueprint)");
			Target = TEXT("???");
			Action = TEXT("??? (implement in Blueprint)");
			Effect = TEXT("???");
			break;

		case ETriggerZonePreset::Custom:
		default:
			// Keep current values for Custom
			break;
		}
	}

	/** Check if description has meaningful content */
	bool IsEmpty() const
	{
		return Trigger.IsEmpty() && Target.IsEmpty() && Action.IsEmpty();
	}
};

//----------------------------------------------------------------
// Stage Unique ID
//----------------------------------------------------------------

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
 * All fields except DisplayName and InitialDataLayerState are managed by StageEditorController.
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
	 * When true, this Act's DataLayer state mirrors the Stage's state.
	 * - Stage Loaded → Act Loaded
	 * - Stage Active → Act Active
	 * - Stage Unloaded → Act Unloaded
	 *
	 * When false, uses InitialDataLayerState (only applied when Stage becomes Active).
	 * Default Act (ID=1) defaults to true.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Act",
		meta = (DisplayName = "Follow Stage State"))
	bool bFollowStageState = false;

	/**
	 * Initial DataLayer state when Stage becomes Active.
	 * Only used when bFollowStageState is false.
	 * - Unloaded: DataLayer not loaded, wait for explicit ActivateAct() call
	 * - Loaded: DataLayer preloaded but not activated (resources in memory)
	 * - Activated: DataLayer fully activated when Stage becomes Active
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Act",
		meta = (DisplayName = "Initial DataLayer State", EditCondition = "!bFollowStageState"))
	EDataLayerRuntimeState InitialDataLayerState = EDataLayerRuntimeState::Unloaded;

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
