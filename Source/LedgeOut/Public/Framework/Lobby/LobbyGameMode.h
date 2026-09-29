#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "LobbyGameMode.generated.h"

class ALedgeOutPlayerState;
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerConnected, ALedgeOutPlayerState*, PS);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerDisconnected, ALedgeOutPlayerState*, PS);

/**
 * 
 */
UCLASS()
class LEDGEOUT_API ALobbyGameMode : public AGameModeBase
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;
	virtual void OnPostLogin(AController* NewPlayer) override;

public:
	UPROPERTY(BlueprintAssignable)
	FOnPlayerConnected OnPlayerConnected;
	UPROPERTY(BlueprintAssignable)
	FOnPlayerDisconnected OnPlayerDisconnected;
	
	
	virtual APlayerController* Login(UPlayer* NewPlayer, ENetRole InRemoteRole, const FString& Portal,
	const FString& Options, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage) override;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TArray<FString> CharacterNames;
	
	UFUNCTION(BlueprintCallable)
	FString GetCharacterNameByIndex(int32 Index);
};
