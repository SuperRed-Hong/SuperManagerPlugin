# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

ExtendEditor is an Unreal Engine 5.6 project containing custom editor plugins for level design and asset management. The primary plugin is **SuperManager** (asset/actor management utilities) with active development focused on the **StageEditor** plugin (dynamic stage management system using DataLayers).

Current branch: `StageEditor-Dev-Phase-1` (branched from `master`)

## Build Commands

### Compile Project
```bat
compile.bat
```
This invokes:
```bat
"C:\Program Files\Epic Games\UE_5.6\Engine\Build\BatchFiles\Build.bat" ExtendEditorEditor Win64 Development -Project="d:\UEProject\ReserchPJ\ExtendEditor\ExtendEditor\ExtendEditor.uproject" -WaitMutex
```

### Open Solution
```
ExtendEditor.sln
```

### Generate Project Files
If .uproject or plugin descriptors change, regenerate via:
- Right-click `ExtendEditor.uproject` → "Generate Visual Studio project files"

## Plugin Architecture

### StageEditor Plugin (Primary Focus)

**Purpose:** A dynamic stage system managing level streaming and state changes via DataLayers using a theatrical metaphor (Stages → Acts → Props).

**Modules:**
- `StageEditorRuntime` (Runtime) - Core data structures and runtime logic
- `StageEditor` (Editor) - Editor tools and UI

**Core Architecture (MVC Pattern):**

**Model (Runtime Data):**
- `AStage` (Plugins/StageEditor/Source/StageEditorRuntime/Public/Actors/Stage.h): "Director" actor that orchestrates scene states
  - Contains `Acts` (TArray<FAct>) - scene configurations
  - `PropRegistry` (TMap<int32, TSoftObjectPtr<AActor>>) - registered Props
  - `ActivateAct(int32 ActID)` - applies Act's PropStates and manages DataLayers
  - `RegisterProp()` / `UnregisterProp()` - Prop lifecycle management

- `UStagePropComponent` (Plugins/StageEditor/Source/StageEditorRuntime/Public/Components/StagePropComponent.h): Component making ANY Actor controllable
  - `PropID` (int32) - unique ID within owning Stage
  - `PropState` (int32) - current state value (bridge between Stage and visual behavior)
  - `OnPropStateChanged` - Blueprint event for state interpretation
  - Can be added to any Actor (not just AProp)

- `FAct` (Core/StageCoreTypes.h): Scene state configuration struct
  - `ActID`, `DisplayName`, `AssociatedDataLayer`
  - `PropStateOverrides` (TMap<int32, int32>) - maps PropID to target state values

**View (Slate UI):**
- `SStageEditorPanel` (Plugins/StageEditor/Source/StageEditor/Public/EditorUI/StageEditorPanel.h): Main UI with hierarchical tree view
  - Tree structure: Stage → Acts Folder → Act N → Props (with states)
  - Drag & drop Props into Acts
  - Selection sync with viewport
  - Context menus for operations

**Controller:**
- `FStageEditorController` (Plugins/StageEditor/Source/StageEditor/Public/EditorLogic/StageEditorController.h): Single bridge between View and Model
  - All modifications wrapped in `FScopedTransaction` for Undo/Redo
  - Stage: `FindStageInWorld()`, `SetActiveStage()`
  - Acts: `CreateNewAct()`, `DeleteAct()`, `PreviewAct()`
  - Props: `RegisterProps()`, `SetPropStateInAct()`, `UnregisterProp()`
  - DataLayers: `CreateDataLayerForAct()`, `AssignDataLayerToAct()`, `SyncPropToDataLayer()`
  - Broadcasts `OnModelChanged` delegate for reactive UI updates
  - Listens to `OnLevelActorAdded/Deleted` for auto-refresh

**Key Design Decisions:**
1. **Component-Based Prop System:** `UStagePropComponent` can be added to ANY Actor; Controller auto-injects if missing during registration
2. **Soft References:** `PropRegistry` uses `TSoftObjectPtr<AActor>` to prevent hard references and support level streaming
3. **Default Act Pattern:** Act 0 is reserved "Default Act" (created in constructor, cannot be deleted)
4. **MVC Strict Separation:** No direct View→Model communication; all via Controller
5. **Transaction Support:** Full Undo/Redo via `FScopedTransaction` wrapping all modifications
6. **DataLayer Integration:** Each Act can have `AssociatedDataLayer` for dynamic resource streaming
7. **Editor-Only Logic:** Runtime module has ZERO editor dependencies; all editor logic in separate module

**State Management Flow:**
1. Stage activates Act
2. Stage reads `PropStateOverrides` from Act
3. Stage calls `SetPropState()` on each affected Prop
4. Prop broadcasts `OnPropStateChanged` delegate
5. Blueprint logic interprets state value (data-driven behavior)

