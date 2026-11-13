#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "LedgeOutGameState.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnRoundTimerUpdated, int32, TimeSeconds, bool, bIsSuddenDeath);

UCLASS()
class LEDGEOUT_API ALedgeOutGameState : public AGameStateBase
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable, Category = "Round")
	FOnRoundTimerUpdated OnRoundTimerUpdated;

	UPROPERTY(ReplicatedUsing = OnRep_RoundTimerUpdated, BlueprintReadOnly, Category = "Round")
	int32 RoundTimeSeconds = 0;

	UPROPERTY(ReplicatedUsing = OnRep_RoundTimerUpdated, BlueprintReadOnly, Category = "Round")
	bool bIsSuddenDeath = false;

	void SetRoundTimer(int32 TimeSeconds, bool bSuddenDeath);

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:
	UFUNCTION()
	void OnRep_RoundTimerUpdated();
};
