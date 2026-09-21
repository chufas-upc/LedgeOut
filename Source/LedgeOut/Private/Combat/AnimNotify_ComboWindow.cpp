#include "Combat/AnimNotify_ComboWindow.h"

#include "Combat/CombatComponent.h"

void UAnimNotifyState_ComboWindow::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                                          float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	if (UCombatComponent* CombatComp = MeshComp->GetOwner()->GetComponentByClass<UCombatComponent>(); IsValid(CombatComp))
	{
		CombatComp->StartComboWindow();
	}
}

void UAnimNotifyState_ComboWindow::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	if (UCombatComponent* CombatComp = MeshComp->GetOwner()->GetComponentByClass<UCombatComponent>(); IsValid(CombatComp))
	{
		CombatComp->ClearComboWindow(AttackIndex);
	}
}
