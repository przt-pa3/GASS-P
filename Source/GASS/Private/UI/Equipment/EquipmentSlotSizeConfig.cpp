// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Equipment/EquipmentSlotSizeConfig.h"

#include "GASS/GASSGameplayTags.h"

UEquipmentSlotSizeConfig::UEquipmentSlotSizeConfig(const FObjectInitializer& ObjectInitializer)
{
}

FVector2D UEquipmentSlotSizeConfig::GetSlotSizeByEquipmentIndex(int32 EquipmentIndex)
{
	FVector2D Result;

	if (EquipmentSlotSizes.IsValidIndex(EquipmentIndex))
	{
		EquipmentSlotSizes[EquipmentIndex].EquipmentItemTag;
		GASSGameplayTags::CustomEquipmentTagMap;
		const uint8* FindResult = GASSGameplayTags::CustomEquipmentTagMap.Find(EquipmentSlotSizes[EquipmentIndex].EquipmentItemTag);

		if (FindResult)
		{
			if (*FindResult == EquipmentIndex)
			{
				Result.X = EquipmentSlotSizes[EquipmentIndex].SlotRowSize;
				Result.Y = EquipmentSlotSizes[EquipmentIndex].SlotColSize;
				return Result;
			}
		}
	}

	if (EquipmentIndex >= 0 && EquipmentIndex < GASSGameplayTags::CustomEquipmentTagMap.Num())
	{
		auto Iter = GASSGameplayTags::CustomEquipmentTagMap.CreateConstIterator();
		for (int32 i = 0; i < EquipmentIndex; ++i)
		{
			++Iter;
		}

		FGameplayTag TargetTag = Iter->Key;

		for (auto SlotSize : EquipmentSlotSizes)
		{
			if (SlotSize.EquipmentItemTag == TargetTag)
			{
				Result.X = SlotSize.SlotRowSize;
				Result.Y = SlotSize.SlotColSize;
				return Result;
			}
		}
	}

	UE_LOG(LogTemp, Error, TEXT("EquipmentSlotConfig :: Invalid Equipment Index !"));

	Result.X = -1;
	Result.Y = -1;
	return Result;
}