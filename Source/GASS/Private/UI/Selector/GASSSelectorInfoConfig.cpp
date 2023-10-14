// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Selector/GASSSelectorInfoConfig.h"

UGASSSelectorInfoConfig::UGASSSelectorInfoConfig(const FObjectInitializer& ObjectInitializer)
{
}

const int32 UGASSSelectorInfoConfig::GetOptionsIndexByOptionName(const FName& InOptionName)
{
	int32 i = 0;

	for (const FGASSSelectorOptionInfo& OptionInfo : SelectorOptionsInfo)
	{
		if (OptionInfo.OptionName == InOptionName)
		{
			return i;
		}
		i++;
	}
	
	UE_LOG(LogTemp, Error, TEXT("Can't Find Option Index for OptionName [%s] on SelectorInfoConfit [%s]"), *InOptionName.ToString(), *GetNameSafe(this));
	return -1;
}

FGASSSelectorOptionInfo UGASSSelectorInfoConfig::GetSelectorOptionInfoByIndex(const int32& InIndex)
{
	if (InIndex < 0 || InIndex >= SelectorOptionsInfo.Num())
	{
		UE_LOG(LogTemp, Error, TEXT("SelectorInfoConfig Error"));
		FGASSSelectorOptionInfo Dummy;
		return Dummy;
	}
	return SelectorOptionsInfo[InIndex];
}

const FText UGASSSelectorInfoConfig::GetDescriptionByIndex(const int32& InIndex)
{
	if (InIndex < 0 || InIndex >= SelectorOptionsInfo.Num())
	{
		UE_LOG(LogTemp, Error, TEXT("SelectorInfoConfig Error"));
		FText None = FText::FromString("None");
		return None;
	}
	return SelectorOptionsInfo[InIndex].Description;
}

const UMaterialInterface* UGASSSelectorInfoConfig::GetIconImageByIndex(const int32& InIndex)
{
	if (InIndex < 0 || InIndex >= SelectorOptionsInfo.Num())
	{
		UE_LOG(LogTemp, Error, TEXT("SelectorInfoConfig Error"));
		return nullptr;
	}
		
	return (SelectorOptionsInfo[InIndex].IconImage);
}

