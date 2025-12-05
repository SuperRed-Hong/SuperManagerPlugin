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

**Purpose:** A dynamic stage system managing level streaming and state changes via DataLayers using a theatrical metaphor (Stages → Acts → Entities).

**Note:** Previously called "Stage-Act-Prop" system, renamed to "Stage-Act-Entity" in Phase 14.5 (2025-12-05) to better reflect that it manages all types of game objects, not just props.

**Modules:**
- `StageEditorRuntime` (Runtime) - Core data structures and runtime logic
- `StageEditor` (Editor) - Editor tools and UI

**Core Architecture (MVC Pattern):**

**Model (Runtime Data):**
- `AStage` (Plugins/StageEditor/Source/StageEditorRuntime/Public/Actors/Stage.h): "Director" actor that orchestrates scene states
  - Contains `Acts` (TArray<FAct>) - scene configurations
  - `EntityRegistry` (TMap<int32, TSoftObjectPtr<AActor>>) - registered Entities
  - `ActivateAct(int32 ActID)` - applies Act's EntityStates and manages DataLayers
  - `RegisterEntity()` / `UnregisterEntity()` - Entity lifecycle management

- `UStageEntityComponent` (Plugins/StageEditor/Source/StageEditorRuntime/Public/Components/StageEntityComponent.h): Component making ANY Actor controllable
  - `SUID` (FSUID) - Stage Unique ID (contains StageID + EntityID)
  - `EntityState` (int32) - current state value (bridge between Stage and visual behavior)
  - `OwnerStage` (TSoftObjectPtr<AStage>) - soft reference to owning Stage
  - `OnEntityStateChanged` - Blueprint event for state interpretation
  - `IsOrphaned()` - checks if OwnerStage was deleted (Phase 15)
  - `ClearOrphanedState()` - resets orphaned Entity to unregistered state (Phase 15)
  - Can be added to any Actor (not just AStageEntity)
  - **Safety:** Prevents adding to Stage actors (would create nested Stage - dangerous)

- `FAct` (Core/StageCoreTypes.h): Scene state configuration struct
  - `ActID`, `DisplayName`, `AssociatedDataLayer`
  - `EntityStateOverrides` (TMap<int32, int32>) - maps EntityID to target state values

**View (Slate UI):**
- `SStageEditorPanel` (Plugins/StageEditor/Source/StageEditor/Public/EditorUI/StageEditorPanel.h): Main UI with hierarchical tree view
  - Tree structure: Stage → Acts Folder → Act N → Entities (with states)
  - Drag & drop Entities into Acts
  - Selection sync with viewport
  - Context menus for operations
  - **Phase 15 UI Enhancements:**
    - "Clean Orphaned" button in toolbar - batch clean orphaned Entities
    - "Delete Stage" button in Stage rows - explicit deletion with confirmation dialog

**Controller:**
- `FStageEditorController` (Plugins/StageEditor/Source/StageEditor/Public/EditorLogic/StageEditorController.h): Single bridge between View and Model
  - All modifications wrapped in `FScopedTransaction` for Undo/Redo
  - **Stage Management:**
    - `FindStageInWorld()`, `SetActiveStage()`
    - `DeleteStageWithConfirmation()` - explicit deletion with dialog (Phase 15)
    - `DeleteStage(bDeleteDataLayers)` - actual deletion logic (Phase 15)
  - **Acts:** `CreateNewAct()`, `DeleteAct()`, `PreviewAct()`
  - **Entities:**
    - `RegisterEntities()` - enforces single-Stage constraint (Phase 15)
    - `SetEntityStateInAct()`, `UnregisterEntity()`
    - `CleanOrphanedEntities()` - batch cleanup utility (Phase 15)
    - `IsEntityRegisteredToOtherStage()` - conflict detection (Phase 15)
  - **DataLayers:** `CreateDataLayerForAct()`, `AssignDataLayerToAct()`, `SyncEntityToDataLayer()`
  - **Delegates:**
    - Broadcasts `OnModelChanged` for reactive UI updates
    - Broadcasts `OnDataLayerRenamed` for rename sync (Phase 12)
    - Listens to `OnLevelActorAdded/Deleted` for auto-refresh

