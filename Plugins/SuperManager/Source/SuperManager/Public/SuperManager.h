// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Modules/ModuleManager.h"

// Test comment to verify Git diff output.
#include "SuperManagerEnums.h"
#include "Subsystems/EditorActorSubsystem.h"
#include "Containers/Set.h"
#include "Editor/Transactor.h"

class SLockedActorsListTab;

struct FLockedActorListItem;


class FSuperManagerModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

private:

#pragma region ContentBrowserMenuExtention
	TArray<FString> SelectedFolderPath;
	void InitCBMenuExtension();
	TSharedRef<FExtender> CustomCBMenuExtender( const TArray<FString>& SelectedPaths);
	void AddCBMenuEntry( class FMenuBuilder& MenuBuilder);
	void OnDeleteUnusedAssetButtonClicked();
	static void FixUpRedirectors();
	void OnDeleteEmptyFoldersButtonClicked();
	void OnAdvancedDelectionButtonClicked();

#pragma endregion

	#pragma region CustomEditorTab
		void RegisterAdvancedDeletionTab();
		void RegisterLockedActorsTab();
		void RegisterTodoListTab();
		TSharedRef<SDockTab> OnSpawnAdvancedDeletionTab(const FSpawnTabArgs& SpawnTabArgs);
		TSharedRef<SDockTab> OnSpawnLockedActorsTab(const FSpawnTabArgs& SpawnTabArgs);
		TSharedRef<SDockTab> OnSpawnTodoListTab(const FSpawnTabArgs& SpawnTabArgs);
	
		TArray< TSharedPtr<FAssetData> > GetAllAssetDataUnderSelectedFolder();
		TArray<TSharedPtr<FLockedActorListItem>> GatherLockedActorsListItems();
		void HandleSetActorLockState(TWeakObjectPtr<AActor> ActorPtr, bool bShouldLock);
		void HandleLockedActorRowDoubleClicked(TWeakObjectPtr<AActor> ActorPtr);
		void HighlightLockedActorRow(TWeakObjectPtr<AActor> ActorPtr);
		void RefreshLockedActorsWidget();
		TWeakPtr<SLockedActorsListTab> LockedActorsListWidget;
		
	#pragma endregion

#pragma region LevelEditorMenuExtention

	void InitLevelEditorMenuExtension();
	void OnLockSelectionButtonClicked();
	void OnUnLockSelectionButtonClicked();
	void PopulateLevelActorSubMenu(UToolMenu* InMenu);
	TSharedRef<FExtender> CreateLEMenuExtender(const TSharedRef<FUICommandList> UICommandList, const TArray<AActor*> SelectedActors);


#pragma endregion

#pragma region Lock Selections

	void InitCustomSelectionEvent();

	void OnActorSelected(UObject* SelectedObject);
	void LockActorSelection(AActor* ActorToProcess);
	void UnlockActorSelection(AActor* ActorToProcess);
	void HandleUndoRedo();
	void HandleTransactionEvent(UObject* TransactedObject, const FTransactionObjectEvent& TransactionEvent);
	static bool CheckIsActorSelectionLocked(const AActor* ActorToProcess);
	void CacheLockedActor(AActor* ActorToCache);
	void RemoveActorFromLockedCache(AActor* ActorToRemove);
	void CompactLockedActorCache();
	void RefreshLockedActorCacheSnapshot();
TSet<TWeakObjectPtr<AActor>> CachedLockedActors;
#pragma endregion


#pragma region CustomUICommands

	TSharedPtr<FUICommandList> CustomUICommandList;

	void InitCustomUICommands();
		void OnLockActorSelectionHotKeyPressed();
		void OnUnlockActorSelectionHotKeyPressed();
		void OnOpenLockedActorsListCommand();
		void OnOpenTodoListCommand();

#pragma endregion


#pragma region Helper Functions
	TWeakObjectPtr< UEditorActorSubsystem> WeakEditorActorSubsystem;
	bool GetEditorActorSubsystem();
	void EnsureRedirectorsFixed();
	bool bRedirectorsUpToDate = false;
#pragma endregion
	
public:

#pragma region ProccessDataForAdvancedDeletionTab

	bool DeleteSingleAssetForAssetList(const FAssetData& AssetDataToDelete);
	bool DeleteAssetsForAssetList(const TArray<FAssetData>& AssetsDataToDelete);
	static void UpdateDisplayedData(TArray<TSharedPtr<FAssetData>>& SourceAssetsDataArray, TArray<TSharedPtr<FAssetData>>& Out_DisplayedAssetsDataArray, EComboBoxOptions
	                                ComboBoxOption);
	static void SyncCBToClickedAssetForAssetList(const FString& SelectedPath);
	void ForceFixUpRedirectors();
#pragma endregion

private:
	// [002] 统一封装同步删除逻辑，便于复用事务描述。
	bool DeleteAssetsInternal(const TArray<FAssetData>& AssetsDataToDelete, const FText& TransactionText, bool bShowProgress);
};