**Documentation:**
- Design docs (Chinese): `Docs/StageEditor/` - comprehensive design documentation including:
  - `HighLevelDesign.md` - Overall architecture (MVC pattern, ID system design)
  - `StageEditorController.md` - Controller implementation details
  - `SStageEditorPanel与TreeView详细设计文档.md` - UI implementation
  - ID allocation service design (future feature - centralized web API for multi-user)

### SuperManager Plugin

**Purpose:** Editor utilities for asset and actor management.

**Module:** `SuperManager` (Editor, PreDefault loading phase)

**Key Features:**
- **Advanced Deletion Widget:** Bulk asset deletion with filters
- **Locked Actors System:** Lock/unlock actor selection in outliner
  - Custom Scene Outliner column for lock state visualization
  - Hotkey support for lock/unlock operations
  - Dedicated tab (`SLockedActorsListTab`) showing all locked actors
  - Undo/Redo transaction support
- **Todo List Widget:** In-editor task management
- **Quick Asset Actions:** Material creation helpers, asset utilities
- **Content Browser Extensions:** Delete unused assets, fix redirectors, delete empty folders

**Architecture:**
- `FSuperManagerModule` (Plugins/SuperManager/Source/SuperManager/Public/SuperManager.h): Main module class
  - Content Browser menu extensions
  - Custom editor tabs (Advanced Deletion, Locked Actors, Todo List)
  - Level editor menu extensions
  - Scene Outliner column extension
  - Custom UI commands/hotkeys
  - Actor lock state management with transaction support

**Style System:**
- Centralized style registry: `FSuperManagerStyleRegistry`
- Style set name: `SuperManager.Style`
- Naming pattern: `SuperManager.Style.[Area].[Purpose]` (e.g., `SuperManager.Style.Card.Primary`)
- All colors/brushes declared in `FSuperManagerPalette::CreateDefault`
- Extension guide: `Docs/README.md` (Chinese)

**Dependencies:**
- Requires `Niagara` and `EditorScriptingUtilities` plugins

## Coding Conventions

### File References
When referencing code locations, use the pattern `file_path:line_number` for navigation.

### Module Dependencies
**StageEditorRuntime.Build.cs:**
- Core, CoreUObject, Engine, Slate, SlateCore
- NO editor dependencies (runtime-safe)

**StageEditor.Build.cs:**
- Depends on: `StageEditorRuntime` (critical dependency)
- Editor modules: UnrealEd, ToolMenus, EditorStyle, PropertyEditor, DataLayerEditor

**SuperManager.Build.cs:**
- Editor-only module: UnrealEd, Slate, SlateCore, EditorStyle, ToolMenus, etc.

### Transaction Pattern (Undo/Redo)
All editor modifications must be wrapped in `FScopedTransaction`:
```cpp
FScopedTransaction Transaction(NSLOCTEXT("StageEditor", "CreateAct", "Create New Act"));
StageActor->Modify();
// ... perform modifications ...
```

### Soft Object References
Use `TSoftObjectPtr<T>` for cross-level references to support level streaming and avoid hard dependencies.

### Editor-Only Code
Use `#if WITH_EDITOR` blocks to isolate editor-specific logic in runtime modules:
```cpp
#if WITH_EDITOR
    // Editor-only refresh logic
#endif
```

## Documentation Practices

The project maintains extensive Chinese-language documentation in `Docs/`:
- `Docs/StageEditor/` - Design documentation for StageEditor
- `Docs/Logs/` - Development logs with timestamps (format: `YYYYMMDD_description.md`)
- `Docs/Guides/` - Developer guides (e.g., `SceneOutlinerDevGuide.md`)
- `Docs/README.md` - SuperManager style extension guide

When making architectural changes, reference existing design docs before modifying.

## Git Workflow

**Main Branch:** `master`
**Current Development Branch:** `StageEditor-Dev-Phase-1`

Use descriptive commit messages following the pattern:
```
<type>: <description>

Examples:
feat: Add new StageEditor selection sync feature
fix: Resolve PropState update race condition
refactor: Clean up StageEditorController transaction handling
```

## Current Development Phase

**StageEditor Phase 1 Status:** Core architecture functional
- ✅ Runtime data structures implemented
- ✅ Editor UI and Controller operational
- ✅ DataLayer integration working
- ✅ Prop registration and state management functional
- ✅ Transaction support for Undo/Redo
- ✅ Viewport selection sync

**Future Phases (Documented but Not Implemented):**
- Centralized ID allocation service (web API for multi-user collaboration)
- Advanced state snapshot system
- Dynamic Prop spawning/despawning
- Cross-Stage communication system
