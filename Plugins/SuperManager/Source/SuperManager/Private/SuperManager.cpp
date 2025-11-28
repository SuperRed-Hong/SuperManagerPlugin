// Copyright Epic Games, Inc. All Rights Reserved.

#include "SuperManager.h"
#include "EditorAssetLibrary.h"
#include "Editor.h"
#include "UObject/UObjectGlobals.h"
#include "Editor/Transactor.h"
#include "ContentBrowserModule.h"
#include "DebugHeader.h"
#include "ObjectTools.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetViewUtils.h"
#include "AssetToolsModule.h"
#include "SlateWidgets/AdvancedDeletionWidget.h"
#include "SlateWidgets/LockedActorsListWidget.h"
#include "SlateWidgets/TodoListWidget.h"
#include "CustomStyle/SuperManagerStyle.h"
#include "CustomUICommands/SuperManagerUICommands.h"
#include "SceneOutlinerModule.h"
#include "Framework/Application/SlateApplication.h"
#include "Widgets/Docking/SDockTab.h"
#include "Framework/Docking/TabManager.h"
#include "IContentBrowserSingleton.h"
#include "LevelEditor.h"
#include "Engine/Selection.h"
#include "Framework/Docking/TabManager.h" // 包含 TabManager
#include "Modules/ModuleManager.h" // 包含 ModuleManager
#include "Subsystems/EditorActorSubsystem.h"
#include "ToolMenus.h"
#include "ToolMenuEntry.h"
#include "Trace/Trace.inl"
#include "ScopedTransaction.h"
#include "CustomOutlinerColumn/OutlinerSelectionColumn.h"
#define LOCTEXT_NAMESPACE "FSuperManagerModule"

void FSuperManagerModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	FSuperManagerStyleSetRegistry::Initialize();
	FSuperManagerUICommands::Register();
	InitCustomUICommands();
	InitCBMenuExtension();
	RegisterAdvancedDeletionTab();
	RegisterLockedActorsTab();
	RegisterTodoListTab();
	InitLevelEditorMenuExtension();
	InitCustomSelectionEvent();
	InitSceneOutlinerColumnExtension();
	FEditorDelegates::PostUndoRedo.AddRaw(this, &FSuperManagerModule::HandleUndoRedo);
	FCoreUObjectDelegates::OnObjectTransacted.AddRaw(this, &FSuperManagerModule::HandleTransactionEvent);
}

void FSuperManagerModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(FName("AdvancedDeletion"));
	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(FName("LockedActorsList"));
	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(FName("SuperManagerTodoList"));
	FSuperManagerUICommands::Unregister();
	FSuperManagerStyleSetRegistry::Shutdown();
	UnRegisterSceneOutlinerColumnExtension();
	FEditorDelegates::PostUndoRedo.RemoveAll(this);
	FCoreUObjectDelegates::OnObjectTransacted.RemoveAll(this);
}

#pragma region ContentBrowserMenuExtention

void FSuperManagerModule::InitCBMenuExtension()
{
	//Get Content Browser Module
	FContentBrowserModule& ContentBrowserModule =
		FModuleManager::LoadModuleChecked<FContentBrowserModule>(TEXT("ContentBrowser"));
	//Get Hold of all Content Browser Menu Extenders
	TArray<FContentBrowserMenuExtender_SelectedPaths>& ContentBrowserModuleMenuExtenders =
		ContentBrowserModule.GetAllPathViewContextMenuExtenders();

	/*FContentBrowserMenuExtender_SelectedPaths CustomCBMenueDelegate;
	CustomCBMenueDelegate.BindRaw(this, &FSuperManagerModule::CustomCBMenuExtender);
	ContentBrowserModuleMenuExtenders.Add(CustomCBMenueDelegate);
	*/
	// Add custom delegate to all the existng delegates
	ContentBrowserModuleMenuExtenders.Add(
		FContentBrowserMenuExtender_SelectedPaths::
		CreateRaw(this, &FSuperManagerModule::CustomCBMenuExtender));
}

/**
 * to Define the position for inserting menu entry
 * @param SelectedPaths 
 * @return 
 */
TSharedRef<FExtender> FSuperManagerModule::CustomCBMenuExtender(const TArray<FString>& SelectedPaths)
{
	TSharedRef<FExtender> MenuExtender(new FExtender());

	if (SelectedPaths.Num() > 0)
	{
		SelectedFolderPath = SelectedPaths;
		MenuExtender->AddMenuExtension(FName("Delete"), //Extention hook, position to insert
		                               EExtensionHook::After, //inserting before or after
		                               TSharedPtr<FUICommandList>(), // custom hot keys
		                               FMenuExtensionDelegate::CreateRaw(this, &FSuperManagerModule::AddCBMenuEntry));
		// ^^ second binding, define details for the menu entry
	}
	return MenuExtender;
}

// define details for the custom menu entry
void FSuperManagerModule::AddCBMenuEntry(FMenuBuilder& MenuBuilder)
{
	MenuBuilder.AddMenuEntry
	(
		FText::FromString(TEXT("Delete Unused Assets")), // Title text for menu entry
		FText::FromString(TEXT("Safely Delete all unused assets under folder")), // Tooltip text
		FSlateIcon(FSuperManagerStyleSetRegistry::GetStyleSetName(), "ContentBrowser.DeleteUnusedAssets"), //Custom icon
		FExecuteAction::CreateRaw(this, &FSuperManagerModule::OnDeleteUnusedAssetButtonClicked)
		// third bind, the action of button
	);
	MenuBuilder.AddMenuEntry
	(
		FText::FromString(TEXT("Delete Empty Folders")), // Title text for menu entry
		FText::FromString(TEXT("Safely Delete all Empty Folders under the selected folder")), // Tooltip text
		FSlateIcon(FSuperManagerStyleSetRegistry::GetStyleSetName(), "ContentBrowser.DeleteEmptyFolders"), //Custom icon
		FExecuteAction::CreateRaw(this, &FSuperManagerModule::OnDeleteEmptyFoldersButtonClicked)
	);
	MenuBuilder.AddMenuEntry
	(
		FText::FromString(TEXT("Advanced Deletion")), // Title text for menu entry
		FText::FromString(TEXT("List assets by specific condition in a tab for deleting")), //Tooltip text
		FSlateIcon(FSuperManagerStyleSetRegistry::GetStyleSetName(), "ContentBrowser.AdvancedDeletion"),
		FExecuteAction::CreateRaw(this, &FSuperManagerModule::OnAdvancedDelectionButtonClicked)
	);
}

