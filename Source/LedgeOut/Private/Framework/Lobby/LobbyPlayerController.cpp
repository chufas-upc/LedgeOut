#include "Framework/Lobby/LobbyPlayerController.h"
#include "Camera/CameraActor.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Framework/Lobby/LobbyGameMode.h"
#include "Framework/LobbyBeaconClient.h"
#include "Framework/LobbyBeaconHostObject.h"
#include "Framework/OnlineSessionsSubsystem.h"
#include "Framework/LobbyPlayerData.h"
#include "Framework/SelectionIndexUtils.h"
#include "InputAction.h"
#include "InputCoreTypes.h"
#include "InputMappingContext.h"
#include "GameFramework/PlayerState.h"
#include "Engine/LocalPlayer.h"
#include "Kismet/GameplayStatics.h"

ALobbyPlayerController::ALobbyPlayerController()
{
	bAutoManageActiveCameraTarget = false;
}

void ALobbyPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (!IsLocalController() || !IsPrimaryPlayer())
	{
		return;
	}

	UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent);
	if (!EnhancedInputComponent)
	{
		UE_LOG(LogTemp, Error, TEXT("Lobby selection input requires an Enhanced Input component."));
		return;
	}

	ULocalPlayer* LocalPlayer = GetLocalPlayer();
	UEnhancedInputLocalPlayerSubsystem* InputSubsystem = LocalPlayer
		? ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(LocalPlayer)
		: nullptr;
	if (!InputSubsystem)
	{
		UE_LOG(LogTemp, Error, TEXT("Lobby selection input requires a local Enhanced Input subsystem."));
		return;
	}

	LobbySelectionMappingContext = NewObject<UInputMappingContext>(this, TEXT("LobbySelectionMappingContext"));
	PreviousCharacterAction = NewObject<UInputAction>(this, TEXT("PreviousCharacterAction"));
	NextCharacterAction = NewObject<UInputAction>(this, TEXT("NextCharacterAction"));
	PreviousSkinAction = NewObject<UInputAction>(this, TEXT("PreviousSkinAction"));
	NextSkinAction = NewObject<UInputAction>(this, TEXT("NextSkinAction"));

	PreviousCharacterAction->ValueType = EInputActionValueType::Boolean;
	NextCharacterAction->ValueType = EInputActionValueType::Boolean;
	PreviousSkinAction->ValueType = EInputActionValueType::Boolean;
	NextSkinAction->ValueType = EInputActionValueType::Boolean;

	LobbySelectionMappingContext->MapKey(PreviousCharacterAction, EKeys::Left);
	LobbySelectionMappingContext->MapKey(NextCharacterAction, EKeys::Right);
	LobbySelectionMappingContext->MapKey(PreviousSkinAction, EKeys::Down);
	LobbySelectionMappingContext->MapKey(NextSkinAction, EKeys::Up);

	if (bLobbySelectionInputEnabled)
	{
		InputSubsystem->AddMappingContext(LobbySelectionMappingContext, 1);
	}

	EnhancedInputComponent->BindAction(
		PreviousCharacterAction, ETriggerEvent::Started, this, &ALobbyPlayerController::OnPreviousCharacterInput);
	EnhancedInputComponent->BindAction(
		NextCharacterAction, ETriggerEvent::Started, this, &ALobbyPlayerController::OnNextCharacterInput);
	EnhancedInputComponent->BindAction(
		PreviousSkinAction, ETriggerEvent::Started, this, &ALobbyPlayerController::OnPreviousSkinInput);
	EnhancedInputComponent->BindAction(
		NextSkinAction, ETriggerEvent::Started, this, &ALobbyPlayerController::OnNextSkinInput);
}

void ALobbyPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UOnlineSessionsSubsystem* SessionsSubsystem = GetGameInstance()
		? GetGameInstance()->GetSubsystem<UOnlineSessionsSubsystem>()
		: nullptr)
	{
		SessionsSubsystem->OnConnectionChanged.RemoveDynamic(
			this, &ALobbyPlayerController::OnLobbyConnectionChanged);
	}

	if (LobbySelectionMappingContext)
	{
		if (ULocalPlayer* LocalPlayer = GetLocalPlayer())
		{
			if (UEnhancedInputLocalPlayerSubsystem* InputSubsystem =
				ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(LocalPlayer))
			{
				InputSubsystem->RemoveMappingContext(LobbySelectionMappingContext);
			}
		}
	}

	Super::EndPlay(EndPlayReason);
}

void ALobbyPlayerController::OnLobbyConnectionChanged(const ELobbyStatus Status)
{
	SetLobbySelectionInputEnabled(Status != ELobbyStatus::Disconnected);
}

void ALobbyPlayerController::SetLobbySelectionInputEnabled(const bool bEnabled)
{
	bLobbySelectionInputEnabled = bEnabled;
	if (!LobbySelectionMappingContext)
	{
		return;
	}

	ULocalPlayer* LocalPlayer = GetLocalPlayer();
	UEnhancedInputLocalPlayerSubsystem* InputSubsystem = LocalPlayer
		? ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(LocalPlayer)
		: nullptr;
	if (!InputSubsystem)
	{
		return;
	}

	if (bEnabled)
	{
		InputSubsystem->AddMappingContext(LobbySelectionMappingContext, 1);
	}
	else
	{
		InputSubsystem->RemoveMappingContext(LobbySelectionMappingContext);
	}
}

void ALobbyPlayerController::OnPreviousCharacterInput()
{
	SelectPreviousCharacter();
}

void ALobbyPlayerController::OnNextCharacterInput()
{
	SelectNextCharacter();
}

void ALobbyPlayerController::OnPreviousSkinInput()
{
	SelectPreviousSkin();
}

void ALobbyPlayerController::OnNextSkinInput()
{
	SelectNextSkin();
}

ACameraActor* ALobbyPlayerController::FindCameraByTag(const FName Tag) const
{
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsWithTag(this, Tag, FoundActors);
	if (FoundActors.Num() > 0)
	{
		return Cast<ACameraActor>(FoundActors[0]);
	}
	return nullptr;
}

void ALobbyPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (UOnlineSessionsSubsystem* SessionsSubsystem = GetGameInstance()
		? GetGameInstance()->GetSubsystem<UOnlineSessionsSubsystem>()
		: nullptr)
	{
		SessionsSubsystem->OnConnectionChanged.AddUniqueDynamic(
			this, &ALobbyPlayerController::OnLobbyConnectionChanged);
		SetLobbySelectionInputEnabled(
			IsValid(SessionsSubsystem->LobbyHostObject) || IsValid(SessionsSubsystem->BeaconClient));
	}

	// if (!HasAuthority()) return;
	// Primary Player Only 
	if (!IsLocalController() || !IsPrimaryPlayer()) return;

	const FName CameraTag =
	(GetNetMode() == ENetMode::NM_Client || GetNetMode() == ENetMode::NM_ListenServer) ? "LobbyCamera" : "MenuCamera";
	
	ACameraActor* CameraActor = FindCameraByTag(CameraTag);
	if (!IsValid(CameraActor)) return;
	{
		SetViewTarget(CameraActor);
	}
}

void ALobbyPlayerController::SwitchToLobbyCamera()
{
	if (!IsLocalController() || !IsPrimaryPlayer()) return;
	ACameraActor* CameraActor = FindCameraByTag("LobbyCamera");
	if (!IsValid(CameraActor)) return;
	
	SetViewTargetWithBlend(CameraActor, 0.5f);
}

void ALobbyPlayerController::SwitchToMenuCamera()
{
	if (!IsLocalController() || !IsPrimaryPlayer()) return;
	ACameraActor* CameraActor = FindCameraByTag("MenuCamera");
	if (!IsValid(CameraActor)) return;
	
	SetViewTargetWithBlend((CameraActor), 1);
}

