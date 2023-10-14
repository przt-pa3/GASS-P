// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Engine/DataAsset.h"

#include "GASSPawnData.generated.h"

class APawn;
class UGASSAbilitySet;
class UGASSAbilityTagMapping;
class UGASSCameraMode;
class UGASSInputConfig;
class UObject;


/**
 * UGASSPawnData
 *
 *	Non-mutable data asset that contains properties used to define a pawn.
 */
UCLASS(BlueprintType, Const, Meta = (DisplayName = "GASS Pawn Data", ShortTooltip = "Data asset used to define a Pawn."))
class GASS_API UGASSPawnData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:

	UGASSPawnData(const FObjectInitializer& ObjectInitializer);

public:

	// Class to instantiate for this pawn (should usually derive from AGASSPawn or AGASSCharacter).
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GASS|Pawn")
	TSubclassOf<APawn> PawnClass;

	// Ability sets to grant to this pawn's ability system.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GASS|Abilities")
	TArray<TObjectPtr<UGASSAbilitySet>> AbilitySets;

	// What mapping of ability tags to use for actions taking by this pawn
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GASS|Abilities")
	TObjectPtr<UGASSAbilityTagMapping> TagMapping;

	// Input configuration used by player controlled pawns to create input mappings and bind input actions.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GASS|Input")
	TObjectPtr<UGASSInputConfig> InputConfig;

	// Default camera mode used by player controlled pawns.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GASS|Camera")
	TSubclassOf<UGASSCameraMode> DefaultCameraMode;
};
