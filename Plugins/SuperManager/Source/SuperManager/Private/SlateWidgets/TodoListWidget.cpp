// Copyright Epic Games, Inc. All Rights Reserved.

#include "SlateWidgets/TodoListWidget.h"

#include "Algo/Count.h"
#include "SlateOptMacros.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"

#define LOCTEXT_NAMESPACE "STodoListWidget"

void STodoListWidget::Construct(const FArguments& InArgs)
{
	bCanSupportFocus = true;
	TodoItems = InArgs._InitialItems;

	if (TodoItems.Num() == 0)
	{
		AddTodoInternal(LOCTEXT("DefaultTodo", "示例：梳理 SuperManager Todo"));
	}

	ChildSlot
	[
		SNew(SBorder)
		.Padding(16.f)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.f, 0.f, 0.f, 8.f)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.FillWidth(1.f)
				[
					SAssignNew(TodoInput, SEditableTextBox)
					.HintText(LOCTEXT("TodoHint", "输入待办后按 Enter 或点击添加"))
					.OnTextCommitted(this, &STodoListWidget::HandleNewTodoCommitted)
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(8.f, 0.f, 0.f, 0.f)
				[
					SNew(SButton)
					.Text(LOCTEXT("AddTodo", "添加"))
					.OnClicked(this, &STodoListWidget::HandleAddTodo)
				]
			]
			+ SVerticalBox::Slot()
			.FillHeight(1.f)
			[
				SAssignNew(TodoListView, SListView<TSharedPtr<FTodoItem>>)
				.ListItemsSource(&TodoItems)
				.SelectionMode(ESelectionMode::None)
				.OnGenerateRow(this, &STodoListWidget::OnGenerateRow)
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.f, 8.f, 0.f, 0.f)
			[
				SNew(SButton)
				.Text_Lambda([this]()
				{
					const int32 CompletedCount = Algo::CountIf(TodoItems, [](const TSharedPtr<FTodoItem>& Item)
					{
						return Item.IsValid() && Item->bCompleted;
					});
					return CompletedCount > 0
						? FText::Format(LOCTEXT("ClearCompletedFormat", "清除已完成 ({0})"), CompletedCount)
						: LOCTEXT("ClearCompleted", "清除已完成");
				})
				.OnClicked(this, &STodoListWidget::HandleClearCompleted)
				.IsEnabled_Lambda([this]() { return HasCompletedTodos(); })
			]
		]
	];
}

FReply STodoListWidget::HandleAddTodo()
{
	if (!TodoInput.IsValid())
	{
		return FReply::Handled();
	}

	AddTodoInternal(TodoInput->GetText());
	return FReply::Handled();
}

void STodoListWidget::HandleNewTodoCommitted(const FText& InText, ETextCommit::Type CommitType)
{
	if (CommitType == ETextCommit::OnEnter)
	{
		AddTodoInternal(InText);
	}
}

void STodoListWidget::HandleToggleTodo(ECheckBoxState NewState, TSharedPtr<FTodoItem> Item)
{
	if (!Item.IsValid())
	{
		return;
	}

	Item->bCompleted = (NewState == ECheckBoxState::Checked);
	RefreshList();
}

FReply STodoListWidget::HandleClearCompleted()
{
	const int32 RemovedCount = TodoItems.RemoveAll([](const TSharedPtr<FTodoItem>& Item)
	{
		return Item.IsValid() && Item->bCompleted;
	});

	if (RemovedCount > 0)
	{
		RefreshList();
	}

	return FReply::Handled();
}

void STodoListWidget::HandleRemoveTodo(TSharedPtr<FTodoItem> Item)
{
	if (!Item.IsValid())
	{
		return;
	}

	TodoItems.Remove(Item);
	RefreshList();
}

TSharedRef<ITableRow> STodoListWidget::OnGenerateRow(TSharedPtr<FTodoItem> Item, const TSharedRef<STableViewBase>& OwnerTable)
{
	return SNew(STableRow<TSharedPtr<FTodoItem>>, OwnerTable)
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(0.f, 0.f, 8.f, 0.f)
		[
			SNew(SCheckBox)
			.IsChecked_Lambda([Item]()
			{
				return Item.IsValid() && Item->bCompleted
					? ECheckBoxState::Checked
					: ECheckBoxState::Unchecked;
			})
			.OnCheckStateChanged(this, &STodoListWidget::HandleToggleTodo, Item)
		]
		+ SHorizontalBox::Slot()
		.FillWidth(1.f)
		.VAlign(VAlign_Center)
		[
			SNew(STextBlock)
			.Text_Lambda([Item]()
			{
				return Item.IsValid() ? Item->Label : FText::GetEmpty();
			})
			.ColorAndOpacity_Lambda([Item]()
			{
				return Item.IsValid() && Item->bCompleted
					? FSlateColor(FLinearColor::FromSRGBColor(FColor::Silver))
					: FSlateColor::UseForeground();
			})
		]
		+ SHorizontalBox::Slot()
		.AutoWidth()
		[
			SNew(SButton)
			.Text(LOCTEXT("RemoveTodo", "删除"))
			.OnClicked_Lambda([this, Item]()
			{
				HandleRemoveTodo(Item);
				return FReply::Handled();
			})
		]
	];
}

void STodoListWidget::AddTodoInternal(const FText& InLabel)
{
	const FString Trimmed = InLabel.ToString().TrimStartAndEnd();
	if (Trimmed.IsEmpty())
	{
		if (TodoInput.IsValid())
		{
			TodoInput->SetError(LOCTEXT("EmptyTodoError", "待办内容不能为空"));
		}
		return;
	}

	if (TodoInput.IsValid())
	{
		TodoInput->SetError(FText::GetEmpty());
	}

	TSharedPtr<FTodoItem> NewItem = MakeShared<FTodoItem>(FGuid::NewGuid(), FText::FromString(Trimmed), false);
	TodoItems.Add(MoveTemp(NewItem));
	RefreshList();

	if (TodoInput.IsValid())
	{
		TodoInput->SetText(FText::GetEmpty());
	}
}

void STodoListWidget::RefreshList()
{
	if (TodoListView.IsValid())
	{
		TodoListView->RequestListRefresh();
	}
}

bool STodoListWidget::HasCompletedTodos() const
{
	return TodoItems.ContainsByPredicate([](const TSharedPtr<FTodoItem>& Item)
	{
		return Item.IsValid() && Item->bCompleted;
	});
}

#undef LOCTEXT_NAMESPACE
