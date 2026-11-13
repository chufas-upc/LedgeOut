#include "Framework/LobbyBeaconClient.h"
#include "Framework/LobbyBeaconHostObject.h"
#include "Framework/LedgeOutGameInstance.h"
#include "Framework/Lobby/LobbyGameMode.h"
#include "Framework/SelectionIndexUtils.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "OnlineSubsystem.h"
#include "GameFramework/PlayerState.h"

ALobbyBeaconClient::ALobbyBeaconClient()
{
	UE_LOG(LogOnline, Warning, TEXT("LobbyClient Initialized"));
}

void ALobbyBeaconClient::OnConnected()
{
	Super::OnConnected();
	UE_LOG(LogOnline, Warning, TEXT("LobbyBeaconClient connected successfully to Beacon Host!"));
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Green, TEXT("LobbyBeaconClient Connected!"));
	}

	// Automatically send initial local player data when connected
	if (MyLocalPlayers.Num() == 0)
	{
		FLobbyPlayerData DefaultP1;
		DefaultP1.PlayerName = TEXT("Player_1");
		if (UWorld* World = GetWorld())
		{
			if (APlayerController* PC = World->GetFirstPlayerController())
			{
				if (APlayerState* PS = PC->PlayerState)
				{
					DefaultP1.PlayerName = PS->GetPlayerName();
					const FUniqueNetIdRepl& UniqueId = PS->GetUniqueId();
					if (UniqueId.GetUniqueNetId().IsValid())
					{
						DefaultP1.PlayerUniqueId = UniqueId.GetUniqueNetId()->ToString();
					}
				}
			}
		}
		DefaultP1.LocalPlayerIndex = 0;
		DefaultP1.SelectedCharacterIndex = 0;
		DefaultP1.bIsReady = false;
		MyLocalPlayers.Add(DefaultP1);
	}

	SendLocalPlayersToHost();
}

void ALobbyBeaconClient::OnFailure()
{
	Super::OnFailure();
	UE_LOG(LogOnline, Error, TEXT("LobbyBeaconClient failed to connect to Beacon Host."));
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, TEXT("LobbyBeaconClient Connection Failed!"));
	}
}

void ALobbyBeaconClient::AddLocalPlayer(const FLobbyPlayerData& NewPlayerData)
{
	MyLocalPlayers.Add(NewPlayerData);
	SendLocalPlayersToHost();
}

void ALobbyBeaconClient::RemoveLocalPlayer(int32 LocalPlayerIndex)
{
	MyLocalPlayers.RemoveAll([LocalPlayerIndex](const FLobbyPlayerData& Data)
	{
		return Data.LocalPlayerIndex == LocalPlayerIndex;
	});
	SendLocalPlayersToHost();
}

void ALobbyBeaconClient::UpdateLocalPlayerData(int32 LocalPlayerIndex, const FLobbyPlayerData& NewData)
{
	bool bFound = false;
	for (FLobbyPlayerData& Data : MyLocalPlayers)
	{
		if (Data.LocalPlayerIndex == LocalPlayerIndex)
		{
			Data = NewData;
			bFound = true;
			break;
		}
	}

	if (!bFound)
	{
		MyLocalPlayers.Add(NewData);
	}

	SendLocalPlayersToHost();
}

void ALobbyBeaconClient::SetAllLocalPlayersReady(bool bIsReady)
{
	for (FLobbyPlayerData& Data : MyLocalPlayers)
	{
		Data.bIsReady = bIsReady;
	}
	SendLocalPlayersToHost();
}

void ALobbyBeaconClient::ToggleAllLocalPlayersReady()
{
	bool bAnyNotReady = false;
	for (const FLobbyPlayerData& Data : MyLocalPlayers)
	{
		if (!Data.bIsReady)
		{
			bAnyNotReady = true;
			break;
		}
	}

	SetAllLocalPlayersReady(bAnyNotReady);
}

void ALobbyBeaconClient::SetLocalPlayerCharacter(int32 LocalPlayerIndex, int32 CharacterIndex)
{
	ServerSetLocalPlayerCharacter(LocalPlayerIndex, CharacterIndex);
}

