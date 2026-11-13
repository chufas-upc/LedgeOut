#include "Framework/LedgeOutPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "Framework/LedgeOutPlayerState.h"
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
