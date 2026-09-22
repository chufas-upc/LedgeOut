#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameWidget.generated.h"

class UTextBlock;
/**
 * 
 */
UCLASS()
class LEDGEOUT_API UGameWidget : public UUserWidget
{
	GENERATED_BODY()
	
	UPROPERTY(EditDefaultsOnly, Category = "Widgets|Player", meta = (BindWidget))
	TObjectPtr<UTextBlock> Health;
	
	//UPROPERTY(EditDefaultsOnly, Category = "Widgets|Player", meta = (BindWidget))
	//TObjectPtr<UHorizontalBox> LivesContainer;

protected:
	virtual void NativeConstruct() override;

private:
	UFUNCTION()
	void UpdateDamage(float Damage);
	
public:
	UFUNCTION()
	void BindPawn(APawn* Pawn);
};
