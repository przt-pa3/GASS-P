// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "GASSAttributeSet.generated.h"

class AActor;
class UGASSAbilitySystemComponent;
class UObject;
class UWorld;

struct FGameplayEffectSpec;

/**
 * This macro defines a set of helper functions for accessing and initializing attributes.
 *
 * The following example of the macro:
 *		ATTRIBUTE_ACCESSORS(UGASSHealthSet, Health)
 * will create the following functions:
 *		static FGameplayAttribute GetHealthAttribute();
 *		float GetHealth() const;
 *		void SetHealth(float NewVal);
 *		void InitHealth(float NewVal);
 */
#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

// Delegate used to broadcast attribute events.
DECLARE_MULTICAST_DELEGATE_FourParams(FGASSAttributeEvent, AActor* /*EffectInstigator*/, AActor* /*EffectCauser*/, const FGameplayEffectSpec& /*EffectSpec*/, float /*EffectMagnitude*/);


/**
 * UGASSAttributeSet
 *
 *	Base attribute set class for the project.
 */
UCLASS()
class GASS_API UGASSAttributeSet : public UAttributeSet
{
	GENERATED_BODY()
	
public:

	UGASSAttributeSet();

	UWorld* GetWorld() const override;

	UGASSAbilitySystemComponent* GetGASSAbilitySystemComponent() const;
};
