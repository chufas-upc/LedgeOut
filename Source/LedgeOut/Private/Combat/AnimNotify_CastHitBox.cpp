#include "Combat/AnimNotify_CastHitBox.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "Engine/OverlapResult.h"
#include "DrawDebugHelpers.h"
#include "Interfaces/Damageable.h"

UAnimNotify_CastHitBox::UAnimNotify_CastHitBox()
{
#if WITH_EDITORONLY_DATA
	bShouldFireInEditor = true;
#endif

	HitboxData.BoxSize = FVector(30.0f, 30.0f, 30.0f);
	HitboxData.DamageData.Amount = 0.05f;
	HitboxData.DamageData.KnockbackVector = FVector(400.0f, 0.0f, 200.0f);
}

void UAnimNotify_CastHitBox::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (!MeshComp)
	{
		return;
	}

	UWorld* World = MeshComp->GetWorld();
	if (!World && !MeshComp->GetOwner()->HasAuthority())
	{
		return;
	}
	
	FTransform BaseTransform;
	if (HitboxData.BoneAttachment != NAME_None && MeshComp->DoesSocketExist(HitboxData.BoneAttachment))
	{
		BaseTransform = MeshComp->GetSocketTransform(HitboxData.BoneAttachment, RTS_World);
	}
	else
	{
		BaseTransform = MeshComp->GetComponentTransform();
	}

	const FVector BoxCenter = BaseTransform.TransformPosition(HitboxData.BoxOffset);
	const FQuat BoxRotation = BaseTransform.GetRotation();
	const FVector BoxExtent = HitboxData.BoxSize.ComponentMax(FVector::ZeroVector);

	if (BoxExtent.IsNearlyZero())
	{
		return;
	}

	AActor* OwnerActor = MeshComp->GetOwner();
	bool bHit = false;

	// In-game collision query and damage
	if (World->IsGameWorld())
	{
		FCollisionObjectQueryParams ObjectParams(CollisionChannel);
		FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(CastHitBox), false, OwnerActor);
		if (OwnerActor)
		{
			QueryParams.AddIgnoredActor(OwnerActor);
		}

		TArray<FOverlapResult> Overlaps;
		const bool bHasOverlaps = World->OverlapMultiByObjectType(
			Overlaps,
			BoxCenter,
			BoxRotation,
			ObjectParams,
			FCollisionShape::MakeBox(BoxExtent),
			QueryParams
		);

		if (bHasOverlaps)
		{
			TSet<AActor*> DamagedActors;

			// Orient knockback relative to attacker's facing direction
			const FRotator AttackerRotation = OwnerActor ? OwnerActor->GetActorRotation() : MeshComp->GetComponentRotation();
			const FRotator YawOnly(0.0f, AttackerRotation.Yaw, 0.0f);
			FDamageData OrientedDamageData = HitboxData.DamageData;
			OrientedDamageData.KnockbackVector = YawOnly.RotateVector(HitboxData.DamageData.KnockbackVector);

			for (const FOverlapResult& Overlap : Overlaps)
			{
				AActor* HitActor = Overlap.GetActor();
				if (!HitActor || HitActor == OwnerActor || DamagedActors.Contains(HitActor))
				{
					continue;
				}

				DamagedActors.Add(HitActor);
				bHit = true;

				// Deliver damage via IDamageable interface
				if (HitActor->Implements<UDamageable>())
				{
					IDamageable::Execute_GetDamage(HitActor, OrientedDamageData);
				}
			}
		}
	}

	// Preview / Debug visualization (shows in both Persona animation editor and in-game)
	if (bDrawDebug)
	{
		const float LifeTime = World->IsGameWorld() ? DebugLifeTime : FMath::Max(DebugLifeTime, 1.0f);
		const FColor Color = bHit ? FColor::Green : FColor::Red;

		DrawDebugBox(
			World,
			BoxCenter,
			BoxExtent,
			BoxRotation,
			Color,
			false,
			LifeTime,
			0,
			1.5f
		);
	}
}
