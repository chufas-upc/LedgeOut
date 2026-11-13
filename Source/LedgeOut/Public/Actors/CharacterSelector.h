#pragma once

#include "CoreMinimal.h"
#include "Framework/LobbyPlayerData.h"
#include "GameFramework/Actor.h"
#include "Engine/StreamableManager.h"
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

	/** Async-load the skeletal mesh supplied by the lobby host */
	void AsyncLoadCharacterMesh(const FString& CharacterMeshPath);

	/** Called when the async load completes */
	void OnCharacterMeshLoaded(TSoftObjectPtr<USkeletalMesh> SoftMesh);

	/** Cancel any in-flight streaming request */
	void CancelPendingMeshLoad();

private:
	/** Handle to cancel in-flight async load requests */
	TSharedPtr<FStreamableHandle> StreamableHandle;

	/** Index that was last requested to load (to discard stale callbacks) */
	int32 LastRequestedCharacterIndex = INDEX_NONE;

	FString LastRequestedMeshPath;

public:
	UFUNCTION(BlueprintCallable)
	void BindPlayerState(ALedgeOutPlayerState* PS);
	
	UFUNCTION(BlueprintCallable)
	void ChangeCharacterMesh(USkeletalMesh* NewCharacterMesh);
};