void ALobbyBeaconClient::StepLocalPlayerCharacter(int32 LocalPlayerIndex, int32 Direction)
{
	ServerStepLocalPlayerCharacter(LocalPlayerIndex, Direction);
}

void ALobbyBeaconClient::SetLocalPlayerSkin(int32 LocalPlayerIndex, int32 SkinIndex)
{
	ServerSetLocalPlayerSkin(LocalPlayerIndex, SkinIndex);
}

void ALobbyBeaconClient::StepLocalPlayerSkin(int32 LocalPlayerIndex, int32 Direction)
{
	ServerStepLocalPlayerSkin(LocalPlayerIndex, Direction);
}

void ALobbyBeaconClient::SendLocalPlayersToHost()
{
	ServerUpdateLocalPlayers(MyLocalPlayers);
}

void ALobbyBeaconClient::ServerUpdateLocalPlayers_Implementation(const TArray<FLobbyPlayerData>& LocalPlayers)
{
	MyLocalPlayers = LocalPlayers;
	PublishLocalPlayersToHost();
}

void ALobbyBeaconClient::ServerSetLocalPlayerCharacter_Implementation(int32 LocalPlayerIndex, int32 CharacterIndex)
{
	const ALobbyGameMode* LobbyGameMode = Cast<ALobbyGameMode>(UGameplayStatics::GetGameMode(this));
	if (LocalPlayerIndex < 0 || !IsValid(LobbyGameMode) || !LobbyGameMode->AvailableCharacters.IsValidIndex(CharacterIndex))
	{
		UE_LOG(LogOnline, Warning, TEXT("Rejected invalid lobby character selection (player %d, character %d)."),
			LocalPlayerIndex, CharacterIndex);
		return;
	}

	FLobbyPlayerData* PlayerData = MyLocalPlayers.FindByPredicate([LocalPlayerIndex](const FLobbyPlayerData& Data)
	{
		return Data.LocalPlayerIndex == LocalPlayerIndex;
	});
	if (!PlayerData)
	{
		UE_LOG(LogOnline, Warning, TEXT("Cannot change character: local player %d is not registered."), LocalPlayerIndex);
		return;
	}

	const bool bCharacterChanged = PlayerData->SelectedCharacterIndex != CharacterIndex;
	PlayerData->SelectedCharacterIndex = CharacterIndex;
	if (bCharacterChanged)
	{
		PlayerData->SelectedSkinIndex = INDEX_NONE;
	}
	PublishLocalPlayersToHost();
}

void ALobbyBeaconClient::ServerStepLocalPlayerCharacter_Implementation(int32 LocalPlayerIndex, int32 Direction)
{
	const ALobbyGameMode* LobbyGameMode = Cast<ALobbyGameMode>(UGameplayStatics::GetGameMode(this));
	if (LocalPlayerIndex < 0 || (Direction != -1 && Direction != 1) || !IsValid(LobbyGameMode))
	{
		UE_LOG(LogOnline, Warning, TEXT("Rejected invalid lobby character step (player %d, direction %d)."),
			LocalPlayerIndex, Direction);
		return;
	}

	const int32 CharacterCount = LobbyGameMode->GetCharacterCount();
	FLobbyPlayerData* PlayerData = MyLocalPlayers.FindByPredicate([LocalPlayerIndex](const FLobbyPlayerData& Data)
	{
		return Data.LocalPlayerIndex == LocalPlayerIndex;
	});
	if (CharacterCount <= 0 || !PlayerData)
	{
		UE_LOG(LogOnline, Warning, TEXT("Cannot step character for local player %d: no characters or player data."), LocalPlayerIndex);
		return;
	}

	const int32 NewCharacterIndex = WrapSelectionIndex(
		PlayerData->SelectedCharacterIndex, Direction, CharacterCount);
	if (NewCharacterIndex != PlayerData->SelectedCharacterIndex)
	{
		PlayerData->SelectedSkinIndex = INDEX_NONE;
	}
	PlayerData->SelectedCharacterIndex = NewCharacterIndex;
	PublishLocalPlayersToHost();
}

