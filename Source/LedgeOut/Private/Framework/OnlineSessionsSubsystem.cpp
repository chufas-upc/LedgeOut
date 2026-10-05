#include "Framework/OnlineSessionsSubsystem.h"

#include "OnlineSubsystem.h"
#include "OnlineSessionSettings.h"
#include "FindSessionsCallbackProxy.h"
#include "OnlineBeaconHost.h"
#include "Framework/LobbyBeaconHostObject.h"
#include "Framework/LobbyBeaconClient.h"

IOnlineSessionPtr UOnlineSessionsSubsystem::GetSessionInterface() const
{
	const IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get();
	return Subsystem ? Subsystem->GetSessionInterface() : nullptr;
}

void UOnlineSessionsSubsystem::Deinitialize()
{
	if (BeaconClient)
	{
		BeaconClient->DestroyBeacon();
		BeaconClient = nullptr;
	}

	if (BeaconHost)
	{
		BeaconHost->DestroyBeacon();
		BeaconHost = nullptr;
		LobbyHostObject = nullptr;
	}

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

bool UOnlineSessionsSubsystem::SearchSessions(const int32 SearchingPlayerNum, const bool bIsLanQuery)
{
	IOnlineSessionPtr SessionInterface = GetSessionInterface();
	if (!SessionInterface.IsValid())
	{
		UE_LOG(LogOnline, Error, TEXT("Session Interface is not valid"));
		return false;
	}
	
	SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegateHandle);
	
	FindSessionsCompleteDelegateHandle = SessionInterface->AddOnFindSessionsCompleteDelegate_Handle(
		FOnFindSessionsCompleteDelegate::CreateUObject(this, &UOnlineSessionsSubsystem::FindSessionsComplete));
	
	SessionSearch = MakeShared<FOnlineSessionSearch>();

	SessionSearch->bIsLanQuery = bIsLanQuery;
	SessionSearch->MaxSearchResults = 10;
	
	return SessionInterface->FindSessions(SearchingPlayerNum, SessionSearch.ToSharedRef());
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

void UOnlineSessionsSubsystem::FindSessionsComplete(bool bWasSuccessful)
{
	IOnlineSessionPtr SessionInterface = GetSessionInterface();
	if (SessionInterface.IsValid())
	{
		SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegateHandle);
	}

	if (!bWasSuccessful || !SessionSearch.IsValid())
	{
		UE_LOG(LogOnline, Warning, TEXT("FindSessions failed or SessionSearch is invalid"));
		return;
	}

	if (SessionSearch->SearchResults.Num() > 0)
	{
		UE_LOG(LogOnline, Log, TEXT("Subsystem found %d sessions"), SessionSearch->SearchResults.Num());

		SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegateHandle);
		JoinSessionCompleteDelegateHandle = SessionInterface->AddOnJoinSessionCompleteDelegate_Handle(
			FOnJoinSessionCompleteDelegate::CreateUObject(this, &UOnlineSessionsSubsystem::OnJoinSessionComplete)
		);

		SessionInterface->JoinSession(0, NAME_GameSession, SessionSearch->SearchResults[0]);
	}
	else
	{
		UE_LOG(LogOnline, Log, TEXT("No sessions found"));
	}
}

void UOnlineSessionsSubsystem::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	IOnlineSessionPtr SessionInterface = GetSessionInterface();
	if (SessionInterface.IsValid())
	{
		SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegateHandle);

		if (Result == EOnJoinSessionCompleteResult::Success || Result == EOnJoinSessionCompleteResult::AlreadyInSession)
		{
			FString ConnectString;
			if (SessionInterface->GetResolvedConnectString(SessionName, ConnectString))
			{
				FString HostIP = ConnectString;
				int32 PortIndex = -1;
				if (HostIP.FindChar(':', PortIndex))
				{
					HostIP = HostIP.Left(PortIndex);
				}

				if (HostIP.IsEmpty() || HostIP == TEXT("0.0.0.0"))
				{
					HostIP = TEXT("127.0.0.1");
				}

				UE_LOG(LogOnline, Warning, TEXT("Joined session successfully. Connecting BeaconClient to Host IP: %s:15000..."), *HostIP);
				ConnectToServer(HostIP, 15000);
			}
		}
		else
		{
			UE_LOG(LogOnline, Warning, TEXT("Failed to join session: %s"), *SessionName.ToString());
		}
	}
}

