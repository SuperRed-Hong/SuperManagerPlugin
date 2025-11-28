# Stage Editor - Phase 1 Verification Walkthrough

## 🎯 Objective
Verify the implementation of "Register Selected Props" and "Create Act" buttons, and ensure the new generic Actor support works.

## 🧪 Test Steps

### 1. Setup
1.  Compile the project.
2.  Open the Unreal Editor.
3.  Open the **Stage Editor** tab (Window -> Stage Editor).

### 2. Verify "Create Act"
1.  Create a new `BP_Stage` (or use an existing one) and open it.
2.  In the Stage Editor Panel, click the **"Create Act"** button.
3.  **Expected Result**:
    -   A new Act (e.g., "Act_1") appears in the "Acts" folder in the Tree View.
    -   A notification "Created new Act: Act_1" appears.

### 3. Verify "Register Selected Props" (Generic Actor Support)
1.  Create a generic Actor Blueprint (e.g., `BP_TestProp`) or use a Cube in the level.
2.  Add the `StagePropComponent` to this Actor.
3.  Place an instance of this Actor in the level.
4.  Select the Actor in the viewport.
5.  In the Stage Editor Panel, click **"Register Selected Props"**.
6.  **Expected Result**:
    -   The Actor appears in the "Registered Props" folder.
    -   The Actor appears under "Default Act" (and other Acts) in the Tree View.
    -   A notification "Registered 1 Props" appears.

### 4. Verify Data Persistence
1.  Save the Stage asset.
2.  Close and reopen the Stage Editor.
3.  **Expected Result**: The created Acts and registered Props are still visible.

## 🛠️ Key Changes
-   **Refactored `AStage`**: Now supports registering *any* `AActor` that has a `UStagePropComponent`. It no longer requires the actor to inherit from `AProp`.
-   **Updated Controller**: `RegisterProps` now scans for `UStagePropComponent` on selected actors.
