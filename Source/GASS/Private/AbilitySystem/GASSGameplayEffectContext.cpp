// Copyright Epic Games, Inc. All Rights Reserved.

#include "AbilitySystem/GASSGameplayEffectContext.h"
#include "AbilitySystem/GASSAbilitySourceInterface.h"
#include "Engine/HitResult.h"
#include "PhysicalMaterials/PhysicalMaterial.h"

#if UE_WITH_IRIS
#include "Iris/ReplicationState/PropertyNetSerializerInfoRegistry.h"
#include "Serialization/GameplayEffectContextNetSerializer.h"
#endif

#include UE_INLINE_GENERATED_CPP_BY_NAME(GASSGameplayEffectContext)

class FArchive;

FGASSGameplayEffectContext* FGASSGameplayEffectContext::ExtractEffectContext(struct FGameplayEffectContextHandle Handle)
{
	FGameplayEffectContext* BaseEffectContext = Handle.Get();
	if ((BaseEffectContext != nullptr) && BaseEffectContext->GetScriptStruct()->IsChildOf(FGASSGameplayEffectContext::StaticStruct()))
	{
		return (FGASSGameplayEffectContext*)BaseEffectContext;
	}

	return nullptr;
}

bool FGASSGameplayEffectContext::NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess)
{
	FGameplayEffectContext::NetSerialize(Ar, Map, bOutSuccess);

	// Not serialized for post-activation use:
	// CartridgeID

	return true;
}

#if UE_WITH_IRIS
namespace UE::Net
{
	// Forward to FGameplayEffectContextNetSerializer
	// Note: If FGASSGameplayEffectContext::NetSerialize() is modified, a custom NetSerializesr must be implemented as the current fallback will no longer be sufficient.
	UE_NET_IMPLEMENT_FORWARDING_NETSERIALIZER_AND_REGISTRY_DELEGATES(GASSGameplayEffectContext, FGameplayEffectContextNetSerializer);
}
#endif

void FGASSGameplayEffectContext::SetAbilitySource(const IGASSAbilitySourceInterface* InObject, float InSourceLevel)
{
	AbilitySourceObject = MakeWeakObjectPtr(Cast<const UObject>(InObject));
	//SourceLevel = InSourceLevel;
}

const IGASSAbilitySourceInterface* FGASSGameplayEffectContext::GetAbilitySource() const
{
	return Cast<IGASSAbilitySourceInterface>(AbilitySourceObject.Get());
}

const UPhysicalMaterial* FGASSGameplayEffectContext::GetPhysicalMaterial() const
{
	if (const FHitResult* HitResultPtr = GetHitResult())
	{
		return HitResultPtr->PhysMaterial.Get();
	}
	return nullptr;
}

