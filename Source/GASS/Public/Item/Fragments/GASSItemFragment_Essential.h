// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Item/GASSItemDefinition.h"
#include "GASSItemFragment_Essential.generated.h"

/**
 * 
 */
UCLASS()
class GASS_API UGASSItemFragment_Essential : public UGASSItemFragment
{
	GENERATED_BODY()
	
public:
	UGASSItemFragment_Essential();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Appearance)
	TObjectPtr<USkeletalMesh> PickupSkeletalMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Appearance)
	TObjectPtr<UMaterialInstance> InventoryIconRotTrue;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Appearance)
	TObjectPtr<UMaterialInstance> InventoryIconRotFalse;
};
