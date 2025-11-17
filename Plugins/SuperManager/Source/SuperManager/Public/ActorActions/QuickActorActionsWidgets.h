// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EditorUtilityWidget.h"
#include "QuickActorActionsWidgets.generated.h"

UENUM(BlueprintType)
enum class E_DuplicationAxis : uint8
{
	EDA_XAxis UMETA(DisplayName = "X Axis"),
	EDA_YAxis UMETA(DisplayName = "Y Axis"),
	EDA_ZAxis UMETA(DisplayName = "Z Axis"),
	EDA_MAX UMETA(DisplayName = "Default Max")
};

UENUM(BlueprintType)
enum class E_SimilarNameMatchRule : uint8
{
	Prefix UMETA(DisplayName = "Starts With"),
	Suffix UMETA(DisplayName = "Ends With"),
	Contains UMETA(DisplayName = "Contains"),
	Exact UMETA(DisplayName = "Exact")
};

USTRUCT(BlueprintType)
struct FRandomRotationSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ActorRandomRotation")
	bool bRandomizePitch = false;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ActorRandomRotation", meta = (EditCondition = "bRandomizePitch", UIMin = "-180.0", UIMax = "180.0"))
	float PitchMin = -45.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ActorRandomRotation", meta = (EditCondition = "bRandomizePitch", UIMin = "-180.0", UIMax = "180.0"))
	float PitchMax = 45.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ActorRandomRotation")
	bool bRandomizeRoll = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ActorRandomRotation", meta = (EditCondition = "bRandomizeRoll", UIMin = "-180.0", UIMax = "180.0"))
	float RollMin = -45.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ActorRandomRotation", meta = (EditCondition = "bRandomizeRoll", UIMin = "-180.0", UIMax = "180.0"))
	float RollMax = 45.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ActorRandomRotation")
	bool bRandomizeYaw = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ActorRandomRotation", meta = (EditCondition = "bRandomizeYaw", UIMin = "-180.0", UIMax = "180.0"))
	float YawMin = -45.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ActorRandomRotation", meta = (EditCondition = "bRandomizeYaw", UIMin = "-180.0", UIMax = "180.0"))
	float YawMax = 45.f;
};

USTRUCT(BlueprintType)
struct FRandomTransformSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RandomTransform")
	bool bRandomizeLocation = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RandomTransform", meta = (EditCondition = "bRandomizeLocation"))
	FVector LocationVariation = FVector(100.f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RandomTransform")
	bool bRandomizeRotation = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RandomTransform", meta = (EditCondition = "bRandomizeRotation"/*, ShowOnlyInnerProperties*/))
	FRandomRotationSettings RotationSettings;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RandomTransform")
	bool bRandomizeScale = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RandomTransform", meta = (EditCondition = "bRandomizeScale"))
	bool bUniformScale = true;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RandomTransform", meta = (EditCondition = "bRandomizeScale", UIMin = "0.0", UIMax = "1.0"))
	float ScaleVariationPercentage = 0.1f;
};

/**
 * 
 */
UCLASS()
class SUPERMANAGER_API UQuickActorActionsWidgets : public UEditorUtilityWidget
{
	GENERATED_BODY()

public:
	/** Allows selecting actors by entering a name string without needing a pre-selection. */
	UFUNCTION(BlueprintCallable, Category = "QuickActorActionsCore")
	void SelectActorsByName();
	UFUNCTION(BlueprintCallable, Category = "QuickActorActionsCore")
	void SelectAllActorWithSimilarName();
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ActorBatchSelection")
	TEnumAsByte<ESearchCase::Type> SearchCase = ESearchCase::CaseSensitive;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ActorBatchSelection")
	E_SimilarNameMatchRule SimilarNameMatchRule = E_SimilarNameMatchRule::Prefix;
	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category = "ActorBatchSelection")
	FString SearchName;
	
#pragma region ActorBatchDuplication
	UFUNCTION(BlueprintCallable, Category = "QuickActorActionsCore")
	void DuplicateActors();
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ActorBatchDuplication")
	E_DuplicationAxis DuplicationAxis = E_DuplicationAxis::EDA_XAxis;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ActorBatchDuplication")
	int32 NumOfDuplicates = 5;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ActorBatchDuplication")
	float OffsetDist = 300.f;
#pragma endregion

#pragma region ActorRandomization
	UFUNCTION(BlueprintCallable, Category = "QuickActorActionsCore")
	void RandomizeActorTransform();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ActorRandomization", meta = (ShowOnlyInnerProperties))
	FRandomTransformSettings TransformSettings;
#pragma endregion
	
private:
	UPROPERTY()
	class UEditorActorSubsystem* EditorActorSubsystem;
	bool GetEditorActorSubsystem();
	bool DoesActorNameMatchRule(const FString& TargetName, const AActor* Candidate) const;
	static FString GetCoreActorName(const AActor* InActor);

};
