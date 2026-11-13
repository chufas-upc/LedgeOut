#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Engine/AssetManager.h"
#include "LobbyGameMode.generated.h"

class ALedgeOutPlayerState;
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerConnected, ALedgeOutPlayerState*, PS);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerDisconnected, ALedgeOutPlayerState*, PS);

/** Defines a selectable skin mesh for a character. */
USTRUCT(BlueprintType)
struct FCharacterSkinDefinition
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character")
	FString DisplayName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character")
	TSoftObjectPtr<USkeletalMesh> SkeletalMesh;
};

/** Defines a selectable character entry — name + soft reference to its skeletal mesh */
USTRUCT(BlueprintType)
struct FCharacterDefinition
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character")
	FString DisplayName;

	/** Soft ref so heavy meshes are NOT loaded until explicitly requested */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character")
	TSoftObjectPtr<USkeletalMesh> SkeletalMesh;

	/** Optional alternate skeletal meshes selectable as skins for this character. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character")
	TArray<FCharacterSkinDefinition> AvailableSkins;
};

/**
 * 
 */
UCLASS()
class LEDGEOUT_API ALobbyGameMode : public AGameModeBase
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;
	virtual void OnPostLogin(AController* NewPlayer) override;

public:
	UPROPERTY(BlueprintAssignable)
	FOnPlayerConnected OnPlayerConnected;
	UPROPERTY(BlueprintAssignable)
	FOnPlayerDisconnected OnPlayerDisconnected;
	
	virtual APlayerController* Login(UPlayer* NewPlayer, ENetRole InRemoteRole, const FString& Portal,
	const FString& Options, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage) override;

	virtual void Logout(AController* Exiting) override;

	/** Roster of characters available for selection. Assign in Blueprint defaults. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Characters")
	TArray<FCharacterDefinition> AvailableCharacters;

	UFUNCTION(BlueprintCallable, Category = "Characters")
	int32 GetCharacterCount() const { return AvailableCharacters.Num(); }

	UFUNCTION(BlueprintCallable, Category = "Characters")
	int32 GetSkinCount(int32 CharacterIndex) const;

	/** Returns the soft mesh ref for a given index, or an invalid ref if out of range */
	UFUNCTION(BlueprintCallable, Category = "Characters")
	TSoftObjectPtr<USkeletalMesh> GetCharacterMeshSoft(int32 Index) const;

	UFUNCTION(BlueprintCallable, Category = "Characters")
	TSoftObjectPtr<USkeletalMesh> GetSkinMeshSoft(int32 CharacterIndex, int32 SkinIndex) const;
};