void FSuperManagerModule::OnAdvancedDelectionButtonClicked()
{
	//DebugHeader::ShowMsgDialog(EAppMsgType::Ok, TEXT("Under construction"), true);
	EnsureRedirectorsFixed();
	FGlobalTabmanager::Get()->TryInvokeTab(FName("AdvancedDeletion"));
}

void FSuperManagerModule::OnDeleteUnusedAssetButtonClicked()
{
	if (ConstructedAdvancedDeletionTab.IsValid())
	{
		DebugHeader::ShowMsgDialog(EAppMsgType::Ok, TEXT("Please close Advanced Deletion Tab Before this Operation"));
		return;
	}
	// check how many 
	if (SelectedFolderPath.Num() > 1)
	{
		DebugHeader::ShowMsgDialog(EAppMsgType::Ok, TEXT("You can only do this to one folder at once"), true);
		return;
	}
	if (!SelectedFolderPath[0].StartsWith(TEXT("/Game/")))
	{
		DebugHeader::ShowMsgDialog(EAppMsgType::Ok, TEXT("You can only do this to folder under /Game/"), true);
		return;
	}

	TArray<FString> AssetsPathArray = UEditorAssetLibrary::ListAssets(SelectedFolderPath[0]);

	if (AssetsPathArray.Num() == 0)
	{
		DebugHeader::ShowMsgDialog(EAppMsgType::Ok, TEXT("No asset found in selected folder"), false);
		return;
	}

	// IF Assets Found
	EAppReturnType::Type UserResponse =
		DebugHeader::ShowMsgDialog(EAppMsgType::YesNo,
		                           TEXT("A total of ") + FString::FromInt(AssetsPathArray.Num()) +
		                           TEXT(" Asset found in folder\n") + TEXT(
			                           "Are you sure you want to delete all unused assets?"), true);
	if (UserResponse != EAppReturnType::Yes) return;


	//确认执行时
	FixUpRedirectors();
	TArray<FAssetData> UnusedAssetsPathArray;

	for (const FString& AssetPath : AssetsPathArray)
	{
		// Don't touch the root folder
		if (AssetPath.Contains(TEXT("Collections")) ||
			AssetPath.Contains(TEXT("Developers")) ||
			AssetPath.Contains(TEXT("__ExternalActors__")) ||
			AssetPath.Contains(TEXT("__ExternalObjects__")))
		{
			continue;
		}
		if (!UEditorAssetLibrary::DoesAssetExist(AssetPath)) continue; //Check if Asset Exists

		TArray<FString> AssetReferencers = UEditorAssetLibrary::FindPackageReferencersForAsset(AssetPath);
		if (AssetReferencers.Num() == 0)
		{
			const FAssetData UnusedAssetData = UEditorAssetLibrary::FindAssetData(AssetPath);
			UnusedAssetsPathArray.Add(UnusedAssetData);
		}
	}

	if (UnusedAssetsPathArray.Num() <= 0)
	{
		DebugHeader::ShowMsgDialog(EAppMsgType::Ok, TEXT("No unused asset found under Selected Folder"), false);
		return;
	}
	FScopedTransaction Transaction(LOCTEXT("DeleteUnusedAssetsTransaction", "Delete Unused Assets"));
	const int32 DeletedCount = ObjectTools::DeleteAssets(UnusedAssetsPathArray);
	if (DeletedCount <= 0)
	{
		Transaction.Cancel();
	}
}

void FSuperManagerModule::FixUpRedirectors()
{
	TRACE_CPUPROFILER_EVENT_SCOPE(FSuperManager_FixUpRedirectors);
	TRACE_BOOKMARK(TEXT("Begin FixUpRedirectors"));
	IAssetRegistry& AssetRegistry =
		FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry")).Get();

	// Form a filter from the paths
	FARFilter Filter;
	Filter.bRecursivePaths = true;
	Filter.PackagePaths.Emplace("/Game");


	Filter.ClassPaths.Add(UObjectRedirector::StaticClass()->GetClassPathName());

	// Query for a list of assets in the selected paths
	TArray<FAssetData> AssetList;
	AssetRegistry.GetAssets(Filter, AssetList);

	if (AssetList.Num() == 0) return;

	TArray<FString> ObjectPaths;
	for (const FAssetData& Asset : AssetList)
	{
		ObjectPaths.Add(Asset.GetObjectPathString());
	}

	TArray<UObject*> Objects;
	const bool bAllowedToPromptToLoadAssets = true;
	const bool bLoadRedirects = true;

	AssetViewUtils::FLoadAssetsSettings Settings;
	Settings.bFollowRedirectors = false;
	Settings.bAllowCancel = true;

	AssetViewUtils::ELoadAssetsResult Result = AssetViewUtils::LoadAssetsIfNeeded(ObjectPaths, Objects, Settings);

	if (Result != AssetViewUtils::ELoadAssetsResult::Cancelled)
	{
		// Transform Objects array to ObjectRedirectors array
		TArray<UObjectRedirector*> Redirectors;
		for (UObject* Object : Objects)
		{
			Redirectors.Add(CastChecked<UObjectRedirector>(Object));
		}

		// Load the asset tools module
		FAssetToolsModule& AssetToolsModule = FModuleManager::LoadModuleChecked<FAssetToolsModule>(TEXT("AssetTools"));
		AssetToolsModule.Get().FixupReferencers(Redirectors);
	}
}

