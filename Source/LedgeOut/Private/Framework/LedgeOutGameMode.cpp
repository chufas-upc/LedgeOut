#include "Framework/LedgeOutGameMode.h"
#include "Characters/LedgeOutCharacter.h"
#include "Framework/LedgeOutGameInstance.h"
#include "Framework/LobbyPlayerData.h"
#include "GameFramework/PlayerState.h"
#include "UObject/SoftObjectPath.h"

ALedgeOutGameMode::ALedgeOutGameMode()
{
	// stub
}

const FLobbyPlayerData* ALedgeOutGameMode::FindLobbySelectionForController(const AController* Controller) const
{
	const ULedgeOutGameInstance* GameInstance = Cast<ULedgeOutGameInstance>(GetGameInstance());
	if (!IsValid(GameInstance) || GameInstance->FinalLobbySelections.IsEmpty() || !IsValid(Controller))
	{
		return nullptr;
	}

	const APlayerState* PlayerState = Controller->PlayerState;
	if (!IsValid(PlayerState))
	{
		return nullptr;
	}

	const FUniqueNetIdRepl& UniqueId = PlayerState->GetUniqueId();
	if (UniqueId.GetUniqueNetId().IsValid())
	{
		const FString PlayerUniqueId = UniqueId.GetUniqueNetId()->ToString();
		const FLobbyPlayerData* Match = GameInstance->FinalLobbySelections.FindByPredicate(
			[&PlayerUniqueId](const FLobbyPlayerData& PlayerData)
			{
				return PlayerData.PlayerUniqueId == PlayerUniqueId;
			});
		if (Match)
		{
			return Match;
		}
	}

	const FString PlayerName = PlayerState->GetPlayerName();
	const FLobbyPlayerData* NameMatch = nullptr;
	for (const FLobbyPlayerData& PlayerData : GameInstance->FinalLobbySelections)
	{
		if (PlayerData.PlayerName != PlayerName)
		{
			continue;
		}

		if (NameMatch)
		{
			UE_LOG(LogTemp, Warning,
				TEXT("Cannot apply lobby selection for '%s': multiple lobby players have that name."),
				*PlayerName);
			return nullptr;
		}

		NameMatch = &PlayerData;
	}

	return NameMatch;
}

UClass* ALedgeOutGameMode::GetDefaultPawnClassForController_Implementation(AController* InController)
{
	const FLobbyPlayerData* PlayerData = FindLobbySelectionForController(InController);
	if (PlayerData && !PlayerData->SelectedCharacterClassPath.IsEmpty())
	{
		const FSoftClassPath CharacterClassPath(PlayerData->SelectedCharacterClassPath);
		if (UClass* CharacterClass = CharacterClassPath.TryLoadClass<ALedgeOutCharacter>())
		{
			return CharacterClass;
		}

		UE_LOG(LogTemp, Error, TEXT("Failed to load selected character class '%s' for player '%s'."),
			*PlayerData->SelectedCharacterClassPath, *PlayerData->PlayerName);
	}
	else if (PlayerData)
	{
		UE_LOG(LogTemp, Warning, TEXT("No character class configured for lobby selection '%s'; using the default pawn."),
			*PlayerData->PlayerName);
	}
	else if (const ULedgeOutGameInstance* GameInstance = Cast<ULedgeOutGameInstance>(GetGameInstance()))
	{
		if (!GameInstance->FinalLobbySelections.IsEmpty())
		{
			UE_LOG(LogTemp, Warning, TEXT("No lobby selection found for player '%s'; using the default pawn."),
				*GetNameSafe(InController ? InController->PlayerState : nullptr));
		}
	}

	return Super::GetDefaultPawnClassForController_Implementation(InController);
}

APawn* ALedgeOutGameMode::SpawnDefaultPawnFor_Implementation(AController* NewPlayer, AActor* StartSpot)
{
	APawn* Pawn = Super::SpawnDefaultPawnFor_Implementation(NewPlayer, StartSpot);
	ApplyLobbySkinToPawn(Pawn, NewPlayer);
	return Pawn;
}

APawn* ALedgeOutGameMode::SpawnDefaultPawnAtTransform_Implementation(
	AController* NewPlayer, const FTransform& SpawnTransform)
{
	APawn* Pawn = Super::SpawnDefaultPawnAtTransform_Implementation(NewPlayer, SpawnTransform);
	ApplyLobbySkinToPawn(Pawn, NewPlayer);
	return Pawn;
}

void ALedgeOutGameMode::ApplyLobbySkinToPawn(APawn* Pawn, const AController* Controller) const
{
	const FLobbyPlayerData* PlayerData = FindLobbySelectionForController(Controller);
	ALedgeOutCharacter* Character = Cast<ALedgeOutCharacter>(Pawn);
	if (PlayerData && Character && !PlayerData->SelectedSkinMeshPath.IsEmpty())
	{
		Character->SetLobbySkinMeshPath(PlayerData->SelectedSkinMeshPath);
	}
}
