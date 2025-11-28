# Project Index: ExtendEditor

> Generated: 2025-11-27
> Token Efficiency: ~3K tokens vs ~58K full read (94% reduction)

---

## Project Overview

**ExtendEditor** is an Unreal Engine 5.6 project with custom editor plugins.

| Plugin | Purpose | Status |
|--------|---------|--------|
| **StageEditor** | Dynamic stage management using DataLayers | Active Development |
| SuperManager | Asset/actor management utilities | Stable |

**Current Branch:** `StageEditor-Dev-Phase-1`

---

## StageEditor Plugin Structure

### Modules

| Module | Type | Purpose |
|--------|------|---------|
| `StageEditorRuntime` | Runtime | Core data structures, state machine, runtime logic |
| `StageEditor` | Editor | UI, Controller, editor-only tools |

### Architecture: MVC Pattern

```
┌─────────────────────────────────────────────────────────────┐
│  View (SStageEditorPanel)                                   │
│  └─ Slate UI: Tree view with Stage → Acts → Props          │
├─────────────────────────────────────────────────────────────┤
│  Controller (FStageEditorController)                        │
│  └─ All Model modifications, transactions, delegates        │
├─────────────────────────────────────────────────────────────┤
│  Model (AStage + UStagePropComponent + FAct)               │
│  └─ Runtime data, state machine, DataLayer integration     │
└─────────────────────────────────────────────────────────────┘
```

---

## Core Source Files

### StageEditorRuntime (Runtime Module)

| File | Purpose |
|------|---------|
| `Actors/Stage.h` | Main Stage actor - orchestrates Acts, Props, TriggerZones |
| `Actors/Prop.h` | Legacy Prop actor (use StagePropComponent instead) |
| `Components/StagePropComponent.h` | Makes ANY actor controllable by Stage |
| `Components/StageTriggerZoneComponent.h` | Trigger zone for state transitions |
| `Core/StageCoreTypes.h` | FAct, FSUID, EStageRuntimeState, EStageTriggerZoneType |
| `Subsystems/StageManagerSubsystem.h` | WorldSubsystem for Stage registration/cross-Stage comms |
| `Debug/StageDebugHUD.h` | Runtime debug display |
| `Debug/StageDebugSettings.h` | Debug configuration (DeveloperSettings) |

### StageEditor (Editor Module)

| File | Purpose |
|------|---------|
| `EditorUI/StageEditorPanel.h` | Main panel UI (Slate) |
| `EditorLogic/StageEditorController.h` | MVC Controller - all model modifications |
| `CustomStyle/StageEditorStyle.h` | Custom Slate styling |
| `Subsystems/StageEditorSubsystem.h` | Editor-only helpers (minimal) |

---

## Key Types Reference

### Enums

```cpp
EStageRuntimeState { Unloaded, Preloading, Loaded, Active, Unloading }
EStageTriggerZoneType { LoadZone, ActivateZone }
```

### Structs

```cpp
FSUID { StageID, ActID }           // Unique identifier
FAct { SUID, DisplayName, AssociatedDataLayer, PropStateOverrides }
```

### Key Properties (AStage)

```cpp
TArray<FAct> Acts;                              // All Acts
TMap<int32, TSoftObjectPtr<AActor>> PropRegistry; // PropID → Actor
TArray<int32> ActiveActIDs;                     // Multi-Act activation
EStageRuntimeState CurrentStageState;           // State machine
UStageTriggerZoneComponent* BuiltInLoadZone;    // Outer trigger
UStageTriggerZoneComponent* BuiltInActivateZone; // Inner trigger
```

---

## Build & Compile

```bat
# Compile project
compile.bat

# Or directly:
"C:\Program Files\Epic Games\UE_5.6\Engine\Build\BatchFiles\Build.bat" ^
  ExtendEditorEditor Win64 Development ^
  -Project="D:\UEProject\ReserchPJ\ExtendEditor\ExtendEditor\ExtendEditor.uproject"
```

---

## Documentation Map

### Active Docs

| Document | Purpose |
|----------|---------|
| `CLAUDE.md` | AI assistant instructions, coding conventions |
| `Docs/StageEditor/TodoList_1127.md` | Current task list |
| `Docs/11.28/H-006_TriggerZone_Redesign_Handoff.md` | TriggerZone architecture redesign |
| `Docs/StageEditor/StageManagerSubsystem.md` | Subsystem API design |
| `Docs/StageEditor/StageEditorController.md` | Controller implementation |
| `Docs/StageEditor/BlueprintAPI/StageEditorBlueprintAPI.md` | Blueprint API |
| `Docs/StageEditor/ImplementationPlan/M-003_StageDebugHUD_Handoff.md` | Debug HUD design |

### Reference Guides

| Document | Purpose |
|----------|---------|
| `Docs/ClaudeGuides/DataLayer_WorkingPrinciple.md` | DataLayer system |
| `Docs/ClaudeGuides/WorldPartition_Enforcement_Guide.md` | WorldPartition |
| `Docs/Guides/SceneOutlinerDevGuide.md` | Scene Outliner customization |

---

## Current Development Tasks

| Priority | ID | Task | Status |
|----------|-----|------|--------|
| High | H-006 | External TriggerZone UX Design | Pending |
| Medium | M-003 | Stage Debug HUD Tool | Designing |
| Low | L-001 | Prop State Visual Preview | Pending |

---

## Key Patterns & Conventions

### Transaction Pattern (Undo/Redo)
```cpp
FScopedTransaction Transaction(NSLOCTEXT("StageEditor", "...", "..."));
StageActor->Modify();
// ... modifications ...
```

### Soft Object References
```cpp
TSoftObjectPtr<AActor>  // For cross-level references
```

### Editor-Only Code
```cpp
#if WITH_EDITOR
    // Editor-specific logic
#endif
```

---

## Quick Navigation

| Need to... | Go to... |
|------------|----------|
| Modify Stage data | `StageEditorController.h` |
| Add UI elements | `StageEditorPanel.h` |
| Change state machine | `Stage.cpp` (GotoState, OnEnterState) |
| Add new core types | `StageCoreTypes.h` |
| Configure debug display | `StageDebugSettings.h` |
| Cross-Stage communication | `StageManagerSubsystem.h` |

---

## File Count Summary

| Category | Count |
|----------|-------|
| Source (.cpp) | 12 (excluding generated) |
| Headers (.h) | 14 (excluding generated) |
| Documentation (.md) | 23 (active) |
| Build scripts (.Build.cs) | 2 |
