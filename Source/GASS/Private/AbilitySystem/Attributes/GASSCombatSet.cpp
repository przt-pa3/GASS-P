// Copyright Epic Games, Inc. All Rights Reserved.

#include "AbilitySystem/Attributes/GASSCombatSet.h"

#include "AbilitySystem/Attributes/GASSAttributeSet.h"
#include "Net/UnrealNetwork.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GASSCombatSet)

class FLifetimeProperty;


UGASSCombatSet::UGASSCombatSet()
	: BaseDamage(0.0f)
	, BaseHeal(0.0f)
{
}

void UGASSCombatSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UGASSCombatSet, BaseDamage, COND_OwnerOnly, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UGASSCombatSet, BaseHeal, COND_OwnerOnly, REPNOTIFY_Always);
}

void UGASSCombatSet::OnRep_BaseDamage(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UGASSCombatSet, BaseDamage, OldValue);
}

void UGASSCombatSet::OnRep_BaseHeal(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UGASSCombatSet, BaseHeal, OldValue);
}

