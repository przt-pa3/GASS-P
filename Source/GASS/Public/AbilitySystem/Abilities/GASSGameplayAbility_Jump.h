// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "AbilitySystem/Abilities/GASSGameplayAbility.h"

#include "GASSGameplayAbility_Jump.generated.h"

class UObject;
struct FFrame;
struct FGameplayAbilityActorInfo;
struct FGameplayTagContainer;


/**
 * UGASSGameplayAbility_Jump
 *
 *	Gameplay ability used for character jumping.
 */
UCLASS(Abstract)
class UGASSGameplayAbility_Jump : public UGASSGameplayAbility
{
	GENERATED_BODY()

public:

	UGASSGameplayAbility_Jump(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:

	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

	UFUNCTION(BlueprintCallable, Category = "GASS|Ability")
	void CharacterJumpStart();

	UFUNCTION(BlueprintCallable, Category = "GASS|Ability")
	void CharacterJumpStop();
};
