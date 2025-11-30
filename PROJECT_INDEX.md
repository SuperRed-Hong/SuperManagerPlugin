# Project Index: ExtendEditor

**Generated:** 2025-11-29
**Engine:** Unreal Engine 5.6
**Platform:** Windows (Win64)

---

## Project Structure

```
ExtendEditor/
├── Source/
│   └── ExtendEditor/              # Main game module (Runtime)
├── Plugins/
│   ├── StageEditor/               # Primary focus - Stage management system
│   │   ├── Source/StageEditorRuntime/  # Runtime module (NO editor deps)
│   │   └── Source/StageEditor/         # Editor module
│   ├── SuperManager/              # Asset/actor management utilities
│   ├── SwitchLanguage/            # Language switching plugin
│   ├── StageEditorModeTest/       # Testing plugin
│   └── Developer/RiderLink/       # JetBrains Rider integration (third-party)
├── Docs/                          # Project documentation (Chinese)
│   ├── StageEditor/               # StageEditor design docs
│   ├── Logs/                      # Development logs
│   ├── ClaudeGuides/              # AI assistant guides
│   └── CLAUDE.md                  # Main AI assistant context
└── compile.bat                    # Build script
```

---

## Entry Points

| Type | Path | Description |
|------|------|-------------|
| Project | `ExtendEditor.uproject` | UE5.6 project file |
| Build Script | `compile.bat` | Invokes UE Build.bat |
| Solution | `ExtendEditor.sln` | Visual Studio solution |

---

## Core Plugins

### StageEditor Plugin (Primary Focus)

**Purpose:** Dynamic stage management system using DataLayers with theatrical metaphor (Stage → Acts → Props)

**Modules:**
- `StageEditorRuntime` (Runtime) - Core data structures, NO editor dependencies
- `StageEditor` (Editor) - UI and editor tools

**Architecture:** MVC Pattern

| Layer | Class | File | Purpose |
|-------|-------|------|---------|
| Model | `AStage` | `StageEditorRuntime/Public/Actors/Stage.h` | "Director" - orchestrates Acts and Props |
| Model | `FAct` | `StageEditorRuntime/Public/Core/StageCoreTypes.h` | Scene configuration struct |
| Model | `UStagePropComponent` | `StageEditorRuntime/Public/Components/StagePropComponent.h` | Makes ANY actor controllable |
| Model | `FSUID` | `StageEditorRuntime/Public/Core/StageCoreTypes.h` | Stage Unique ID (StageID.ActID.PropID) |
| View | `SStageEditorPanel` | `StageEditor/Public/EditorUI/StageEditorPanel.h` | Main editor UI with tree view |
| Controller | `FStageEditorController` | `StageEditor/Public/EditorLogic/StageEditorController.h` | Bridge between View and Model |
| Subsystem | `UStageManagerSubsystem` | `StageEditorRuntime/Public/Subsystems/StageManagerSubsystem.h` | Stage registration, cross-stage communication |

**Key Features:**
- DataLayer integration for level streaming
- Multi-Act activation with priority system
- TriggerZone system with preset actions
- 5-state internal / 3-state user API
- Full Undo/Redo transaction support
- Viewport selection sync

**State Flow:**
```
User API:    Unloaded ←→ Loaded ←→ Active
Internal:    Unloaded → Preloading → Loaded → Active
                  ↑                              ↓
                  └── Unloading ←────────────────┘
```

---

### SuperManager Plugin

**Purpose:** Editor utilities for asset and actor management

**Module:** `SuperManager` (Editor, PreDefault loading)

| Component | File | Purpose |
|-----------|------|---------|
| Module | `SuperManager/Public/SuperManager.h` | Main module class |
| Widget | `SlateWidgets/AdvancedDeletionWidget.*` | Bulk asset deletion |
| Widget | `SlateWidgets/LockedActorsListWidget.*` | Actor lock system |
| Widget | `SlateWidgets/TodoListWidget.*` | In-editor task list |
| Style | `CustomStyle/SuperManagerStyle.*` | Centralized style registry |
| Column | `CustomOutlinerColumn/OutlinerSelectionColumn.*` | Scene Outliner extension |

**Key Features:**
- Locked Actors system with hotkeys
- Advanced Deletion with filters
- Quick Asset Actions (material creation)
- Content Browser extensions

