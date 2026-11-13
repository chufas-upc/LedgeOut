#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Framework/LobbyPlayerData.h"
#include "LedgeOutGameInstance.generated.h"

/**
 * Persistent GameInstance that carries lobby selections into the game level.
 * Assign this class in Project Settings → Maps & Modes → Game Instance Class.
 */
UCLASS()
class LEDGEOUT_API ULedgeOutGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	/**
	 * Final player selections written by the host just before server travel.
	 * Available on every machine (host writes it, clients receive it via travel URL or
	 * you can replicate it through the game state after loading the game map).
	 */
	UPROPERTY(BlueprintReadWrite, Category = "Lobby")
	TArray<FLobbyPlayerData> FinalLobbySelections;

	/** Convenience: clear stored selections (call before a new lobby session) */
	UFUNCTION(BlueprintCallable, Category = "Lobby")
	void ClearLobbySelections() { FinalLobbySelections.Empty(); }
};
