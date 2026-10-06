// Fill out your copyright notice in the Description page of Project Settings.


#include "Public/Widgets/MainMenuWidget.h"

#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/CheckBox.h"
#include "Components/WidgetSwitcher.h"
#include "Framework/OnlineSessionsSubsystem.h"
#include "GameFramework/PlayerState.h"
#include "Kismet/GameplayStatics.h"

void UMainMenuWidget::NativeConstruct()
{
	// Bind OnlineSubsystem status change
	UOnlineSessionsSubsystem* OnlineSessionsSubsystem = UGameplayStatics::GetGameInstance(this)->GetSubsystem<UOnlineSessionsSubsystem>();
	if (IsValid(OnlineSessionsSubsystem))
	{
		OnlineSessionsSubsystem->OnConnectionChanged.AddDynamic(this, &UMainMenuWidget::ConnectionStatusChanged);
	}
	
	// Bind Buttons to functions
	// Main Menu
	SearchLobby_Button->OnClicked.AddDynamic(this, &UMainMenuWidget::SearchLobby);
	CreateLobby_Button->OnClicked.AddDynamic(this, &UMainMenuWidget::HostLobby);
	ExitGame_Button->OnClicked.AddDynamic(this, &UMainMenuWidget::ExitGame);
	
	// Lobby
	StartMatch_Button->OnClicked.AddDynamic(this, &UMainMenuWidget::StartMatch);
	
	// Retriving Username
	if (IsValid(GetOwningPlayerState()))
	{
		Username_TextBlock->SetText(FText::FromString(GetOwningPlayerState()->GetPlayerName()));
	}
	else
	{
		Username_TextBlock->SetText(FText::FromString(TEXT("<Username>")));
	}
	
	// Setting up Input mode
	APlayerController* PC = GetOwningPlayer();
	if (IsValid(PC))
	{
		const FInputModeGameAndUI InputMode;
		PC->SetInputMode(InputMode);
		PC->SetShowMouseCursor(true);
	}
}

void UMainMenuWidget::ConnectionStatusChanged(const ELobbyStatus Status)
{
	if (GEngine){GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Green, TEXT("Connection Status: "));}
	switch (Status) {
	case ELobbyStatus::Disconnected:
		GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Orange, TEXT("Disconnected"));
		break;
	case ELobbyStatus::Host:
		GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Green, TEXT("Hosting"));
		break;
	case ELobbyStatus::Client:
		GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Cyan, TEXT("Client"));
		break;
	}
	
	const int MenuWidgetIndex = 0;
	const int LobbyWidgetIndex = 1;
	
	if (Status == ELobbyStatus::Disconnected)
	{
		WidgetSwitcher_Container->SetActiveWidgetIndex(MenuWidgetIndex);
		return;
	}
	
	WidgetSwitcher_Container->SetActiveWidgetIndex(LobbyWidgetIndex);
	if (Status == ELobbyStatus::Client)
	{
		StartMatch_Button->SetVisibility(ESlateVisibility::Collapsed);
		ReadyLobby_Button->SetVisibility(ESlateVisibility::Visible);
	}
	else // Must be Host
	{
		StartMatch_Button->SetVisibility(ESlateVisibility::Visible);
		ReadyLobby_Button->SetVisibility(ESlateVisibility::Collapsed);
	}
	
}

void UMainMenuWidget::SearchLobby()
{
	UE_LOG(LogTemp, Log, TEXT("Buscando lobby..."));
	
	if (UOnlineSessionsSubsystem* OnlineSessionsSubsystem = UGameplayStatics::GetGameInstance(this)->
		GetSubsystem<UOnlineSessionsSubsystem>(); IsValid(OnlineSessionsSubsystem))
	{
		bool bIsLan = false;
		if (const ECheckBoxState LanState = CheckBox_IsLan->GetCheckedState(); LanState == ECheckBoxState::Checked)
		{
			bIsLan = true;
		}
		
		OnlineSessionsSubsystem->SearchSessions(0, bIsLan);
	}
}

void UMainMenuWidget::HostLobby()
{
	UE_LOG(LogTemp, Log, TEXT("Creando lobby..."));

	if (UOnlineSessionsSubsystem* OnlineSessionsSubsystem = UGameplayStatics::GetGameInstance(this)->
		GetSubsystem<UOnlineSessionsSubsystem>(); IsValid(OnlineSessionsSubsystem))
	{
		bool bIsLan = false;
		if (const ECheckBoxState LanState = CheckBox_IsLan->GetCheckedState(); LanState == ECheckBoxState::Checked)
		{
			bIsLan = true;
		}
		
		OnlineSessionsSubsystem->HostSession(4, bIsLan);
	}
}

void UMainMenuWidget::ExitGame()
{
	UE_LOG(LogTemp, Log, TEXT("Saliendo del juego"));
}

void UMainMenuWidget::StartMatch()
{
	if (UOnlineSessionsSubsystem* OnlineSessionsSubsystem = UGameplayStatics::GetGameInstance(this)->
		GetSubsystem<UOnlineSessionsSubsystem>(); IsValid(OnlineSessionsSubsystem))
	{
		OnlineSessionsSubsystem->StartGameLevel(GameLevelPath);
	}
}

void UMainMenuWidget::SetReady(bool bIsReady)
{
}

void UMainMenuWidget::LeaveLobby()
{
}
