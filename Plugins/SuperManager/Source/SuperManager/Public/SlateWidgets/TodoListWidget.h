// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Widgets/SCompoundWidget.h"
#include "Widgets/Views/SListView.h"

class SEditableTextBox;

/**
 * Simple in-editor todo item model.
 */
struct FTodoItem
{
	FTodoItem()
		: Id(FGuid::NewGuid())
	{
	}

	FTodoItem(const FGuid& InId, const FText& InLabel, bool bInCompleted)
		: Id(InId)
		, Label(InLabel)
		, bCompleted(bInCompleted)
	{
	}

	FGuid Id;
	FText Label;
	bool bCompleted = false;
};

/**
 * Slate widget that renders a lightweight todo list for editor workflows.
 */
class STodoListWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(STodoListWidget)
		: _InitialItems()
	{
	}

		SLATE_ARGUMENT(TArray<TSharedPtr<FTodoItem>>, InitialItems)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

private:
	FReply HandleAddTodo();
	void HandleNewTodoCommitted(const FText& InText, ETextCommit::Type CommitType);
	void HandleToggleTodo(ECheckBoxState NewState, TSharedPtr<FTodoItem> Item);
	FReply HandleClearCompleted();
	void HandleRemoveTodo(TSharedPtr<FTodoItem> Item);
	TSharedRef<ITableRow> OnGenerateRow(TSharedPtr<FTodoItem> Item, const TSharedRef<STableViewBase>& OwnerTable);

	void AddTodoInternal(const FText& InLabel);
	void RefreshList();
	bool HasCompletedTodos() const;

private:
	TArray<TSharedPtr<FTodoItem>> TodoItems;
	TSharedPtr<SListView<TSharedPtr<FTodoItem>>> TodoListView;
	TSharedPtr<SEditableTextBox> TodoInput;
};
