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

	UFUNCTION(BlueprintCallable, Category = "Lobby Beacon Host")
	void BroadcastLobbyState();

	UFUNCTION(BlueprintCallable, Category = "Lobby Beacon Host")
	void StartGameForLobby(const FString& MapAddress, const FString& URL);

private:
	TMap<ALobbyBeaconClient*, TArray<FLobbyPlayerData>> ConnectedPlayersMap;
};
