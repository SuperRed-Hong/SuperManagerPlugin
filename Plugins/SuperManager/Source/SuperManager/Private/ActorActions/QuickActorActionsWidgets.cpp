// Fill out your copyright notice in the Description page of Project Settings.


#include "ActorActions/QuickActorActionsWidgets.h"
#include "Subsystems/EditorActorSubsystem.h"
#include "AssetRegistry/AssetRegistryHelpers.h"
#include "Materials/Material.h"
#include "DebugHeader.h"
#include "ScopedTransaction.h"

#define LOCTEXT_NAMESPACE "QuickActorActions"

void UQuickActorActionsWidgets::SelectAllActorWithSimilarName()
{
	if (!GetEditorActorSubsystem()) { return; }

	TArray<AActor*> SelectedActors = EditorActorSubsystem->GetSelectedLevelActors();
	if (SelectedActors.Num() == 0)
	{
		DebugHeader::ShowNotifyInfo(TEXT("No Actor Selected"));
		return;
	}
	if (SelectedActors.Num() > 1)
	{
		DebugHeader::ShowNotifyInfo(TEXT("You can only select one actor"));
		return;
	}

	AActor* SelectedActor = SelectedActors[0];
	const FString SeedLabel = SelectedActor->GetActorLabel();
	const FString SeedCoreName = GetCoreActorName(SelectedActor);
	// 如果成功提取到“核心名字”，优先用它作为匹配关键字，避免硬编码裁剪后缀。
	const FString& NameToSearch = SeedCoreName.IsEmpty() ? SeedLabel : SeedCoreName;

	if (NameToSearch.IsEmpty())
	{
		DebugHeader::ShowNotifyInfo(TEXT("Actor name is empty"));
		return;
	}

	FScopedTransaction Transaction(LOCTEXT("SelectActorsTransaction", "Select Actors With Similar Name"));

	const TArray<AActor*> AllActorsInLevel = EditorActorSubsystem->GetAllLevelActors();
	TArray<AActor*> MatchingActors;
	MatchingActors.Reserve(AllActorsInLevel.Num());

	for (AActor* Actor : AllActorsInLevel)
	{
		if (DoesActorNameMatchRule(NameToSearch, Actor))
		{
			MatchingActors.Add(Actor);
		}
	}

	if (MatchingActors.Num() == 0)
	{
		DebugHeader::ShowNotifyInfo(TEXT("No Actors Found with similar name"));
		Transaction.Cancel();
		return;
	}

	EditorActorSubsystem->SetSelectedLevelActors(MatchingActors);
	DebugHeader::ShowNotifyInfo(
		TEXT("Successfully Selected ") + FString::FromInt(MatchingActors.Num()) + TEXT(" actors"));
}

void UQuickActorActionsWidgets::SelectActorsByName()
{
	if (!GetEditorActorSubsystem())
	{
		return;
	}

	// Trim whitespace so the name comparison behaves predictably.
	const FString TrimmedName = SearchName;
	if (TrimmedName.IsEmpty())
	{
		DebugHeader::ShowNotifyInfo(TEXT("Please provide a valid actor name"));
		return;
	}

	FScopedTransaction Transaction(LOCTEXT("SelectActorsByNameTransaction", "Select Actors By Name"));

	const TArray<AActor*> AllActorsInLevel = EditorActorSubsystem->GetAllLevelActors();
	TArray<AActor*> MatchingActors;
	MatchingActors.Reserve(AllActorsInLevel.Num());

	for (AActor* Actor : AllActorsInLevel)
	{
		if (DoesActorNameMatchRule(TrimmedName, Actor))
		{
			MatchingActors.Add(Actor);
		}
	}

	if (MatchingActors.Num() == 0)
	{
		DebugHeader::ShowNotifyInfo(TEXT("No actors found for the given name"));
		Transaction.Cancel();
		return;
	}

	EditorActorSubsystem->SetSelectedLevelActors(MatchingActors);
	DebugHeader::ShowNotifyInfo(
		TEXT("Successfully Selected ") + FString::FromInt(MatchingActors.Num()) + TEXT(" actors"));
}


