// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Interactables/MovableActorBase.h"
#include "Interaction/IInteractableTarget.h"
#include "GASSSimpleDoor.generated.h"

/**
 * 
 */
UCLASS()
class GASS_API AGASSSimpleDoor : public AMovableActorBase, public IInteractableTarget
{
	GENERATED_BODY()
	
public:

	AGASSSimpleDoor();

	virtual void GatherInteractionOptions(const FInteractionQuery& InteractQuery, FInteractionOptionBuilder& InteractionBuilder) override;

protected:
	UPROPERTY(EditAnywhere)
	FInteractionOption Option;
};