void FSuperManagerModule::OnDeleteEmptyFoldersButtonClicked()
{
	if (ConstructedAdvancedDeletionTab.IsValid())
	{
		DebugHeader::ShowMsgDialog(EAppMsgType::Ok, TEXT("Please close Advanced Deletion Tab Before this Operation"));
		return;
	}
	if (SelectedFolderPath.Num() > 1)
	{
		DebugHeader::ShowMsgDialog(EAppMsgType::Ok, TEXT("You can only do this to one folder at once"), true);
		return;
	}
	FString SelectedPath = SelectedFolderPath[0];
	if (!SelectedPath.StartsWith(TEXT("/Game/")))
	{
		DebugHeader::ShowMsgDialog(EAppMsgType::Ok, TEXT("You can only do this to folder under /Game/"), true);
		return;
	}
	FixUpRedirectors();
	int32 Count = 0;
	TArray<FString> EmptyFolderPathsArray;
	FString EmptyFoldersPathName = TEXT("EmptyFolders: \n");
	TArray<FString> AssetsPathArray = UEditorAssetLibrary::ListAssets(SelectedPath, true, true);
	for (const FString& AssetPath : AssetsPathArray)
	{
		if (AssetPath.Contains(TEXT("Collections")) ||
			AssetPath.Contains(TEXT("Developers")) ||
			AssetPath.Contains(TEXT("__ExternalActors__")) ||
			AssetPath.Contains(TEXT("__ExternalObjects__")))
		{
			continue;
		}
		FString FixedPathName = AssetPath;
		FixedPathName.RemoveFromEnd(TEXT("/"));
		if (!UEditorAssetLibrary::DoesDirectoryExist(FixedPathName)) continue;
		if (!UEditorAssetLibrary::DoesDirectoryHaveAssets(FixedPathName))
		{
			EmptyFolderPathsArray.Add(AssetPath);
			EmptyFoldersPathName += AssetPath + TEXT("\n");
		}
	}


	if (EmptyFolderPathsArray.Num() == 0)
	{
		DebugHeader::ShowMsgDialog(EAppMsgType::Ok, TEXT("No folder found in selected folder"), false);
		return;
	}

	EAppReturnType::Type UserResponse = DebugHeader::ShowMsgDialog(EAppMsgType::YesNo,
	                                                               TEXT("A total of ") +
	                                                               FString::FromInt(EmptyFolderPathsArray.Num()) +
	                                                               TEXT(" Empty folders found in folder\n") +
	                                                               EmptyFoldersPathName + TEXT(
		                                                               "Are you sure you want to delete all empty folders?"),
	                                                               true);

	if (UserResponse != EAppReturnType::Yes) return;
	FScopedTransaction Transaction(LOCTEXT("DeleteEmptyFoldersTransaction", "Delete Empty Folders"));
	for (const FString& EmptyFolderPath : EmptyFolderPathsArray)
	{
		if (UEditorAssetLibrary::DeleteDirectory(EmptyFolderPath))
		{
			Count++;
		}
		else
		{
			DebugHeader::Print(TEXT("Failed to delete folder: ") + EmptyFolderPath, FColor::Red);
		}
	}
	if (Count > 0)
	{
		DebugHeader::ShowNotifyInfo(TEXT("Successfully deleted ") + FString::FromInt(Count) + TEXT(" empty folders"));
	}
	else
	{
		Transaction.Cancel();
	}
}


#pragma endregion

#pragma region LevelEditorMenuExtention

void FSuperManagerModule::InitLevelEditorMenuExtension()
{
	FLevelEditorModule& LevelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>("LevelEditor");
	if (CustomUICommandList.IsValid())
	{
		TSharedRef<FUICommandList> ExistingLevelCommands = LevelEditorModule.GetGlobalLevelEditorActions();
		ExistingLevelCommands->Append(CustomUICommandList.ToSharedRef());
	}

	if (UToolMenus* ToolMenus = UToolMenus::TryGet())
	{
		if (UToolMenu* WindowMenu = ToolMenus->ExtendMenu("LevelEditor.MainMenu.Window"))
		{
			FToolMenuSection& Section = WindowMenu->FindOrAddSection("SuperManagerWindows");
		}
	}
	TArray<FLevelEditorModule::FLevelViewportMenuExtender_SelectedActors>& LevelEditorMenuExtenders = LevelEditorModule.
		GetAllLevelViewportContextMenuExtenders();
	LevelEditorMenuExtenders.Add(
		FLevelEditorModule::FLevelViewportMenuExtender_SelectedActors::CreateRaw(
			this, &FSuperManagerModule::CreateLEMenuExtender));
}


TSharedRef<FExtender> FSuperManagerModule::CreateLEMenuExtender(const TSharedRef<FUICommandList> UICommandList,
                                                                const TArray<AActor*> SelectedActors)
{
	TSharedRef<FExtender> MenuExtender = MakeShared<FExtender>();

	if (SelectedActors.Num() > 0)
	{
		UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.ActorContextMenu");
		FToolMenuSection& Section = Menu->FindOrAddSection("ActorOptions");
		FToolMenuEntry SubMenuEntry = FToolMenuEntry::InitSubMenu(
			"SuperManager.LevelActorActions",
			LOCTEXT("SuperManagerLevelActorActionsLabel", "Level Actor Actions"),
			LOCTEXT("SuperManagerLevelActorActionsTooltip", "SuperManager 提供的 Actor 操作"),
			FNewToolMenuDelegate::CreateRaw(this, &FSuperManagerModule::PopulateLevelActorSubMenu),
			false,
			FSlateIcon(FSuperManagerStyleSetRegistry::GetStyleSetName(), TEXT("LevelEditor.SubMenu")));
		SubMenuEntry.InsertPosition = FToolMenuInsert("EditSubMenu", EToolMenuInsertType::First);
		Section.AddEntry(SubMenuEntry);
	}


	return MenuExtender;
}

