#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameWidget.generated.h"

class UTextBlock;
class UHorizontalBox;
class ALedgeOutPlayerState;
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

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

private:
	TWeakObjectPtr<ALedgeOutPlayerState> BoundPlayerState;

	UFUNCTION()
	void UpdateDamage(float Damage);
	
	UFUNCTION()
	void UpdateLives(int32 Lives);
	
public:
	UFUNCTION()
	void BindPawn(APawn* Pawn);

	void BindPlayerState(ALedgeOutPlayerState* PlayerState);
};
