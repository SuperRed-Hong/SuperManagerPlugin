# Stage Editor Plugin - Context Summary for Migration

## 📅 Date: 2025-11-20
## 🎯 Objective
Develop the "Stage Editor" Unreal Engine plugin, a tool for managing stage performances with Acts and Props.

## 🚀 Current Status: Phase 1 (Prototype) - In Progress
We are currently in **Phase 1**, focusing on the core "Stage-Act-Prop" loop and the Editor Panel UI.

### ✅ Recently Completed Achievements (High Priority P0/P1)
1.  **UI Refactor (TreeView)**:
    -   Replaced flat `SListView` with hierarchical `STreeView` in `SStageEditorPanel`.
    -   Implemented "Acts" and "Registered Props" folder structure.
2.  **Default Act Logic**:
    -   `AStage` now automatically creates a "Default Act" (ID 0) on construction.
    -   `RegisterProp` automatically adds new Props to the Default Act.
3.  **Quick Create Features**:
    -   Added "Create Stage BP" and "Create Prop BP" buttons to the panel toolbar.
4.  **DebugHeader Integration**:
    -   Integrated `DebugHeader.h` for consistent notifications.
5.  **Button Logic & Refactor**:
    -   **Refactored `AStage`**: Now supports generic `AActor` with `UStagePropComponent` (removed `AProp` dependency).
    -   **"Register Selected Props"**: Connected to selection and detects `UStagePropComponent`.
    -   **"Create Act"**: Fully implemented and connected to Controller.

### 🚧 Pending Tasks (Immediate Next Steps)
1.  **Prop State Preview**:
    -   Implement logic to preview Prop states in the editor viewport.

## 📂 Key Artifacts (Exported to `Docs/Artifacts/`)
-   **`task.md`**: The master checklist. **Use this to track progress.**
-   **`detailed_specification.md`**: The Single Source of Truth (SSOT) for design requirements.
-   **`implementation_plan.md`**: Detailed technical steps for Phase 1, 2, and 3.
-   **`walkthrough.md`**: Guide for testing the current implementation.

## 🔗 Critical Code Locations
-   **Panel UI**: `Plugins/StageEditor/Source/StageEditor/Private/EditorUI/StageEditorPanel.cpp`
-   **Controller**: `Plugins/StageEditor/Source/StageEditor/Private/EditorLogic/StageEditorController.cpp`
-   **Runtime Stage**: `Plugins/StageEditor/Source/StageEditorRuntime/Private/Actors/Stage.cpp`

## 📝 Instructions for Next Session
1.  **Load `task.md`** to see the current state.
2.  **Resume Phase 1**: Start with implementing the logic for "Register Selected Props" and "Create Act" buttons.
3.  **Verify**: Ensure the TreeView updates correctly when these actions are performed.
