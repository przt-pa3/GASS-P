// Copyright Epic Games, Inc. All Rights Reserved.

#include "Inventory/GASSInventoryItemInstance.h"

#include "Item/GASSItemDefinition.h"
#include "Net/UnrealNetwork.h"

#if UE_WITH_IRIS
#include "Iris/ReplicationSystem/ReplicationFragmentUtil.h"
#endif // UE_WITH_IRIS

#include UE_INLINE_GENERATED_CPP_BY_NAME(GASSInventoryItemInstance)

class FLifetimeProperty;

UGASSInventoryItemInstance::UGASSInventoryItemInstance(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	UE_LOG(LogTemp, Warning, TEXT("ItemInstance Const"));
}

void UGASSInventoryItemInstance::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, ItemDef);
	DOREPLIFETIME(ThisClass, ItemSizeLonger);
	DOREPLIFETIME(ThisClass, ItemSizeShorter);
	DOREPLIFETIME(ThisClass, IndItemWeight);
	DOREPLIFETIME(ThisClass, ItemTag);
	DOREPLIFETIME(ThisClass, bCanItemStack);
	DOREPLIFETIME(ThisClass, MaxItemStackCount);
	DOREPLIFETIME(ThisClass, CurrItemStackCount);
	DOREPLIFETIME(ThisClass, TotalItemWeight);
}

#if UE_WITH_IRIS
void UGASSInventoryItemInstance::RegisterReplicationFragments(UE::Net::FFragmentRegistrationContext& Context, UE::Net::EFragmentRegistrationFlags RegistrationFlags)
{
	using namespace UE::Net;

	Super::RegisterReplicationFragments(Context, RegistrationFlags);

	// Build descriptors and allocate PropertyReplicationFragments for this object
	FReplicationFragmentUtil::CreateAndRegisterFragmentsForObject(this, Context, RegistrationFlags);
}
#endif // UE_WITH_IRIS

void UGASSInventoryItemInstance::SetItemDef(TSubclassOf<UGASSItemDefinition> InDef)
{
	ItemDef = InDef;
}

void UGASSInventoryItemInstance::InitInventoryItemInstance(uint8 StackAmount)
{
	if (ItemDef == nullptr) 
	{
		// Check Error
		return;
	}

	const UGASSItemDefinition* ItemCDO = GetDefault<UGASSItemDefinition>(ItemDef);

	bCanItemStack = ItemCDO->bCanStack;
	IndItemWeight = ItemCDO->ItemWeight;
	MaxItemStackCount = ItemCDO->MaxStackCount;
	ItemSizeLonger = ItemCDO->ItemSizeLonger;
	ItemSizeShorter = ItemCDO->ItemSizeShorter;
	ItemTag = ItemCDO->ItemTag;

	CurrItemStackCount = StackAmount;
	TotalItemWeight = StackAmount * IndItemWeight;
}

const UGASSItemFragment* UGASSInventoryItemInstance::FindFragmentByClass(TSubclassOf<UGASSItemFragment> FragmentClass) const
{
	if ((ItemDef != nullptr) && (FragmentClass != nullptr))
	{
		return GetDefault<UGASSItemDefinition>(ItemDef)->FindFragmentByClass(FragmentClass);
	}

	return nullptr;
}

void UGASSInventoryItemInstance::StackItem(uint8 StackAmount)
{
	CurrItemStackCount += StackAmount;
	float deltaWeight = StackAmount * IndItemWeight;
	TotalItemWeight += deltaWeight;
}

void UGASSInventoryItemInstance::UnStackItem(uint8 UnStackAmount)
{
	CurrItemStackCount -= UnStackAmount;
	float deltaWeight = UnStackAmount * IndItemWeight;
	TotalItemWeight -= deltaWeight;
}

uint8 UGASSInventoryItemInstance::GetCurrStackCount()
{
	return CurrItemStackCount;
}

void UGASSInventoryItemInstance::StackInstance(UGASSInventoryItemInstance* InInstance, uint8 StackAmount)
{
	this->CurrItemStackCount += StackAmount;
	this->TotalItemWeight += StackAmount * IndItemWeight;
}
