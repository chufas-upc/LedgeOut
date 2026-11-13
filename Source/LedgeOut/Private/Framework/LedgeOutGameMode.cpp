#include "Framework/LedgeOutGameMode.h"
#include "Characters/LedgeOutCharacter.h"
#include "Framework/LedgeOutGameInstance.h"
#include "Framework/LedgeOutPlayerState.h"
#include "Framework/LobbyPlayerData.h"
#include "GameFramework/Controller.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "UObject/SoftObjectPath.h"

ALedgeOutGameMode::ALedgeOutGameMode()
{
	PlayerStateClass = ALedgeOutPlayerState::StaticClass();
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
	RegisterPawnForDeath(Pawn, NewPlayer);
	return Pawn;
}

APawn* ALedgeOutGameMode::SpawnDefaultPawnAtTransform_Implementation(
	AController* NewPlayer, const FTransform& SpawnTransform)
{
	APawn* Pawn = Super::SpawnDefaultPawnAtTransform_Implementation(NewPlayer, SpawnTransform);
	ApplyLobbySkinToPawn(Pawn, NewPlayer);
	RegisterPawnForDeath(Pawn, NewPlayer);
	return Pawn;
}

APawn* ALedgeOutGameMode::RespawnCharacter(AController* NewPlayer, AActor* StartSpot)
{
	return SpawnDefaultPawnFor(NewPlayer, StartSpot);
}

void ALedgeOutGameMode::RegisterPawnForDeath(APawn* Pawn, AController* Controller)
{
	if (IsValid(Pawn) && IsValid(Controller))
	{
		PawnControllers.Add(TWeakObjectPtr<APawn>(Pawn), TWeakObjectPtr<AController>(Controller));
		Pawn->OnDestroyed.AddDynamic(this, &ALedgeOutGameMode::HandlePawnDestroyed);
	}
}

void ALedgeOutGameMode::HandlePawnDestroyed(AActor* DestroyedActor)
{
	APawn* DestroyedPawn = Cast<APawn>(DestroyedActor);
	if (!DestroyedPawn)
	{
		return;
	}

	const TWeakObjectPtr<APawn> PawnRef(DestroyedPawn);
	const TWeakObjectPtr<AController> ControllerRef = PawnControllers.FindRef(PawnRef);
	PawnControllers.Remove(PawnRef);

	AController* Controller = ControllerRef.Get();
	if (!IsValid(Controller) || (IsValid(Controller->GetPawn()) && Controller->GetPawn() != DestroyedPawn))
	{
		return;
	}

	APlayerController* PlayerController = Cast<APlayerController>(Controller);
	ALedgeOutPlayerState* PlayerState = Controller->GetPlayerState<ALedgeOutPlayerState>();
	if (!IsValid(PlayerState))
	{
		UE_LOG(LogTemp, Error, TEXT("Cannot handle destroyed pawn '%s': player has no LedgeOutPlayerState."),
			*GetNameSafe(DestroyedPawn));
		if (IsValid(PlayerController))
		{
			SpectateRemainingPlayers(PlayerController);
		}
		return;
	}

	if (!PlayerState->bIsDead)
	{
		PlayerState->HandleDead();
	}

	if (PlayerState->Lives > 0)
	{
		GetWorldTimerManager().SetTimerForNextTick(
			FTimerDelegate::CreateUObject(this, &ALedgeOutGameMode::RespawnPlayer, Controller));
	}
	else if (IsValid(PlayerController))
	{
		SpectateRemainingPlayers(PlayerController);
	}
}

void ALedgeOutGameMode::RespawnPlayer(AController* Controller)
{
	if (!IsValid(Controller))
	{
		return;
	}

	ALedgeOutPlayerState* PlayerState = Controller->GetPlayerState<ALedgeOutPlayerState>();
	if (!IsValid(PlayerState))
	{
		UE_LOG(LogTemp, Error, TEXT("Cannot respawn player: controller has no LedgeOutPlayerState."));
		return;
	}

	if (!IsValid(Controller->GetPawn()))
	{
		RestartPlayer(Controller);
	}

	if (IsValid(Controller->GetPawn()))
	{
		PlayerState->bIsDead = false;
		PlayerState->ForceNetUpdate();
		return;
	}

	UE_LOG(LogTemp, Error, TEXT("Failed to respawn player '%s': no pawn was created."),
		*GetNameSafe(Controller->PlayerState));
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		SpectateRemainingPlayers(PlayerController);
	}
}

void ALedgeOutGameMode::SpectateRemainingPlayers(APlayerController* SpectatorController) const
{
	if (!IsValid(SpectatorController))
	{
		return;
	}

	SpectatorController->StartSpectatingOnly();

	if (UWorld* World = GetWorld())
	{
		for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
		{
			APlayerController* OtherController = It->Get();
			if (!IsValid(OtherController) || OtherController == SpectatorController)
			{
				continue;
			}

			APawn* OtherPawn = OtherController->GetPawn();
			if (IsValid(OtherPawn))
			{
				SpectatorController->SetViewTarget(OtherPawn);
				return;
			}
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("No remaining player pawn is available to spectate."));
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
