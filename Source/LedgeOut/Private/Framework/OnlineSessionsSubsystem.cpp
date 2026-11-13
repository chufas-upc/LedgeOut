#include "Framework/OnlineSessionsSubsystem.h"

#include "OnlineSubsystem.h"
#include "OnlineSessionSettings.h"
#include "FindSessionsCallbackProxy.h"
#include "Interfaces/OnlineIdentityInterface.h"
#include "OnlineBeaconHost.h"
#include "OnlineSubsystemUtils.h"
#include "Framework/LobbyBeaconHostObject.h"
#include "Framework/LobbyBeaconClient.h"
#include "GameFramework/PlayerState.h"

void UOnlineSessionsSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	IOnlineSessionPtr SessionInterface = GetSessionInterface();
	if (!SessionInterface.IsValid())
	{
		UE_LOG(LogOnline, Error, TEXT("Cannot register session invite handling: Session Interface is not valid."));
		return;
	}

	SessionUserInviteAcceptedDelegateHandle = SessionInterface->AddOnSessionUserInviteAcceptedDelegate_Handle(
		FOnSessionUserInviteAcceptedDelegate::CreateUObject(
			this, &UOnlineSessionsSubsystem::OnSessionUserInviteAccepted));
}

IOnlineSessionPtr UOnlineSessionsSubsystem::GetSessionInterface() const
{
	const IOnlineSubsystem* Subsystem = Online::GetSubsystem(GetWorld());
	return Subsystem ? Subsystem->GetSessionInterface() : nullptr;
}

FString UOnlineSessionsSubsystem::GetPlayerDisplayName(const APlayerState* PlayerState)
{
	if (!IsValid(PlayerState))
	{
		return FString();
	}

	const FUniqueNetIdRepl& UniqueId = PlayerState->GetUniqueId();
	if (UniqueId.GetUniqueNetId().IsValid())
	{
		if (const IOnlineSubsystem* Subsystem = Online::GetSubsystem(PlayerState->GetWorld()))
		{
			const IOnlineIdentityPtr Identity = Subsystem->GetIdentityInterface();
			if (Identity.IsValid())
			{
				const FString Nickname = Identity->GetPlayerNickname(*UniqueId.GetUniqueNetId());
				if (!Nickname.IsEmpty())
				{
					return Nickname;
				}
			}
		}
	}

	return PlayerState->GetPlayerName();
}

void UOnlineSessionsSubsystem::Deinitialize()
{
	CachedLobbyPlayers.Empty();

	if (BeaconClient)
	{
		BeaconClient->OnLobbyUpdated.RemoveAll(this);
		BeaconClient->DestroyBeacon();
		BeaconClient = nullptr;
	}

	if (BeaconHost)
	{
		if (LobbyHostObject)
		{
			LobbyHostObject->OnHostLobbyUpdated.RemoveAll(this);
		}
		BeaconHost->DestroyBeacon();
		BeaconHost = nullptr;
		LobbyHostObject = nullptr;
	}

	IOnlineSessionPtr SessionInterface = GetSessionInterface();
	if (SessionInterface.IsValid())
	{
		SessionInterface->ClearOnSessionUserInviteAcceptedDelegate_Handle(SessionUserInviteAcceptedDelegateHandle);
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
	SessionSettings.bAllowJoinInProgress = true;
	SessionSettings.bShouldAdvertise = true;
	SessionSettings.bUsesPresence = true;
	SessionSettings.bAllowJoinViaPresence = true;
	SessionSettings.bUseLobbiesIfAvailable = true;

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
		JoinSessionInternal(0, NAME_GameSession, SessionSearch->SearchResults[0]);
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
			OnConnectionChanged.Broadcast(ELobbyStatus::Client);
			
			FString ConnectString;
			if (SessionInterface->GetResolvedConnectString(SessionName, ConnectString))
			{
				UE_LOG(LogOnline, Log, TEXT("Joined session successfully. Connecting BeaconClient through the resolved session address."));
				ConnectToServer(ConnectString, 15000);
			}
			else
			{
				UE_LOG(LogOnline, Error, TEXT("Joined session, but the session connection address could not be resolved."));
			}
		}
		else
		{
			UE_LOG(LogOnline, Warning, TEXT("Failed to join session: %s"), *SessionName.ToString());
		}
	}
}

