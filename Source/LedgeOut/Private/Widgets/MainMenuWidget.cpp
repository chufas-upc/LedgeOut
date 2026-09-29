// Fill out your copyright notice in the Description page of Project Settings.


#include "Public/Widgets/MainMenuWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Framework/OnlineSessionsSubsystem.h"
#include "GameFramework/PlayerState.h"
#include "Kismet/GameplayStatics.h"

void UMainMenuWidget::NativeConstruct()
{
	// Bind Buttons to functions
	SearchLobby_Button->OnClicked.AddDynamic(this, &UMainMenuWidget::SearchLobby);
	CreateLobby_Button->OnClicked.AddDynamic(this, &UMainMenuWidget::HostLobby);
	ExitGame_Button->OnClicked.AddDynamic(this, &UMainMenuWidget::ExitGame);
	
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
		const FInputModeUIOnly InputMode;
		PC->SetInputMode(InputMode);
		PC->SetShowMouseCursor(true);
	}
}

void UMainMenuWidget::SearchLobby()
{
	UE_LOG(LogTemp, Log, TEXT("Buscando lobby..."));
	
	FString IP = "127.0.0.1";
	UGameplayStatics::OpenLevel(this, *IP);
}

void UMainMenuWidget::HostLobby()
{
	UE_LOG(LogTemp, Log, TEXT("Creando lobby..."));

	if (UOnlineSessionsSubsystem* OnlineSessionsSubsystem =  UGameplayStatics::GetGameInstance(this)->
		GetSubsystem<UOnlineSessionsSubsystem>(); IsValid(OnlineSessionsSubsystem))
	{
		OnlineSessionsSubsystem->HostSession(4, true);
	}
}

void UMainMenuWidget::ExitGame()
{
	UE_LOG(LogTemp, Log, TEXT("Saliendo del juego"));
}
