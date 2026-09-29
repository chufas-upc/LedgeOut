#include "Actors/CharacterSelector.h"
#include "Components/SceneComponent.h"
#include "Framework/LedgeOutPlayerState.h"
#include "Framework/Lobby/LobbyGameMode.h"
#include "Framework/Lobby/LobbyPlayerController.h"
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
	
	if (!HasAuthority()) return;
	
	ALobbyGameMode* GM = Cast<ALobbyGameMode>(UGameplayStatics::GetGameMode(this));
	if (IsValid(GM))
	{
		GM->OnPlayerConnected.AddDynamic(this, &ACharacterSelector::BindPlayerState);
	}
}

void ACharacterSelector::BindPlayerState(ALedgeOutPlayerState* PS)
{
	UE_LOG(LogTemp, Log, TEXT("%d"), PS->GetPlayerId());
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
