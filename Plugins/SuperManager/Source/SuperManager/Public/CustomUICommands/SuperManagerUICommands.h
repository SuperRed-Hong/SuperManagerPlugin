// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Framework/Commands/Commands.h"

class FSuperManagerUICommands : public TCommands<FSuperManagerUICommands>
{
public:
	FSuperManagerUICommands(): TCommands<FSuperManagerUICommands>(
		FName("SuperManager"),
		FText::FromString("Super Manager UI Commands List"),
		NAME_None,
		FName("SuperManager")
		){}
	TSharedPtr<FUICommandInfo> LockActorSelectionCommand;
	TSharedPtr<FUICommandInfo> UnlockActorSelectionCommand;
	TSharedPtr<FUICommandInfo> OpenLockedActorsListCommand;
	TSharedPtr<FUICommandInfo> OpenTodoListCommand;
	virtual void RegisterCommands() override;
private:
};
