// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "LedgeOutPlayerController.generated.h"

class UGameWidget;
class UInputMappingContext;
class UUserWidget;

/**
 *  Basic PlayerController class for a third person game
 *  Manages input mappings
 */
UCLASS(abstract)
class ALedgeOutPlayerController : public APlayerController
{
	GENERATED_BODY()
	
protected:

	/** Input Mapping Contexts */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category ="Input|Input Mappings")
	TArray<UInputMappingContext*> DefaultMappingContexts;

	/** Gameplay initialization */
	virtual void BeginPlay() override;

	/** Input mapping context setup */
	virtual void SetupInputComponent() override;

public:
	virtual void AcknowledgePossession(APawn* P) override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category ="Widgets")
	TSubclassOf<UGameWidget> GameWidgetClass;
	
	UPROPERTY(BlueprintReadOnly, Category ="Widgets")
	TObjectPtr<UGameWidget> GameWidget;
};