void UOnlineSessionsSubsystem::OnSessionUserInviteAccepted(
	const bool bWasSuccessful,
	const int32 LocalUserNum,
	TSharedPtr<const FUniqueNetId>,
	const FOnlineSessionSearchResult& InviteResult)
{
	if (!bWasSuccessful || !InviteResult.IsValid())
	{
		UE_LOG(LogOnline, Warning, TEXT("Steam session invite was accepted without a valid session result."));
		return;
	}

	UE_LOG(LogOnline, Log, TEXT("Steam session invite accepted. Joining the invited session."));
	JoinSessionInternal(LocalUserNum, NAME_GameSession, InviteResult);
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

void UOnlineSessionsSubsystem::ResetAfterRound()
{
	bCreateSessionOnDestroy = false;
	DestroySession();

	CachedLobbyPlayers.Empty();
	OnLobbyPlayersUpdated.Broadcast(CachedLobbyPlayers);
	OnConnectionChanged.Broadcast(ELobbyStatus::Disconnected);
}

void UOnlineSessionsSubsystem::ConnectToServer(FString IPAddr, int32 Port)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		UE_LOG(LogOnline, Error, TEXT("Cannot connect to lobby beacon: World is not valid."));
		return;
	}

	if (IPAddr.IsEmpty())
	{
		UE_LOG(LogOnline, Error, TEXT("Cannot connect to lobby beacon: resolved session address is empty."));
		return;
	}

	if (BeaconClient)
	{
		BeaconClient->DestroyBeacon();
		BeaconClient = nullptr;
	}

	BeaconClient = World->SpawnActor<ALobbyBeaconClient>(ALobbyBeaconClient::StaticClass());
	if (BeaconClient)
	{
		BeaconClient->OnLobbyUpdated.AddDynamic(this, &UOnlineSessionsSubsystem::HandleLobbyUpdated);

		FURL Destination(nullptr, *IPAddr, TRAVEL_Absolute);
		Destination.Port = Port;
		UE_LOG(LogOnline, Log, TEXT("BeaconClient connecting through the resolved session address on port %d..."), Destination.Port);
		
		if (!BeaconClient->InitClient(Destination))
		{
			UE_LOG(LogOnline, Error, TEXT("BeaconClient InitClient failed to initialize the network driver."));
		}
		else
		{
			UE_LOG(LogOnline, Log, TEXT("BeaconClient InitClient returned true, waiting for connection..."));
		}
	}
	else
	{
		UE_LOG(LogOnline, Error, TEXT("Failed to spawn ALobbyBeaconClient actor!"));
	}
}

