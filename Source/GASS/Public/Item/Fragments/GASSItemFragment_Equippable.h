// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Item/GASSItemDefinition.h"
#include "Templates/SubclassOf.h"

#include "GASSItemFragment_Equippable.generated.h"

class UObject;
class UGASSEquipmentDefinition;

UCLASS()
class UGASSItemFragment_Equippable : public UGASSItemFragment
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category=GASS)
	TSubclassOf<UGASSEquipmentDefinition> EquipmentDefinition;
};
