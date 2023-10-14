// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayAbilitySpec.h"
#include "Abilities/GameplayAbility.h"
#include "GASSAbilityCost.generated.h"

class UGASSGameplayAbility;

/**
 *		GASSAbilityCost
 *		Base class to check if GASSGameplayAbility has (e.g., ammo ...)
 */

UCLASS(DefaultToInstanced, EditInlineNew, Abstract)
class GASS_API UGASSAbilityCost : public UObject
{
	GENERATED_BODY()
	
public:
	UGASSAbilityCost()
	{
	}

	/**
	 * Checks if Ability user can afford this cost.
	 * 
	 * return true if user can afford cost.
	 * 
	 * Failure reason tag can be added to OptionalRelevantTags
	 *	=> Queries elsewhere to determine how to provide user feedback (e.g., beep sound with no cost)
	 *
	 * Ability and ActorInfo are guaranteed to be non-null on entry, OptionalRelevantTags can be nullptr
	 */
	virtual bool CheckCost(const UGASSGameplayAbility* Ability, const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, FGameplayTagContainer* OptionalRelevantTags) const
	{
		return true;
	}

	/**
	 * Applies the ability's cost to the target
	 *
	 * Notes:
	 * - Your implementation don't need to check ShouldOnlyApplyCostOnHit(), the caller does that for you.
	 * - Ability and ActorInfo are guaranteed to be non-null on entry.
	 */
	virtual void ApplyCost(const UGASSGameplayAbility* Ability, const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo)
	{
	}

	/** If true, this cost should only be applied if this ability hits successfully */
	bool ShouldOnlyApplyCostOnHit() const { return bOnlyApplyCostOnHit; }

protected:
	/** If true, this cost should only be applied if this ability hits successfully */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Costs)
	bool bOnlyApplyCostOnHit = false;
};