void ALobbyPlayerController::ChangeCharacter(const int32 CharacterIndex, const int32 LocalPlayerIndex)
{
	if (!IsLocalController())
	{
		return;
	}

	if (CharacterIndex < 0 || LocalPlayerIndex < 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot change lobby character: character and local player indices must be non-negative."));
		return;
	}

	UOnlineSessionsSubsystem* SessionsSubsystem = GetGameInstance()
		? GetGameInstance()->GetSubsystem<UOnlineSessionsSubsystem>()
		: nullptr;
	if (!IsValid(SessionsSubsystem))
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot change lobby character: online sessions subsystem is unavailable."));
		return;
	}

	ALobbyBeaconHostObject* LobbyHostObject = SessionsSubsystem->LobbyHostObject;
	if (HasAuthority() && IsValid(LobbyHostObject))
	{
		ALobbyGameMode* LobbyGameMode = Cast<ALobbyGameMode>(UGameplayStatics::GetGameMode(this));
		if (!IsValid(LobbyGameMode))
		{
			UE_LOG(LogTemp, Warning, TEXT("Cannot change lobby character: lobby game mode is unavailable."));
			return;
		}

		if (CharacterIndex >= LobbyGameMode->GetCharacterCount())
		{
			UE_LOG(LogTemp, Warning, TEXT("Cannot change lobby character: character index %d is out of range."), CharacterIndex);
			return;
		}

		const TArray<FLobbyPlayerData> HostPlayers = LobbyHostObject->GetHostLocalPlayers();
		const FLobbyPlayerData* PlayerData = HostPlayers.FindByPredicate([LocalPlayerIndex](const FLobbyPlayerData& Data)
		{
			return Data.LocalPlayerIndex == LocalPlayerIndex;
		});

		if (!PlayerData)
		{
			FLobbyPlayerData NewPlayerData;
			NewPlayerData.LocalPlayerIndex = LocalPlayerIndex;
			NewPlayerData.SelectedCharacterIndex = CharacterIndex;
			if (PlayerState)
			{
				NewPlayerData.PlayerName = UOnlineSessionsSubsystem::GetPlayerDisplayName(PlayerState);
				const FUniqueNetIdRepl& UniqueId = PlayerState->GetUniqueId();
				if (UniqueId.GetUniqueNetId().IsValid())
				{
					NewPlayerData.PlayerUniqueId = UniqueId.GetUniqueNetId()->ToString();
				}
			}
			LobbyHostObject->UpdateHostLocalPlayerData(LocalPlayerIndex, NewPlayerData);
			return;
		}

		FLobbyPlayerData UpdatedPlayerData = *PlayerData;
		const bool bCharacterChanged = UpdatedPlayerData.SelectedCharacterIndex != CharacterIndex;
		UpdatedPlayerData.SelectedCharacterIndex = CharacterIndex;
		if (bCharacterChanged)
		{
			UpdatedPlayerData.SelectedSkinIndex = INDEX_NONE;
		}
		LobbyHostObject->UpdateHostLocalPlayerData(LocalPlayerIndex, UpdatedPlayerData);
		return;
	}

	ALobbyBeaconClient* BeaconClient = SessionsSubsystem->BeaconClient;
	if (!IsValid(BeaconClient))
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot change lobby character: lobby beacon client is unavailable."));
		return;
	}

	BeaconClient->SetLocalPlayerCharacter(LocalPlayerIndex, CharacterIndex);
}

void ALobbyPlayerController::SelectNextCharacter(const int32 LocalPlayerIndex)
{
	StepCharacterSelection(1, LocalPlayerIndex);
}

void ALobbyPlayerController::SelectPreviousCharacter(const int32 LocalPlayerIndex)
{
	StepCharacterSelection(-1, LocalPlayerIndex);
}

