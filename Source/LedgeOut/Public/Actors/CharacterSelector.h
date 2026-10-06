#pragma once

#include "CoreMinimal.h"
#include "Framework/LobbyPlayerData.h"
#include "GameFramework/Actor.h"
#include "CharacterSelector.generated.h"

struct FLobbyPlayerData;
class ALedgeOutPlayerState;
class ALobbyPlayerController;

UCLASS()
class LEDGEOUT_API ACharacterSelector : public AActor
{
	GENERATED_BODY()
	
	/* Mesh for the character to preview */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USkeletalMeshComponent> CharacterMesh;
	
	/* Mesh where the character stands */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStaticMeshComponent> StandMesh;
	
	/* Array of Materials to set per each player */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TArray<TObjectPtr<UMaterialInterface>> MaterialPerPlayer;

public:
	ACharacterSelector();
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	FLobbyPlayerData PLayerData;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly)
	int32 SelectorIndex = 0;

protected:
	virtual void BeginPlay() override;

public:
	UFUNCTION(BlueprintCallable)
	void BindPlayerState(ALedgeOutPlayerState* PS);
	
	UFUNCTION(BlueprintCallable)
	void ChangeCharacterMesh(USkeletalMesh* NewCharacterMesh);
};
