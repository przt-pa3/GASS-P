// Copyright Epic Games, Inc. All Rights Reserved.

#include "Inventory/IStorage.h"

#include "GameFramework/Actor.h"
#include "Inventory/GASSInventoryManagerComponent.h"
#include "Inventory/GASSInventoryItemInstance.h"
#include "Item/GASSItemDefinition.h"
#include "UObject/ScriptInterface.h"
#include "Math/NumericLimits.h"

#include "/Program Files/Epic Games/UE_5.2/Engine/Source/Runtime/Engine/Private/Net/NetSubObjectRegistryGetter.h"
//#include "/UnrealEngine5.2.0/Engine/Source/Runtime/Engine/Private/Net/NetSubObjectRegistryGetter.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(IStorage)

class UActorComponent;

UStorageStatics::UStorageStatics()
	: Super(FObjectInitializer::Get())
{
	
}

UGASSInventoryManagerComponent* UStorageStatics::GetIMCFromStorageActor(AActor* Actor)
{
	TScriptInterface<IStorage> StorageActor(Actor);

	if (StorageActor)
	{
		return StorageActor->GetIMC();
	}
	
	TArray<UActorComponent*> StorageComponents = Actor ? Actor->GetComponentsByInterface(UStorage::StaticClass()) : TArray<UActorComponent*>();

	if (StorageComponents.Num() > 0)
	{
		// Get first pickupable, if the user needs more sophisticated pickup distinction, will need to be solved elsewhere.
		return TScriptInterface<IStorage>(StorageComponents[0])->GetIMC();
	}
	
	return nullptr;
}

void UStorageStatics::InitStorageIMC(FStorageTemplate& StorageTemplate, AActor* OwnerActor)
{
	if (!(OwnerActor->HasAuthority()))
	{
		return;
	}

	check(StorageTemplate.StorageIMC);

	StorageTemplate.StorageIMC->InitInventoryActorInfo(OwnerActor, OwnerActor);
	StorageTemplate.StorageIMC->InitializeInventory(TNumericLimits<float>::Max(), StorageTemplate.StorageMaxCol, StorageTemplate.StorageMaxRow);

	for (const FStorageItem& StorageItem : StorageTemplate.StorageItems)
	{
		const UGASSItemDefinition* ItemCDO = GetDefault<UGASSItemDefinition>(StorageItem.ItemDef);
		int32 MaxStackCount = ItemCDO->MaxStackCount;
		int32 NeedToStackAmount = StorageItem.StackCount;

		while (NeedToStackAmount > 0)
		{
			int32 StackingAmount;

			if (ItemCDO->bCanStack)
			{
				StackingAmount = (NeedToStackAmount >= MaxStackCount) ? MaxStackCount : NeedToStackAmount;
			}
			else
			{
				StackingAmount = 1;
			}
			NeedToStackAmount -= StackingAmount;
			StorageTemplate.StorageIMC->AutoAddItemDefinition(StorageItem.ItemDef, StackingAmount);
		}
	}
}

const UGASSInventoryItemInstance* UStorageStatics::GetInventoryInstance(AActor* StorageActor, TSubclassOf<UGASSItemDefinition> InItemDef)
{
	UGASSInventoryManagerComponent* TargetIMC = GetIMCFromStorageActor(StorageActor);
	if (!TargetIMC)
	{
		return nullptr;
	}

	const UE::Net::FReplicatedComponentInfo* ComponentInfo = UE::Net::FSubObjectRegistryGetter::GetReplicatedComponentInfoForComponent(StorageActor, TargetIMC);
	const TArray<UE::Net::FSubObjectRegistry::FEntry>& Entries = (ComponentInfo->SubObjects.GetRegistryList());

	for (const UE::Net::FSubObjectRegistry::FEntry& Entry : Entries)
	{
		const UObject* SubObject = Entry.GetSubObject();
		const UGASSInventoryItemInstance* ItemInstance = Cast<UGASSInventoryItemInstance>(SubObject);
		if (ItemInstance != nullptr)
		{
			if (ItemInstance->GetItemDef() == InItemDef)
			{
				return ItemInstance;
			}
		}
	}

	return nullptr;
}