void ALobbyPlayerController::StepCharacterSelection(const int32 Direction, const int32 LocalPlayerIndex)
{
	if (!IsLocalController())
	{
		return;
	}

	if (LocalPlayerIndex < 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot change lobby character: local player index must be non-negative."));
		return;
	}

	UOnlineSessionsSubsystem* SessionsSubsystem = GetGameInstance()
		? GetGameInstance()->GetSubsystem<UOnlineSessionsSubsystem>()
		: nullptr;
	if (!IsValid(SessionsSubsystem))
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot change lobby character: online sessions subsystem is unavailable."));
		return;
	}

	ALobbyBeaconHostObject* LobbyHostObject = SessionsSubsystem->LobbyHostObject;
	if (HasAuthority() && IsValid(LobbyHostObject))
	{
		const ALobbyGameMode* LobbyGameMode = Cast<ALobbyGameMode>(UGameplayStatics::GetGameMode(this));
		if (!IsValid(LobbyGameMode))
		{
			UE_LOG(LogTemp, Warning, TEXT("Cannot change lobby character: lobby game mode is unavailable."));
			return;
		}

		const int32 CharacterCount = LobbyGameMode->GetCharacterCount();
		if (CharacterCount <= 0)
		{
			UE_LOG(LogTemp, Warning, TEXT("Cannot change lobby character: no characters are configured."));
			return;
		}

		const TArray<FLobbyPlayerData> HostPlayers = LobbyHostObject->GetHostLocalPlayers();
		const FLobbyPlayerData* PlayerData = HostPlayers.FindByPredicate([LocalPlayerIndex](const FLobbyPlayerData& Data)
		{
			return Data.LocalPlayerIndex == LocalPlayerIndex;
		});
		const int32 CurrentIndex = PlayerData ? PlayerData->SelectedCharacterIndex : 0;
		ChangeCharacter(WrapSelectionIndex(CurrentIndex, Direction, CharacterCount), LocalPlayerIndex);
		return;
	}

	ALobbyBeaconClient* BeaconClient = SessionsSubsystem->BeaconClient;
	if (!IsValid(BeaconClient))
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot change lobby character: lobby beacon client is unavailable."));
		return;
	}

	BeaconClient->StepLocalPlayerCharacter(LocalPlayerIndex, Direction);
}

void ALobbyPlayerController::ChangeSkin(const int32 SkinIndex, const int32 LocalPlayerIndex)
{
	if (!IsLocalController())
	{
		return;
	}

	if (SkinIndex < INDEX_NONE || LocalPlayerIndex < 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot change lobby skin: skin index must be valid and local player index non-negative."));
		return;
	}

	UOnlineSessionsSubsystem* SessionsSubsystem = GetGameInstance()
		? GetGameInstance()->GetSubsystem<UOnlineSessionsSubsystem>()
		: nullptr;
	if (!IsValid(SessionsSubsystem))
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot change lobby skin: online sessions subsystem is unavailable."));
		return;
	}

	ALobbyBeaconHostObject* LobbyHostObject = SessionsSubsystem->LobbyHostObject;
	if (HasAuthority() && IsValid(LobbyHostObject))
	{
		const ALobbyGameMode* LobbyGameMode = Cast<ALobbyGameMode>(UGameplayStatics::GetGameMode(this));
		if (!IsValid(LobbyGameMode))
		{
			UE_LOG(LogTemp, Warning, TEXT("Cannot change lobby skin: lobby game mode is unavailable."));
			return;
		}

		const TArray<FLobbyPlayerData> HostPlayers = LobbyHostObject->GetHostLocalPlayers();
		const FLobbyPlayerData* PlayerData = HostPlayers.FindByPredicate([LocalPlayerIndex](const FLobbyPlayerData& Data)
		{
			return Data.LocalPlayerIndex == LocalPlayerIndex;
		});
		if (!PlayerData || SkinIndex >= LobbyGameMode->GetSkinCount(PlayerData->SelectedCharacterIndex))
		{
			UE_LOG(LogTemp, Warning, TEXT("Cannot change lobby skin: skin index %d is unavailable for local player %d."),
				SkinIndex, LocalPlayerIndex);
			return;
		}

		FLobbyPlayerData UpdatedPlayerData = *PlayerData;
		UpdatedPlayerData.SelectedSkinIndex = SkinIndex;
		LobbyHostObject->UpdateHostLocalPlayerData(LocalPlayerIndex, UpdatedPlayerData);
		return;
	}

	ALobbyBeaconClient* BeaconClient = SessionsSubsystem->BeaconClient;
	if (!IsValid(BeaconClient))
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot change lobby skin: lobby beacon client is unavailable."));
		return;
	}

	BeaconClient->SetLocalPlayerSkin(LocalPlayerIndex, SkinIndex);
}

