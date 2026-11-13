#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameWidget.generated.h"

class UTextBlock;
class UHorizontalBox;
class ALedgeOutPlayerState;
class ALedgeOutGameState;
/**
 * 
 */
UCLASS()
class LEDGEOUT_API UGameWidget : public UUserWidget
{
	GENERATED_BODY()
	
	UPROPERTY(EditDefaultsOnly, Category = "Widgets|Player", meta = (BindWidget))
	TObjectPtr<UTextBlock> Health;
	
	UPROPERTY(EditDefaultsOnly, Category = "Widgets|Player", meta = (BindWidget))
	TObjectPtr<UHorizontalBox> LivesContainer;

	UPROPERTY(EditDefaultsOnly, Category = "Widgets|Round", meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> RoundTimer_TextBlock;

	UPROPERTY(EditDefaultsOnly, Category = "Widgets|Round", meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> VictoryMessage_TextBlock;

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

private:
	TWeakObjectPtr<ALedgeOutPlayerState> BoundPlayerState;
	TWeakObjectPtr<ALedgeOutGameState> BoundGameState;

	UFUNCTION()
	void UpdateDamage(float Damage);
	
	UFUNCTION()
	void UpdateLives(int32 Lives);

	UFUNCTION()
	void UpdateRoundTimer(int32 TimeSeconds, bool bIsSuddenDeath);

	void TryBindGameState();
	
public:
	UFUNCTION()
	void BindPawn(APawn* Pawn);

	void BindPlayerState(ALedgeOutPlayerState* PlayerState);
	void ShowVictoryMessage(const FString& WinnerName);

	UFUNCTION(BlueprintImplementableEvent, Category = "Round")
	void OnRoundTimerChanged(int32 TimeSeconds, bool bIsSuddenDeath);

	UFUNCTION(BlueprintImplementableEvent, Category = "Round")
	void OnVictoryMessageShown(const FString& WinnerName);
};
