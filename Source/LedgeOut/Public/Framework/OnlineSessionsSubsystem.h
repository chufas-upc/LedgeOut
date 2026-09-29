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
	UOnlineSessionsSubsystem();
	
	UFUNCTION(BlueprintCallable, Category = "Online|Sessions")
	void HostSession(int32 MaxPlayers, bool isLan);
	
	UFUNCTION(BlueprintCallable, Category = "Online|Sessions|Test")
	void ConnectToServer(FString IPAddr);
	
private:
	void OnCreateSessionComplete(FName SessionName, bool bWasSuccessful);

	IOnlineSessionPtr SessionInterface;
	FDelegateHandle CreateSessionCompleteDelegateHandle;
};
