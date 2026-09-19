#pragma once

#include "CoreMinimal.h"
#include "DamageTypes.generated.h"

/**
 * Data describing damage and impulse applied to an actor
 */
USTRUCT(BlueprintType)
struct LEDGEOUT_API FDamageData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
	float Amount = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
	FVector KnockbackVector = FVector::ZeroVector;

	FDamageData() = default;

	FDamageData(float InAmount, const FVector& InKnockbackVector)
		: Amount(InAmount)
		, KnockbackVector(InKnockbackVector)
	{
	}
};

/**
 * Data describing a hitbox and its associated damage properties
 */
USTRUCT(BlueprintType)
struct LEDGEOUT_API FHitboxData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hitbox")
	FDamageData DamageData;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hitbox")
	FVector BoxSize = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hitbox")
	FVector BoxOffset = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hitbox")
	FName BoneAttachment = NAME_None;

	FHitboxData() = default;

	FHitboxData(const FDamageData& InDamageData, const FVector& InBoxSize, const FVector& InBoxOffset, FName InBoneAttachment)
		: DamageData(InDamageData)
		, BoxSize(InBoxSize)
		, BoxOffset(InBoxOffset)
		, BoneAttachment(InBoneAttachment)
	{
	}
};