void UOnlineSessionsSubsystem::StartGameLevel(const FString MapAddress)
{
	IOnlineSessionPtr SessionInterface = GetSessionInterface();
	if (!SessionInterface.IsValid() || !IsValid(LobbyHostObject))
	{
		UE_LOG(LogOnline, Error, TEXT("Cannot start the lobby game: Session Interface or LobbyHostObject is not valid."));
		return;
	}

	FString URL;
	if (!SessionInterface->GetResolvedConnectString(NAME_GameSession, URL))
	{
		UE_LOG(LogOnline, Error, TEXT("Cannot start the lobby game: session connection address could not be resolved."));
		return;
	}

	FURL GameURL(nullptr, *URL, TRAVEL_Absolute);
	GameURL.Port = 7777;
	URL = GameURL.ToString();
	LobbyHostObject->StartGameForLobby(MapAddress, URL, CachedLobbyPlayers);
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
	
	OnConnectionChanged.Broadcast(ELobbyStatus::Host);

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
				UE_LOG(LogOnline, Log, TEXT("BeaconHost InitHost SUCCEEDED listening on port %d"), BeaconHost->GetListenPort());

				LobbyHostObject = World->SpawnActor<ALobbyBeaconHostObject>(ALobbyBeaconHostObject::StaticClass());
				if (LobbyHostObject)
				{
					LobbyHostObject->OnHostLobbyUpdated.AddDynamic(this, &UOnlineSessionsSubsystem::HandleLobbyUpdated);
					BeaconHost->RegisterHost(LobbyHostObject);
					UE_LOG(LogOnline, Log, TEXT("Online Beacon Host registered ALobbyBeaconHostObject successfully!"));

					// Register host's initial local player
					FLobbyPlayerData HostP1;
					HostP1.PlayerName = TEXT("Host_Player");
					if (APlayerController* PC = World->GetFirstPlayerController())
					{
						if (APlayerState* PS = PC->PlayerState)
						{
							HostP1.PlayerName = GetPlayerDisplayName(PS);
							const FUniqueNetIdRepl& UniqueId = PS->GetUniqueId();
							if (UniqueId.GetUniqueNetId().IsValid())
							{
								HostP1.PlayerUniqueId = UniqueId.GetUniqueNetId()->ToString();
							}
						}
					}
					HostP1.LocalPlayerIndex = 0;
					HostP1.SelectedCharacterIndex = 0;
					HostP1.bIsReady = true;

					LobbyHostObject->AddHostLocalPlayer(HostP1);
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
	CachedLobbyPlayers.Empty();
	OnLobbyPlayersUpdated.Broadcast(CachedLobbyPlayers);

	IOnlineSessionPtr SessionInterface = GetSessionInterface();
	if (SessionInterface.IsValid())
	{
		SessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegateHandle);
	}
	
	OnConnectionChanged.Broadcast(ELobbyStatus::Disconnected);

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

void UOnlineSessionsSubsystem::ServerTravel(FString Map, bool bIsAbsolute)
{
	if (!bIsAbsolute && !IsValid(LobbyHostObject))
	{
		UE_LOG(LogOnline, Error, TEXT("LobbyHostObject is not valid"));
		return;
	}
	
	if (bIsAbsolute)
	{
		GetWorld()->ServerTravel(Map, true);
	}
	else
	{
		
		IOnlineSessionPtr SessionInterface = GetSessionInterface();
		if (!SessionInterface.IsValid())
		{
			UE_LOG(LogOnline, Error, TEXT("Cannot travel from the lobby: Session Interface is not valid."));
			return;
		}

		FString URL;
		if (!SessionInterface->GetResolvedConnectString(NAME_GameSession, URL))
		{
			UE_LOG(LogOnline, Error, TEXT("Cannot travel from the lobby: session connection address could not be resolved."));
			return;
		}

		FURL GameURL(nullptr, *URL, TRAVEL_Absolute);
		GameURL.Port = 7777;
		URL = GameURL.ToString();
		LobbyHostObject->StartGameForLobby(Map, URL, CachedLobbyPlayers);
	}
}

bool UOnlineSessionsSubsystem::JoinSession(int32 LocalUserNum, FName SessionName, const FBlueprintSessionResult& SearchResult)
{
	return JoinSessionInternal(LocalUserNum, SessionName, SearchResult.OnlineResult);
}

bool UOnlineSessionsSubsystem::JoinSessionInternal(
	const int32 LocalUserNum,
	const FName SessionName,
	const FOnlineSessionSearchResult& SearchResult)
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

	if (!SessionInterface->JoinSession(LocalUserNum, SessionName, SearchResult))
	{
		SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegateHandle);
		UE_LOG(LogOnline, Error, TEXT("Online subsystem rejected the request to join session '%s'."), *SessionName.ToString());
		return false;
	}

	return true;
}

void UOnlineSessionsSubsystem::ForceCreateHostBeacon()
{
	OnCreateSessionComplete(NAME_GameSession, true);
}

