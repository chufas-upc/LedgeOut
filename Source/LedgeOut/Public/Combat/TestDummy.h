#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interfaces/Damageable.h"
#include "TestDummy.generated.h"

class UBoxComponent;

UCLASS()
class LEDGEOUT_API ATestDummy : public AActor, public IDamageable
{
	GENERATED_BODY()
	
	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UBoxComponent> Collision;

public:
	ATestDummy();

protected:
	virtual void BeginPlay() override;

public:
	virtual void GetDamage_Implementation(FDamageData DamageData) override;
};