#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "LobbyPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class LEDGEOUT_API ALobbyPlayerController : public APlayerController
{
	GENERATED_BODY()

	ACameraActor* FindCameraByTag(FName Tag) const;

protected:
	virtual void BeginPlay() override;

public:
	
	UFUNCTION(BlueprintCallable)
	void SwitchToLobbyCamera();
	
	UFUNCTION(BlueprintCallable)
	void SwitchToMenuCamera();
};
