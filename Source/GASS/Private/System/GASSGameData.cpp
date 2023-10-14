// Copyright Epic Games, Inc. All Rights Reserved.

#include "System/GASSGameData.h"
#include "System/GASSAssetManager.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GASSGameData)

UGASSGameData::UGASSGameData()
{
}

const UGASSGameData& UGASSGameData::UGASSGameData::Get()
{
	return UGASSAssetManager::Get().GetGameData();
}
