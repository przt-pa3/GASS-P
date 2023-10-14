// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "GameFramework/Actor.h"
#include "Interaction/IInteractableTarget.h"
#include "Interaction/InteractionOption.h"
#include "Item/IPickupable.h"

#include "GASSWorldCollectable.generated.h"

class UObject;
struct FInteractionQuery;

/**
 * 
 */
UCLASS(Abstract, Blueprintable)
class AGASSWorldCollectable : public AActor, public IInteractableTarget, public IPickupable
{
	GENERATED_BODY()

public:

	AGASSWorldCollectable();

	virtual void GatherInteractionOptions(const FInteractionQuery& InteractQuery, FInteractionOptionBuilder& InteractionBuilder) override;
	virtual FInventoryPickup& GetPickupInventory() override;

protected:
	UFUNCTION()
	void SetAvatarStaticMesh();

protected:
	UPROPERTY(EditAnywhere)
	FName AvatarStaticMesh;

	UPROPERTY(EditAnywhere)
	FInteractionOption Option;

	UPROPERTY(EditAnywhere)
	FInventoryPickup StaticInventory;
};