void FSuperManagerModule::PopulateLevelActorSubMenu(UToolMenu* InMenu)
{
	FToolMenuSection& NewSection = InMenu->AddSection("SuperManagerSection");

	FUIAction LockActorSelectionAction(
		FExecuteAction::CreateRaw(this, &FSuperManagerModule::OnLockSelectionButtonClicked));
	FUIAction UnlockActorSelection(
		FExecuteAction::CreateRaw(this, &FSuperManagerModule::OnUnLockSelectionButtonClicked));
	NewSection.AddMenuEntry(
		"SuperManager.LockActorSelection",
		LOCTEXT("LockActorSelectionMenuLabel", "Lock Actor Selection"),
		LOCTEXT("LockActorSelectionMenuTooltip", "锁定当前选中的 Actor"),
		FSlateIcon(FSuperManagerStyleSetRegistry::GetStyleSetName(), TEXT("LevelEditor.LockActorSelection")),
		LockActorSelectionAction);
	NewSection.AddMenuEntry(
		"SuperManager.UnLockAllActorsSelection",
		LOCTEXT("UnLockAllActorSelectionMenuLabel", "UnLock All Actors Selection"),
		LOCTEXT("UnLockAllActorSelectionMenuLabel", "UnLock All Actors Selection"),
		FSlateIcon(FSuperManagerStyleSetRegistry::GetStyleSetName(), TEXT("LevelEditor.UnlockActorSelection")),
		UnlockActorSelection
	);
}

void FSuperManagerModule::OnLockSelectionButtonClicked()
{
	if (!GetEditorActorSubsystem()) return;
	TArray<AActor*> SelectedActors = WeakEditorActorSubsystem->GetSelectedLevelActors();
	if (SelectedActors.Num() == 0)
	{
		DebugHeader::ShowNotifyInfo(TEXT("No Actor Selected"));
		return;
	}
	FScopedTransaction Transaction(LOCTEXT("LockActorSelectionTransaction", "Lock Actor Selection"));
	FString CurrenLockedActorNames = TEXT("Locked selection for: ");
	for (AActor* Actor : SelectedActors)
	{
		LockActorSelection(Actor);
		WeakEditorActorSubsystem->SetActorSelectionState(Actor, false);
		CurrenLockedActorNames.Append(TEXT("\n"));
		CurrenLockedActorNames.Append(Actor->GetActorLabel());
	}
	DebugHeader::ShowNotifyInfo(CurrenLockedActorNames);
}

void FSuperManagerModule::OnUnLockSelectionButtonClicked()
{
	//Unlock All Level Actors

	if (!GetEditorActorSubsystem()) return;

	if (CachedLockedActors.Num() == 0)
	{
		RefreshLockedActorCacheSnapshot();
	}
	CompactLockedActorCache();


	if (CachedLockedActors.Num() == 0)
	{
		DebugHeader::ShowNotifyInfo(TEXT("No Selection Locked Actor Currently"));
		return;
	}
	FScopedTransaction Transaction(LOCTEXT("UnlockActorSelectionTransaction", "Unlock Actor Selection"));
	FString UnlockedLockedActorNames = TEXT("Lifted selection constraint for: ");
	const TArray<TWeakObjectPtr<AActor>> LockedActorsArray = CachedLockedActors.Array();
	for (const TWeakObjectPtr<AActor>& LockedActorPtr : LockedActorsArray)
	{
		AActor* Actor = LockedActorPtr.Get();
		if (!Actor)
		{
			continue;
		}

		UnlockActorSelection(Actor);
		UnlockedLockedActorNames.Append(TEXT("\n"));
		UnlockedLockedActorNames.Append(Actor->GetActorLabel());
	}
	DebugHeader::ShowNotifyInfo(UnlockedLockedActorNames);
}
#pragma endregion
#pragma region Lock Selections
void FSuperManagerModule::InitCustomSelectionEvent()
{
	USelection* UserSelection = GEditor->GetSelectedActors();

	UserSelection->SelectObjectEvent.AddRaw(this, &FSuperManagerModule::OnActorSelected);
}

void FSuperManagerModule::OnActorSelected(UObject* SelectedObject)
{
	if (!GetEditorActorSubsystem()) return;
	if (AActor* SelectedActor = Cast<AActor>(SelectedObject))
	{
		if (CheckIsActorSelectionLocked(SelectedActor))
		{
			WeakEditorActorSubsystem->SetActorSelectionState(SelectedActor, false);
			HighlightLockedActorRow(SelectedActor);
		}
	}
}

void FSuperManagerModule::LockActorSelection(AActor* ActorToProcess)
{
	if (!ActorToProcess) return;
	if (!GetEditorActorSubsystem()) return;
	ActorToProcess->Modify();
	if (!ActorToProcess->Tags.Contains(FName("LockActorSelection")))
	{
		ActorToProcess->Tags.Add(FName("LockActorSelection"));
	}
	CacheLockedActor(ActorToProcess);
	RefreshLockedActorsWidget();
	RefreshSceneOutliner();
}

void FSuperManagerModule::UnlockActorSelection(AActor* ActorToProcess)
{
	if (!ActorToProcess) return;
	if (!GetEditorActorSubsystem()) return;
	ActorToProcess->Modify();
	if (ActorToProcess->Tags.Contains(FName("LockActorSelection")))
	{
		ActorToProcess->Tags.Remove(FName("LockActorSelection"));
	}
	RemoveActorFromLockedCache(ActorToProcess);
	RefreshLockedActorsWidget();
	RefreshSceneOutliner();
}

