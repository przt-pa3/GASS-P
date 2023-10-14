// Copyright Epic Games, Inc. All Rights Reserved.

#include "AbilitySystem/GASSGameplayAbilityTargetData_SingleTargetHit.h"

#include "AbilitySystem/GASSGameplayEffectContext.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GASSGameplayAbilityTargetData_SingleTargetHit)

struct FGameplayEffectContextHandle;

//////////////////////////////////////////////////////////////////////

void FGASSGameplayAbilityTargetData_SingleTargetHit::AddTargetDataToContext(FGameplayEffectContextHandle& Context, bool bIncludeActorArray) const
{
	FGameplayAbilityTargetData_SingleTargetHit::AddTargetDataToContext(Context, bIncludeActorArray);

	// Add game-specific data
	if (FGASSGameplayEffectContext* TypedContext = FGASSGameplayEffectContext::ExtractEffectContext(Context))
	{
		TypedContext->CartridgeID = CartridgeID;
	}
}

bool FGASSGameplayAbilityTargetData_SingleTargetHit::NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess)
{
	FGameplayAbilityTargetData_SingleTargetHit::NetSerialize(Ar, Map, bOutSuccess);

	Ar << CartridgeID;

	return true;
}

