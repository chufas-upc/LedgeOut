#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "CombatComponent.generated.h"

class ALedgeOutCharacter;
struct FDamageData;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDamageChange, float, Damage);
/**
 * 
 */
UCLASS()
class LEDGEOUT_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()
	
	UCombatComponent();
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat", ReplicatedUsing = OnRep_Damage, meta = (AllowPrivateAccess = "true"))
	float Damage = 0.f;
	
	UPROPERTY()
	FVector KnockbackVector = FVector(0, 0, 0);
	
	UPROPERTY()
	bool bHasKnockback = false;
	
	UPROPERTY()
	bool bWantsToAttack = false;
	
	UPROPERTY()
	bool bCanAttack = true;
	
	UPROPERTY(EditDefaultsOnly, Category = "Combat|Combo", Replicated)
	int32 ComboCounter = 0;
	
	UPROPERTY()
	TObjectPtr<ALedgeOutCharacter> Character;
	
	UFUNCTION(NetMulticast, Reliable, BlueprintCallable, Category = "Combat|Animations")
	void NetMulticast_ExecuteAttack();
	
	// Networking Replication
	UFUNCTION()
	void OnRep_Damage();

public:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	/* Evento al recibir daño */
	FOnDamageChange OnDamageChange;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float BaseKnockback = 0.1f;

	UFUNCTION(BlueprintCallable, Server, Reliable ,Category = "Combat|Inputs")
	void Server_TryAttack();
	
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
