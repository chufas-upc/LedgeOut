// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "LedgeOutGameMode.generated.h"

struct FLobbyPlayerData;
class AActor;
class AController;
class APawn;
class APlayerController;
class ALedgeOutPlayerController;
class ALedgeOutPlayerState;

/**
 *  Simple GameMode for a third person game
 */
UCLASS(abstract)
class ALedgeOutGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	
	/** Constructor */
	ALedgeOutGameMode();

	virtual UClass* GetDefaultPawnClassForController_Implementation(AController* InController) override;
	virtual APawn* SpawnDefaultPawnFor_Implementation(AController* NewPlayer, AActor* StartSpot) override;
	virtual APawn* SpawnDefaultPawnAtTransform_Implementation(AController* NewPlayer, const FTransform& SpawnTransform) override;
	virtual void StartPlay() override;

	UFUNCTION(BlueprintCallable)
	APawn* RespawnCharacter(AController* NewPlayer, AActor* StartSpot);

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Round", meta = (ClampMin = "0", UIMin = "0"))
	int32 RoundDurationSeconds = 300;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Round", meta = (ClampMin = "0.1", UIMin = "0.1"))
	float VictoryMessageDurationSeconds = 5.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Round")
	FString MenuMapPath = TEXT("/Game/Maps/L_Menu?listen");

private:
	const FLobbyPlayerData* FindLobbySelectionForController(const AController* Controller) const;
	void ApplyLobbySkinToPawn(APawn* Pawn, const AController* Controller) const;
	void RegisterPawnForDeath(APawn* Pawn, AController* Controller);
	void RespawnPlayer(AController* Controller);
	void SpectateRemainingPlayers(APlayerController* SpectatorController) const;
	void UpdateRoundTimer();
	void CheckForWinner();
	void EndRound(ALedgeOutPlayerState* Winner);
	void ReturnToMenu();
	void TravelHostToMenu();

	UFUNCTION()
	void HandlePawnDestroyed(AActor* DestroyedActor);

	TMap<TWeakObjectPtr<APawn>, TWeakObjectPtr<AController>> PawnControllers;
	FTimerHandle RoundTimerHandle;
	FTimerHandle ReturnToMenuTimerHandle;
	int32 RemainingRoundSeconds = 0;
	int32 SuddenDeathSeconds = 0;
	bool bIsSuddenDeath = false;
	bool bRoundEnded = false;
};



