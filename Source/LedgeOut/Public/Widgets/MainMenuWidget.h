// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MainMenuWidget.generated.h"

class UButton;
class UTextBlock;

/**
 * 
 */
UCLASS()
class LEDGEOUT_API UMainMenuWidget : public UUserWidget
{
	GENERATED_BODY()
	
	// Binding Widgets
	
	UPROPERTY(Category = "Buttons", EditDefaultsOnly, meta = (BindWidget))
	TObjectPtr<UButton>  SearchLobby_Button;
	
	UPROPERTY(Category = "Buttons", EditDefaultsOnly, meta = (BindWidget))
	TObjectPtr<UButton>  CreateLobby_Button;
	
	UPROPERTY(Category = "Buttons", EditDefaultsOnly, meta = (BindWidget))
	TObjectPtr<UButton>  ExitGame_Button;
	
	UPROPERTY(Category = "Text", EditDefaultsOnly, meta = (BindWidget))
	TObjectPtr<UTextBlock> Username_TextBlock;
	
public:
	
private:
	UFUNCTION()
	void SearchLobby();
	
	UFUNCTION()
	void HostLobby();
	
	UFUNCTION()
	void ExitGame();

protected:
	virtual void NativeConstruct() override;
};
