// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "GameplayTagContainer.h"
#include "Equipment/GASSEquipmentDefinition.h"
#include "GASSItemDefinition.generated.h"

template <typename T> class TSubclassOf;

class UGASSEquipmentDefinition;
class UGASSInventoryItemInstance;
struct FFrame;


//////////////////////////////////////////////////////////////////////

// Represents a fragment of an item definition
UCLASS(DefaultToInstanced, EditInlineNew, Abstract)
class GASS_API UGASSItemFragment : public UObject
{
	GENERATED_BODY()

public:
	virtual void OnInstanceCreated(UGASSInventoryItemInstance* Instance) const {}
};

//////////////////////////////////////////////////////////////////////

/**
 * UGASSItemDefinition
 */
UCLASS(Blueprintable, Const, Abstract)
class UGASSItemDefinition : public UObject
{
	GENERATED_BODY()

public:
	UGASSItemDefinition(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Item)
	FGameplayTag ItemTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Item)
	float ItemWeight = 0.0;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Item)
	bool bCanStack = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Item)
	uint8 MaxStackCount = 1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Item)
	uint8 ItemSizeLonger = 1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Item)
	uint8 ItemSizeShorter = 1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=Display)
	FText DisplayName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Display, Instanced)
	TObjectPtr<UGASSItemFragment> EssentialFragment;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=Display, Instanced)
	TArray<TObjectPtr<UGASSItemFragment>> Fragments;
	
public:
	const UGASSItemFragment* FindFragmentByClass(TSubclassOf<UGASSItemFragment> FragmentClass) const;
	TSubclassOf<UGASSEquipmentDefinition> GetEquipmentDefinition() const;
};

//@TODO: Make into a subsystem instead?
UCLASS()
class UGASSItemFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

	UFUNCTION(BlueprintCallable, meta=(DeterminesOutputType=FragmentClass))
	static const UGASSItemFragment* FindItemDefinitionFragment(TSubclassOf<UGASSItemDefinition> ItemDef, TSubclassOf<UGASSItemFragment> FragmentClass);
};
