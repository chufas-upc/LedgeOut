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
	
protected:
	// Binding Widgets
	
	/* General */
	
	UPROPERTY(BlueprintReadOnly, Category = "General|Text", EditDefaultsOnly, meta = (BindWidget))
	TObjectPtr<UTextBlock> Username_TextBlock;
		
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, meta = (BindWidget))
	TObjectPtr<UWidgetSwitcher> WidgetSwitcher_Container;
	
	/* Main Menu */
	
	UPROPERTY(BlueprintReadOnly, Category = "Menu|Buttons", EditDefaultsOnly, meta = (BindWidget))
	TObjectPtr<UButton>  SearchLobby_Button;
	
	UPROPERTY(BlueprintReadOnly, Category = "Menu|Buttons", EditDefaultsOnly, meta = (BindWidget))
	TObjectPtr<UButton>  CreateLobby_Button;
	
	UPROPERTY(BlueprintReadOnly, Category = "Menu|Buttons", EditDefaultsOnly, meta = (BindWidget))
	TObjectPtr<UButton>  ExitGame_Button;
	
	UPROPERTY(BlueprintReadOnly, Category = "Menu|CheckBox", EditDefaultsOnly, meta = (BindWidget))
	TObjectPtr<UCheckBox> CheckBox_IsLan;
	
	/* Lobby */
	
	UPROPERTY(BlueprintReadOnly, Category = "Lobby|Buttons", EditDefaultsOnly, meta = (BindWidget))
	TObjectPtr<UButton>  StartMatch_Button;
	
	UPROPERTY(BlueprintReadOnly, Category = "Lobby|Buttons", EditDefaultsOnly, meta = (BindWidget))
	TObjectPtr<UButton>  ReadyLobby_Button;
	
	UPROPERTY(BlueprintReadOnly, Category = "Lobby|Text", EditDefaultsOnly, meta = (BindWidget))
	TObjectPtr<UTextBlock> ReadyLobby_TextBlock;
	
	UPROPERTY(BlueprintReadOnly, Category = "Lobby|Buttons", EditDefaultsOnly, meta = (BindWidget))
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

	UFUNCTION(BlueprintCallable, Category = "Lobby")
	void StartMatch();
	
	UFUNCTION(BlueprintCallable, Category = "Lobby")
	void ToggleReady();
	
	UFUNCTION(BlueprintCallable, Category = "Lobby")
	void SetReady(bool bIsReady);
	
	UFUNCTION(BlueprintCallable, Category = "Lobby")
	void LeaveLobby();

	UPROPERTY()
	bool bIsLocalReady = false;

protected:
	/** Can be implemented or overridden in Blueprints to update button visuals (colors, text, animations) */
	UFUNCTION(BlueprintNativeEvent, Category = "Lobby")
	void OnReadyStateChanged(bool bNewReady);
	virtual void OnReadyStateChanged_Implementation(bool bNewReady);
	
	

protected:
	UFUNCTION()
	virtual void NativeConstruct() override;
};
