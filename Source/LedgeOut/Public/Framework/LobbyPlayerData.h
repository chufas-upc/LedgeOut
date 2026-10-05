#pragma once

#include "CoreMinimal.h"
#include "LobbyPlayerData.generated.h"

USTRUCT(BlueprintType)
struct FLobbyPlayerData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Lobby")
	FString PlayerName;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Lobby")
	int32 LocalPlayerIndex = 0;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Lobby")
	int32 SelectedCharacterIndex = 0;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Lobby")
	bool bIsReady = false;
};
