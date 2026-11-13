#include "Framework/Lobby/LobbyGameMode.h"
#include "Framework/LedgeOutPlayerState.h"

void ALobbyGameMode::BeginPlay()
{
	Super::BeginPlay();
}

APlayerController* ALobbyGameMode::Login(UPlayer* NewPlayer, ENetRole InRemoteRole, const FString& Portal,
	const FString& Options, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage)
{
	GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Green, FString::Printf(TEXT("Logging in player")));
	return Super::Login(NewPlayer, InRemoteRole, Portal, Options, UniqueId, ErrorMessage);
}

void ALobbyGameMode::Logout(AController* Exiting)
{
	Super::Logout(Exiting);
	GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("Player controller disconnected!")));
}

void ALobbyGameMode::OnPostLogin(AController* NewPlayer)
{
	Super::OnPostLogin(NewPlayer);
	GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Green, FString::Printf(TEXT("Player controller connected!")));
	
	ALedgeOutPlayerState* PS = NewPlayer->GetPlayerState<ALedgeOutPlayerState>();
	if (IsValid(PS))
	{
		OnPlayerConnected.Broadcast(PS);
	}
}

TSoftObjectPtr<USkeletalMesh> ALobbyGameMode::GetCharacterMeshSoft(const int32 Index) const
{
	if (AvailableCharacters.IsValidIndex(Index))
	{
		return AvailableCharacters[Index].SkeletalMesh;
	}
	return TSoftObjectPtr<USkeletalMesh>();
}

int32 ALobbyGameMode::GetSkinCount(const int32 CharacterIndex) const
{
	return AvailableCharacters.IsValidIndex(CharacterIndex)
		? AvailableCharacters[CharacterIndex].AvailableSkins.Num()
		: 0;
}

TSoftObjectPtr<USkeletalMesh> ALobbyGameMode::GetSkinMeshSoft(const int32 CharacterIndex, const int32 SkinIndex) const
{
	if (AvailableCharacters.IsValidIndex(CharacterIndex)
		&& AvailableCharacters[CharacterIndex].AvailableSkins.IsValidIndex(SkinIndex))
	{
		return AvailableCharacters[CharacterIndex].AvailableSkins[SkinIndex].SkeletalMesh;
	}
	return TSoftObjectPtr<USkeletalMesh>();
}
