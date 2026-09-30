#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "OnlineSubsystem.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "OnlineSessionsSubsystem.generated.h"

/**
 * 
 */
UCLASS()
class LEDGEOUT_API UOnlineSessionsSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
	
public:
	virtual void Deinitialize() override;
	
	UFUNCTION(BlueprintCallable, Category = "Online|Sessions")
	void HostSession(int32 MaxPlayers, bool isLan);

	UFUNCTION(BlueprintCallable, Category = "Online|Sessions")
	void DestroySession();
	
	UFUNCTION(BlueprintCallable, Category = "Online|Sessions|Test")
	void ConnectToServer(FString IPAddr);
	
private:
	IOnlineSessionPtr GetSessionInterface() const;
	void CreateSessionInternal(int32 MaxPlayers, bool bIsLan);

	void OnCreateSessionComplete(FName SessionName, bool bWasSuccessful);
	void OnDestroySessionComplete(FName SessionName, bool bWasSuccessful);

	FDelegateHandle CreateSessionCompleteDelegateHandle;
	FDelegateHandle DestroySessionCompleteDelegateHandle;

	bool bCreateSessionOnDestroy = false;
	int32 PendingMaxPlayers = 4;
	bool bPendingIsLan = false;
};
