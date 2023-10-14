// Fill out your copyright notice in the Description page of Project Settings.

#include "Interactables/GASSStorage.h"
#include "Interaction/InteractionQuery.h"
#include "Inventory/GASSInventoryManagerComponent.h"

AGASSStorage::AGASSStorage(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	
	InventoryManagerComponent = ObjectInitializer.CreateDefaultSubobject<UGASSInventoryManagerComponent>(this, TEXT("InventorySystemComponent"));
	InventoryManagerComponent->SetIsReplicated(true);
	bReplicateUsingRegisteredSubObjectList = true;

}

void AGASSStorage::BeginPlay()
{
	Super::BeginPlay();
	SetAndInitializeStorageIMC();
}

void AGASSStorage::GatherInteractionOptions(const FInteractionQuery& InteractQuery, FInteractionOptionBuilder& InteractionBuilder)
{
	InteractQuery.OptionalObjectData;
	
	InteractionBuilder.AddInteractionOption(Option);
}

void AGASSStorage::SetAndInitializeStorageIMC()
{
	StorageTemplate.StorageIMC = InventoryManagerComponent;
	UStorageStatics::InitStorageIMC(StorageTemplate, this);
}

UGASSInventoryManagerComponent* AGASSStorage::GetIMC() const
{
	return InventoryManagerComponent;
	//return nullptr;
}

void AGASSStorage::AddCurrentInteractingPlayer()
{
	CurrentInteractingPlayer++;
}

void AGASSStorage::SubCurrentInteractingPlayer()
{
	CurrentInteractingPlayer > 0 ? CurrentInteractingPlayer-- : CurrentInteractingPlayer = 0;
}

bool AGASSStorage::IsCurrentInteractingPlayerExist()
{
	return CurrentInteractingPlayer > 0;
}

