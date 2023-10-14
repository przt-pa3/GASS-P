// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Templates/SubclassOf.h"

#include "GASSEquipmentDefinition.generated.h"

class AActor;
class UGASSAbilitySet;
class UGASSEquipmentInstance;
class UGASSItemDefinition;

USTRUCT()
struct FGASSEquipmentActorToSpawn
{
	GENERATED_BODY()

	FGASSEquipmentActorToSpawn()
	{}

	UPROPERTY(EditAnywhere, Category = Equipment)
	bool bSpawnWhenEquipped = true;

	UPROPERTY(EditAnywhere, Category=Equipment)
	TSubclassOf<AActor> ActorToSpawn;

	UPROPERTY(EditAnywhere, Category=Equipment)
	FName AttachSocket;

	UPROPERTY(EditAnywhere, Category=Equipment)
	FTransform AttachTransform;
};


/**
 * UGASSEquipmentDefinition
 *
 * Definition of a piece of equipment that can be applied to a pawn
 */
UCLASS(Blueprintable, Const, Abstract, BlueprintType)
class UGASSEquipmentDefinition : public UObject
{
	GENERATED_BODY()

public:
	UGASSEquipmentDefinition(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	// Item Definition
	UPROPERTY(EditDefaultsOnly, Category = Equipment)
	TSubclassOf<UGASSItemDefinition> ItemDefinition;

	// Class to spawn
	UPROPERTY(EditDefaultsOnly, Category=Equipment)
	TSubclassOf<UGASSEquipmentInstance> InstanceType;

	// Gameplay ability sets to grant when this is equipped
	UPROPERTY(EditDefaultsOnly, Category=Equipment)
	TArray<TObjectPtr<const UGASSAbilitySet>> AbilitySetsToGrant;

	UPROPERTY(EditDefaultsOnly, Category = Equipment)
	TArray<TObjectPtr<const UGASSAbilitySet>> AbilitySetsToGrantWhenArmed;

	// Actors to spawn on the pawn when this is equipped
	UPROPERTY(EditDefaultsOnly, Category=Equipment)
	TArray<FGASSEquipmentActorToSpawn> ActorsToSpawn;
};
