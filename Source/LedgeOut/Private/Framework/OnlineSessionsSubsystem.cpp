#include "Framework/OnlineSessionsSubsystem.h"

#include "OnlineSubsystem.h"
#include "OnlineSessionSettings.h"

UOnlineSessionsSubsystem::UOnlineSessionsSubsystem()
{
	if (const IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get())
	{
		SessionInterface = Subsystem->GetSessionInterface();
	}
}

void UOnlineSessionsSubsystem::HostSession(const int32 MaxPlayers, const bool bIsLan)
{
	if (!SessionInterface.IsValid())
	{
		UE_LOG(LogOnline, Error, TEXT("Session Interface is not valid"));
	}
	
	SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegateHandle);
	
	CreateSessionCompleteDelegateHandle = SessionInterface->AddOnCreateSessionCompleteDelegate_Handle(
		FOnCreateSessionCompleteDelegate::CreateUObject(this, &UOnlineSessionsSubsystem::OnCreateSessionComplete)
	);
	
	// Basic session config
	FOnlineSessionSettings SessionSettings;
	SessionSettings.bIsLANMatch = bIsLan;
	SessionSettings.NumPublicConnections = MaxPlayers;
	SessionSettings.bAllowJoinInProgress = false;
	SessionSettings.bShouldAdvertise = true;
	SessionSettings.bUsesPresence = true;
	SessionSettings.bAllowJoinViaPresence = true;

	const ULocalPlayer* LocalPlayer = GetWorld() ? GetWorld()->GetFirstLocalPlayerFromController() : nullptr;
	const FUniqueNetIdRepl NetId = LocalPlayer ? LocalPlayer->GetPreferredUniqueNetId() : FUniqueNetIdRepl();

	if (NetId.IsValid() && NetId.GetUniqueNetId().IsValid())
	{
		UE_LOG(LogOnline, Log, TEXT("Creating session with UniqueNetID"));
		SessionInterface->CreateSession(*NetId.GetUniqueNetId(), NAME_GameSession, SessionSettings);
	}
	else
	{
		UE_LOG(LogOnline, Warning, TEXT("Creating session without UniqueNetID"));
		SessionInterface->CreateSession(0, NAME_GameSession, SessionSettings);
	}
}

void UOnlineSessionsSubsystem::ConnectToServer(FString IPAddr)
{
	
}

void UOnlineSessionsSubsystem::OnCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
	if (SessionInterface.IsValid())
	{
		SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegateHandle);
	}

	if (bWasSuccessful)
	{
		UWorld* World = GetWorld();
		if (World)
		{
			FString CurrentMapName = World->GetMapName();
			CurrentMapName.RemoveFromStart(World->StreamingLevelsPrefix);
			
			World->ServerTravel(FString::Printf(TEXT("%s?listen"), *CurrentMapName));
		}
	}
}
