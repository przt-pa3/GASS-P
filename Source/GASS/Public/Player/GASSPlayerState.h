// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "AbilitySystemInterface.h"
#include "ModularPlayerState.h"
#include "System/GameplayTagStack.h"
//#include "Teams/GASSTeamAgentInterface.h"

#include "GASSPlayerState.generated.h"

//struct FGASSVerbMessage;

class AController;
class AGASSPlayerController;
class APlayerState;
class FName;
class UAbilitySystemComponent;
class UGASSAbilitySystemComponent;
//class UGASSInventoryManagerComponent;
//class UGASSInventoryItemInstance;
//class UGASSItemDefinition;
class UGASSExperienceDefinition;
class UGASSPawnData;
class UObject;
struct FFrame;
struct FGameplayTag;

/** Defines the types of client connected */
UENUM()
enum class EGASSPlayerConnectionType : uint8
{
	// An active player
	Player = 0,

	// Spectator connected to a running game
	LiveSpectator,

	// Spectating a demo recording offline
	ReplaySpectator,

	// A deactivated player (disconnected)
	InactivePlayer
};

/**
 * AGASSPlayerState
 *
 *	Base player state class used by this project.
 */
UCLASS(Config = Game)
class GASS_API AGASSPlayerState : public AModularPlayerState, public IAbilitySystemInterface//, public IGASSTeamAgentInterface
{
	GENERATED_BODY()

public:
	AGASSPlayerState(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	UFUNCTION(BlueprintCallable, Category = "GASS|PlayerState")
	AGASSPlayerController* GetGASSPlayerController() const;

	UFUNCTION(BlueprintCallable, Category = "GASS|PlayerState")
	UGASSAbilitySystemComponent* GetGASSAbilitySystemComponent() const { return AbilitySystemComponent; }
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	// TODO : Do PlayerStates need to Have Reference to Inventory Manager Component?
	//UFUNCTION(BlueprintCallable, Category = "GASS|PlayerState")
	//UGASSInventoryManagerComponent* GetGASSInventoryManagerComponent() const { return InventoryManagerComponent; }

	template <class T>
	const T* GetPawnData() const { return Cast<T>(PawnData); }

	void SetPawnData(const UGASSPawnData* InPawnData);

	//~AActor interface
	virtual void PreInitializeComponents() override;
	virtual void PostInitializeComponents() override;
	//~End of AActor interface

	//~APlayerState interface
	virtual void Reset() override;
	virtual void ClientInitialize(AController* C) override;
	virtual void CopyProperties(APlayerState* PlayerState) override;
	virtual void OnDeactivated() override;
	virtual void OnReactivated() override;
	//~End of APlayerState interface

	////~IGASSTeamAgentInterface interface
	//virtual void SetGenericTeamId(const FGenericTeamId& NewTeamID) override;
	//virtual FGenericTeamId GetGenericTeamId() const override;
	//virtual FOnGASSTeamIndexChangedDelegate* GetOnTeamIndexChangedDelegate() override;
	////~End of IGASSTeamAgentInterface interface

	static const FName NAME_GASSAbilityReady;

	void SetPlayerConnectionType(EGASSPlayerConnectionType NewType);
	EGASSPlayerConnectionType GetPlayerConnectionType() const { return MyPlayerConnectionType; }

	//const UGASSInventoryItemInstance* GetInventoryInstance(TSubclassOf<UGASSItemDefinition> InItemDef);

	///** Returns the Squad ID of the squad the player belongs to. */
	//UFUNCTION(BlueprintCallable)
	//	int32 GetSquadId() const
	//{
	//	return MySquadID;
	//}

	///** Returns the Team ID of the team the player belongs to. */
	//UFUNCTION(BlueprintCallable)
	//	int32 GetTeamId() const
	//{
	//	return GenericTeamIdToInteger(MyTeamID);
	//}

	//void SetSquadID(int32 NewSquadID);

	//// Adds a specified number of stacks to the tag (does nothing if StackCount is below 1)
	//UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = Teams)
	//void AddStatTagStack(FGameplayTag Tag, int32 StackCount);

	//// Removes a specified number of stacks from the tag (does nothing if StackCount is below 1)
	//UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = Teams)
	//void RemoveStatTagStack(FGameplayTag Tag, int32 StackCount);

	//// Returns the stack count of the specified tag (or 0 if the tag is not present)
	//UFUNCTION(BlueprintCallable, Category = Teams)
	//int32 GetStatTagStackCount(FGameplayTag Tag) const;

	//// Returns true if there is at least one stack of the specified tag
	//UFUNCTION(BlueprintCallable, Category = Teams)
	//bool HasStatTag(FGameplayTag Tag) const;

	//// Send a message to just this player
	//// (use only for client notifications like accolades, quest toasts, etc... that can handle being occasionally lost)
	//UFUNCTION(Client, Unreliable, BlueprintCallable, Category = "GASS|PlayerState")
	//void ClientBroadcastMessage(const FGASSVerbMessage Message);

private:
	void OnExperienceLoaded(const UGASSExperienceDefinition* CurrentExperience);

protected:
	UFUNCTION()
	void OnRep_PawnData();

protected:

	UPROPERTY(ReplicatedUsing = OnRep_PawnData)
	TObjectPtr<const UGASSPawnData> PawnData;

private:

	// The ability system component sub-object used by player characters.
	UPROPERTY(VisibleAnywhere, Category = "GASS|PlayerState")
	TObjectPtr<UGASSAbilitySystemComponent> AbilitySystemComponent;

	//UPROPERTY(VisibleAnywhere, Category = "GASS|PlayerState")
	//TObjectPtr<UGASSInventoryManagerComponent> InventoryManagerComponent;

	UPROPERTY(Replicated)
	EGASSPlayerConnectionType MyPlayerConnectionType;

	//UPROPERTY()
	//FOnGASSTeamIndexChangedDelegate OnTeamChangedDelegate;

	//UPROPERTY(ReplicatedUsing = OnRep_MyTeamID)
	//FGenericTeamId MyTeamID;

	//UPROPERTY(ReplicatedUsing = OnRep_MySquadID)
	//int32 MySquadID;

	UPROPERTY(Replicated)
	FGameplayTagStackContainer StatTags;

private:
	//UFUNCTION()
	//void OnRep_MyTeamID(FGenericTeamId OldTeamID);

	//UFUNCTION()
	//void OnRep_MySquadID();
};
