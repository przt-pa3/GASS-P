// Copyright Epic Games, Inc. All Rights Reserved.

#include "Equipment/GASSGameplayAbility_FromEquipment.h"
#include "Equipment/GASSEquipmentInstance.h"
#include "Inventory/GASSInventoryItemInstance.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GASSGameplayAbility_FromEquipment)

UGASSGameplayAbility_FromEquipment::UGASSGameplayAbility_FromEquipment(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UGASSEquipmentInstance* UGASSGameplayAbility_FromEquipment::GetAssociatedEquipment() const
{
	if (FGameplayAbilitySpec* Spec = UGameplayAbility::GetCurrentAbilitySpec())
	{
		return Cast<UGASSEquipmentInstance>(Spec->SourceObject.Get());
	}

	return nullptr;
}

UGASSInventoryItemInstance* UGASSGameplayAbility_FromEquipment::GetAssociatedItem() const
{
	if (UGASSEquipmentInstance* Equipment = GetAssociatedEquipment())
	{
		return Cast<UGASSInventoryItemInstance>(Equipment->GetInstigator());
	}
	return nullptr;
}


#if WITH_EDITOR
EDataValidationResult UGASSGameplayAbility_FromEquipment::IsDataValid(TArray<FText>& ValidationErrors)
{
	EDataValidationResult Result = Super::IsDataValid(ValidationErrors);

	if (InstancingPolicy == EGameplayAbilityInstancingPolicy::NonInstanced)
	{
		ValidationErrors.Add(NSLOCTEXT("GASS", "EquipmentAbilityMustBeInstanced", "Equipment ability must be instanced"));
		Result = EDataValidationResult::Invalid;
	}

	return Result;
}

#endif
