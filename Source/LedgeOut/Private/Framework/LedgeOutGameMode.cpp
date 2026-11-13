#include "Framework/LedgeOutGameMode.h"
#include "Characters/LedgeOutCharacter.h"
#include "Framework/LedgeOutGameInstance.h"
#include "Framework/LedgeOutGameState.h"
#include "Framework/LedgeOutPlayerController.h"
#include "Framework/LedgeOutPlayerState.h"
#include "Framework/LobbyPlayerData.h"
#include "Framework/OnlineSessionsSubsystem.h"
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
	GameStateClass = ALedgeOutGameState::StaticClass();
}

void ALedgeOutGameMode::StartPlay()
{
	Super::StartPlay();

	RemainingRoundSeconds = FMath::Max(0, RoundDurationSeconds);
	bIsSuddenDeath = RemainingRoundSeconds == 0;
	SuddenDeathSeconds = 0;

	if (ALedgeOutGameState* RoundGameState = GetGameState<ALedgeOutGameState>())
	{
		RoundGameState->SetRoundTimer(RemainingRoundSeconds, bIsSuddenDeath);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Cannot start round timer: LedgeOutGameState is not configured."));
	}

	GetWorldTimerManager().SetTimer(
		RoundTimerHandle, this, &ALedgeOutGameMode::UpdateRoundTimer, 1.0f, true);
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

	CheckForWinner();
	if (bRoundEnded)
	{
		return;
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
		CheckForWinner();
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

void ALedgeOutGameMode::UpdateRoundTimer()
{
	if (bRoundEnded)
	{
		GetWorldTimerManager().ClearTimer(RoundTimerHandle);
		return;
	}

	if (!bIsSuddenDeath)
	{
		RemainingRoundSeconds = FMath::Max(0, RemainingRoundSeconds - 1);
		if (RemainingRoundSeconds == 0)
		{
			bIsSuddenDeath = true;
			SuddenDeathSeconds = 0;
		}
	}
	else
	{
		++SuddenDeathSeconds;
	}

	if (ALedgeOutGameState* RoundGameState = GetGameState<ALedgeOutGameState>())
	{
		RoundGameState->SetRoundTimer(
			bIsSuddenDeath ? SuddenDeathSeconds : RemainingRoundSeconds,
			bIsSuddenDeath);
	}
}

void ALedgeOutGameMode::CheckForWinner()
{
	if (bRoundEnded)
	{
		return;
	}

	ALedgeOutGameState* RoundGameState = GetGameState<ALedgeOutGameState>();
	if (!IsValid(RoundGameState))
	{
		UE_LOG(LogTemp, Error, TEXT("Cannot check round winner: LedgeOutGameState is invalid."));
		return;
	}

	int32 PlayerCount = 0;
	ALedgeOutPlayerState* LastPlayerWithLives = nullptr;
	for (APlayerState* BasePlayerState : RoundGameState->PlayerArray)
	{
		ALedgeOutPlayerState* PlayerState = Cast<ALedgeOutPlayerState>(BasePlayerState);
		if (!IsValid(PlayerState))
		{
			continue;
		}

		++PlayerCount;
		if (PlayerState->Lives > 0)
		{
			if (LastPlayerWithLives)
			{
				return;
			}
			LastPlayerWithLives = PlayerState;
		}
	}

	if (PlayerCount < 2 || !IsValid(LastPlayerWithLives) || LastPlayerWithLives->bIsDead)
	{
		return;
	}

	AController* WinnerController = nullptr;
	if (UWorld* World = GetWorld())
	{
		for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
		{
			APlayerController* PlayerController = It->Get();
			if (IsValid(PlayerController)
				&& PlayerController->GetPlayerState<ALedgeOutPlayerState>() == LastPlayerWithLives)
			{
				WinnerController = PlayerController;
				break;
			}
		}
	}

	if (!IsValid(WinnerController) || !IsValid(WinnerController->GetPawn()))
	{
		return;
	}

	EndRound(LastPlayerWithLives);
}

void ALedgeOutGameMode::EndRound(ALedgeOutPlayerState* Winner)
{
	if (bRoundEnded || !IsValid(Winner))
	{
		return;
	}

	bRoundEnded = true;
	GetWorldTimerManager().ClearTimer(RoundTimerHandle);

	const FString WinnerName = Winner->GetPlayerName();
	if (UWorld* World = GetWorld())
	{
		for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
		{
			if (ALedgeOutPlayerController* PlayerController = Cast<ALedgeOutPlayerController>(It->Get()))
			{
				PlayerController->SetIgnoreMoveInput(true);
				PlayerController->SetIgnoreLookInput(true);
				PlayerController->ClientShowVictoryMessage(WinnerName);
			}
		}
	}

	GetWorldTimerManager().SetTimer(
		ReturnToMenuTimerHandle, this, &ALedgeOutGameMode::ReturnToMenu,
		FMath::Max(0.1f, VictoryMessageDurationSeconds), false);
}

void ALedgeOutGameMode::ReturnToMenu()
{
	if (UWorld* World = GetWorld())
	{
		if (MenuMapPath.IsEmpty())
		{
			UE_LOG(LogTemp, Error, TEXT("Cannot return to menu: MenuMapPath is empty."));
			return;
		}

		if (ULedgeOutGameInstance* GameInstance = Cast<ULedgeOutGameInstance>(GetGameInstance()))
		{
			GameInstance->ClearLobbySelections();
			if (UOnlineSessionsSubsystem* Sessions = GameInstance->GetSubsystem<UOnlineSessionsSubsystem>())
			{
				Sessions->ResetAfterRound();
			}
		}

		for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
		{
			APlayerController* PlayerController = It->Get();
			if (IsValid(PlayerController) && !PlayerController->IsLocalController())
			{
				if (ALedgeOutPlayerController* LedgeOutController = Cast<ALedgeOutPlayerController>(PlayerController))
				{
					LedgeOutController->ClientDisconnectAfterRound();
				}
				else
				{
					PlayerController->ClientReturnToMainMenuWithTextReason(FText::GetEmpty());
				}
			}
		}

		GetWorldTimerManager().SetTimer(
			ReturnToMenuTimerHandle, this, &ALedgeOutGameMode::TravelHostToMenu, 1.0f, false);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Cannot return to menu: world is invalid."));
	}
}

void ALedgeOutGameMode::TravelHostToMenu()
{
	if (UWorld* World = GetWorld())
	{
		if (!World->ServerTravel(MenuMapPath, true))
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to travel back to menu map '%s'."), *MenuMapPath);
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Cannot travel to menu: world is invalid."));
	}
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
