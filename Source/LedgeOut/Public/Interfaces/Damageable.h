#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "DamageTypes.h"
#include "Damageable.generated.h"

// This class does not need to be modified.
UINTERFACE()
class UDamageable : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class LEDGEOUT_API IDamageable
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void GetDamage(FDamageData DamageData);
};