void ALobbyBeaconClient::ServerSetLocalPlayerSkin_Implementation(int32 LocalPlayerIndex, int32 SkinIndex)
{
	const ALobbyGameMode* LobbyGameMode = Cast<ALobbyGameMode>(UGameplayStatics::GetGameMode(this));
	FLobbyPlayerData* PlayerData = MyLocalPlayers.FindByPredicate([LocalPlayerIndex](const FLobbyPlayerData& Data)
	{
		return Data.LocalPlayerIndex == LocalPlayerIndex;
	});
	if (LocalPlayerIndex < 0 || SkinIndex < INDEX_NONE || !IsValid(LobbyGameMode) || !PlayerData
		|| SkinIndex >= LobbyGameMode->GetSkinCount(PlayerData->SelectedCharacterIndex))
	{
		UE_LOG(LogOnline, Warning, TEXT("Rejected invalid lobby skin selection (player %d, skin %d)."),
			LocalPlayerIndex, SkinIndex);
		return;
	}

	PlayerData->SelectedSkinIndex = SkinIndex;
	PublishLocalPlayersToHost();
}

void ALobbyBeaconClient::ServerStepLocalPlayerSkin_Implementation(int32 LocalPlayerIndex, int32 Direction)
{
	const ALobbyGameMode* LobbyGameMode = Cast<ALobbyGameMode>(UGameplayStatics::GetGameMode(this));
	FLobbyPlayerData* PlayerData = MyLocalPlayers.FindByPredicate([LocalPlayerIndex](const FLobbyPlayerData& Data)
	{
		return Data.LocalPlayerIndex == LocalPlayerIndex;
	});
	if (LocalPlayerIndex < 0 || (Direction != -1 && Direction != 1) || !IsValid(LobbyGameMode) || !PlayerData)
	{
		UE_LOG(LogOnline, Warning, TEXT("Rejected invalid lobby skin step (player %d, direction %d)."),
			LocalPlayerIndex, Direction);
		return;
	}

	const int32 SkinCount = LobbyGameMode->GetSkinCount(PlayerData->SelectedCharacterIndex);
	if (SkinCount <= 0)
	{
		UE_LOG(LogOnline, Warning, TEXT("Cannot step skin for character %d: no skins are configured."),
			PlayerData->SelectedCharacterIndex);
		return;
	}

	PlayerData->SelectedSkinIndex = WrapSelectionIndex(
		PlayerData->SelectedSkinIndex + 1, Direction, SkinCount + 1) - 1;
	PublishLocalPlayersToHost();
}

void ALobbyBeaconClient::ClientUpdateLocalPlayers_Implementation(const TArray<FLobbyPlayerData>& LocalPlayers)
{
	MyLocalPlayers = LocalPlayers;
}

void ALobbyBeaconClient::PublishLocalPlayersToHost()
{
	if (ALobbyBeaconHostObject* HostObj = Cast<ALobbyBeaconHostObject>(BeaconOwner))
	{
		HostObj->UpdateClientLocalPlayers(this, MyLocalPlayers);
	}
	else
	{
		UE_LOG(LogOnline, Warning, TEXT("Cannot update lobby player data: beacon host object is unavailable."));
	}
}

void ALobbyBeaconClient::ClientUpdateLobbyList_Implementation(const TArray<FLobbyPlayerData>& PlayerList)
{
	OnLobbyUpdated.Broadcast(PlayerList);
}

void ALobbyBeaconClient::ClientGameStarting_Implementation(
	const FString& ConnectURL, const TArray<FLobbyPlayerData>& FinalLobbySelections)
{
	if (ULedgeOutGameInstance* GameInstance = Cast<ULedgeOutGameInstance>(GetGameInstance()))
	{
		GameInstance->FinalLobbySelections = FinalLobbySelections;
	}
	else
	{
		UE_LOG(LogOnline, Error, TEXT("Cannot save lobby selections before travel: LedgeOutGameInstance is not configured."));
	}

	OnLobbyGameStarting.Broadcast(ConnectURL);

	if (APlayerController* PC = GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr)
	{
		PC->ClientTravel(ConnectURL, ETravelType::TRAVEL_Absolute);
	}
	
	DestroyBeacon();
}
