#pragma once

#include "CoreMinimal.h"
#include "OnlineBeaconClient.h"
#include "Framework/LobbyPlayerData.h"
#include "LobbyBeaconClient.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLobbyUpdatedDelegate, const TArray<FLobbyPlayerData>&, PlayerList);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLobbyGameStartingDelegate, const FString&, ConnectURL);

UCLASS()
class LEDGEOUT_API ALobbyBeaconClient : public AOnlineBeaconClient
{
	GENERATED_BODY()

public:
	ALobbyBeaconClient();

	UPROPERTY(BlueprintAssignable, Category = "Lobby Beacon")
	FOnLobbyUpdatedDelegate OnLobbyUpdated;

	UPROPERTY(BlueprintAssignable, Category = "Lobby Beacon")
	FOnLobbyGameStartingDelegate OnLobbyGameStarting;

	// Update data for a specific local player (e.g. LocalPlayerIndex 0, 1 for splitscreen)
	UFUNCTION(BlueprintCallable, Category = "Lobby Beacon")
	void UpdateLocalPlayerData(int32 LocalPlayerIndex, const FLobbyPlayerData& NewData);

	// Add a new local splitscreen player to this client instance
	UFUNCTION(BlueprintCallable, Category = "Lobby Beacon")
	void AddLocalPlayer(const FLobbyPlayerData& NewPlayerData);

	// Remove a local splitscreen player
	UFUNCTION(BlueprintCallable, Category = "Lobby Beacon")
	void RemoveLocalPlayer(int32 LocalPlayerIndex);

	// Send current local players array to host
	UFUNCTION(BlueprintCallable, Category = "Lobby Beacon")
	void SendLocalPlayersToHost();

	UFUNCTION(Client, Reliable)
	void ClientUpdateLobbyList(const TArray<FLobbyPlayerData>& PlayerList);

	UFUNCTION(Client, Reliable)
	void ClientGameStarting(const FString& ConnectURL);

	virtual void OnConnected() override;
	virtual void OnFailure() override;

protected:
	UFUNCTION(Server, Reliable)
	void ServerUpdateLocalPlayers(const TArray<FLobbyPlayerData>& LocalPlayers);

	UPROPERTY()
	TArray<FLobbyPlayerData> MyLocalPlayers;
};
