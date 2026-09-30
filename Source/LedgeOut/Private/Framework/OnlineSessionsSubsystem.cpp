#include "Framework/OnlineSessionsSubsystem.h"

#include "OnlineSubsystem.h"
#include "OnlineSessionSettings.h"
#include "Kismet/GameplayStatics.h"

IOnlineSessionPtr UOnlineSessionsSubsystem::GetSessionInterface() const
{
	const IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get();
	return Subsystem ? Subsystem->GetSessionInterface() : nullptr;
}

void UOnlineSessionsSubsystem::Deinitialize()
{
	IOnlineSessionPtr SessionInterface = GetSessionInterface();
	if (SessionInterface.IsValid())
	{
		if (SessionInterface->GetNamedSession(NAME_GameSession) != nullptr)
		{
			SessionInterface->DestroySession(NAME_GameSession);
		}
	}

	Super::Deinitialize();
}

void UOnlineSessionsSubsystem::HostSession(const int32 MaxPlayers, const bool bIsLan)
{
	IOnlineSessionPtr SessionInterface = GetSessionInterface();
	if (!SessionInterface.IsValid())
	{
		UE_LOG(LogOnline, Error, TEXT("Session Interface is not valid"));
		return;
	}

	FNamedOnlineSession* ExistingSession = SessionInterface->GetNamedSession(NAME_GameSession);
	if (ExistingSession != nullptr)
	{
		UE_LOG(LogOnline, Log, TEXT("Existing session found. Destroying session before creating a new one..."));
		bCreateSessionOnDestroy = true;
		PendingMaxPlayers = MaxPlayers;
		bPendingIsLan = bIsLan;
		DestroySession();
		return;
	}

	CreateSessionInternal(MaxPlayers, bIsLan);
}

void UOnlineSessionsSubsystem::CreateSessionInternal(const int32 MaxPlayers, const bool bIsLan)
{
	IOnlineSessionPtr SessionInterface = GetSessionInterface();
	if (!SessionInterface.IsValid())
	{
		return;
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

void UOnlineSessionsSubsystem::DestroySession()
{
	IOnlineSessionPtr SessionInterface = GetSessionInterface();
	if (!SessionInterface.IsValid())
	{
		UE_LOG(LogOnline, Error, TEXT("Session Interface is not valid"));
		return;
	}

	SessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegateHandle);
	DestroySessionCompleteDelegateHandle = SessionInterface->AddOnDestroySessionCompleteDelegate_Handle(
		FOnDestroySessionCompleteDelegate::CreateUObject(this, &UOnlineSessionsSubsystem::OnDestroySessionComplete)
	);

	if (!SessionInterface->DestroySession(NAME_GameSession))
	{
		SessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegateHandle);
	}
}

void UOnlineSessionsSubsystem::ConnectToServer(FString IPAddr)
{
	IOnlineSessionPtr SessionInterface = GetSessionInterface();
	if (!SessionInterface.IsValid())
	{
		UE_LOG(LogOnline, Error, TEXT("Session Interface is not valid"));
		return;
	}
	UGameplayStatics::OpenLevel(this, *IPAddr);
}

void UOnlineSessionsSubsystem::OnCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
	IOnlineSessionPtr SessionInterface = GetSessionInterface();
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

void UOnlineSessionsSubsystem::OnDestroySessionComplete(FName SessionName, bool bWasSuccessful)
{
	IOnlineSessionPtr SessionInterface = GetSessionInterface();
	if (SessionInterface.IsValid())
	{
		SessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegateHandle);
	}

	if (bCreateSessionOnDestroy)
	{
		bCreateSessionOnDestroy = false;
		CreateSessionInternal(PendingMaxPlayers, bPendingIsLan);
	}
}
