// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "AbilitySystemInterface.h"
#include "ModularGameState.h"

#include "GASSGameState.generated.h"

struct FGASSVerbMessage;

class APlayerState;
class UAbilitySystemComponent;
class UGASSAbilitySystemComponent;
class UGASSExperienceManagerComponent;
class UObject;
struct FFrame;

/**
 * AGASSGameState
 *
 *	The base game state class used by this project.
 */
UCLASS(Config = Game)
class GASS_API AGASSGameState : public AModularGameStateBase, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:

	AGASSGameState(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	float GetServerFPS() const { return ServerFPS; }

	//~AActor interface
	virtual void PreInitializeComponents() override;
	virtual void PostInitializeComponents() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	//~End of AActor interface

	//~AGameStateBase interface
	virtual void AddPlayerState(APlayerState* PlayerState) override;
	virtual void RemovePlayerState(APlayerState* PlayerState) override;
	virtual void SeamlessTravelTransitionCheckpoint(bool bToTransitionMap) override;
	//~End of AGameStateBase interface

	//~IAbilitySystemInterface
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	//~End of IAbilitySystemInterface

	UFUNCTION(BlueprintCallable, Category = "GASS|GameState")
	UGASSAbilitySystemComponent* GetGASSAbilitySystemComponent() const { return AbilitySystemComponent; }

	//// Send a message that all clients will (probably) get
	//// (use only for client notifications like eliminations, server join messages, etc... that can handle being lost)
	//UFUNCTION(NetMulticast, Unreliable, BlueprintCallable, Category = "GASS|GameState")
	//void MulticastMessageToClients(const FGASSVerbMessage Message);

	//// Send a message that all clients will be guaranteed to get
	//// (use only for client notifications that cannot handle being lost)
	//UFUNCTION(NetMulticast, Reliable, BlueprintCallable, Category = "GASS|GameState")
	//void MulticastReliableMessageToClients(const FGASSVerbMessage Message);

private:
	UPROPERTY()
	TObjectPtr<UGASSExperienceManagerComponent> ExperienceManagerComponent;

	// The ability system component subobject for game-wide things (primarily gameplay cues)
	UPROPERTY(VisibleAnywhere, Category = "GASS|GameState")
	TObjectPtr<UGASSAbilitySystemComponent> AbilitySystemComponent;


protected:

	virtual void Tick(float DeltaSeconds) override;

protected:
	UPROPERTY(Replicated)
	float ServerFPS;
};
