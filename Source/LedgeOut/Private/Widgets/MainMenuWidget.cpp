// Fill out your copyright notice in the Description page of Project Settings.


#include "Public/Widgets/MainMenuWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"

void UMainMenuWidget::NativeConstruct()
{
	// Bind Buttons to functions
	SearchLobby_Button->OnClicked.AddDynamic(this, &UMainMenuWidget::SearchLobby);
	CreateLobby_Button->OnClicked.AddDynamic(this, &UMainMenuWidget::HostLobby);
	ExitGame_Button->OnClicked.AddDynamic(this, &UMainMenuWidget::ExitGame);
	
	// Retriving Username
	Username_TextBlock->SetText(FText::FromString(TEXT("Xx_Pablosaurio_xX")));
}

void UMainMenuWidget::SearchLobby()
{
	UE_LOG(LogTemp, Log, TEXT("Buscando lobby..."));
}

void UMainMenuWidget::HostLobby()
{
	UE_LOG(LogTemp, Log, TEXT("Creando lobby..."));
	
}

void UMainMenuWidget::ExitGame()
{
	UE_LOG(LogTemp, Log, TEXT("Saliendo del juego"));
}
