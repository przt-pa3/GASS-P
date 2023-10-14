// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameplayEffectTypes.h"

#include "GASSGameplayEffectContext.generated.h"

class AAcotr;
class FArchive;
class IGASSAbilitySourceInterface;
class UObject;
class UPhysicalMaterial;

/**
 * 
 */
USTRUCT()
struct FGASSGameplayEffectContext : public FGameplayEffectContext
{
	GENERATED_BODY()

	FGASSGameplayEffectContext()
		: FGameplayEffectContext()
	{
	}

	FGASSGameplayEffectContext(AActor* InInstigator, AActor* InEffectCauser)
		: FGameplayEffectContext(InInstigator, InEffectCauser)
	{
	}

	/** Returns the wrapped FGASSGameplayEffectContext from the handle, or nullptr if it doesn't exist or is the wrong type */
	static GASS_API FGASSGameplayEffectContext* ExtractEffectContext(struct FGameplayEffectContextHandle Handle);

	/** Sets the object used as the ability source */
	void SetAbilitySource(const IGASSAbilitySourceInterface* InObject, float InSourceLevel);

	/** Returns the ability source interface associated with the source object. Only valid on the authority. */
	const IGASSAbilitySourceInterface* GetAbilitySource() const;

	virtual FGameplayEffectContext* Duplicate() const override
	{
		FGASSGameplayEffectContext* NewContext = new FGASSGameplayEffectContext();
		*NewContext = *this;
		if (GetHitResult())
		{
			// Does a deep copy of the hit result
			NewContext->AddHitResult(*GetHitResult(), true);
		}
		return NewContext;
	}

	virtual UScriptStruct* GetScriptStruct() const override
	{
		return FGASSGameplayEffectContext::StaticStruct();
	}

	/** Overridden to serialize new fields */
	virtual bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess) override;

	/** Returns the physical material from the hit result if there is one */
	const UPhysicalMaterial* GetPhysicalMaterial() const;

public:
	/** ID to allow the identification of multiple bullets that were part of the same cartridge */
	UPROPERTY()
		int32 CartridgeID = -1;

protected:
	/** Ability Source object (should implement IGASSAbilitySourceInterface). NOT replicated currently */
	UPROPERTY()
		TWeakObjectPtr<const UObject> AbilitySourceObject;
};

template<>
struct TStructOpsTypeTraits<FGASSGameplayEffectContext> : public TStructOpsTypeTraitsBase2<FGASSGameplayEffectContext>
{
	enum
	{
		WithNetSerializer = true,
		WithCopy = true
	};
};

