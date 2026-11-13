#include "Framework/LobbyBeaconHostObject.h"
#include "Framework/LobbyBeaconClient.h"
#include "Framework/LedgeOutGameInstance.h"
#include "Framework/Lobby/LobbyGameMode.h"
#include "Kismet/GameplayStatics.h"
#include "OnlineSubsystem.h"

ALobbyBeaconHostObject::ALobbyBeaconHostObject()
{
	ClientBeaconActorClass = ALobbyBeaconClient::StaticClass();
	BeaconTypeName = ClientBeaconActorClass->GetName();
	UE_LOG(LogOnline, Log, TEXT("LobbyHost Initialized, BeaconTypeName: %s"), *BeaconTypeName);
}

void ALobbyBeaconHostObject::OnClientConnected(AOnlineBeaconClient* NewClientActor, UNetConnection* ClientConnection)
{
	Super::OnClientConnected(NewClientActor, ClientConnection);

	if (ALobbyBeaconClient* LobbyClient = Cast<ALobbyBeaconClient>(NewClientActor))
	{
		ConnectedPlayersMap.FindOrAdd(LobbyClient);
		UE_LOG(LogOnline, Log, TEXT("Host registered new Beacon Client instance for splitscreen support!"));
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Green, TEXT("Host: Client Beacon Connected!"));
		}
	}
}

void ALobbyBeaconHostObject::NotifyClientDisconnected(AOnlineBeaconClient* LeavingClientActor)
{
	Super::NotifyClientDisconnected(LeavingClientActor);

	if (ALobbyBeaconClient* LobbyClient = Cast<ALobbyBeaconClient>(LeavingClientActor))
	{
		ConnectedPlayersMap.Remove(LobbyClient);
		BroadcastLobbyState();

		UE_LOG(LogOnline, Log, TEXT("Host unregistered disconnected Beacon Client"));
	}
}

void ALobbyBeaconHostObject::UpdateClientLocalPlayers(ALobbyBeaconClient* Client, const TArray<FLobbyPlayerData>& LocalPlayers)
{
	if (ConnectedPlayersMap.Contains(Client))
	{
		ConnectedPlayersMap[Client] = LocalPlayers;
		BroadcastLobbyState();
	}
}

void ALobbyBeaconHostObject::SetHostLocalPlayers(const TArray<FLobbyPlayerData>& NewHostPlayers)
{
	HostLocalPlayers = NewHostPlayers;
	BroadcastLobbyState();
}

void ALobbyBeaconHostObject::AddHostLocalPlayer(const FLobbyPlayerData& NewPlayerData)
{
	HostLocalPlayers.Add(NewPlayerData);
	BroadcastLobbyState();
}

void ALobbyBeaconHostObject::RemoveHostLocalPlayer(int32 LocalPlayerIndex)
{
	HostLocalPlayers.RemoveAll([LocalPlayerIndex](const FLobbyPlayerData& Data)
	{
		return Data.LocalPlayerIndex == LocalPlayerIndex;
	});
	BroadcastLobbyState();
}

void ALobbyBeaconHostObject::UpdateHostLocalPlayerData(int32 LocalPlayerIndex, const FLobbyPlayerData& NewData)
{
	bool bFound = false;
	for (FLobbyPlayerData& Data : HostLocalPlayers)
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
		HostLocalPlayers.Add(NewData);
	}

	BroadcastLobbyState();
}