void UOnlineSessionsSubsystem::DebugPrintCurrentSession() const
{
    IOnlineSessionPtr SessionInterface = GetSessionInterface();

    if (!SessionInterface.IsValid())
    {
        UE_LOG(LogOnline, Error, TEXT("========== SESSION DEBUG =========="));
        UE_LOG(LogOnline, Error, TEXT("SessionInterface is INVALID"));
        UE_LOG(LogOnline, Error, TEXT("==================================="));
        return;
    }

    const FNamedOnlineSession* NamedSession =
        SessionInterface->GetNamedSession(NAME_GameSession);

    UE_LOG(LogOnline, Log, TEXT(""));
    UE_LOG(LogOnline, Log, TEXT("========== SESSION DEBUG =========="));

    if (!NamedSession)
    {
        UE_LOG(LogOnline, Log, TEXT("No named session found"));

        UE_LOG(LogOnline, Log, TEXT("==================================="));
        return;
    }

	const FOnlineSession& Session = *NamedSession;

    UE_LOG(LogOnline, Log, TEXT("Session Name: %s"),
        *NamedSession->SessionName.ToString());

    UE_LOG(LogOnline, Log, TEXT("Session State: %d"),
        static_cast<int32>(NamedSession->SessionState));

    UE_LOG(LogOnline, Log, TEXT("Num Public Connections: %d"),
        Session.SessionSettings.NumPublicConnections);

    UE_LOG(LogOnline, Log, TEXT("Num Private Connections: %d"),
        Session.SessionSettings.NumPrivateConnections);

    UE_LOG(LogOnline, Log, TEXT("Num Open Public Connections: %d"),
        Session.NumOpenPublicConnections);

    UE_LOG(LogOnline, Log, TEXT("Num Open Private Connections: %d"),
        Session.NumOpenPrivateConnections);

    UE_LOG(LogOnline, Log, TEXT("bIsLANMatch: %s"),
        Session.SessionSettings.bIsLANMatch ? TEXT("true") : TEXT("false"));

    UE_LOG(LogOnline, Log, TEXT("bShouldAdvertise: %s"),
        Session.SessionSettings.bShouldAdvertise ? TEXT("true") : TEXT("false"));

    UE_LOG(LogOnline, Log, TEXT("bAllowJoinInProgress: %s"),
        Session.SessionSettings.bAllowJoinInProgress ? TEXT("true") : TEXT("false"));

    UE_LOG(LogOnline, Log, TEXT("bAllowJoinViaPresence: %s"),
        Session.SessionSettings.bAllowJoinViaPresence ? TEXT("true") : TEXT("false"));

    UE_LOG(LogOnline, Log, TEXT("bAllowJoinViaPresenceFriendsOnly: %s"),
        Session.SessionSettings.bAllowJoinViaPresenceFriendsOnly ? TEXT("true") : TEXT("false"));

    UE_LOG(LogOnline, Log, TEXT("bUsesPresence: %s"),
        Session.SessionSettings.bUsesPresence ? TEXT("true") : TEXT("false"));

    UE_LOG(LogOnline, Log, TEXT("bUsesStats: %s"),
        Session.SessionSettings.bUsesStats ? TEXT("true") : TEXT("false"));

    UE_LOG(LogOnline, Log, TEXT("bIsDedicated: %s"),
        Session.SessionSettings.bIsDedicated ? TEXT("true") : TEXT("false"));

    UE_LOG(LogOnline, Log, TEXT("bUseLobbiesIfAvailable: %s"),
        Session.SessionSettings.bUseLobbiesIfAvailable ? TEXT("true") : TEXT("false"));

    UE_LOG(LogOnline, Log, TEXT("bUseLobbiesVoiceChatIfAvailable: %s"),
        Session.SessionSettings.bUseLobbiesVoiceChatIfAvailable ? TEXT("true") : TEXT("false"));

    UE_LOG(LogOnline, Log, TEXT("bAntiCheatProtected: %s"),
        Session.SessionSettings.bAntiCheatProtected ? TEXT("true") : TEXT("false"));

    UE_LOG(LogOnline, Log, TEXT("Advertisement Type: %d"),
        static_cast<int32>(Session.SessionSettings.bShouldAdvertise));

    UE_LOG(LogOnline, Log, TEXT(""));
    UE_LOG(LogOnline, Log, TEXT("--- Custom Session Settings ---"));

    for (const TPair<FName, FOnlineSessionSetting>& Setting :
         Session.SessionSettings.Settings)
    {
        FString ValueString;

        Setting.Value.Data.GetValue(ValueString);

        UE_LOG(
            LogOnline,
            Log,
            TEXT("  [%s] = %s | AdvertisementType=%d | ID=%d"),
            *Setting.Key.ToString(),
            *ValueString,
            static_cast<int32>(Setting.Value.AdvertisementType),
            Setting.Value.ID
        );
    }

    UE_LOG(LogOnline, Log, TEXT(""));
    UE_LOG(LogOnline, Log, TEXT("--- Registered Players ---"));

    // for (const TSharedRef<const FUniqueNetId>& Player :
    //      Session.RegisteredPlayers)
    // {
    //     UE_LOG(
    //         LogOnline,
    //         Log,
    //         TEXT("  Player: %s"),
    //         *Player->ToString()
    //     );
    // }
	
	IOnlineSubsystem* OSS = Online::GetSubsystem(GetWorld());

	if (OSS)
	{
		UE_LOG(
			LogOnline,
			Log,
			TEXT("[SESSION] OSS = %s"),
			*OSS->GetSubsystemName().ToString()
		);
	}
	else
	{
		UE_LOG(LogOnline, Warning, TEXT("No OSS"));
	}

    UE_LOG(LogOnline, Log, TEXT("==================================="));
    UE_LOG(LogOnline, Log, TEXT(""));
	
	SessionInterface->DumpSessionState();
}

void UOnlineSessionsSubsystem::HandleLobbyUpdated(const TArray<FLobbyPlayerData>& PlayerList)
{
	CachedLobbyPlayers = PlayerList;
	OnLobbyPlayersUpdated.Broadcast(CachedLobbyPlayers);
}