#pragma region ActorBatchDuplication
void UQuickActorActionsWidgets::DuplicateActors()
{
	if (!GetEditorActorSubsystem()) { return; }

	FScopedTransaction Transaction(LOCTEXT("DuplicateActorsTransaction", "Duplicate Actors"));
	
	TArray<AActor*> SelectedActors = EditorActorSubsystem->GetSelectedLevelActors();
	uint32 DuplicationCounter = 0;
	if (SelectedActors.Num() == 0)
	{
		DebugHeader::ShowNotifyInfo(TEXT("Select an Actor First"));
		return;
	}
	if (NumOfDuplicates <= 0 || OffsetDist <= 0.f || DuplicationAxis == E_DuplicationAxis::EDA_MAX)
	{
		DebugHeader::ShowNotifyInfo(
			TEXT("Please Specify a number of duplicates, offset distance amd duplication axis"));
		return;
	}

	// It's good practice to clear the selection before populating it with the new duplicated actors
	//EditorActorSubsystem->SetSelectedLevelActors(TArray<AActor*>());

	const int32 TotalDuplicationOps = SelectedActors.Num() * NumOfDuplicates;
	TUniquePtr<FScopedSlowTask> DuplicationTask;
	if (TotalDuplicationOps > 0)
	{
		DuplicationTask = DebugHeader::CreateProgressTask(
			static_cast<float>(TotalDuplicationOps),
			LOCTEXT("DuplicateActorsProgress", "Duplicating selected actors..."));
	}
	bool bWasCancelled = false;
	
	for (AActor* SelectedActor : SelectedActors)
	{
		if (!SelectedActor) { continue; }
		if (bWasCancelled) { break; }

		for (int32 i = 0; i < NumOfDuplicates; i++)
		{
			if (bWasCancelled) { break; }

			if (DuplicationTask.IsValid())
			{
				const FText DetailText = SelectedActor
					? FText::Format(LOCTEXT("DuplicateActorsProgressDetail", "Duplicating {0} ({1}/{2})"),
					                FText::FromString(SelectedActor->GetActorLabel()),
					                FText::AsNumber(i + 1),
					                FText::AsNumber(NumOfDuplicates))
					: LOCTEXT("DuplicateActorsProgressDetailUnknown", "Duplicating actor");

				DuplicationTask->EnterProgressFrame(1.f, DetailText);

				if (DuplicationTask->ShouldCancel())
				{
					bWasCancelled = true;
					break;
				}
			}

			float DuplicationOffset = OffsetDist * (i + 1);
			AActor* DuplicatedActor = nullptr;
			switch (DuplicationAxis)
			{
			case E_DuplicationAxis::EDA_XAxis:
				DuplicatedActor = EditorActorSubsystem->DuplicateActor(SelectedActor, SelectedActor->GetWorld(),
				                                                       FVector(DuplicationOffset, 0, 0));
				break;
			case E_DuplicationAxis::EDA_YAxis:
				DuplicatedActor = EditorActorSubsystem->DuplicateActor(SelectedActor, SelectedActor->GetWorld(),
				                                                       FVector(0, DuplicationOffset, 0));
				break;
			case E_DuplicationAxis::EDA_ZAxis:
				DuplicatedActor = EditorActorSubsystem->DuplicateActor(SelectedActor, SelectedActor->GetWorld(),
				                                                       FVector(0, 0, DuplicationOffset));
				break;
			case E_DuplicationAxis::EDA_MAX:
				break;
			}
			if (DuplicatedActor)
			{
				EditorActorSubsystem->SetActorSelectionState(DuplicatedActor, true);
				DuplicationCounter++;
			}
			else
			{
				DebugHeader::Print(TEXT("Duplicate actor fail  ")) ;
			}
		}

		if (bWasCancelled)
		{
			break;
		}
	}

	if (bWasCancelled)
	{
		DebugHeader::ShowNotifyInfo(TEXT("Actor duplication cancelled by user."));
		return;
	}

	if (DuplicationCounter > 0)
	{
		DebugHeader::ShowNotifyInfo(
			TEXT("Successfully Duplicated ") + FString::FromInt(DuplicationCounter) + TEXT(" actors"));
	}
}
#pragma endregion
#pragma region Randomize Actor Transform
void UQuickActorActionsWidgets::RandomizeActorTransform()
{
	if (!GetEditorActorSubsystem()) { return; }

	FScopedTransaction Transaction(LOCTEXT("RandomizeActorTransformTransaction", "Randomize Actor Transform"));

	TArray<AActor*> SelectedActors = EditorActorSubsystem->GetSelectedLevelActors();

	if (SelectedActors.Num() == 0)
	{
		DebugHeader::ShowNotifyInfo(TEXT("No actor selected to randomize transform."));
		return;
	}

	if (!TransformSettings.bRandomizeLocation && !TransformSettings.bRandomizeRotation && !TransformSettings.bRandomizeScale)
	{
		DebugHeader::ShowNotifyInfo(TEXT("Please select at least one transform property to randomize."));
		return;
	}

	for (AActor* SelectedActor : SelectedActors)
	{
		if(!SelectedActor) continue;

		SelectedActor->Modify();

		FTransform NewTransform = SelectedActor->GetActorTransform();
		
		// Randomize Location
		if (TransformSettings.bRandomizeLocation)
		{
			FVector OriginalLocation = NewTransform.GetLocation();
			FVector LocationOffset = FVector(
				FMath::FRandRange(-TransformSettings.LocationVariation.X, TransformSettings.LocationVariation.X),
				FMath::FRandRange(-TransformSettings.LocationVariation.Y, TransformSettings.LocationVariation.Y),
				FMath::FRandRange(-TransformSettings.LocationVariation.Z, TransformSettings.LocationVariation.Z)
			);
			NewTransform.SetLocation(OriginalLocation + LocationOffset);
		}

		// Randomize Rotation
		if (TransformSettings.bRandomizeRotation)
		{
			FRotator NewRotation = NewTransform.GetRotation().Rotator();
			const FRandomRotationSettings& RotationSettings = TransformSettings.RotationSettings;

			if (RotationSettings.bRandomizePitch)
			{
				NewRotation.Pitch += FMath::FRandRange(RotationSettings.PitchMin, RotationSettings.PitchMax);
			}
			if (RotationSettings.bRandomizeRoll)
			{
				NewRotation.Roll += FMath::FRandRange(RotationSettings.RollMin, RotationSettings.RollMax);
			}
			if (RotationSettings.bRandomizeYaw)
			{
				NewRotation.Yaw += FMath::FRandRange(RotationSettings.YawMin, RotationSettings.YawMax);
			}
			NewTransform.SetRotation(NewRotation.Quaternion());
		}

		// Randomize Scale
		if (TransformSettings.bRandomizeScale)
		{
			FVector OriginalScale = NewTransform.GetScale3D();
			float ScaleMultiplier = FMath::FRandRange(
				1.f - TransformSettings.ScaleVariationPercentage,
				1.f + TransformSettings.ScaleVariationPercentage
			);

			if (TransformSettings.bUniformScale)
			{
				NewTransform.SetScale3D(OriginalScale * ScaleMultiplier);
			}
			else
			{
				FVector NonUniformScale;
				NonUniformScale.X = OriginalScale.X * FMath::FRandRange(1.f - TransformSettings.ScaleVariationPercentage, 1.f + TransformSettings.ScaleVariationPercentage);
				NonUniformScale.Y = OriginalScale.Y * FMath::FRandRange(1.f - TransformSettings.ScaleVariationPercentage, 1.f + TransformSettings.ScaleVariationPercentage);
				NonUniformScale.Z = OriginalScale.Z * FMath::FRandRange(1.f - TransformSettings.ScaleVariationPercentage, 1.f + TransformSettings.ScaleVariationPercentage);
				NewTransform.SetScale3D(NonUniformScale);
			}
		}
		
		SelectedActor->SetActorTransform(NewTransform);
	}

	DebugHeader::ShowNotifyInfo(TEXT("Randomized transform for ") + FString::FromInt(SelectedActors.Num()) + TEXT(" actors."));
}