**Key Design Decisions:**
1. **Component-Based Entity System:** `UStageEntityComponent` can be added to ANY Actor; Controller auto-injects if missing during registration
2. **Soft References:** `EntityRegistry` uses `TSoftObjectPtr<AActor>` to prevent hard references and support level streaming
3. **Default Act Pattern:** Act 0 is reserved "Default Act" (created in constructor, cannot be deleted)
4. **MVC Strict Separation:** No direct View→Model communication; all via Controller
5. **Transaction Support:** Full Undo/Redo via `FScopedTransaction` wrapping all modifications
6. **DataLayer Integration:** Each Act can have `AssociatedDataLayer` for dynamic resource streaming
7. **Editor-Only Logic:** Runtime module has ZERO editor dependencies; all editor logic in separate module
8. **Single-Stage Constraint:** One Entity can only be registered to one Stage at a time (Phase 15)
9. **Orphaned Entity Detection:** Entities track OwnerStage validity; provide cleanup utilities (Phase 15)
10. **Explicit Stage Deletion:** Stage deletion requires user confirmation via UI button, not automatic (Phase 15)

**State Management Flow:**
1. Stage activates Act
2. Stage reads `EntityStateOverrides` from Act
3. Stage calls `SetEntityState()` on each affected Entity
4. Entity broadcasts `OnEntityStateChanged` delegate
5. Blueprint logic interprets state value (data-driven behavior)

**Safety Mechanisms (Phase 15):**
1. **Orphaned Entity Protection:**
   - Entities detect when OwnerStage is deleted
   - Batch cleanup via "Clean Orphaned" UI button
   - Manual reset to unregistered state
2. **Registration Conflict Prevention:**
   - Detects if Entity already registered to another Stage
   - Shows user dialog: move or skip
   - All operations transactional (Undo support)
3. **Blueprint Reconstruction Safety:**
   - Checks `GIsReconstructingBlueprintInstances` flag
   - Prevents unwanted unregistration during BP compile

**Documentation:**
- Design docs (Chinese): `Docs/StageEditor/` - comprehensive design documentation including:
  - `Overview.md` - Development progress overview with Phase index
  - `HighLevelDesign.md` - Overall architecture (MVC pattern, ID system design)
  - `StageEditorController.md` - Controller implementation details
  - `SStageEditorPanel与TreeView详细设计文档.md` - UI implementation
  - `EditorFeatures/Phase15_EntityManagement_SafetyEnhancements.md` - Latest safety features (Phase 15)
  - `Refactoring/PropToEntity_RenamingPlan.md` - Phase 14.5 renaming details
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

**StageEditor Phase 15 Status:** Entity management safety enhancements complete
- ✅ Phase 1-14: Core architecture, DataLayer integration, Import system
- ✅ Phase 14.5: Prop → Entity architecture renaming (2025-12-05)
- ✅ Phase 15: Entity management safety enhancements (2025-12-05)
  - Orphaned Entity detection and cleanup
  - Single-Stage registration constraint
  - Explicit Stage deletion with confirmation
  - Complete Undo/Redo transaction support
  - Blueprint reconstruction safety
- ✅ Runtime data structures implemented
- ✅ Editor UI and Controller operational
- ✅ DataLayer integration working
- ✅ Entity registration and state management functional
- ✅ Transaction support for Undo/Redo
- ✅ Viewport selection sync
- ✅ Import from existing DataLayers
- ✅ Blueprint class support for Stages and Entities

**Current Status:**
- Phase 13 (StageRegistry persistence) in design review
- Phase 15 safety enhancements completed and compiled successfully

**Future Phases (Documented but Not Implemented):**
- Phase 13 implementation (StageRegistry persistence architecture)
- Centralized ID allocation service (web API for multi-user collaboration)
- Advanced state snapshot system
- Dynamic Entity spawning/despawning
- Cross-Stage communication system
