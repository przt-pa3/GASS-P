// Fill out your copyright notice in the Description page of Project Settings.


#include "Interactables/GASSSimpleDoor.h"

#include "Interaction/InteractionQuery.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GASSSimpleDoor)

AGASSSimpleDoor::AGASSSimpleDoor()
{

}

void AGASSSimpleDoor::GatherInteractionOptions(const FInteractionQuery& InteractQuery, FInteractionOptionBuilder& InteractionBuilder)
{
	InteractQuery.OptionalObjectData;
	InteractionBuilder.AddInteractionOption(Option);
}