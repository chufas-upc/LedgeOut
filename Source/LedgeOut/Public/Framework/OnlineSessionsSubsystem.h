#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "OnlineSubsystem.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "OnlineSessionSettings.h"
#include "FindSessionsCallbackProxy.h"
#include "OnlineSessionsSubsystem.generated.h"

class AOnlineBeaconHost;
class ALobbyBeaconHostObject;
class ALobbyBeaconClient;

UCLASS()
class LEDGEOUT_API UOnlineSessionsSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
	virtual void Deinitialize() override;
	
	UFUNCTION(BlueprintCallable, Category = "Online|Sessions")
	void HostSession(int32 MaxPlayers, bool isLan);
	
	UFUNCTION(BlueprintCallable, Category = "Online|Sessions")
	bool SearchSessions(int32 SearchingPlayerNum, bool bIsLanQuery);

	UFUNCTION(BlueprintCallable, Category = "Online|Sessions")
	void DestroySession();
	
	UFUNCTION(BlueprintCallable, Category = "Online|Sessions|Test")
	void ConnectToServer(FString IPAddr, int32 Port = 15000);

	UPROPERTY(Transient, BlueprintReadOnly, Category = "Online|Beacons")
	TObjectPtr<AOnlineBeaconHost> BeaconHost;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "Online|Beacons")
	TObjectPtr<ALobbyBeaconHostObject> LobbyHostObject;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "Online|Beacons")
	TObjectPtr<ALobbyBeaconClient> BeaconClient;
	
private:
	IOnlineSessionPtr GetSessionInterface() const;
	void CreateSessionInternal(int32 MaxPlayers, bool bIsLan);
	void FindSessionsComplete(bool bWasSuccessful);

	void OnCreateSessionComplete(FName SessionName, bool bWasSuccessful);
	void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);
	void OnDestroySessionComplete(FName SessionName, bool bWasSuccessful);

	FDelegateHandle CreateSessionCompleteDelegateHandle;
	FDelegateHandle FindSessionsCompleteDelegateHandle;
	FDelegateHandle JoinSessionCompleteDelegateHandle;
	FDelegateHandle DestroySessionCompleteDelegateHandle;

	bool bCreateSessionOnDestroy = false;
	int32 PendingMaxPlayers = 4;
	bool bPendingIsLan = false;
	TSharedPtr<FOnlineSessionSearch> SessionSearch;

public:
	UFUNCTION(BlueprintCallable, Category = "Online|Sessions|Test")
	void ClientTravel(APlayerController* PC, FString URL, ETravelType TravelTipe, bool bIsSeamless);
	
	UFUNCTION(BlueprintCallable, Category = "Online|Sessions|Test")
	void ServerTravel(FString URL, bool bIsAbsolute);

	UFUNCTION(BlueprintCallable, Category = "Online|Sessions|Test")
	bool JoinSession(int32 LocalUserNum, FName SessionName, const FBlueprintSessionResult& SearchResult);
};
