#include "Actors/CharacterSelector.h"
#include "Components/SceneComponent.h"
#include "Framework/LedgeOutPlayerState.h"
#include "Framework/Lobby/LobbyGameMode.h"
#include "Framework/Lobby/LobbyPlayerController.h"
#include "Framework/OnlineSessionsSubsystem.h"
#include "Kismet/GameplayStatics.h"

ACharacterSelector::ACharacterSelector()
{
	PrimaryActorTick.bCanEverTick = false;
	
	// Root Component
	USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
	SetRootComponent(Root);

	// Components
	CharacterMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Character Mesh"));
	CharacterMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	CharacterMesh->SetupAttachment(RootComponent);
	
	StandMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Stand"));
	StandMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	StandMesh->SetupAttachment(RootComponent);
}

void ACharacterSelector::BeginPlay()
{
	Super::BeginPlay();
	
	// Subscribe to OnlineSessionsSubsystem lobby updates (runs on both Host & Client)
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UOnlineSessionsSubsystem* SessionsSubsystem = GI->GetSubsystem<UOnlineSessionsSubsystem>())
		{
			SessionsSubsystem->OnLobbyPlayersUpdated.AddDynamic(this, &ACharacterSelector::OnLobbyPlayersUpdated);
			// Process initial state if players already exist
			OnLobbyPlayersUpdated(SessionsSubsystem->GetCurrentLobbyPlayers());
		}
	}

	if (HasAuthority())
	{
		ALobbyGameMode* GM = Cast<ALobbyGameMode>(UGameplayStatics::GetGameMode(this));
		if (IsValid(GM))
		{
			GM->OnPlayerConnected.AddDynamic(this, &ACharacterSelector::BindPlayerState);
		}
	}
}

void ACharacterSelector::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UOnlineSessionsSubsystem* SessionsSubsystem = GI->GetSubsystem<UOnlineSessionsSubsystem>())
		{
			SessionsSubsystem->OnLobbyPlayersUpdated.RemoveAll(this);
		}
	}

	Super::EndPlay(EndPlayReason);
}

void ACharacterSelector::OnLobbyPlayersUpdated(const TArray<FLobbyPlayerData>& PlayerList)
{
	bool bFoundPlayer = false;
	FLobbyPlayerData FoundPlayerData;

	if (PlayerList.IsValidIndex(SelectorIndex))
	{
		bFoundPlayer = true;
		FoundPlayerData = PlayerList[SelectorIndex];
	}

	if (bIsOccupied != bFoundPlayer || (bFoundPlayer && PLayerData.SelectedCharacterIndex != FoundPlayerData.SelectedCharacterIndex))
	{
		bIsOccupied = bFoundPlayer;
		PLayerData = FoundPlayerData;
		OnSlotOccupiedStateChanged(bIsOccupied, PLayerData);
	}
}

void ACharacterSelector::OnSlotOccupiedStateChanged_Implementation(bool bOccupied, const FLobbyPlayerData& OccupyingPlayerData)
{
	if (bOccupied)
	{
		if (MaterialPerPlayer.IsValidIndex(SelectorIndex) && StandMesh)
		{
			StandMesh->SetMaterial(0, MaterialPerPlayer[SelectorIndex]);
		}
		if (CharacterMesh)
		{
			CharacterMesh->SetVisibility(true);
		}
	}
	else
	{
		if (StandMesh)
		{
			StandMesh->SetMaterial(0, nullptr);
		}
		if (CharacterMesh)
		{
			CharacterMesh->SetVisibility(false);
		}
	}
}

void ACharacterSelector::BindPlayerState(ALedgeOutPlayerState* PS)
{
	if (PS->GetPlayerId())
	{
		if (MaterialPerPlayer.IsValidIndex(SelectorIndex))
			StandMesh->SetMaterial(0, MaterialPerPlayer[SelectorIndex]);
	}
}

void ACharacterSelector::ChangeCharacterMesh(USkeletalMesh* NewCharacterMesh)
{
	CharacterMesh->SetSkeletalMesh(NewCharacterMesh);
}
