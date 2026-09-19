#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "LedgeOutPlayerState.generated.h"

/**
 * 
 */
UCLASS()
class LEDGEOUT_API ALedgeOutPlayerState : public APlayerState
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PlayerState")
	int32 Lives = 3;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PlayerState")
	bool bIsDead = false;
	
	UFUNCTION(BlueprintCallable, Category = "PlayerState")
	void HandleDead();
};
