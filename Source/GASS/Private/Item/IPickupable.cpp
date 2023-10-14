// Copyright Epic Games, Inc. All Rights Reserved.

#include "Item/IPickupable.h"

#include "GameFramework/Actor.h"
#include "Inventory/GASSInventoryManagerComponent.h"
#include "Inventory/GASSInventoryItemInstance.h"
#include "UObject/ScriptInterface.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(IPickupable)

class UActorComponent;

UPickupableStatics::UPickupableStatics()
	: Super(FObjectInitializer::Get())
{
}

TScriptInterface<IPickupable> UPickupableStatics::GetFirstPickupableFromActor(AActor* Actor)
{
	// If the actor is directly pickupable, return that.
	TScriptInterface<IPickupable> PickupableActor(Actor);
	if (PickupableActor)
	{
		return PickupableActor;
	}

	// If the actor isn't pickupable, it might have a component that has a pickupable interface.
	TArray<UActorComponent*> PickupableComponents = Actor ? Actor->GetComponentsByInterface(UPickupable::StaticClass()) : TArray<UActorComponent*>();
	if (PickupableComponents.Num() > 0)
	{
		// Get first pickupable, if the user needs more sophisticated pickup distinction, will need to be solved elsewhere.
		return TScriptInterface<IPickupable>(PickupableComponents[0]);
	}

	return TScriptInterface<IPickupable>();
}

void UPickupableStatics::AddPickupToInventory(UGASSInventoryManagerComponent* InventoryComponent, TScriptInterface<IPickupable> Pickup, bool& IsCompletelyAdded, bool& ShouldSendCue)
{
	UE_LOG(LogTemp, Warning, TEXT("AddPickupToInventory Called"));
	IsCompletelyAdded = true;
	ShouldSendCue = false;
	if (InventoryComponent && Pickup)
	{
		FInventoryPickup& PickupInventory = Pickup->GetPickupInventory();
		
		for (FPickupTemplate& Template : PickupInventory.Templates)
		{
			int32 PickupAmount = Template.StackCount;
			uint8 AddedAmount = InventoryComponent->AutoAddItemDefinition(Template.ItemDef, Template.StackCount);
			
			Template.StackCount -= AddedAmount;

			if (AddedAmount != PickupAmount)
			{
				IsCompletelyAdded = false;
			}
			if (AddedAmount > 0)
			{
				ShouldSendCue = true;
			}
		}

		for (FPickupInstance& Instance : PickupInventory.Instances)
		{
			int32 PickupAmount = Instance.Item->GetCurrStackCount();
			uint8 AddedAmount = InventoryComponent->AutoAddItemDefinition(Instance.Item->GetItemDef(), Instance.Item->GetCurrStackCount());

			Instance.Item->UnStackItem(AddedAmount);

			if (AddedAmount == PickupAmount)
			{
				IsCompletelyAdded = false;
			}
			if (AddedAmount > 0)
			{
				ShouldSendCue = true;
			}
		}
	}
	else
	{
		if (!InventoryComponent)
		{
			UE_LOG(LogTemp, Error, TEXT("IMC Not Found"));
		}
		if (!Pickup)
		{
			UE_LOG(LogTemp, Error, TEXT("Pickup Not Found"));
		}

		IsCompletelyAdded = false;
		ShouldSendCue = false;
		return;
	}
}