void ALobbyPlayerController::SelectNextSkin(const int32 LocalPlayerIndex)
{
	StepSkinSelection(1, LocalPlayerIndex);
}

void ALobbyPlayerController::SelectPreviousSkin(const int32 LocalPlayerIndex)
{
	StepSkinSelection(-1, LocalPlayerIndex);
}

void ALobbyPlayerController::StepSkinSelection(const int32 Direction, const int32 LocalPlayerIndex)
{
	if (!IsLocalController())
	{
		return;
	}

	if (LocalPlayerIndex < 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot change lobby skin: local player index must be non-negative."));
		return;
	}

	UOnlineSessionsSubsystem* SessionsSubsystem = GetGameInstance()
		? GetGameInstance()->GetSubsystem<UOnlineSessionsSubsystem>()
		: nullptr;
	if (!IsValid(SessionsSubsystem))
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot change lobby skin: online sessions subsystem is unavailable."));
		return;
	}

	ALobbyBeaconHostObject* LobbyHostObject = SessionsSubsystem->LobbyHostObject;
	if (HasAuthority() && IsValid(LobbyHostObject))
	{
		const ALobbyGameMode* LobbyGameMode = Cast<ALobbyGameMode>(UGameplayStatics::GetGameMode(this));
		if (!IsValid(LobbyGameMode))
		{
			UE_LOG(LogTemp, Warning, TEXT("Cannot change lobby skin: lobby game mode is unavailable."));
			return;
		}

		const TArray<FLobbyPlayerData> HostPlayers = LobbyHostObject->GetHostLocalPlayers();
		const FLobbyPlayerData* PlayerData = HostPlayers.FindByPredicate([LocalPlayerIndex](const FLobbyPlayerData& Data)
		{
			return Data.LocalPlayerIndex == LocalPlayerIndex;
		});
		if (!PlayerData)
		{
			UE_LOG(LogTemp, Warning, TEXT("Cannot change lobby skin: local player %d is not registered."), LocalPlayerIndex);
			return;
		}

		const int32 SkinCount = LobbyGameMode->GetSkinCount(PlayerData->SelectedCharacterIndex);
		if (SkinCount <= 0)
		{
			UE_LOG(LogTemp, Warning, TEXT("Cannot change lobby skin: no skins are configured for character %d."),
				PlayerData->SelectedCharacterIndex);
			return;
		}

		const int32 NewSkinIndex = WrapSelectionIndex(PlayerData->SelectedSkinIndex + 1, Direction, SkinCount + 1) - 1;
		ChangeSkin(NewSkinIndex, LocalPlayerIndex);
		return;
	}

	ALobbyBeaconClient* BeaconClient = SessionsSubsystem->BeaconClient;
	if (!IsValid(BeaconClient))
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot change lobby skin: lobby beacon client is unavailable."));
		return;
	}

	BeaconClient->StepLocalPlayerSkin(LocalPlayerIndex, Direction);
}
