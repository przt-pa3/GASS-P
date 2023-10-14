// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "ModularPawn.h"
#include "Equipment/GASSEquipmentManagerComponent.h"
#include "GASSPawn.generated.h"

class AController;
class UObject;
class UGASSEquipmentManagerComponent;
struct FFrame;

/**
 * AGASSPawn
 */
UCLASS()
class GASS_API AGASSPawn : public AModularPawn
{
	GENERATED_BODY()

public:

	AGASSPawn(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	//~AActor interface
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PreInitializeComponents() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	//~End of AActor interface

	//~APawn interface
	virtual void PossessedBy(AController* NewController) override;
	virtual void UnPossessed() override;
	//~End of APawn interface

	////~IGASSTeamAgentInterface interface
	//virtual void SetGenericTeamId(const FGenericTeamId& NewTeamID) override;
	//virtual FGenericTeamId GetGenericTeamId() const override;
	//virtual FOnGASSTeamIndexChangedDelegate* GetOnTeamIndexChangedDelegate() override;
	////~End of IGASSTeamAgentInterface interface


protected:
	//// Called to determine what happens to the team ID when possession ends
	//virtual FGenericTeamId DetermineNewTeamAfterPossessionEnds(FGenericTeamId OldTeamID) const
	//{
	//	// This could be changed to return, e.g., OldTeamID if you want to keep it assigned afterwards, or return an ID for some neutral faction, or etc...
	//	return FGenericTeamId::NoTeam;
	//}

private:
	//UFUNCTION()
	//void OnControllerChangedTeam(UObject* TeamAgent, int32 OldTeam, int32 NewTeam);

private:
	//UPROPERTY(ReplicatedUsing = OnRep_MyTeamID)
	//FGenericTeamId MyTeamID;

	//UPROPERTY()
	//FOnGASSTeamIndexChangedDelegate OnTeamChangedDelegate;

private:
	//UFUNCTION()
	//void OnRep_MyTeamID(FGenericTeamId OldTeamID);
};
