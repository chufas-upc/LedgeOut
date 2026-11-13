#include "Framework/LedgeOutPlayerState.h"
#include "Net/UnrealNetwork.h"

void ALedgeOutPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ALedgeOutPlayerState, Lives);
	DOREPLIFETIME(ALedgeOutPlayerState, bIsDead);
}

void ALedgeOutPlayerState::HandleDead()
{
	if (!HasAuthority() || bIsDead)
	{
		return;
	}

	bIsDead = true;
	Lives = FMath::Max(0, Lives - 1);
	OnLivesChanged.Broadcast(Lives);
	ForceNetUpdate();
}

void ALedgeOutPlayerState::OnRep_UpdateLives()
{
	OnLivesChanged.Broadcast(Lives);
}
