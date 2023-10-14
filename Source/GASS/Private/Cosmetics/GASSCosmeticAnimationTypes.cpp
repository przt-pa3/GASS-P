// Copyright Epic Games, Inc. All Rights Reserved.

#include "Cosmetics/GASSCosmeticAnimationTypes.h"

#include "Animation/AnimInstance.h"
#include "Engine/SkeletalMesh.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GASSCosmeticAnimationTypes)

TSubclassOf<UAnimInstance> FGASSAnimLayerSelectionSet::SelectBestLayer(const FGameplayTagContainer& CosmeticTags) const
{
	for (const FGASSAnimLayerSelectionEntry& Rule : LayerRules)
	{
		if ((Rule.Layer != nullptr) && CosmeticTags.HasAll(Rule.RequiredTags))
		{
			return Rule.Layer;
		}
	}

	return DefaultLayer;
}

USkeletalMesh* FGASSAnimBodyStyleSelectionSet::SelectBestBodyStyle(const FGameplayTagContainer& CosmeticTags) const
{
	for (const FGASSAnimBodyStyleSelectionEntry& Rule : MeshRules)
	{
		if ((Rule.Mesh != nullptr) && CosmeticTags.HasAll(Rule.RequiredTags))
		{
			return Rule.Mesh;
		}
	}

	return DefaultMesh;
}