void UOnlineSessionsSubsystem::DestroySession()
{
	if (BeaconClient)
	{
		BeaconClient->DestroyBeacon();
		BeaconClient = nullptr;
	}

	if (BeaconHost)
	{
		BeaconHost->DestroyBeacon();
		BeaconHost = nullptr;
		LobbyHostObject = nullptr;
	}

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

void UOnlineSessionsSubsystem::ConnectToServer(FString IPAddr, int32 Port)
{
	UWorld* World = GetWorld();
	if (!World) return;

	if (IPAddr.IsEmpty())
	{
		IPAddr = TEXT("127.0.0.1");
	}

	if (BeaconClient)
	{
		BeaconClient->DestroyBeacon();
		BeaconClient = nullptr;
	}

	BeaconClient = World->SpawnActor<ALobbyBeaconClient>(ALobbyBeaconClient::StaticClass());
	if (BeaconClient)
	{
		FURL Destination(nullptr, *IPAddr, TRAVEL_Absolute);
		Destination.Port = Port;
		UE_LOG(LogOnline, Warning, TEXT("BeaconClient connecting to %s:%d..."), *IPAddr, Destination.Port);
		
		if (!BeaconClient->InitClient(Destination))
		{
			UE_LOG(LogOnline, Error, TEXT("BeaconClient InitClient FAILED to initialize net driver for destination %s:%d!"), *IPAddr, Destination.Port);
		}
		else
		{
			UE_LOG(LogOnline, Warning, TEXT("BeaconClient InitClient returned true, waiting for connection..."));
		}
	}
	else
	{
		UE_LOG(LogOnline, Error, TEXT("Failed to spawn ALobbyBeaconClient actor!"));
	}
}

void UOnlineSessionsSubsystem::OnCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
	IOnlineSessionPtr SessionInterface = GetSessionInterface();
	if (SessionInterface.IsValid())
	{
		SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegateHandle);
	}

	if (!bWasSuccessful)
	{
		UE_LOG(LogOnline, Warning, TEXT("Failed to create session"));
		return;
	}

	if (UWorld* World = GetWorld())
	{
		if (BeaconHost)
		{
			BeaconHost->DestroyBeacon();
			BeaconHost = nullptr;
		}

		BeaconHost = World->SpawnActor<AOnlineBeaconHost>(AOnlineBeaconHost::StaticClass());
		if (BeaconHost)
		{
			BeaconHost->ListenPort = 15000;
			if (BeaconHost->InitHost())
			{
				BeaconHost->PauseBeaconRequests(false);
				UE_LOG(LogOnline, Warning, TEXT("BeaconHost InitHost SUCCEEDED listening on port %d"), BeaconHost->GetListenPort());

				LobbyHostObject = World->SpawnActor<ALobbyBeaconHostObject>(ALobbyBeaconHostObject::StaticClass());
				if (LobbyHostObject)
				{
					BeaconHost->RegisterHost(LobbyHostObject);
					UE_LOG(LogOnline, Warning, TEXT("Online Beacon Host registered ALobbyBeaconHostObject successfully!"));
				}
				else
				{
					UE_LOG(LogOnline, Error, TEXT("Failed to spawn ALobbyBeaconHostObject actor!"));
				}
			}
			else
			{
				UE_LOG(LogOnline, Error, TEXT("Failed to initialize Online Beacon Host!"));
			}
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

void UOnlineSessionsSubsystem::ClientTravel(APlayerController* PC, FString URL, ETravelType TravelTipe, bool bIsSeamless)
{
	IOnlineSessionPtr SessionInterface = GetSessionInterface();
	if (!SessionInterface.IsValid())
	{
		UE_LOG(LogOnline, Error, TEXT("Session Interface is not valid"));
		return;
	}
	
	PC->ClientTravel(URL, TravelTipe, bIsSeamless);
}

void UOnlineSessionsSubsystem::ServerTravel(FString URL, bool bIsAbsolute)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		UE_LOG(LogOnline, Error, TEXT("World is not valid"));
		return;
	}
	
	World->ServerTravel(URL, bIsAbsolute);
}

bool UOnlineSessionsSubsystem::JoinSession(int32 LocalUserNum, FName SessionName, const FBlueprintSessionResult& SearchResult)
{
	IOnlineSessionPtr SessionInterface = GetSessionInterface();
	if (!SessionInterface.IsValid())
	{
		UE_LOG(LogOnline, Error, TEXT("Session Interface is not valid"));
		return false;
	}
	
	SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegateHandle);
	JoinSessionCompleteDelegateHandle = SessionInterface->AddOnJoinSessionCompleteDelegate_Handle(
		FOnJoinSessionCompleteDelegate::CreateUObject(this, &UOnlineSessionsSubsystem::OnJoinSessionComplete)
	);

	return SessionInterface->JoinSession(LocalUserNum, SessionName, SearchResult.OnlineResult);
}
