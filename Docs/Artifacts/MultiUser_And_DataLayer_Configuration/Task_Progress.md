# Task List: MultiUser Collaboration Fixes

## Phase 1: Stage DataLayer Asset Reference
- [x] Add `StageDataLayerAsset` to `AStage`
- [x] Update `StageDataLayerName` visibility
- [x] Implement `PostEditChangeProperty` for auto-sync
- [x] Fix compilation errors in `StageEditorPanel.cpp`
- [x] Verify compilation

## Phase 2: Controller Logic Migration
- [x] Rewrite `FindStageDataLayerInstance` to use Asset reference
- [x] Update `AssignPropToStageDataLayer` to use Asset lookup
- [x] Update `RemovePropFromStageDataLayer` to use Asset lookup
- [x] Migrate all `StageDataLayerName` usages

## Phase 3: DataLayer Creation Uniqueness
- [ ] Add conflict detection to `CreateDataLayerAsset`
- [ ] Implement auto-increment suffix for duplicates
- [ ] Update `CreateDataLayerForStage`
- [ ] Update `CreateDataLayerForAct`

## Phase 4: UI Display Name Handling
- [ ] Detect duplicate Stage names in `RefreshUI`
- [ ] Add GUID suffix for duplicates
- [ ] Implement similar logic for Props
- [ ] Add Tooltips

## Phase 5: Backward Compatibility
- [ ] Add migration logic for old data
- [ ] Add editor warnings

## Phase 6: Testing
- [ ] Single user test
- [ ] Multi-user simulation test