void FSuperManagerModule::HandleUndoRedo()
{
	RefreshLockedActorCacheSnapshot();
	RefreshLockedActorsWidget();
	RefreshSceneOutliner();
}

void FSuperManagerModule::HandleTransactionEvent(UObject* TransactedObject,
                                                 const FTransactionObjectEvent& TransactionEvent)
{
	if (TransactionEvent.GetEventType() != ETransactionObjectEventType::UndoRedo)
	{
		return;
	}

	if (TransactedObject && !TransactedObject->IsA<AActor>())
	{
		return;
	}

	RefreshLockedActorCacheSnapshot();
	RefreshLockedActorsWidget();
	RefreshSceneOutliner();
}

bool FSuperManagerModule::CheckIsActorSelectionLocked(const AActor* ActorToProcess)
{
	if (!ActorToProcess)
	{
		return false;
	}

	if (ActorToProcess->Tags.Contains(FName("LockActorSelection")))
	{
		return true;
	}
	return false;
}

void FSuperManagerModule::CacheLockedActor(AActor* ActorToCache)
{
	if (!ActorToCache)
	{
		return;
	}

	CachedLockedActors.Add(ActorToCache);
}

void FSuperManagerModule::RemoveActorFromLockedCache(AActor* ActorToRemove)
{
	if (!ActorToRemove)
	{
		return;
	}

	CachedLockedActors.Remove(ActorToRemove);
}

void FSuperManagerModule::HandleSetActorLockState(TWeakObjectPtr<AActor> ActorPtr, bool bShouldLock)
{
	AActor* Actor = ActorPtr.Get();
	if (!Actor)
	{
		return;
	}

	if (bShouldLock)
	{
		LockActorSelection(Actor);
		if (GetEditorActorSubsystem())
		{
			WeakEditorActorSubsystem->SetActorSelectionState(Actor, false);
		}
	}
	else
	{
		UnlockActorSelection(Actor);
	}
}

void FSuperManagerModule::CompactLockedActorCache()
{
	for (auto CacheIt = CachedLockedActors.CreateIterator(); CacheIt; ++CacheIt)
	{
		const TWeakObjectPtr<AActor> ActorPtr = *CacheIt;
		if (!ActorPtr.IsValid() || !CheckIsActorSelectionLocked(ActorPtr.Get()))
		{
			CacheIt.RemoveCurrent();
		}
	}
}

void FSuperManagerModule::RefreshLockedActorCacheSnapshot()
{
	if (!GetEditorActorSubsystem())
	{
		CachedLockedActors.Reset();
		return;
	}

	const TArray<AActor*> AllLevelActors = WeakEditorActorSubsystem->GetAllLevelActors();
	CachedLockedActors.Reset();
	if (AllLevelActors.Num() == 0)
	{
		return;
	}

	DebugHeader::ShowNotifyInfo(TEXT("Checking All Actors Locking Status"));
	TUniquePtr<FScopedSlowTask> RefreshTask = DebugHeader::CreateProgressTask(
		static_cast<float>(AllLevelActors.Num()),
		LOCTEXT("RefreshLockedActorsTask", "Checking All actors Locked Status..."));
	bool bWasCancelled = false;

	for (AActor* Actor : AllLevelActors)
	{
		if (RefreshTask.IsValid())
		{
			const FText DetailText = Actor
				                         ? FText::Format(LOCTEXT("RefreshLockedActorsDetail", "Scanning {0}"),
				                                         FText::FromString(Actor->GetActorLabel()))
				                         : LOCTEXT("RefreshLockedActorsDetailUnknown", "Scanning actor (invalid)");
			RefreshTask->EnterProgressFrame(1.f, DetailText);

			if (RefreshTask->ShouldCancel())
			{
				bWasCancelled = true;
				break;
			}
		}

		if (Actor && CheckIsActorSelectionLocked(Actor))
		{
			CachedLockedActors.Add(Actor);
		}
	}

	if (bWasCancelled)
	{
		DebugHeader::ShowNotifyInfo(TEXT("已取消 Actor 锁定状态全量刷新，保留当前已扫描的缓存。"));
	}
}
#pragma endregion


#pragma region CustomUICommands

void FSuperManagerModule::InitCustomUICommands()
{
	CustomUICommandList = MakeShareable(new FUICommandList());
	CustomUICommandList->MapAction(FSuperManagerUICommands::Get().LockActorSelectionCommand,
	                               FExecuteAction::CreateRaw(
		                               this, &FSuperManagerModule::OnLockActorSelectionHotKeyPressed));
	CustomUICommandList->MapAction(FSuperManagerUICommands::Get().UnlockActorSelectionCommand,
	                               FExecuteAction::CreateRaw(
		                               this, &FSuperManagerModule::OnUnlockActorSelectionHotKeyPressed));
	CustomUICommandList->MapAction(FSuperManagerUICommands::Get().OpenLockedActorsListCommand,
	                               FExecuteAction::CreateRaw(
		                               this, &FSuperManagerModule::OnOpenLockedActorsListCommand));
	CustomUICommandList->MapAction(FSuperManagerUICommands::Get().OpenTodoListCommand,
	                               FExecuteAction::CreateRaw(
		                               this, &FSuperManagerModule::OnOpenTodoListCommand));
}

void FSuperManagerModule::OnLockActorSelectionHotKeyPressed()
{
	OnLockSelectionButtonClicked();
}

void FSuperManagerModule::OnUnlockActorSelectionHotKeyPressed()
{
	OnUnLockSelectionButtonClicked();
}

void FSuperManagerModule::OnOpenLockedActorsListCommand()
{
	FGlobalTabmanager::Get()->TryInvokeTab(FName("LockedActorsList"));
}

