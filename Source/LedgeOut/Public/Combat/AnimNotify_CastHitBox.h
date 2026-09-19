#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "DamageTypes.h"
#include "AnimNotify_CastHitBox.generated.h"

/**
 * AnimNotify that casts a hitbox using FHitboxData from DamageTypes.h.
 * Can be previewed directly in the Animation Editor viewport.
 */
UCLASS(meta = (DisplayName = "Cast HitBox"))
class LEDGEOUT_API UAnimNotify_CastHitBox : public UAnimNotify
{
	GENERATED_BODY()

public:
	UAnimNotify_CastHitBox();

	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

	/** Hitbox data defining shape, bone attachment, offset, and damage */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hitbox")
	FHitboxData HitboxData;

	/** Collision object channel to detect (defaults to ECC_Pawn) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hitbox")
	TEnumAsByte<ECollisionChannel> CollisionChannel = ECC_Pawn;

	/** If true, draws the hitbox in the Animation Editor and in-game */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hitbox|Preview")
	bool bDrawDebug = true;

	/** Duration (seconds) to display the debug hitbox */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hitbox|Preview", meta = (EditCondition = "bDrawDebug"))
	float DebugLifeTime = 1.0f;
};
