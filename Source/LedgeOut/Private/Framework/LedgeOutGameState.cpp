#include "Framework/LedgeOutGameState.h"

#include "Net/UnrealNetwork.h"

void ALedgeOutGameState::SetRoundTimer(const int32 TimeSeconds, const bool bSuddenDeath)
{
	if (!HasAuthority())
	{
		return;
	}

	RoundTimeSeconds = FMath::Max(0, TimeSeconds);
	bIsSuddenDeath = bSuddenDeath;
	OnRoundTimerUpdated.Broadcast(RoundTimeSeconds, bIsSuddenDeath);
	ForceNetUpdate();
}

void ALedgeOutGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ALedgeOutGameState, RoundTimeSeconds);
	DOREPLIFETIME(ALedgeOutGameState, bIsSuddenDeath);
}

void ALedgeOutGameState::OnRep_RoundTimerUpdated()
{
	OnRoundTimerUpdated.Broadcast(RoundTimeSeconds, bIsSuddenDeath);
}