void FSuperManagerModule::OnOpenTodoListCommand()
{
	FGlobalTabmanager::Get()->TryInvokeTab(FName("SuperManagerTodoList"));
}

#pragma endregion

#pragma region SceneOutlinerExtension

void FSuperManagerModule::InitSceneOutlinerColumnExtension()
{
	FSceneOutlinerModule& SceneOutlinerModule = FModuleManager::LoadModuleChecked<FSceneOutlinerModule>(
		TEXT("SceneOutliner"));
	FSceneOutlinerColumnInfo SelectionLockColumnInfo
	(
		ESceneOutlinerColumnVisibility::Visible,
		1,
		FCreateSceneOutlinerColumn::CreateRaw(this, &FSuperManagerModule::OnGenerateSceneOutlinerColumn)
	);

	SceneOutlinerModule.RegisterDefaultColumnType<FOutlinerSelectionLockColumn>(SelectionLockColumnInfo);
}

TSharedRef<ISceneOutlinerColumn> FSuperManagerModule::OnGenerateSceneOutlinerColumn(ISceneOutliner& SceneOutliner)
{
	return MakeShareable(new FOutlinerSelectionLockColumn(SceneOutliner));
}

void FSuperManagerModule::RefreshSceneOutliner()
{
	FLevelEditorModule& LevelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>("LevelEditor");
	TArray<TWeakPtr<ISceneOutliner>> SceneOutliners = LevelEditorModule.GetFirstLevelEditor()->GetAllSceneOutliners();
	for (TWeakPtr<ISceneOutliner> SceneOutliner : SceneOutliners)
	{
		if (SceneOutliner.IsValid())
		{
			SceneOutliner.Pin()->FullRefresh();
		}
	}
}

void FSuperManagerModule::UnRegisterSceneOutlinerColumnExtension()
{
	FSceneOutlinerModule& SceneOutlinerModule = FModuleManager::LoadModuleChecked<FSceneOutlinerModule>(
		TEXT("SceneOutliner"));
	SceneOutlinerModule.UnRegisterColumnType<FOutlinerSelectionLockColumn>();
}

void FSuperManagerModule::ProcessLockingForOutliner(AActor* ActorToProcess, bool bShouldBeLock)
{
	if (!GetEditorActorSubsystem()) return;
	if (!ActorToProcess) return;
	FScopedTransaction Transaction(LOCTEXT("Process Locking for Outliner", "Process Locking for Outliner"));
	if (bShouldBeLock)
	{
		LockActorSelection(ActorToProcess);
		WeakEditorActorSubsystem->SetActorSelectionState(ActorToProcess, false);
		RefreshSceneOutliner();
		DebugHeader::ShowNotifyInfo(TEXT("Locked Selection for: \n ") + ActorToProcess->GetActorLabel());
	}
	else
	{
		UnlockActorSelection(ActorToProcess);
		RefreshSceneOutliner();
		DebugHeader::ShowNotifyInfo(TEXT("Removed Selection  Lock for:  \n") + ActorToProcess->GetActorLabel());
	}
}

#pragma endregion

#pragma region CustomEditorTab
void FSuperManagerModule::RegisterAdvancedDeletionTab()
{
	

	{
		FGlobalTabmanager::Get()->RegisterNomadTabSpawner(FName("AdvancedDeletion"), FOnSpawnTab::
		                                                  CreateRaw(
			                                                  this, &FSuperManagerModule::OnSpawnAdvancedDeletionTab)).
		                          SetDisplayName(FText::FromString(TEXT("Advanced Deletion")))
		                          .SetIcon(FSlateIcon(FSuperManagerStyleSetRegistry::GetStyleSetName(),
		                                              "ContentBrowser.AdvancedDeletion"))
		                          .SetAutoGenerateMenuEntry(true);
	}
}

void FSuperManagerModule::RegisterLockedActorsTab()
{
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(FName("LockedActorsList"), FOnSpawnTab::CreateRaw(
		                                                  this, &FSuperManagerModule::OnSpawnLockedActorsTab)).
	                          SetDisplayName(LOCTEXT("LockedActorsTabTitle", "Locked Actors"))
	                          .SetIcon(FSlateIcon(FSuperManagerStyleSetRegistry::GetStyleSetName(),
	                                              "LevelEditor.LockActorSelection"))
	                          .SetAutoGenerateMenuEntry(true);
}

void FSuperManagerModule::RegisterTodoListTab()
{
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(FName("SuperManagerTodoList"), FOnSpawnTab::CreateRaw(
		                                                  this, &FSuperManagerModule::OnSpawnTodoListTab)).
	                          SetDisplayName(LOCTEXT("SuperManagerTodoTabTitle", "SuperManager Todo List"))
	                          .SetIcon(FSlateIcon(FSuperManagerStyleSetRegistry::GetStyleSetName(), "LevelEditor.SubMenu"))
	                          .SetAutoGenerateMenuEntry(true);
}

TSharedRef<SDockTab> FSuperManagerModule::OnSpawnAdvancedDeletionTab(const FSpawnTabArgs& SpawnTabArgs)
{
	if (SelectedFolderPath.Num() == 0)
	{
		return SNew(SDockTab);
	}

	ConstructedAdvancedDeletionTab = SNew(SDockTab).TabRole(NomadTab)
	[
		SNew(SAdvancedDeletionTab)
		.AssetsDataToStore(GetAllAssetDataUnderSelectedFolder())
		.SelectedFolder(SelectedFolderPath[0])
	];
	//const

	ConstructedAdvancedDeletionTab->SetOnTabClosed(
		SDockTab::FOnTabClosedCallback::CreateRaw(this, &FSuperManagerModule::OnAdvancedDeletionTabClosed));
	return ConstructedAdvancedDeletionTab.ToSharedRef();
}

