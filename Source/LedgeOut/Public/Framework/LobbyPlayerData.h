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

	/** INDEX_NONE selects the character's default mesh; non-negative values select a configured skin. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Lobby")
	int32 SelectedSkinIndex = INDEX_NONE;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Lobby")
	FString SelectedCharacterMeshPath;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Lobby")
	FString SelectedSkinMeshPath;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Lobby")
	bool bIsReady = false;
};

UENUM(BlueprintType)
enum class ELobbyStatus: uint8
{
	Disconnected,
	Host,
	Client
};
