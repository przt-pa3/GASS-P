// Copyright Epic Games, Inc. All Rights Reserved.

#include "Item/GASSWorldCollectable.h"

#include "Async/TaskGraphInterfaces.h"
#include "Inventory/GASSInventoryItemInstance.h"
#include "Item/GASSItemDefinition.h"
#include UE_INLINE_GENERATED_CPP_BY_NAME(GASSWorldCollectable)

struct FInteractionQuery;

AGASSWorldCollectable::AGASSWorldCollectable()
{
}

void AGASSWorldCollectable::GatherInteractionOptions(const FInteractionQuery& InteractQuery, FInteractionOptionBuilder& InteractionBuilder)
{
	InteractionBuilder.AddInteractionOption(Option);
}

FInventoryPickup& AGASSWorldCollectable::GetPickupInventory()
{
	return StaticInventory;
}

void AGASSWorldCollectable::SetAvatarStaticMesh()
{
	FName TargetAvatarStaticMeshName;

	if (AvatarStaticMesh.IsNone())
	{
		FGameplayTag ItemTag;

		if (StaticInventory.Templates.Num())
		{
			const UGASSItemDefinition* ItemCDO = GetDefault<UGASSItemDefinition>(StaticInventory.Templates[0].ItemDef);
			ItemTag = ItemCDO->ItemTag;
		}
		else if (StaticInventory.Instances.Num())
		{
			ItemTag = StaticInventory.Instances[0].Item->ItemTag;
		}
		else
		{
			return;
		}
		
		// Set Mesh With ItemTag
		TargetAvatarStaticMeshName;
		ItemTag;
	}
	else
	{
		TargetAvatarStaticMeshName = AvatarStaticMesh;
	}

}

