// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "System/GameplayTagStack.h"
#include "Templates/SubclassOf.h"

#include "GASSInventoryItemInstance.generated.h"

class FLifetimeProperty;

class UGASSItemDefinition;
class UGASSItemFragment;
struct FFrame;
struct FGameplayTag;

/**
 * UGASSInventoryItemInstance
 */
UCLASS(BlueprintType)
class UGASSInventoryItemInstance : public UObject
{
	GENERATED_BODY()

public:
	UGASSInventoryItemInstance(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	//~UObject interface
	virtual bool IsSupportedForNetworking() const override { return true; } 
	//~End of UObject interface

	TSubclassOf<UGASSItemDefinition> GetItemDef() const
	{
		return ItemDef;
	}

	UFUNCTION(BlueprintCallable, BlueprintPure=false, meta=(DeterminesOutputType=FragmentClass))
	const UGASSItemFragment* FindFragmentByClass(TSubclassOf<UGASSItemFragment> FragmentClass) const;

	template <typename ResultClass>
	const ResultClass* FindFragmentByClass() const
	{
		return (ResultClass*)FindFragmentByClass(ResultClass::StaticClass());
	}

	UFUNCTION(BlueprintCallable, Category = Inventory)
	void StackItem(uint8 StackAmount);

	UFUNCTION()
	void UnStackItem(uint8 UnStackAmount);

	UFUNCTION()
	uint8 GetCurrStackCount();

	UFUNCTION()
	void StackInstance(UGASSInventoryItemInstance* InInstance, uint8 StackAmount);

private:
#if UE_WITH_IRIS
	/** Register all replication fragments */
	virtual void RegisterReplicationFragments(UE::Net::FFragmentRegistrationContext& Context, UE::Net::EFragmentRegistrationFlags RegistrationFlags) override;
#endif // UE_WITH_IRIS

	friend struct FGASSInventoryList;

	void SetItemDef(TSubclassOf<UGASSItemDefinition> InDef);

	void InitInventoryItemInstance(uint8 StackAmount);

public:

	UPROPERTY(Replicated)
	uint8 ItemSizeLonger = 1;

	UPROPERTY(Replicated)
	uint8 ItemSizeShorter = 1;

	UPROPERTY(Replicated)
	float IndItemWeight = 0;

	UPROPERTY(Replicated)
	FGameplayTag ItemTag;

	UPROPERTY(Replicated)
	bool bCanItemStack = false;
	
	UPROPERTY(Replicated)
	uint8 MaxItemStackCount = 1;

private:
	// The item definition
	UPROPERTY(Replicated)
	TSubclassOf<UGASSItemDefinition> ItemDef;

	UPROPERTY(Replicated)
	uint32 CurrItemStackCount = 0;

	UPROPERTY(Replicated)
	float TotalItemWeight = 0;
};
