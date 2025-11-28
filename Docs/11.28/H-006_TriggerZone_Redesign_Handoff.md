# H-006: TriggerZone Architecture Redesign - Handoff Document

> Date: 2025-11-28
> Status: Design Complete, Ready for Implementation

---

## Executive Summary

This document captures the complete design decisions for the TriggerZone system redesign, addressing UX problems with External TriggerZones and establishing a more flexible, user-centric architecture.

### Key Design Principles

1. **Don't preset user behavior** - C++ provides capabilities, Blueprint defines logic
2. **Zone is a special Prop** - Inherits from AProp for Act/DataLayer management
3. **Component is core** - Can be attached to any Actor, auto-registers to Stage
4. **Layered abstraction** - Internal 5-state machine, User-facing 3-state API

---

## Part 1: TriggerZone Architecture

### Problem Statement

The original External TriggerZone UX had several issues:
1. Complex configuration (5+ manual steps)
2. Non-intuitive reference mechanism (Component without Actor)
3. Unclear relationship with Built-in Zones
4. Difficult debugging

### Solution: Component + Optional Actor Pattern

```
UTriggerZoneComponentBase (Core)
├── OwnerStage: TSoftObjectPtr<AStage>
├── TriggerActorTags / bRequirePawn (filtering)
├── OnActorEnter / OnActorExit (Blueprint events)
├── Description (documentation template)
├── BeginPlay() → auto-register to OwnerStage
└── Can be attached to ANY Actor

ATriggerZoneActor : public AProp (Convenience class, optional)
├── Inherits UStagePropComponent (SUID indexing, Act management)
├── Contains UTriggerZoneComponentBase
├── PropState: 0=disabled, non-0=enabled
└── For standalone placement scenarios

AStage (Extended)
├── RegisteredTriggerZones: TArray<UTriggerZoneComponent*>
├── RegisterTriggerZone() / UnregisterTriggerZone()
└── GetAllTriggerZones() // For debugging
```

### Built-in vs External Zones

| Type | Behavior | Use Case |
|------|----------|----------|
| **Built-in Zones** | Hardcoded logic (unchanged) | 80% default scenarios |
| **External Zones** | Blueprint events, user-defined logic | Any custom requirements |

### User Workflows

**Workflow A: Place Standalone Actor**
```
1. Place Actors → Trigger Zone Actor
2. Set OwnerStage reference
3. Create Blueprint subclass
4. Bind OnActorEnter event
5. Implement custom logic in Blueprint
```

**Workflow B: Attach Component to Existing Actor**
```
1. Select any Actor (e.g., Door)
2. Add Component → Trigger Zone Component
3. Set OwnerStage reference
4. Bind events in that Actor's Blueprint
```

---

## Part 2: Description Template System

### Purpose

Provide a structured way for users to document TriggerZone purpose, enabling:
- Self-documenting level design
- Debug tool visualization
- Flow tracing and debugging

### Structure

```cpp
USTRUCT(BlueprintType)
struct FTriggerZoneDescription
{
    // Template preset (quick-fill)
    ETriggerZonePreset Preset;

    // WHO/WHAT triggers this zone?
    FString Trigger;      // e.g., "Player", "Any Pawn"

    // WHEN does action execute? (pre-conditions, optional)
    FString Condition;    // e.g., "Has key item", "" (always)

    // WHAT is affected?
    FString Target;       // e.g., "Stage_BossRoom", "Act_Battle (ID:2)"

    // WHAT ACTION is performed?
    FString Action;       // e.g., "LoadStage()", "ActivateAct(2)"

    // WHY - gameplay purpose?
    FString Effect;       // e.g., "Preload boss room for seamless entry"

    // Generate readable text
    FString ToReadableString() const;
    FText ToReadableText() const;  // Localized
};
```

### Preset Templates

| Preset | Trigger | Target | Action | Effect |
|--------|---------|--------|--------|--------|
| Stage Load | Player | Stage_??? | LoadStage() | Preload resources |
| Stage Activate | Player | Stage_??? | ActivateStage() | Fully activate |
| Act Activate | Player | Stage/Act | ActivateAct(?) | Activate config |
| Act Deactivate | Player | Stage/Act | DeactivateAct(?) | Deactivate config |
| Prop State Change | Player | Prop_??? | SetPropState(?) | Change state |
| Conditional | Player | ??? | ??? | ??? |

### Field Tooltips (for Editor)

| Field | Tooltip |
|-------|---------|
| **Trigger** | WHO/WHAT triggers this zone? Examples: Player, Any Pawn, Actor with tag 'Enemy' |
| **Condition** | WHEN does action execute? Pre-conditions checked in Blueprint. Leave empty if always trigger. |
| **Target** | WHAT is affected? Include identifiers (Stage name, Act ID, Prop ID) for clarity. |
| **Action** | WHAT ACTION is performed? Reference actual function names for debugging clarity. |
| **Effect** | WHY - gameplay purpose? High-level intent without reading Blueprint code. |

---

## Part 3: Stage State API Simplification

### Problem

Current 5-state enum is developer-oriented, too complex for users:
```cpp
enum class EStageRuntimeState : uint8
{
    Unloaded,    // Internal
    Preloading,  // Internal (transition)
    Loaded,      // Internal
    Active,      // User cares
    Unloading    // Internal (transition)
};
```

### Solution: Layered Abstraction

**Internal (Developer/Debug)**: 5-state machine for async loading, transitions
**User API**: 3-state simplified interface

