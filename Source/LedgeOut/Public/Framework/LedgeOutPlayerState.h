#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "LedgeOutPlayerState.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLivesChanged, int32, Lives);
/**
 * 
 */
UCLASS()
class LEDGEOUT_API ALedgeOutPlayerState : public APlayerState
{
	GENERATED_BODY()
	
public:
	UPROPERTY(BlueprintAssignable)
	FOnLivesChanged OnLivesChanged;
	
	UPROPERTY(ReplicatedUsing= OnRep_UpdateLives, EditDefaultsOnly, BlueprintReadOnly, Category = "PlayerState")
	int32 Lives = 3;
	
	UPROPERTY(Replicated, EditDefaultsOnly, BlueprintReadOnly, Category = "PlayerState")
	bool bIsDead = false;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	UFUNCTION(BlueprintCallable, Category = "PlayerState")
	void HandleDead();
	
	UFUNCTION()
	void OnRep_UpdateLives();
};
