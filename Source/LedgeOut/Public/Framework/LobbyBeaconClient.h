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

	UFUNCTION(BlueprintCallable, Category = "Lobby Beacon")
	TArray<FLobbyPlayerData> GetLocalPlayers() const { return MyLocalPlayers; }

	// Set ready state for all local players on this machine and notify the host
	UFUNCTION(BlueprintCallable, Category = "Lobby Beacon")
	void SetAllLocalPlayersReady(bool bIsReady);

	// Toggle ready state for all local players on this machine and notify the host
	UFUNCTION(BlueprintCallable, Category = "Lobby Beacon")
	void ToggleAllLocalPlayersReady();

	/**
	 * Change the selected character for a specific local player on this machine.
	 * @param LocalPlayerIndex  Which local player (0 = P1, 1 = P2 splitscreen, …)
	 * @param CharacterIndex    Index into ALobbyGameMode::AvailableCharacters
	 */
	UFUNCTION(BlueprintCallable, Category = "Lobby Beacon")
	void SetLocalPlayerCharacter(int32 LocalPlayerIndex, int32 CharacterIndex);

	UFUNCTION(BlueprintCallable, Category = "Lobby Beacon")
	void StepLocalPlayerCharacter(int32 LocalPlayerIndex, int32 Direction);

	UFUNCTION(BlueprintCallable, Category = "Lobby Beacon")
	void SetLocalPlayerSkin(int32 LocalPlayerIndex, int32 SkinIndex);

	UFUNCTION(BlueprintCallable, Category = "Lobby Beacon")
	void StepLocalPlayerSkin(int32 LocalPlayerIndex, int32 Direction);

	// Send current local players array to host
	UFUNCTION(BlueprintCallable, Category = "Lobby Beacon")
	void SendLocalPlayersToHost();

	UFUNCTION(Client, Reliable)
	void ClientUpdateLobbyList(const TArray<FLobbyPlayerData>& PlayerList);

	UFUNCTION(Client, Reliable)
	void ClientUpdateLocalPlayers(const TArray<FLobbyPlayerData>& LocalPlayers);

	UFUNCTION(Client, Reliable)
	void ClientGameStarting(const FString& ConnectURL);

	virtual void OnConnected() override;
	virtual void OnFailure() override;

protected:
	UFUNCTION(Server, Reliable)
	void ServerUpdateLocalPlayers(const TArray<FLobbyPlayerData>& LocalPlayers);

	UFUNCTION(Server, Reliable)
	void ServerSetLocalPlayerCharacter(int32 LocalPlayerIndex, int32 CharacterIndex);

	UFUNCTION(Server, Reliable)
	void ServerStepLocalPlayerCharacter(int32 LocalPlayerIndex, int32 Direction);

	UFUNCTION(Server, Reliable)
	void ServerSetLocalPlayerSkin(int32 LocalPlayerIndex, int32 SkinIndex);

	UFUNCTION(Server, Reliable)
	void ServerStepLocalPlayerSkin(int32 LocalPlayerIndex, int32 Direction);

private:
	void PublishLocalPlayersToHost();

	UPROPERTY()
	TArray<FLobbyPlayerData> MyLocalPlayers;
};
