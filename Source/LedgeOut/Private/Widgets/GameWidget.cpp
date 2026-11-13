#include "Widgets/GameWidget.h"

#include "Characters/LedgeOutCharacter.h"
#include "Combat/CombatComponent.h"
#include "Components/TextBlock.h"
#include "Components/HorizontalBox.h"
#include "Framework/LedgeOutGameState.h"
#include "Framework/LedgeOutPlayerState.h"

void UGameWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	if (Health)
	{
		Health->SetText(FText::FromString(TEXT("0.0")));
	}

	if (VictoryMessage_TextBlock)
	{
		VictoryMessage_TextBlock->SetVisibility(ESlateVisibility::Collapsed);
	}

	BindPlayerState(Cast<ALedgeOutPlayerState>(GetOwningPlayerState()));

	TryBindGameState();
}

void UGameWidget::NativeDestruct()
{
	BindPlayerState(nullptr);
	if (ALedgeOutGameState* GameState = BoundGameState.Get())
	{
		GameState->OnRoundTimerUpdated.RemoveDynamic(this, &UGameWidget::UpdateRoundTimer);
	}
	BoundGameState.Reset();
	Super::NativeDestruct();
}

void UGameWidget::NativeTick(const FGeometry& MyGeometry, const float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	TryBindGameState();
}

void UGameWidget::TryBindGameState()
{
	if (BoundGameState.IsValid())
	{
		return;
	}

	if (UWorld* World = GetWorld())
	{
		ALedgeOutGameState* GameState = World->GetGameState<ALedgeOutGameState>();
		if (IsValid(GameState))
		{
			BoundGameState = GameState;
			GameState->OnRoundTimerUpdated.AddUniqueDynamic(this, &UGameWidget::UpdateRoundTimer);
			UpdateRoundTimer(GameState->RoundTimeSeconds, GameState->bIsSuddenDeath);
		}
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

void UGameWidget::UpdateRoundTimer(const int32 TimeSeconds, const bool bIsSuddenDeath)
{
	if (RoundTimer_TextBlock)
	{
		const int32 Minutes = FMath::Max(0, TimeSeconds) / 60;
		const int32 Seconds = FMath::Max(0, TimeSeconds) % 60;
		RoundTimer_TextBlock->SetText(
			FText::FromString(FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds)));
		RoundTimer_TextBlock->SetColorAndOpacity(
			FSlateColor(bIsSuddenDeath ? FLinearColor::Red : FLinearColor::White));
	}

	OnRoundTimerChanged(TimeSeconds, bIsSuddenDeath);
}

void UGameWidget::ShowVictoryMessage(const FString& WinnerName)
{
	if (VictoryMessage_TextBlock)
	{
		const FText VictoryText = FText::Format(
			NSLOCTEXT("LedgeOut", "VictoryMessage", "Victory! {0} wins!"),
			FText::FromString(WinnerName));
		VictoryMessage_TextBlock->SetText(VictoryText);
		VictoryMessage_TextBlock->SetVisibility(ESlateVisibility::Visible);
	}

	OnVictoryMessageShown(WinnerName);
}