#pragma endregion

#pragma region Helper Functions
bool UQuickActorActionsWidgets::GetEditorActorSubsystem()
{
	if (!EditorActorSubsystem)
	{
		EditorActorSubsystem = GEditor->GetEditorSubsystem<UEditorActorSubsystem>();
	}
	return EditorActorSubsystem != nullptr;
}

bool UQuickActorActionsWidgets::DoesActorNameMatchRule(const FString& TargetName, const AActor* Candidate) const
{
	if (!Candidate || TargetName.IsEmpty())
	{
		return false;
	}

	const FString CandidateCoreName = GetCoreActorName(Candidate);
	const FString CandidateLabel = Candidate->GetActorLabel();
	const FString& NameToTest = CandidateCoreName.IsEmpty() ? CandidateLabel : CandidateCoreName;

	const ESearchCase::Type CaseOption = SearchCase;

	// FString::StartsWith / EndsWith / Contains / Equals compare prefix, suffix, substring, or exact text.
	// Passing SearchCase lets us decide whether the comparison is case sensitive.
	switch (SimilarNameMatchRule)
	{
	case E_SimilarNameMatchRule::Prefix:
		return NameToTest.StartsWith(TargetName, CaseOption);
	case E_SimilarNameMatchRule::Suffix:
		return NameToTest.EndsWith(TargetName, CaseOption);
	case E_SimilarNameMatchRule::Contains:
		return NameToTest.Contains(TargetName, CaseOption);
	case E_SimilarNameMatchRule::Exact:
		return NameToTest.Equals(TargetName, CaseOption);
	default:
		return false;
	}
}

FString UQuickActorActionsWidgets::GetCoreActorName(const AActor* InActor)
{
	if (!InActor)
	{
		return FString();
	}

	const FString ActorName = InActor->GetActorLabel();
	if (ActorName.IsEmpty())
	{
		return FString();
	}

	int32 StartIndex = 0;
	const int32 FirstUnderscore = ActorName.Find(TEXT("_"));
	if (FirstUnderscore != INDEX_NONE)
	{
		StartIndex = FirstUnderscore + 1;
	}

	int32 EndIndex = ActorName.Len() - 1;
	while (EndIndex >= StartIndex)
	{
		const TCHAR Char = ActorName[EndIndex];
		// Strip trailing digits/underscores so numbered suffixes (e.g. _01) do not affect the core name.
		if (!FChar::IsDigit(Char) && Char != TEXT('_'))
		{
			break;
		}
		--EndIndex;
	}

	if (EndIndex < StartIndex)
	{
		return FString();
	}

	return ActorName.Mid(StartIndex, EndIndex - StartIndex + 1);
}
#pragma endregion
#undef LOCTEXT_NAMESPACE