```cpp
// User-facing enum (simplified)
UENUM(BlueprintType)
enum class EStageState : uint8
{
    Unloaded,   // Not loaded
    Loaded,     // Loaded but not active
    Active      // Fully active
};

// User-facing API
UFUNCTION(BlueprintCallable, Category = "Stage")
void RequestStageState(EStageState DesiredState);

// Mapping:
// RequestStageState(Unloaded) → Internal: GotoState(Unloading) → Unloaded
// RequestStageState(Loaded)   → Internal: GotoState(Preloading) → Loaded
// RequestStageState(Active)   → Internal: GotoState(Active)
```

### API Comparison

| User Calls | Internal Transition |
|------------|---------------------|
| `LoadStage()` | Unloaded → Preloading → Loaded |
| `ActivateStage()` | Loaded → Active |
| `UnloadStage()` | Active/Loaded → Unloading → Unloaded |

### Convenience Functions

```cpp
// Simple user API
UFUNCTION(BlueprintCallable, Category = "Stage")
void LoadStage();      // Request Loaded state

UFUNCTION(BlueprintCallable, Category = "Stage")
void ActivateStage();  // Request Active state

UFUNCTION(BlueprintCallable, Category = "Stage")
void UnloadStage();    // Request Unloaded state

// Query (user-facing)
UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Stage")
EStageState GetStageState() const;  // Returns simplified 3-state

// Query (developer/debug)
UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Stage|Debug")
EStageRuntimeState GetStageRuntimeState() const;  // Returns internal 5-state
```

---

## Part 4: Implementation Tasks

### Priority Order

| # | Task | Description | Effort |
|---|------|-------------|--------|
| 1 | UTriggerZoneComponentBase | Base class with Description, events, auto-register | Medium |
| 2 | Refactor UStageTriggerZoneComponent | Inherit from base class | Low |
| 3 | ATriggerZoneActor | Convenience Actor inheriting AProp | Low |
| 4 | AStage Zone Registration API | RegisterTriggerZone, GetAllTriggerZones | Low |
| 5 | Stage State API Simplification | 3-state user API wrapping 5-state internal | Medium |
| 6 | Editor Visualization | Zone connection lines in viewport | Medium |
| 7 | M-003 Debug Extension | Zone info in Debug HUD | Low |

### File Changes

**New Files:**
- `StageEditorRuntime/Public/Components/TriggerZoneComponentBase.h`
- `StageEditorRuntime/Private/Components/TriggerZoneComponentBase.cpp`
- `StageEditorRuntime/Public/Actors/TriggerZoneActor.h`
- `StageEditorRuntime/Private/Actors/TriggerZoneActor.cpp`

**Modified Files:**
- `StageCoreTypes.h` - Add FTriggerZoneDescription, ETriggerZonePreset, EStageState
- `Stage.h/cpp` - Add Zone registration API, simplified state API
- `StageTriggerZoneComponent.h/cpp` - Inherit from TriggerZoneComponentBase

---

## Part 5: Blueprint Example Flow

```
// User's Blueprint in TriggerZoneActor subclass

Event OnActorEnter (AActor* Actor)
│
├─► Branch: Actor Has Tag "Player"?
│   │
│   ├─► NO: Return (ignore non-players)
│   │
│   └─► YES: Continue
│       │
│       ├─► [Optional] Check custom conditions
│       │   ├─ Has Key Item?
│       │   ├─ Quest Completed?
│       │   └─ etc.
│       │
│       └─► Call Stage API
│           ├─ Get Stage Reference (via SUID or direct ref)
│           ├─ LoadStage() / ActivateStage() / ActivateAct(ID)
│           └─ [Optional] Play feedback (sound, VFX)
```

---

## Part 6: Debug Visualization (Future)

### Debug HUD Display

```
Stage_01 (ID:1)
├─ State: Active (Internal: Active)
├─ TriggerZones (5):
│   ├─ BuiltIn_Load: 1 actor [Player enters → Load Stage]
│   ├─ BuiltIn_Activate: 1 actor [Player enters → Activate Stage]
│   ├─ Zone_East (on Door_01): 0 actors [Player + HasKey → ActivateAct(2)]
│   └─ Zone_West: disabled
└─ Last Triggered: Zone_East (2.3s ago)
```

### Editor Viewport

```
┌─────────────────────────────────────────┐
│              Viewport                   │
│                                         │
│    ┌─────────┐    [Description Box]    │
│    │ Zone_1  │───▶ "Player enters →    │
│    └────┬────┘     Load Stage_01"      │
│         │                               │
│         ▼                               │
│    ┌─────────┐                         │
│    │Stage_01 │                         │
│    └─────────┘                         │
│                                         │
│  [Select Zone to see connection lines] │
└─────────────────────────────────────────┘
```

---

## Appendix: Design Decision Log

| Decision | Choice | Rationale |
|----------|--------|-----------|
| Zone inherits AProp | Yes | Enables SUID indexing, Act/DataLayer management |
| Component auto-registers | Yes | Simplifies user workflow, ensures tracking |
| Hardcode Built-in Zone logic | Yes | 80% use case, can disable if needed |
| External Zone logic in BP | Yes | Maximum flexibility, no presets |
| Description template | Structured fields | Enables parsing for debug tools |
| Preset templates | 6 common patterns | Quick-fill for typical scenarios |
| State API layering | 3 user / 5 internal | Simplicity for users, precision for devs |

---

## Next Steps

1. Review this document with the team
2. Begin implementation starting with UTriggerZoneComponentBase
3. Write unit tests for state API mapping
4. Update M-003 Debug HUD design to include Zone info
