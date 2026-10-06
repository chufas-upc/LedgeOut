#include "Framework/LobbyBeaconClient.h"
#include "Framework/LobbyBeaconHostObject.h"
#include "GameFramework/PlayerController.h"
#include "OnlineSubsystem.h"

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

void ALobbyBeaconClient::SendLocalPlayersToHost()
{
	ServerUpdateLocalPlayers(MyLocalPlayers);
}

void ALobbyBeaconClient::ServerUpdateLocalPlayers_Implementation(const TArray<FLobbyPlayerData>& LocalPlayers)
{
	MyLocalPlayers = LocalPlayers;
	if (ALobbyBeaconHostObject* HostObj = Cast<ALobbyBeaconHostObject>(BeaconOwner))
	{
		HostObj->UpdateClientLocalPlayers(this, LocalPlayers);
	}
}

void ALobbyBeaconClient::ClientUpdateLobbyList_Implementation(const TArray<FLobbyPlayerData>& PlayerList)
{
	OnLobbyUpdated.Broadcast(PlayerList);
}

void ALobbyBeaconClient::ClientGameStarting_Implementation(const FString& ConnectURL)
{
	OnLobbyGameStarting.Broadcast(ConnectURL);

	if (APlayerController* PC = GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr)
	{
		PC->ClientTravel(ConnectURL, ETravelType::TRAVEL_Absolute);
	}
	
	DestroyBeacon();
}
