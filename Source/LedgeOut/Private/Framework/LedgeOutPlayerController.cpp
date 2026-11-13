#include "Framework/LedgeOutPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "Framework/LedgeOutGameInstance.h"
#include "Framework/LedgeOutPlayerState.h"
#include "Framework/OnlineSessionsSubsystem.h"
#include "InputMappingContext.h"
#include "Widgets/GameWidget.h"

void ALedgeOutPlayerController::BeginPlay()
{
	Super::BeginPlay();
}

void ALedgeOutPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	// only add IMCs for local player controllers
	if (IsLocalPlayerController())
	{
		// Add Input Mapping Contexts
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
		{
			for (UInputMappingContext* CurrentContext : DefaultMappingContexts)
			{
				Subsystem->AddMappingContext(CurrentContext, 0);
			}
		}
		
		// Setup GameInput and Cursor
		SetShowMouseCursor(false);
		const FInputModeGameOnly InputMode; 
		SetInputMode(InputMode);
		
		// Setting up Game Widget
		GameWidget = CreateWidget<UGameWidget>(this, GameWidgetClass);
		if (GameWidget)
		{
			GameWidget->AddToPlayerScreen();
			BindGameWidgetToPlayerState();
		}
	}
}

void ALedgeOutPlayerController::AcknowledgePossession(APawn* P)
{
	Super::AcknowledgePossession(P);
	if (GameWidget)
	{
		GameWidget->BindPawn(P);
	}
}

void ALedgeOutPlayerController::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();
	BindGameWidgetToPlayerState();
}

void ALedgeOutPlayerController::BindGameWidgetToPlayerState()
{
	if (GameWidget)
	{
		GameWidget->BindPlayerState(GetPlayerState<ALedgeOutPlayerState>());
	}
}

void ALedgeOutPlayerController::ClientShowVictoryMessage_Implementation(const FString& WinnerName)
{
	SetIgnoreMoveInput(true);
	SetIgnoreLookInput(true);

	if (GameWidget)
	{
		GameWidget->ShowVictoryMessage(WinnerName);
	}
}

void ALedgeOutPlayerController::ClientDisconnectAfterRound_Implementation()
{
	if (ULedgeOutGameInstance* GameInstance = Cast<ULedgeOutGameInstance>(GetGameInstance()))
	{
		GameInstance->ClearLobbySelections();
		if (UOnlineSessionsSubsystem* Sessions = GameInstance->GetSubsystem<UOnlineSessionsSubsystem>())
		{
			Sessions->ResetAfterRound();
		}
	}

	if (UGameInstance* GameInstance = GetGameInstance())
	{
		GameInstance->ReturnToMainMenu();
	}
}
