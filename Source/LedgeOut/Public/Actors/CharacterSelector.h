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
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Lobby")
	FLobbyPlayerData PLayerData;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Lobby")
	int32 SelectorIndex = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lobby")
	bool bIsOccupied = false;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION()
	void OnLobbyPlayersUpdated(const TArray<FLobbyPlayerData>& PlayerList);

	/** Called when a player occupies or leaves this slot. Can be overridden in Blueprints. */
	UFUNCTION(BlueprintNativeEvent, Category = "Lobby")
	void OnSlotOccupiedStateChanged(bool bOccupied, const FLobbyPlayerData& OccupyingPlayerData);
	virtual void OnSlotOccupiedStateChanged_Implementation(bool bOccupied, const FLobbyPlayerData& OccupyingPlayerData);

public:
	UFUNCTION(BlueprintCallable)
	void BindPlayerState(ALedgeOutPlayerState* PS);
	
	UFUNCTION(BlueprintCallable)
	void ChangeCharacterMesh(USkeletalMesh* NewCharacterMesh);
};
