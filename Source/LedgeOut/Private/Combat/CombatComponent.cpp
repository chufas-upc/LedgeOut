#include "Combat/CombatComponent.h"
#include "DamageTypes.h"
#include "Characters/LedgeOutCharacter.h"
#include "Net/UnrealNetwork.h"

UCombatComponent::UCombatComponent()
{
	// Start with tick enabled = false
	SetIsReplicated(true);
}

void UCombatComponent::PlayAttackAnimation()
{
	Character->PlayAnimMontage(PrimaryComboAnimations[ComboCounter]);
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			-1,
			5.f,
			FColor::Red,
			FString::Printf(TEXT("Playing attack: %d"), ComboCounter));
	}
	
	bCanAttack = false;
	bWantsToAttack = false;
	
	ComboCounter++;
	if (ComboCounter >= PrimaryComboAnimations.Num())
	{
		UE_LOG(LogTemp, Log, TEXT("Resto!"));
		ComboCounter = 0;
	}
}

void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();
	Character = Cast<ALedgeOutCharacter>(GetOwner()); 
}

void UCombatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UCombatComponent, Damage);
	DOREPLIFETIME(UCombatComponent, ComboCounter);
}

void UCombatComponent::TryAttack()
{
	UE_LOG(LogTemp, Log, TEXT("TryAttack - bIsAttacking: %s, bCanAttack: %s, ComboCounter: %d"),
		false ? TEXT("true") : TEXT("false"),
		bCanAttack ? TEXT("true") : TEXT("false"),
		ComboCounter);

	// Basic fallbacks
	if (!bCanAttack)
	{
		bWantsToAttack = true;
		return;
	}
	
	if (PrimaryComboAnimations.IsEmpty())
	{
		Character->PlayAnimMontage(TestAnimationAsset);
		return;
	}
	
	PlayAttackAnimation();
}

void UCombatComponent::StartComboWindow()
{
	bCanAttack = true;
	if (bWantsToAttack)
	{
		TryAttack();
	}
}

void UCombatComponent::ClearComboWindow(const int32 AttackIndex)
{
	if (ComboCounter == AttackIndex + 1)
	{
		UE_LOG(LogTemp, Log, TEXT("Clearing Combo Window"));
		bCanAttack = true;
		ComboCounter = 0;
	}
}

void UCombatComponent::HandleDamage(FDamageData DamageData)
{
	Damage += DamageData.Amount;
	Character->LaunchCharacter(DamageData.KnockbackVector * (BaseKnockback + Damage), true, true);
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			-1,
			5.f,
			FColor::Yellow,
			FString::Printf(TEXT("Damage Dealth: %f Current Damage: %f"), DamageData.Amount, Damage));
	}
}

void UCombatComponent::RecoverFromDamage()
{
}