void ALobbyBeaconHostObject::BroadcastLobbyState()
{
	TArray<FLobbyPlayerData> FullPlayerList;
	const ALobbyGameMode* LobbyGameMode = Cast<ALobbyGameMode>(UGameplayStatics::GetGameMode(this));

	const auto AppendPlayersWithCharacterPaths = [LobbyGameMode, &FullPlayerList](const TArray<FLobbyPlayerData>& Players)
	{
		for (FLobbyPlayerData Player : Players)
		{
			Player.SelectedCharacterMeshPath.Reset();
			Player.SelectedSkinMeshPath.Reset();
			Player.SelectedCharacterClassPath.Reset();
			if (IsValid(LobbyGameMode))
			{
				Player.SelectedCharacterMeshPath = LobbyGameMode->GetCharacterMeshSoft(
					Player.SelectedCharacterIndex).ToSoftObjectPath().ToString();
				Player.SelectedCharacterClassPath = LobbyGameMode->AvailableCharacters.IsValidIndex(
					Player.SelectedCharacterIndex)
					? LobbyGameMode->AvailableCharacters[Player.SelectedCharacterIndex].CharacterClass
						.ToSoftObjectPath().ToString()
					: FString();
				Player.SelectedSkinMeshPath = LobbyGameMode->GetSkinMeshSoft(
					Player.SelectedCharacterIndex, Player.SelectedSkinIndex).ToSoftObjectPath().ToString();
			}
			FullPlayerList.Add(MoveTemp(Player));
		}
	};

	// 1. Include the host's local players
	AppendPlayersWithCharacterPaths(HostLocalPlayers);

	// 2. Include all connected client players
	for (const auto& Pair : ConnectedPlayersMap)
	{
		AppendPlayersWithCharacterPaths(Pair.Value);
	}

	// 3. Replicate update to all connected clients
	for (const auto& Pair : ConnectedPlayersMap)
	{
		if (Pair.Key)
		{
			Pair.Key->ClientUpdateLobbyList(FullPlayerList);

			TArray<FLobbyPlayerData> ClientPlayers = Pair.Value;
			for (FLobbyPlayerData& Player : ClientPlayers)
			{
				Player.SelectedCharacterMeshPath.Reset();
				Player.SelectedSkinMeshPath.Reset();
				Player.SelectedCharacterClassPath.Reset();
				if (IsValid(LobbyGameMode))
				{
					Player.SelectedCharacterMeshPath = LobbyGameMode->GetCharacterMeshSoft(
						Player.SelectedCharacterIndex).ToSoftObjectPath().ToString();
					Player.SelectedCharacterClassPath = LobbyGameMode->AvailableCharacters.IsValidIndex(
						Player.SelectedCharacterIndex)
						? LobbyGameMode->AvailableCharacters[Player.SelectedCharacterIndex].CharacterClass
							.ToSoftObjectPath().ToString()
						: FString();
					Player.SelectedSkinMeshPath = LobbyGameMode->GetSkinMeshSoft(
						Player.SelectedCharacterIndex, Player.SelectedSkinIndex).ToSoftObjectPath().ToString();
				}
			}
			Pair.Key->ClientUpdateLocalPlayers(ClientPlayers);
		}
	}

	// 4. Notify local host listeners/UI
	OnHostLobbyUpdated.Broadcast(FullPlayerList);
}

void ALobbyBeaconHostObject::StartGameForLobby(
	const FString& MapAddress, const FString& URL, const TArray<FLobbyPlayerData>& FinalLobbySelections)
{
	UE_LOG(LogBeacon, Log, TEXT("Connected Players num: %d"), ConnectedPlayersMap.Num());
	UE_LOG(LogBeacon, Log, TEXT("Connecting to : %s"), *URL);

	if (ULedgeOutGameInstance* GameInstance = Cast<ULedgeOutGameInstance>(GetGameInstance()))
	{
		GameInstance->FinalLobbySelections = FinalLobbySelections;
	}
	else
	{
		UE_LOG(LogBeacon, Error, TEXT("Cannot save lobby selections before server travel: LedgeOutGameInstance is not configured."));
	}

	for (const auto& Pair : ConnectedPlayersMap)
	{
		if (Pair.Key)
		{
			Pair.Key->ClientGameStarting(URL, FinalLobbySelections);
		}
	}

	if (UWorld* World = GetWorld())
	{
		World->ServerTravel(MapAddress);
	}
}
