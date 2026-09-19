#include "Combat/TestDummy.h"
#include "DamageTypes.h"
#include "DrawDebugHelpers.h"
#include "Components/BoxComponent.h"

ATestDummy::ATestDummy()
{
	PrimaryActorTick.bCanEverTick = false;
	
	Collision = CreateDefaultSubobject<UBoxComponent>(TEXT("Collision"));
	Collision->SetHiddenInGame(false);
}

void ATestDummy::BeginPlay()
{
	Super::BeginPlay();
	
}

void ATestDummy::GetDamage_Implementation(FDamageData DamageData)
{
	UE_LOG(LogTemp, Log, TEXT("Damage: %f"), DamageData.Amount);

	const FVector StartLine = GetActorLocation();
	const FVector EndLine = GetActorLocation() + DamageData.KnockbackVector;
	
	DrawDebugLine(GetWorld(), StartLine, EndLine, FColor::Green, true, 10.0f);
}
