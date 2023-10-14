// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/GASSAbilitySystemGlobals.h"
#include "AbilitySystem/GASSGameplayEffectContext.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GASSAbilitySystemGlobals)

struct FGameplayEffectContext;

UGASSAbilitySystemGlobals::UGASSAbilitySystemGlobals(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

FGameplayEffectContext* UGASSAbilitySystemGlobals::AllocGameplayEffectContext() const
{
	return nullptr;
}
