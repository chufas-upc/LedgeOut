#include "Widgets/GameWidget.h"

#include "Characters/LedgeOutCharacter.h"
#include "Combat/CombatComponent.h"
#include "Components/TextBlock.h"
#include "Components/HorizontalBox.h"
#include "Framework/LedgeOutPlayerState.h"

void UGameWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	if (Health)
	{
		Health->SetText(FText::FromString(TEXT("0.0")));
	}
	
	BindPlayerState(Cast<ALedgeOutPlayerState>(GetOwningPlayerState()));
}

void UGameWidget::NativeDestruct()
{
	BindPlayerState(nullptr);
	Super::NativeDestruct();
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
	UpdateDamage(0.f);
}

void UGameWidget::BindPlayerState(ALedgeOutPlayerState* PlayerState)
{
	if (BoundPlayerState.Get() == PlayerState)
	{
		if (IsValid(PlayerState))
		{
			UpdateLives(PlayerState->Lives);
		}
		return;
	}

	if (ALedgeOutPlayerState* PreviousPlayerState = BoundPlayerState.Get())
	{
		PreviousPlayerState->OnLivesChanged.RemoveDynamic(this, &UGameWidget::UpdateLives);
	}

	BoundPlayerState = PlayerState;
	if (IsValid(PlayerState))
	{
		PlayerState->OnLivesChanged.AddUniqueDynamic(this, &UGameWidget::UpdateLives);
		UpdateLives(PlayerState->Lives);
	}
}

void UGameWidget::UpdateDamage(const float Damage)
{
	if (Health)
	{
		UE_LOG(LogTemp, Warning, TEXT("Actualizando daño a %f"), Damage);
		Health->SetText(FText::FromString(FString::Printf(TEXT("%.1f"), Damage * 100)));
	}
}

void UGameWidget::UpdateLives(int32 Lives)
{
	if (!LivesContainer)
	{
		return;
	}

	const int32 StocksSprites = LivesContainer->GetChildrenCount();
	if (StocksSprites < Lives) {return;}
	
	for (int32 i = StocksSprites; i > Lives; i--)
	{
		LivesContainer->RemoveChildAt(i - 1);
	}
}
