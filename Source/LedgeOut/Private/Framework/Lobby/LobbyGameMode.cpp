#include "Framework/Lobby/LobbyGameMode.h"
#include "Framework/LedgeOutPlayerState.h"

void ALobbyGameMode::BeginPlay()
{
	Super::BeginPlay();
}

APlayerController* ALobbyGameMode::Login(UPlayer* NewPlayer, ENetRole InRemoteRole, const FString& Portal,
	const FString& Options, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage)
{
	GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Green, FString::Printf(TEXT("Loggin player")));
	return Super::Login(NewPlayer, InRemoteRole, Portal, Options, UniqueId, ErrorMessage);
}

void ALobbyGameMode::Logout(AController* Exiting)
{
	Super::Logout(Exiting);
	GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("Player logged out!")));
}

void ALobbyGameMode::OnPostLogin(AController* NewPlayer)
{
	Super::OnPostLogin(NewPlayer);
	GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Green, FString::Printf(TEXT("Player logged!")));
	
	ALedgeOutPlayerState* PS = NewPlayer->GetPlayerState<ALedgeOutPlayerState>();
	if (IsValid(PS))
	{
		OnPlayerConnected.Broadcast(PS);
	}
}

FString ALobbyGameMode::GetCharacterNameByIndex(const int32 Index)
{
	return !CharacterNames.IsValidIndex(Index) ? CharacterNames[Index] : "NO VALID";
}
