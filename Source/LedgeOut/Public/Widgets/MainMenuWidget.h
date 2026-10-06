// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Framework/LobbyPlayerData.h"
#include "MainMenuWidget.generated.h"

class UButton;
class UTextBlock;
class UCheckBox;
class UWidgetSwitcher;

/**
 * 
 */
UCLASS()
class LEDGEOUT_API UMainMenuWidget : public UUserWidget
{
	GENERATED_BODY()
	
	// Binding Widgets
	
	/* General */
	
	UPROPERTY(Category = "General|Text", EditDefaultsOnly, meta = (BindWidget))
	TObjectPtr<UTextBlock> Username_TextBlock;
		
	UPROPERTY(EditDefaultsOnly, meta = (BindWidget))
	TObjectPtr<UWidgetSwitcher> WidgetSwitcher_Container;
	
	/* Main Menu */
	
	UPROPERTY(Category = "Menu|Buttons", EditDefaultsOnly, meta = (BindWidget))
	TObjectPtr<UButton>  SearchLobby_Button;
	
	UPROPERTY(Category = "Menu|Buttons", EditDefaultsOnly, meta = (BindWidget))
	TObjectPtr<UButton>  CreateLobby_Button;
	
	UPROPERTY(Category = "Menu|Buttons", EditDefaultsOnly, meta = (BindWidget))
	TObjectPtr<UButton>  ExitGame_Button;
	
	UPROPERTY(Category = "Menu|CheckBox", EditDefaultsOnly, meta = (BindWidget))
	TObjectPtr<UCheckBox> CheckBox_IsLan;
	
	/* Lobby */
	
	UPROPERTY(Category = "Lobby|Buttons", EditDefaultsOnly, meta = (BindWidget))
	TObjectPtr<UButton>  StartMatch_Button;
	
	UPROPERTY(Category = "Lobby|Buttons", EditDefaultsOnly, meta = (BindWidget))
	TObjectPtr<UButton>  ReadyLobby_Button;
	
	UPROPERTY(Category = "Lobby|Buttons", EditDefaultsOnly, meta = (BindWidget))
	TObjectPtr<UButton>  ExitLobby_Button;

public:
	
	/* Testing level path in editor */
	UPROPERTY(EditDefaultsOnly)
	FString GameLevelPath = "";
	
private:
	
	/* General Functions */ 
	
	UFUNCTION()
	void SearchLobby();
	
	UFUNCTION()
	void HostLobby();
	
	UFUNCTION()
	void ExitGame();
	
	UFUNCTION()
	void ConnectionStatusChanged(ELobbyStatus Status);
	
	/* Lobby Managing */

	UFUNCTION()
	void StartMatch();
	
	UFUNCTION()
	void SetReady(bool bIsReady);
	
	UFUNCTION()
	void LeaveLobby();
	
	

protected:
	virtual void NativeConstruct() override;
};