void FSuperManagerModule::OnAdvancedDeletionTabClosed(TSharedRef<SDockTab> TabToClose)
{
	if (ConstructedAdvancedDeletionTab.IsValid())
	{
		ConstructedAdvancedDeletionTab.Reset();
		SelectedFolderPath.Empty();
	}
}

TSharedRef<SDockTab> FSuperManagerModule::OnSpawnLockedActorsTab(const FSpawnTabArgs& SpawnTabArgs)
{
	TSharedRef<SLockedActorsListTab> LockedActorsWidget =
		SNew(SLockedActorsListTab)
		.InitialItems(GatherLockedActorsListItems())
		.OnSetActorLockState(FOnSetActorLockState::CreateRaw(this, &FSuperManagerModule::HandleSetActorLockState))
		.OnUnlockAllActors(FOnUnlockAllActors::CreateRaw(this, &FSuperManagerModule::OnUnLockSelectionButtonClicked))
		.OnRequestRefreshData(
			FOnRequestLockedActorRows::CreateRaw(this, &FSuperManagerModule::GatherLockedActorsListItems))
		.OnRowDoubleClicked(
			FOnLockedActorRowDoubleClicked::CreateRaw(this, &FSuperManagerModule::HandleLockedActorRowDoubleClicked));

	LockedActorsListWidget = LockedActorsWidget;

	return SNew(SDockTab)
		.TabRole(ETabRole::NomadTab)
		[
			LockedActorsWidget
		];
}

TSharedRef<SDockTab> FSuperManagerModule::OnSpawnTodoListTab(const FSpawnTabArgs& SpawnTabArgs)
{
	return SNew(SDockTab)
		.TabRole(ETabRole::NomadTab)
		[
			SNew(STodoListWidget)
		];
}

TArray<TSharedPtr<FLockedActorListItem>> FSuperManagerModule::GatherLockedActorsListItems()
{
	TArray<TSharedPtr<FLockedActorListItem>> Result;
	if (!GetEditorActorSubsystem())
	{
		return Result;
	}

	const TArray<AActor*> AllActors = WeakEditorActorSubsystem->GetAllLevelActors();
	Result.Reserve(AllActors.Num());

	for (AActor* Actor : AllActors)
	{
		if (!Actor)
		{
			continue;
		}

		TSharedPtr<FLockedActorListItem> Item = MakeShared<FLockedActorListItem>();
		Item->Actor = Actor;
		Item->bIsLocked = CheckIsActorSelectionLocked(Actor);
		Result.Add(MoveTemp(Item));
	}

	return Result;
}

void FSuperManagerModule::HandleLockedActorRowDoubleClicked(TWeakObjectPtr<AActor> ActorPtr)
{
	AActor* Actor = ActorPtr.Get();
	if (!Actor)
	{
		return;
	}

	if (!GetEditorActorSubsystem())
	{
		return;
	}

	GEditor->SelectNone(false, true);
	WeakEditorActorSubsystem->SetActorSelectionState(Actor, true);
	GEditor->SelectActor(Actor, true, true);
	GEditor->NoteSelectionChange();
	GEditor->MoveViewportCamerasToActor(*Actor, false);

	HighlightLockedActorRow(Actor);
}

void FSuperManagerModule::HighlightLockedActorRow(TWeakObjectPtr<AActor> ActorPtr)
{
	if (TSharedPtr<SLockedActorsListTab> LockedWidget = LockedActorsListWidget.Pin())
	{
		LockedWidget->HighlightActorRow(ActorPtr);
	}
}

void FSuperManagerModule::RefreshLockedActorsWidget()
{
	if (TSharedPtr<SLockedActorsListTab> LockedWidget = LockedActorsListWidget.Pin())
	{
		LockedWidget->RequestRefresh();
	}
}

#pragma endregion


#pragma region ProccessDataForAdvancedDeletionTab

TArray<TSharedPtr<FAssetData>> FSuperManagerModule::GetAllAssetDataUnderSelectedFolder()
{
	EnsureRedirectorsFixed();
	TRACE_CPUPROFILER_EVENT_SCOPE(FSuperManager_GetAllAssetsUnderFolder);
	TArray<TSharedPtr<FAssetData>> AvailableAssetsData;
	if (SelectedFolderPath.Num() == 0)
	{
		//DebugHeader::ShowMsgDialog(EAppMsgType::Ok, TEXT("Please select a folder in Content Browser first."), true);
		DebugHeader::Print(TEXT("Error: No folder selected before opening Advanced Deletion Tab."), FColor::Red);
		return AvailableAssetsData;
	}
	if (SelectedFolderPath.Num() > 1)
	{
		DebugHeader::ShowMsgDialog(EAppMsgType::Ok, TEXT("You can only do this to one folder at once"), true);
		return AvailableAssetsData;
	}
	FString SelectedPath = SelectedFolderPath[0];
	if (!SelectedPath.StartsWith(TEXT("/Game/")))
	{
		DebugHeader::ShowMsgDialog(EAppMsgType::Ok, TEXT("You can only do this to folder under /Game/"), true);
		return AvailableAssetsData;
	}
	TArray<FString> AssetsPathArray = UEditorAssetLibrary::ListAssets(SelectedPath);
	for (const FString& AssetPath : AssetsPathArray)
	{
		if (AssetPath.Contains(TEXT("Collections")) ||
			AssetPath.Contains(TEXT("Developers")) ||
			AssetPath.Contains(TEXT("__ExternalActors__")) ||
			AssetPath.Contains(TEXT("__ExternalObjects__")))
		{
			continue;
		}
		if (!UEditorAssetLibrary::DoesAssetExist(AssetPath)) continue; //Check if Asset Exists

		const FAssetData Data = UEditorAssetLibrary::FindAssetData(AssetPath);
		AvailableAssetsData.Add(MakeShared<FAssetData>(Data));
	}
	return AvailableAssetsData;
}


