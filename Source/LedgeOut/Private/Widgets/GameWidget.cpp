#include "Widgets/GameWidget.h"

#include "Characters/LedgeOutCharacter.h"
#include "Combat/CombatComponent.h"
#include "Components/TextBlock.h"

void UGameWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	if (Health)
	{
		Health->SetText(FText::FromString(TEXT("0.0")));
	}
}

void UGameWidget::BindPawn(APawn* Pawn)
{
	//Binding Events
	ALedgeOutCharacter* Character = Cast<ALedgeOutCharacter>(Pawn);
	if (!IsValid(Character))
	{
		UE_LOG(LogTemp, Warning, TEXT("Character is invalid"));
		return;
	}
	
	UCombatComponent* CombatComponent = Character->GetComponentByClass<UCombatComponent>();
	if (!IsValid(CombatComponent))
	{
		UE_LOG(LogTemp, Warning, TEXT("Component is invalid"));
		return;
	}
	CombatComponent->OnDamageChange.AddDynamic(this, &UGameWidget::UpdateDamage);
}

void UGameWidget::UpdateDamage(const float Damage)
{
	if (Health)
	{
		UE_LOG(LogTemp, Warning, TEXT("Actualizando daño a %f"), Damage);
		Health->SetText(FText::FromString(FString::Printf(TEXT("%.1f"), Damage * 100)));
	}
}
