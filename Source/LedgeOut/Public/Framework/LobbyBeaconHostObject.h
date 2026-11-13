#pragma once

#include "CoreMinimal.h"
#include "OnlineBeaconHostObject.h"
#include "Framework/LobbyPlayerData.h"
#include "LobbyBeaconHostObject.generated.h"

class ALobbyBeaconClient;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHostLobbyUpdatedDelegate, const TArray<FLobbyPlayerData>&, PlayerList);

UCLASS()
class LEDGEOUT_API ALobbyBeaconHostObject : public AOnlineBeaconHostObject
{
	GENERATED_BODY()

public:
	ALobbyBeaconHostObject();

	UPROPERTY(BlueprintAssignable, Category = "Lobby Beacon Host")
	FOnHostLobbyUpdatedDelegate OnHostLobbyUpdated;

	virtual void OnClientConnected(AOnlineBeaconClient* NewClientActor, UNetConnection* ClientConnection) override;
	virtual void NotifyClientDisconnected(AOnlineBeaconClient* LeavingClientActor) override;

	void UpdateClientLocalPlayers(ALobbyBeaconClient* Client, const TArray<FLobbyPlayerData>& LocalPlayers);

	// --- Host Local Player Management (e.g. splitscreen / host player 0) ---
	UFUNCTION(BlueprintCallable, Category = "Lobby Beacon Host")
	void SetHostLocalPlayers(const TArray<FLobbyPlayerData>& NewHostPlayers);

	UFUNCTION(BlueprintCallable, Category = "Lobby Beacon Host")
	void AddHostLocalPlayer(const FLobbyPlayerData& NewPlayerData);

	UFUNCTION(BlueprintCallable, Category = "Lobby Beacon Host")
	void RemoveHostLocalPlayer(int32 LocalPlayerIndex);

	UFUNCTION(BlueprintCallable, Category = "Lobby Beacon Host")
	void UpdateHostLocalPlayerData(int32 LocalPlayerIndex, const FLobbyPlayerData& NewData);

	UFUNCTION(BlueprintCallable, Category = "Lobby Beacon Host")
	TArray<FLobbyPlayerData> GetHostLocalPlayers() const { return HostLocalPlayers; }

	UFUNCTION(BlueprintCallable, Category = "Lobby Beacon Host")
	void BroadcastLobbyState();

	UFUNCTION(BlueprintCallable, Category = "Lobby Beacon Host")
	void StartGameForLobby(const FString& MapAddress, const FString& URL);

private:
	UPROPERTY()
	TArray<FLobbyPlayerData> HostLocalPlayers;

	TMap<ALobbyBeaconClient*, TArray<FLobbyPlayerData>> ConnectedPlayersMap;
};