bool FSuperManagerModule::DeleteSingleAssetForAssetList(const FAssetData& AssetDataToDelete)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(FSuperManager_DeleteSingleAsset);
	EnsureRedirectorsFixed();
	return DeleteAssetsInternal(TArray{AssetDataToDelete},
	                            LOCTEXT("AdvancedDeletionDeleteSingleTransaction",
	                                    "Delete Asset From Advanced Deletion"), false);
}

bool FSuperManagerModule::DeleteAssetsForAssetList(const TArray<FAssetData>& AssetsDataToDelete)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(FSuperManager_DeleteMultipleAssets);
	EnsureRedirectorsFixed();
	return DeleteAssetsInternal(AssetsDataToDelete,
	                            LOCTEXT("AdvancedDeletionDeleteMultipleTransaction",
	                                    "Delete Assets From Advanced Deletion"), true);
}

void FSuperManagerModule::UpdateDisplayedData(TArray<TSharedPtr<FAssetData>>& SourceAssetsDataArray,
                                              TArray<TSharedPtr<FAssetData>>& Out_DisplayedAssetsDataArray,
                                              EComboBoxOptions ComboBoxOption)
{
	switch (ComboBoxOption)
	{
	case EComboBoxOptions::E_ListAll:
		Out_DisplayedAssetsDataArray = SourceAssetsDataArray;

		break;
	case EComboBoxOptions::E_ListUnused:
		Out_DisplayedAssetsDataArray.Empty();
		for (TSharedPtr<FAssetData> DataSharedPtr : SourceAssetsDataArray)
		{
			FString DataPath = DataSharedPtr->GetObjectPathString();
			auto DataReferencers =
				UEditorAssetLibrary::FindPackageReferencersForAsset(DataPath);
			if (DataReferencers.Num() == 0)
			{
				Out_DisplayedAssetsDataArray.Add(DataSharedPtr);
			}
		}
		break;
	case EComboBoxOptions::E_ListUsed:
		break;
	case EComboBoxOptions::E_ListSameNameAssets:
		{
			TMultiMap<FString, TSharedPtr<FAssetData>> SameNameAssetsMap;
			Out_DisplayedAssetsDataArray.Empty();
			for (TSharedPtr<FAssetData> DataSharedPtr : SourceAssetsDataArray)
			{
				if (DataSharedPtr.IsValid())
				{
					FString AssetName = DataSharedPtr->AssetName.ToString();
					SameNameAssetsMap.Emplace(AssetName, DataSharedPtr);
				}
			}
			TArray<FString> UniqueKeys;
			SameNameAssetsMap.GetKeys(UniqueKeys);
			for (const FString& UniqueKey : UniqueKeys)
			{
				TArray<TSharedPtr<FAssetData>> ValuesForKey;
				SameNameAssetsMap.MultiFind(UniqueKey, ValuesForKey);
				if (ValuesForKey.Num() > 1)
				{
					Out_DisplayedAssetsDataArray.Append(ValuesForKey);
				}
			}
			break;
		}
	default:
		break; // 通常 Rider 还会为您生成一个 default 分支
	}
}

void FSuperManagerModule::SyncCBToClickedAssetForAssetList(const FString& SelectedPath)
{
	if (SelectedPath.IsEmpty())
	{
		return;
	}

	UEditorAssetLibrary::SyncBrowserToObjects(TArray<FString>{SelectedPath});

	FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateLambda(
		[=](float /*DeltaTime*/) -> bool
		{
			// 使用您指出的正确方式调用 GetActiveTab()
			TSharedPtr<SDockTab> ActiveTab = FGlobalTabmanager::Get()->GetActiveTab();

			if (ActiveTab.IsValid() && ActiveTab->GetLayoutIdentifier().TabType.ToString().StartsWith(
				TEXT("ContentBrowserTab")))
			{
				TSharedPtr<SWindow> ParentWindow = FSlateApplication::Get().FindWidgetWindow(ActiveTab.ToSharedRef());
				if (ParentWindow.IsValid())
				{
					ParentWindow->FlashWindow();
				}
			}

			return false; // Ticker 只执行一次
		}
	));
}
#pragma endregion

#pragma region Helper Functions

bool FSuperManagerModule::GetEditorActorSubsystem()
{
	if (!WeakEditorActorSubsystem.IsValid())
	{
		WeakEditorActorSubsystem = GEditor->GetEditorSubsystem<UEditorActorSubsystem>();
	}
	return WeakEditorActorSubsystem.IsValid();
}

bool FSuperManagerModule::DeleteAssetsInternal(const TArray<FAssetData>& AssetsDataToDelete,
                                               const FText& TransactionText, bool bShowProgress)
{
	if (AssetsDataToDelete.Num() == 0)
	{
		return false;
	}
	TUniquePtr<FScopedSlowTask> SlowTask;
	if (bShowProgress)
	{
		const float TotalSteps = static_cast<float>(AssetsDataToDelete.Num());
		SlowTask = DebugHeader::CreateProgressTask(TotalSteps, TransactionText);
	}
	FScopedTransaction Transaction(TransactionText);
	if (ObjectTools::DeleteAssets(AssetsDataToDelete) > 0)
	{
		return true;
	}
	Transaction.Cancel();
	return false;
}

void FSuperManagerModule::EnsureRedirectorsFixed()
{
	if (bRedirectorsUpToDate)
	{
		return;
	}
	TRACE_CPUPROFILER_EVENT_SCOPE(FSuperManager_EnsureRedirectorsFixed);
	FixUpRedirectors();
	bRedirectorsUpToDate = true;
}

void FSuperManagerModule::ForceFixUpRedirectors()
{
	bRedirectorsUpToDate = false;
	EnsureRedirectorsFixed();
}
#pragma endregion
#undef LOCTEXT_NAMESPACE
IMPLEMENT_MODULE(FSuperManagerModule, SuperManager)
