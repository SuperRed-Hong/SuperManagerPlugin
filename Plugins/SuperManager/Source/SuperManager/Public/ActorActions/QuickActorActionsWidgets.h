// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EditorUtilityWidget.h"
#include "QuickActorActionsWidgets.generated.h"

/** 批量复制 Actor 时沿哪个轴进行偏移。 */
UENUM(BlueprintType)
enum class E_DuplicationAxis : uint8
{
	EDA_XAxis UMETA(DisplayName = "X Axis"),
	EDA_YAxis UMETA(DisplayName = "Y Axis"),
	EDA_ZAxis UMETA(DisplayName = "Z Axis"),
	EDA_MAX UMETA(DisplayName = "Default Max")
};

/** 批量筛选 Actor 名称时的匹配方式。 */
UENUM(BlueprintType)
enum class E_SimilarNameMatchRule : uint8
{
	Prefix UMETA(DisplayName = "Starts With"),
	Suffix UMETA(DisplayName = "Ends With"),
	Contains UMETA(DisplayName = "Contains"),
	Exact UMETA(DisplayName = "Exact")
};

/** 批量随机旋转的轴向与角度区间设置。 */
USTRUCT(BlueprintType)
struct FRandomRotationSettings
{
	GENERATED_BODY()

	/** 是否随机 Pitch。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ActorRandomRotation")
	bool bRandomizePitch = false;

	/** Pitch 随机下限（度）。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ActorRandomRotation", meta = (EditCondition = "bRandomizePitch", UIMin = "-180.0", UIMax = "180.0"))
	float PitchMin = -45.f;

	/** Pitch 随机上限（度）。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ActorRandomRotation", meta = (EditCondition = "bRandomizePitch", UIMin = "-180.0", UIMax = "180.0"))
	float PitchMax = 45.f;

	/** 是否随机 Roll。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ActorRandomRotation")
	bool bRandomizeRoll = false;

	/** Roll 随机下限（度）。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ActorRandomRotation", meta = (EditCondition = "bRandomizeRoll", UIMin = "-180.0", UIMax = "180.0"))
	float RollMin = -45.f;

	/** Roll 随机上限（度）。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ActorRandomRotation", meta = (EditCondition = "bRandomizeRoll", UIMin = "-180.0", UIMax = "180.0"))
	float RollMax = 45.f;

	/** 是否随机 Yaw。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ActorRandomRotation")
	bool bRandomizeYaw = true;

	/** Yaw 随机下限（度）。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ActorRandomRotation", meta = (EditCondition = "bRandomizeYaw", UIMin = "-180.0", UIMax = "180.0"))
	float YawMin = -45.f;

	/** Yaw 随机上限（度）。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ActorRandomRotation", meta = (EditCondition = "bRandomizeYaw", UIMin = "-180.0", UIMax = "180.0"))
	float YawMax = 45.f;
};

/** 批量随机变换（位置/旋转/缩放）的综合配置。 */
USTRUCT(BlueprintType)
struct FRandomTransformSettings
{
	GENERATED_BODY()

	/** 是否对位置做随机偏移。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RandomTransform")
	bool bRandomizeLocation = false;

	/** 位置变动范围。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RandomTransform", meta = (EditCondition = "bRandomizeLocation"))
	FVector LocationVariation = FVector(100.f);

	/** 是否对旋转做随机偏移。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RandomTransform")
	bool bRandomizeRotation = false;

	/** 旋转随机化设定。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RandomTransform", meta = (EditCondition = "bRandomizeRotation"))
	FRandomRotationSettings RotationSettings;

	/** 是否对缩放做随机偏移。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RandomTransform")
	bool bRandomizeScale = false;

	/** 是否统一缩放。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RandomTransform", meta = (EditCondition = "bRandomizeScale"))
	bool bUniformScale = true;

	/** 缩放浮动百分比。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RandomTransform", meta = (EditCondition = "bRandomizeScale", UIMin = "0.0", UIMax = "1.0"))
	float ScaleVariationPercentage = 0.1f;
};

/** 提供批量选择、复制以及随机化 Actor 的编辑器实用面板。 */
UCLASS()
class SUPERMANAGER_API UQuickActorActionsWidgets : public UEditorUtilityWidget
{
	GENERATED_BODY()

public:
	/** 根据输入名称选中场景中的 Actor。 */
	UFUNCTION(BlueprintCallable, Category = "QuickActorActionsCore")
	void SelectActorsByName();

	/** 按照匹配规则批量选中同名 Actor。 */
	UFUNCTION(BlueprintCallable, Category = "QuickActorActionsCore")
	void SelectAllActorWithSimilarName();

	/** 控制名称搜索时的大小写敏感行为。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ActorBatchSelection")
	TEnumAsByte<ESearchCase::Type> SearchCase = ESearchCase::CaseSensitive;

	/** 定义查找类似名称的匹配规则。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ActorBatchSelection")
	E_SimilarNameMatchRule SimilarNameMatchRule = E_SimilarNameMatchRule::Prefix;

	/** 用户输入的关键字。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ActorBatchSelection")
	FString SearchName;
	
#pragma region ActorBatchDuplication
	/** 沿指定轴批量复制当前所选 Actor。 */
	UFUNCTION(BlueprintCallable, Category = "QuickActorActionsCore")
	void DuplicateActors();

	/** 复制时沿哪个轴偏移。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ActorBatchDuplication")
	E_DuplicationAxis DuplicationAxis = E_DuplicationAxis::EDA_XAxis;

	/** 需要生成多少个副本。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ActorBatchDuplication")
	int32 NumOfDuplicates = 5;

	/** 相邻副本之间的距离。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ActorBatchDuplication")
	float OffsetDist = 300.f;
#pragma endregion

#pragma region ActorRandomization
	/** 按照 TransformSettings 为所选 Actor 随机变换。 */
	UFUNCTION(BlueprintCallable, Category = "QuickActorActionsCore")
	void RandomizeActorTransform();

	/** 存放随机化的位移/旋转/缩放配置。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ActorRandomization", meta = (ShowOnlyInnerProperties))
	FRandomTransformSettings TransformSettings;
#pragma endregion
	
private:
	UPROPERTY()
	class UEditorActorSubsystem* EditorActorSubsystem;

	/** 获取或缓存 UEditorActorSubsystem。 */
	bool GetEditorActorSubsystem();
	/** 判断候选 Actor 名称是否符合匹配规则。 */
	bool DoesActorNameMatchRule(const FString& TargetName, const AActor* Candidate) const;
	/** 去掉附加后缀得到核心名称。 */
	static FString GetCoreActorName(const AActor* InActor);

};
