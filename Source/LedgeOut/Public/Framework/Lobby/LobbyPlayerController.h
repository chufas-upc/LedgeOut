#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Framework/LobbyPlayerData.h"
#include "LobbyPlayerController.generated.h"

class UInputAction;
class UInputMappingContext;

/**
 * 
 */
UCLASS()
class LEDGEOUT_API ALobbyPlayerController : public APlayerController
{
	GENERATED_BODY()
	ALobbyPlayerController();

	ACameraActor* FindCameraByTag(FName Tag) const;

protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	
	UFUNCTION(BlueprintCallable)
	void SwitchToLobbyCamera();
	
	UFUNCTION(BlueprintCallable)
	void SwitchToMenuCamera();

	UFUNCTION(BlueprintCallable, Category = "Lobby|Character")
	void ChangeCharacter(int32 CharacterIndex, int32 LocalPlayerIndex = 0);

	UFUNCTION(BlueprintCallable, Category = "Lobby|Character")
	void SelectNextCharacter(int32 LocalPlayerIndex = 0);

	UFUNCTION(BlueprintCallable, Category = "Lobby|Character")
	void SelectPreviousCharacter(int32 LocalPlayerIndex = 0);

	UFUNCTION(BlueprintCallable, Category = "Lobby|Skin")
	void ChangeSkin(int32 SkinIndex, int32 LocalPlayerIndex = 0);

	UFUNCTION(BlueprintCallable, Category = "Lobby|Skin")
	void SelectNextSkin(int32 LocalPlayerIndex = 0);

	UFUNCTION(BlueprintCallable, Category = "Lobby|Skin")
	void SelectPreviousSkin(int32 LocalPlayerIndex = 0);

private:
	void StepCharacterSelection(int32 Direction, int32 LocalPlayerIndex);
	void StepSkinSelection(int32 Direction, int32 LocalPlayerIndex);
	void OnPreviousCharacterInput();
	void OnNextCharacterInput();
	void OnPreviousSkinInput();
	void OnNextSkinInput();

	UFUNCTION()
	void OnLobbyConnectionChanged(ELobbyStatus Status);

	void SetLobbySelectionInputEnabled(bool bEnabled);

	bool bLobbySelectionInputEnabled = false;

	UPROPERTY(Transient)
	TObjectPtr<UInputMappingContext> LobbySelectionMappingContext;

	UPROPERTY(Transient)
	TObjectPtr<UInputAction> PreviousCharacterAction;

	UPROPERTY(Transient)
	TObjectPtr<UInputAction> NextCharacterAction;

	UPROPERTY(Transient)
	TObjectPtr<UInputAction> PreviousSkinAction;

	UPROPERTY(Transient)
	TObjectPtr<UInputAction> NextSkinAction;
};
