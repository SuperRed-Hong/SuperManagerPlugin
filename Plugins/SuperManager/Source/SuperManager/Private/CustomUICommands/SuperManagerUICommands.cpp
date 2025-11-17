// Fill out your copyright notice in the Description page of Project Settings.


#include "CustomUICommands/SuperManagerUICommands.h"
#include "Internationalization/Text.h"

#define LOCTEXT_NAMESPACE "FSuperManagerUICommands"

void FSuperManagerUICommands::RegisterCommands()
{
	UI_COMMAND(
		LockActorSelectionCommand,
		 "Lock Actor Selection",
		 "Lock actor selections in level, once triggered, actor can no longer be selected",
		EUserInterfaceActionType::Button,
		FInputChord(EKeys::W, EModifierKey::Alt)
	);
	UI_COMMAND(
		UnlockActorSelectionCommand,
		 "Unlock All Actor Selection",
		 "Remove selection lock on all actors",
		EUserInterfaceActionType::Button,
		FInputChord(EKeys::W, EModifierKey::Alt | EModifierKey::Shift)
	);
	UI_COMMAND(
		OpenLockedActorsListCommand,
		 "Open Locked Actors List",
		 "Show all actors and toggle their lock state",
		EUserInterfaceActionType::Button,
		FInputChord(EKeys::L, EModifierKey::Alt)
	);
	UI_COMMAND(
		OpenTodoListCommand,
		 "Open SuperManager Todo List",
		 "Open a lightweight todo list tab for editor tasks",
		EUserInterfaceActionType::Button,
		FInputChord(EKeys::T, EModifierKey::Alt )
	);
}

#undef LOCTEXT_NAMESPACE
