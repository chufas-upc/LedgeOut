// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "LedgeOutGameMode.generated.h"

struct FLobbyPlayerData;

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

private:
	const FLobbyPlayerData* FindLobbySelectionForController(const AController* Controller) const;
	void ApplyLobbySkinToPawn(APawn* Pawn, const AController* Controller) const;
};



