#include "Framework/LobbyBeaconHostObject.h"
#include "Framework/LobbyBeaconClient.h"
#include "OnlineSubsystem.h"

ALobbyBeaconHostObject::ALobbyBeaconHostObject()
{
	ClientBeaconActorClass = ALobbyBeaconClient::StaticClass();
	BeaconTypeName = ClientBeaconActorClass->GetName();
	UE_LOG(LogOnline, Warning, TEXT("LobbyHost Initialized, BeaconTypeName: %s"), *BeaconTypeName);
}

void ALobbyBeaconHostObject::OnClientConnected(AOnlineBeaconClient* NewClientActor, UNetConnection* ClientConnection)
{
	Super::OnClientConnected(NewClientActor, ClientConnection);

	if (ALobbyBeaconClient* LobbyClient = Cast<ALobbyBeaconClient>(NewClientActor))
	{
		ConnectedPlayersMap.FindOrAdd(LobbyClient);
		UE_LOG(LogOnline, Warning, TEXT("Host registered new Beacon Client instance for splitscreen support!"));
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

void ALobbyBeaconHostObject::BroadcastLobbyState()
{
	TArray<FLobbyPlayerData> FullPlayerList;

	for (const auto& Pair : ConnectedPlayersMap)
	{
		FullPlayerList.Append(Pair.Value);
	}

	for (const auto& Pair : ConnectedPlayersMap)
	{
		if (Pair.Key)
		{
			Pair.Key->ClientUpdateLobbyList(FullPlayerList);
		}
	}

	OnHostLobbyUpdated.Broadcast(FullPlayerList);
}

void ALobbyBeaconHostObject::StartGameForLobby(const FString& MapURL)
{
	for (const auto& Pair : ConnectedPlayersMap)
	{
		if (Pair.Key)
		{
			Pair.Key->ClientGameStarting(MapURL);
		}
	}
}
