// Copyright Epic Games, Inc. All Rights Reserved.

#include "AbilitySystem/Abilities/GASSGameplayAbility_Jump.h"

#include "AbilitySystem/Abilities/GASSGameplayAbility.h"
#include "Character/GASSCharacter.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GASSGameplayAbility_Jump)

struct FGameplayTagContainer;


UGASSGameplayAbility_Jump::UGASSGameplayAbility_Jump(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
}

bool UGASSGameplayAbility_Jump::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!ActorInfo || !ActorInfo->AvatarActor.IsValid())
	{
		return false;
	}

	const AGASSCharacter* GASSCharacter = Cast<AGASSCharacter>(ActorInfo->AvatarActor.Get());
	if (!GASSCharacter || !GASSCharacter->CanJump())
	{
		return false;
	}

	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}

	return true;
}

void UGASSGameplayAbility_Jump::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	// Stop jumping in case the ability blueprint doesn't call it.
	CharacterJumpStop();

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UGASSGameplayAbility_Jump::CharacterJumpStart()
{
	if (AGASSCharacter* GASSCharacter = GetGASSCharacterFromActorInfo())
	{
		if (GASSCharacter->IsLocallyControlled() && !GASSCharacter->bPressedJump)
		{
			GASSCharacter->UnCrouch();
			GASSCharacter->Jump();
		}
	}
}

void UGASSGameplayAbility_Jump::CharacterJumpStop()
{
	if (AGASSCharacter* GASSCharacter = GetGASSCharacterFromActorInfo())
	{
		if (GASSCharacter->IsLocallyControlled() && GASSCharacter->bPressedJump)
		{
			GASSCharacter->StopJumping();
		}
	}
}

