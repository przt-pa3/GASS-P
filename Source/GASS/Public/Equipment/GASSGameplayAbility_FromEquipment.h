// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "AbilitySystem/Abilities/GASSGameplayAbility.h"

#include "GASSGameplayAbility_FromEquipment.generated.h"

class UGASSEquipmentInstance;
class UGASSInventoryItemInstance;

/**
 * UGASSGameplayAbility_FromEquipment
 *
 * An ability granted by and associated with an equipment instance
 */
UCLASS()
class UGASSGameplayAbility_FromEquipment : public UGASSGameplayAbility
{
	GENERATED_BODY()

public:

	UGASSGameplayAbility_FromEquipment(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	UFUNCTION(BlueprintCallable, Category="GASS|Ability")
	UGASSEquipmentInstance* GetAssociatedEquipment() const;

	UFUNCTION(BlueprintCallable, Category = "GASS|Ability")
	UGASSInventoryItemInstance* GetAssociatedItem() const;

#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(TArray<FText>& ValidationErrors) override;
#endif

};
