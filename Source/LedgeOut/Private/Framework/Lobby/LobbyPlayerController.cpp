#include "Framework/Lobby/LobbyPlayerController.h"
#include "Camera/CameraActor.h"
#include "Kismet/GameplayStatics.h"

ACameraActor* ALobbyPlayerController::FindCameraByTag(const FName Tag) const
{
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsWithTag(this, Tag, FoundActors);
	if (FoundActors.Num() > 0)
	{
		return Cast<ACameraActor>(FoundActors[0]);
	}
	return nullptr;
}

void ALobbyPlayerController::BeginPlay()
{
	Super::BeginPlay();
	
	// Primary Player Only 
	if (!IsLocalController() || !IsPrimaryPlayer()) return;

	const FName CameraTag =
	(GetNetMode() == ENetMode::NM_Client || GetNetMode() == ENetMode::NM_ListenServer) ? "LobbyCamera" : "MenuCamera";
	
	ACameraActor* CameraActor = FindCameraByTag(CameraTag);
	if (!IsValid(CameraActor)) return;
	{
		SetViewTarget(CameraActor);
	}
}

void ALobbyPlayerController::SwitchToLobbyCamera()
{
	if (!IsLocalController() || !IsPrimaryPlayer()) return;
	ACameraActor* CameraActor = FindCameraByTag("LobbyCamera");
	if (!IsValid(CameraActor)) return;
	
	SetViewTargetWithBlend(CameraActor, 0.5f);
}

void ALobbyPlayerController::SwitchToMenuCamera()
{
	if (!IsLocalController() || !IsPrimaryPlayer()) return;
	ACameraActor* CameraActor = FindCameraByTag("MenuCamera");
	if (!IsValid(CameraActor)) return;
	
	SetViewTargetWithBlend((CameraActor), 1);
}
