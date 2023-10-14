// Fill out your copyright notice in the Description page of Project Settings.

#include "AbilitySystem/Attributes/GASSAttributeSet.h"

#include "AbilitySystem/GASSAbilitySystemComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GASSAttributeSet)

class UWorld;

UGASSAttributeSet::UGASSAttributeSet()
{
}

UWorld* UGASSAttributeSet::GetWorld() const
{
	const UObject* Outer = GetOuter();
	check(Outer);

	return Outer->GetWorld();
}

UGASSAbilitySystemComponent* UGASSAttributeSet::GetGASSAbilitySystemComponent() const
{
	return Cast<UGASSAbilitySystemComponent>(GetOwningAbilitySystemComponent());
}

