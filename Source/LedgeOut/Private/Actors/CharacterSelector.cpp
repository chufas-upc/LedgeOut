#include "Actors/CharacterSelector.h"
#include "Components/SceneComponent.h"
#include "Engine/AssetManager.h"
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
	CancelPendingMeshLoad();

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

	const bool bOccupancyChanged = (bIsOccupied != bFoundPlayer);
	const bool bCharacterChanged = bFoundPlayer && (PLayerData.SelectedCharacterIndex != FoundPlayerData.SelectedCharacterIndex);
	const bool bMeshPathChanged = bFoundPlayer && (PLayerData.SelectedCharacterMeshPath != FoundPlayerData.SelectedCharacterMeshPath);
	const bool bSkinChanged = bFoundPlayer && (PLayerData.SelectedSkinIndex != FoundPlayerData.SelectedSkinIndex);
	const bool bSkinMeshPathChanged = bFoundPlayer && (PLayerData.SelectedSkinMeshPath != FoundPlayerData.SelectedSkinMeshPath);

	if (bOccupancyChanged || bCharacterChanged || bMeshPathChanged || bSkinChanged || bSkinMeshPathChanged)
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

		const FString& MeshPath = OccupyingPlayerData.SelectedSkinMeshPath.IsEmpty()
			? OccupyingPlayerData.SelectedCharacterMeshPath
			: OccupyingPlayerData.SelectedSkinMeshPath;
		AsyncLoadCharacterMesh(MeshPath);
	}
	else
	{
		if (StandMesh)
		{
			StandMesh->SetMaterial(0, nullptr);
		}

		// Cancel any pending load and clear the mesh
		CancelPendingMeshLoad();
		if (CharacterMesh)
		{
			CharacterMesh->SetSkeletalMesh(nullptr);
			CharacterMesh->SetVisibility(false);
		}
	}
}

void ACharacterSelector::AsyncLoadCharacterMesh(const FString& CharacterMeshPath)
{
	CancelPendingMeshLoad();
	LastRequestedCharacterIndex = PLayerData.SelectedCharacterIndex;

	if (CharacterMeshPath.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("ACharacterSelector [%d]: No mesh path supplied for the selected character."), SelectorIndex);
		if (CharacterMesh)
		{
			CharacterMesh->SetSkeletalMesh(nullptr);
			CharacterMesh->SetVisibility(false);
		}
		return;
	}

	const FSoftObjectPath MeshPath(CharacterMeshPath);
	TSoftObjectPtr<USkeletalMesh> SoftMesh(MeshPath);
	LastRequestedMeshPath = MeshPath.ToString();

	// If it's already loaded in memory — use it immediately
	if (SoftMesh.IsValid())
	{
		CharacterMesh->SetSkeletalMesh(SoftMesh.Get());
		CharacterMesh->SetVisibility(true);
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("ACharacterSelector [%d]: Async loading mesh for character index %d."),
		SelectorIndex, PLayerData.SelectedCharacterIndex);

	StreamableHandle = UAssetManager::GetStreamableManager().RequestAsyncLoad(
		SoftMesh.ToSoftObjectPath(),
		FStreamableDelegate::CreateUObject(this, &ACharacterSelector::OnCharacterMeshLoaded, SoftMesh)
	);
}

void ACharacterSelector::OnCharacterMeshLoaded(TSoftObjectPtr<USkeletalMesh> SoftMesh)
{
	// Guard: ignore stale callbacks (e.g. player changed character again before load finished)
	const int32 LoadedIndex = PLayerData.SelectedCharacterIndex;
	if (LoadedIndex != LastRequestedCharacterIndex)
	{
		UE_LOG(LogTemp, Log, TEXT("ACharacterSelector [%d]: Discarding stale mesh load (expected %d, got %d)."),
			SelectorIndex, LastRequestedCharacterIndex, LoadedIndex);
		return;
	}

	if (SoftMesh.ToSoftObjectPath().ToString() != LastRequestedMeshPath)
	{
		return;
	}

	if (!bIsOccupied)
	{
		return;
	}

	if (USkeletalMesh* LoadedMesh = SoftMesh.Get())
	{
		UE_LOG(LogTemp, Log, TEXT("ACharacterSelector [%d]: Mesh loaded successfully."), SelectorIndex);
		if (CharacterMesh)
		{
			CharacterMesh->SetSkeletalMesh(LoadedMesh);
			CharacterMesh->SetVisibility(true);
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("ACharacterSelector [%d]: Mesh loaded but pointer is null."), SelectorIndex);
	}
}

void ACharacterSelector::CancelPendingMeshLoad()
{
	if (StreamableHandle.IsValid())
	{
		StreamableHandle->CancelHandle();
		StreamableHandle.Reset();
	}
	LastRequestedCharacterIndex = INDEX_NONE;
	LastRequestedMeshPath.Reset();
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
