// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "AbilitySystemGlobals.h"
#include "GASSAbilitySystemGlobals.generated.h"

struct FGameplayEffectContext;
class UObject;

/**
 * 
 */
UCLASS(Config = Game)
class UGASSAbilitySystemGlobals : public UAbilitySystemGlobals
{
	GENERATED_UCLASS_BODY()

	//~UAbilitySystemGlobals interface
	virtual FGameplayEffectContext* AllocGameplayEffectContext() const override;
	//~End of UAbilitySystemGlobals interface
	
};
