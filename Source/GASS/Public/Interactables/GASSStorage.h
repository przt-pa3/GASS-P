// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Interactables/MovableActorBase.h"
#include "Interaction/IInteractableTarget.h"
#include "Inventory/IStorage.h"
#include "GASSStorage.generated.h"

/**
 * 
 */
UCLASS()
class GASS_API AGASSStorage : public AMovableActorBase, public IInteractableTarget, public IStorage
{
	GENERATED_BODY()
	
public:
	AGASSStorage(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	virtual void BeginPlay() override;

// InteractableTarget Interface
public:
	virtual void GatherInteractionOptions(const FInteractionQuery& InteractQuery, FInteractionOptionBuilder& InteractionBuilder) override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FInteractionOption Option;
// ~ InteractableTarget Interface

// Storage Interface
public:
	UFUNCTION()
	virtual void SetAndInitializeStorageIMC() override;

	UFUNCTION(BlueprintCallable)
	virtual UGASSInventoryManagerComponent* GetIMC() const override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FStorageTemplate StorageTemplate;
// ~ Storage Interface

public:
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly)
	void AddCurrentInteractingPlayer();

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly)
	void SubCurrentInteractingPlayer();

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly)
	bool IsCurrentInteractingPlayerExist();

protected:
	UPROPERTY()
	UGASSInventoryManagerComponent* InventoryManagerComponent;

	UPROPERTY()
	int32 CurrentInteractingPlayer = 0;
};
