// Copyright Epic Games, Inc. All Rights Reserved.

#include "Equipment/GASSEquipmentDefinition.h"
#include "Equipment/GASSEquipmentInstance.h"
#include "Item/GASSItemDefinition.h"
#include UE_INLINE_GENERATED_CPP_BY_NAME(GASSEquipmentDefinition)

UGASSEquipmentDefinition::UGASSEquipmentDefinition(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	InstanceType = UGASSEquipmentInstance::StaticClass();
}

