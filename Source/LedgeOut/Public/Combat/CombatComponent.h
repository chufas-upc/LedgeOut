#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "CombatComponent.generated.h"

class ALedgeOutCharacter;
struct FDamageData;

DECLARE_MULTICAST_DELEGATE_OneParam(F, const FDamageData&);
/**
 * 
 */
UCLASS()
class LEDGEOUT_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()
	
	UCombatComponent();
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	float Damage = 0.f;
	
	UPROPERTY()
	FVector KnockbackVector = FVector(0, 0, 0);
	
	UPROPERTY()
	bool bHasKnockback = false;
	
	UPROPERTY()
	bool bWantsToAttack = false;
	
	UPROPERTY()
	bool bCanAttack = true;
	
	UPROPERTY(EditDefaultsOnly, Category = "Combat|Combo")
	int32 ComboCounter = 0;
	
	UPROPERTY()
	TObjectPtr<ALedgeOutCharacter> Character;
	
	UFUNCTION()
	void PlayAttackAnimation();

public:
	virtual void BeginPlay() override;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float BaseKnockback = 0.1f;

	UFUNCTION(BlueprintCallable, Category = "Combat|Inputs")
	void TryAttack();
	
	UPROPERTY(EditDefaultsOnly, Category = "Combat|Animations")
	TArray<TObjectPtr<UAnimMontage>> PrimaryComboAnimations;
	
	UFUNCTION(BlueprintCallable, Category = "Combat|Animations")
	void StartComboWindow();
	
	UFUNCTION(BlueprintCallable, Category = "Combat|Animations")
	void ClearComboWindow(int32 AttackIndex);
	
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void HandleDamage(FDamageData DamageData);
	
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void RecoverFromDamage();
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|DEBUG")
	TObjectPtr<UAnimMontage> TestAnimationAsset;
};
