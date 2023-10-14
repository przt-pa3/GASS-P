// Copyright Epic Games, Inc. All Rights Reserved.

#include "Item/GASSItemDefinition.h"
#include "Equipment/GASSEquipmentDefinition.h"
#include "Item/Fragments/GASSItemFragment_Equippable.h"
#include "Templates/SubclassOf.h"
#include "UObject/ObjectPtr.h"


#include UE_INLINE_GENERATED_CPP_BY_NAME(GASSItemDefinition)

//////////////////////////////////////////////////////////////////////
// UGASSItemDefinition

UGASSItemDefinition::UGASSItemDefinition(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

const UGASSItemFragment* UGASSItemDefinition::FindFragmentByClass(TSubclassOf<UGASSItemFragment> FragmentClass) const
{
	if (FragmentClass != nullptr)
	{
		for (UGASSItemFragment* Fragment : Fragments)
		{
			if (Fragment && Fragment->IsA(FragmentClass))
			{
				return Fragment;
			}
		}
	}

	return nullptr;
}

TSubclassOf<UGASSEquipmentDefinition> UGASSItemDefinition::GetEquipmentDefinition() const
{
	for (UGASSItemFragment* Fragment : Fragments)
	{
		UGASSItemFragment_Equippable* EquipFrag = Cast<UGASSItemFragment_Equippable>(Fragment);
		if (EquipFrag)
		{
			return EquipFrag->EquipmentDefinition;
		}
	}
	return nullptr;
}

//////////////////////////////////////////////////////////////////////
// UGASSItemDefinition

const UGASSItemFragment* UGASSItemFunctionLibrary::FindItemDefinitionFragment(TSubclassOf<UGASSItemDefinition> ItemDef, TSubclassOf<UGASSItemFragment> FragmentClass)
{
	if ((ItemDef != nullptr) && (FragmentClass != nullptr))
	{
		return GetDefault<UGASSItemDefinition>(ItemDef)->FindFragmentByClass(FragmentClass);
	}
	return nullptr;
}