---

## Configuration

| File | Purpose |
|------|---------|
| `ExtendEditor.uproject` | Project settings, plugin list |
| `StageEditor.uplugin` | StageEditor plugin config |
| `SuperManager.uplugin` | SuperManager plugin config |
| `AGENTS.md` | AI assistant development guidelines |
| `Docs/CLAUDE.md` | AI assistant context file |

---

## Documentation

| Path | Topic |
|------|-------|
| `Docs/CLAUDE.md` | Primary AI assistant context |
| `Docs/StageEditor/API_Reference.md` | StageEditor API docs |
| `Docs/StageEditor/API_Reference_CN.md` | Chinese API reference |
| `Docs/StageEditor/ImplementationPlan/` | Development task lists |
| `Docs/StageEditor/FeatureSpecs/` | Feature specifications |
| `Docs/StageEditor/Obslete/` | Archived design documents |
| `Docs/Logs/` | Development logs (YYYYMMDD_*.md) |
| `Docs/Guides/` | Developer guides |

---

## Key Dependencies

**StageEditorRuntime:**
- Core, CoreUObject, Engine, Slate, SlateCore
- WorldPartition (DataLayer)
- NO editor dependencies (runtime-safe)

**StageEditor:**
- StageEditorRuntime (critical)
- UnrealEd, ToolMenus, EditorStyle, PropertyEditor
- DataLayerEditor

**SuperManager:**
- Niagara, EditorScriptingUtilities
- UnrealEd, Slate, ToolMenus

---

## Build & Run

```bat
# Compile project
compile.bat

# Or manually:
"C:\Program Files\Epic Games\UE_5.6\Engine\Build\BatchFiles\Build.bat" ExtendEditorEditor Win64 Development -Project="D:\UEProject\ReserchPJ\ExtendEditor\ExtendEditor\ExtendEditor.uproject" -WaitMutex
```

---

## Development Conventions

1. **Transaction Pattern:** All editor modifications wrapped in `FScopedTransaction`
2. **Soft References:** Use `TSoftObjectPtr<T>` for cross-level references
3. **Editor-Only Code:** Use `#if WITH_EDITOR` blocks in runtime modules
4. **UE Naming:** F (Structs), U (UObjects), A (Actors), S (Slate), E (Enums)
5. **Documentation:** Chinese docs in `Docs/`, logs with timestamp prefix

---

## Current Development Status

**Phase:** StageEditor Phase 1 (Core Architecture)

| Feature | Status |
|---------|--------|
| Runtime data structures | ✅ Complete |
| Editor UI and Controller | ✅ Complete |
| DataLayer integration | ✅ Complete |
| Prop registration | ✅ Complete |
| TriggerZone system | ✅ Complete |
| Debug HUD (basic) | ✅ Complete |
| Debug HUD Zone extension | 🔴 Pending |
| Editor zone visualization | 🔴 Pending |

---

## Quick Reference

### StageEditor Class Hierarchy
```
AStage (Actor)
├── Acts (TArray<FAct>)
├── PropRegistry (TMap<int32, TSoftObjectPtr<AActor>>)
├── BuiltInLoadZone (UStageTriggerZoneComponent)
├── BuiltInActivateZone (UStageTriggerZoneComponent)
└── StageDataLayerAsset (UDataLayerAsset)

UStagePropComponent (Component)
├── PropID (int32)
├── PropState (int32)
└── OnPropStateChanged (Delegate)

UTriggerZoneComponentBase (Component)
└── ATriggerZoneActor : AProp (Convenience Actor)
```

### Key APIs
```cpp
// Stage State (User)
Stage->GotoState(EStageState::Active);
Stage->LoadStage() / ActivateStage() / UnloadStage();
EStageState State = Stage->GetStageState();

// Act Control
Stage->ActivateAct(ActID);
Stage->DeactivateAct(ActID);
TArray<int32> ActiveActs = Stage->GetActiveActIDs();

// Prop Control
Stage->SetPropStateByID(PropID, NewState);
int32 State = Stage->GetPropStateByID(PropID);
AActor* Prop = Stage->GetPropActorByID(PropID);
```

---

*Index Size: ~5KB | Last Updated: 2025-11-29*
