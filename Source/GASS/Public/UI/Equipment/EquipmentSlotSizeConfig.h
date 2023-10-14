// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "EquipmentSlotSizeConfig.generated.h"

USTRUCT(BlueprintType)
struct FEquipmentSlotSize
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTag EquipmentItemTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	int32 SlotRowSize;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	int32 SlotColSize;
};

/**
 * 
 */
UCLASS()
class GASS_API UEquipmentSlotSizeConfig : public UDataAsset
{
	GENERATED_BODY()
	
public:
	UEquipmentSlotSizeConfig(const FObjectInitializer& ObjectInitializer);

	UFUNCTION()
	FVector2D GetSlotSizeByEquipmentIndex(int32 EquipmentIndex);

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Meta = (TitleProperty = "EquipmentSlotSize"))
	TArray<FEquipmentSlotSize> EquipmentSlotSizes;
};
