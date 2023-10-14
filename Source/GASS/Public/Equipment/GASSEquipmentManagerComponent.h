// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "AbilitySystem/GASSAbilitySet.h"
#include "Components/ActorComponent.h"
#include "Components/PawnComponent.h"
#include "Net/Serialization/FastArraySerializer.h"

#include "GASSEquipmentManagerComponent.generated.h"

class UPawnComponent;
class UGASSAbilitySystemComponent;
class UGASSEquipmentDefinition;
class UGASSEquipmentInstance;
class UGASSEquipmentManagerComponent;
class UGASSInventoryManagerComponent;
class UObject;
struct FFrame;
struct FGASSEquipmentList;
struct FNetDeltaSerializeInfo;
struct FReplicationFlags;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FGASSEquipment_Equipped, bool, IsMainEquipment, uint16, Index, TSubclassOf<UGASSItemDefinition>, ItemDef);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FGASSEquipment_Unequipped, bool, IsMainEquipment, uint16, Index);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FGASSEquipment_EquipCanceled, bool, IsMainEquipment, uint16, Index);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FGASSEquipment_UnequipCanceled, bool, IsMainEquipment, uint16, Index);

/** A single piece of applied equipment */
USTRUCT(BlueprintType)
struct FGASSAppliedEquipmentEntry : public FFastArraySerializerItem
{
	GENERATED_BODY()

	FGASSAppliedEquipmentEntry()
	{}

	FString GetDebugString() const;

private:
	friend FGASSEquipmentList;
	friend UGASSEquipmentManagerComponent;

	// The equipment class that got equipped
	UPROPERTY()
	TSubclassOf<UGASSEquipmentDefinition> EquipmentDefinition;

	UPROPERTY()
	TObjectPtr<UGASSEquipmentInstance> Instance = nullptr;

	UPROPERTY(NotReplicated)
	TObjectPtr<UGASSEquipmentInstance> LastObservedInstance = nullptr;

	// Authority-only list of granted handles
	UPROPERTY(NotReplicated)
	FGASSAbilitySet_GrantedHandles GrantedHandles;

	UPROPERTY(NotReplicated)
	FGASSAbilitySet_GrantedHandles GrantedHandlesWhenArmed;
};

/** List of applied equipment */
USTRUCT(BlueprintType)
struct FGASSEquipmentList : public FFastArraySerializer
{
	GENERATED_BODY()

	FGASSEquipmentList()
		: OwnerComponent(nullptr)
	{
	}

	FGASSEquipmentList(UActorComponent* InOwnerComponent, bool bInImMainList)
		: OwnerComponent(InOwnerComponent)
		, bImMainList(bInImMainList)
	{
	}

public:
	//~FFastArraySerializer contract
	void PreReplicatedRemove(const TArrayView<int32> RemovedIndices, int32 FinalSize);
	void PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize);
	void PostReplicatedChange(const TArrayView<int32> ChangedIndices, int32 FinalSize);
	//~End of FFastArraySerializer contract

	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms)
	{
		return FFastArraySerializer::FastArrayDeltaSerialize<FGASSAppliedEquipmentEntry, FGASSEquipmentList>(Entries, DeltaParms, *this);
	}

	bool IsEntryEmpty(int32 EntryIndex);
	void AddEmptyEntry(int32 EntryNum);
	void RemoveEntry(int32 EntryIndex);

	int32 SearchAdditionalEntryWithDefinition(TSubclassOf<UGASSEquipmentDefinition> EquipmentDefinition);

	UGASSEquipmentInstance* CreateInstanceByDefinition(TSubclassOf<UGASSEquipmentDefinition> EquipmentDefinition);

	// Check Entry is empty, Create Instance, Attach Equipment to Entry
	UGASSEquipmentInstance* AttachEquipmentWithEntry(int32 EntryIndex, TSubclassOf<UGASSEquipmentDefinition> EquipmentDefinition);
	// Detach, Cache Instance's ED, Remove Instance, Return Ed
	TSubclassOf<UGASSEquipmentDefinition> DettachEquipmentWithEntry(int32 EntryIndex, OUT UGASSEquipmentInstance*& DettachedInstance);

	UGASSEquipmentInstance* EquipAdditionalEntry(TSubclassOf<UGASSEquipmentDefinition> EquipmentDefinition);
	UGASSEquipmentInstance* UnequipAdditionalEntry(int32 Index);
	UGASSEquipmentInstance* UnequipAdditionalEntry(TSubclassOf<UGASSEquipmentDefinition> EquipmentDefinition);

private:
	UGASSAbilitySystemComponent* GetAbilitySystemComponent() const;
	

	friend UGASSEquipmentManagerComponent;

private:
	// Replicated list of equipment entries
	UPROPERTY()
	TArray<FGASSAppliedEquipmentEntry> Entries;

	// Owner Should be Pawn
	UPROPERTY(NotReplicated)
	TObjectPtr<UActorComponent> OwnerComponent;

	UPROPERTY()
	bool bImMainList = false;
};

template<>
struct TStructOpsTypeTraits<FGASSEquipmentList> : public TStructOpsTypeTraitsBase2<FGASSEquipmentList>
{
	enum { WithNetDeltaSerializer = true };
};

/**
 * Manages equipment applied to a pawn
 */
UCLASS(BlueprintType) //Const ??
class UGASSEquipmentManagerComponent : public UPawnComponent
{
	GENERATED_BODY()

public:
	UGASSEquipmentManagerComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	UFUNCTION(BlueprintCallable)
	void Test_PrintEquip();

private:
	UGASSInventoryManagerComponent* GetInventoryManagerComponent() const;

// ----------------------------------------------------------------------------------------------------------------
//	Actor interaction
// ----------------------------------------------------------------------------------------------------------------	
public:
	void SetOwnerActor(AActor* NewOwnerActor);
	AActor* GetOwnerActor() const { return OwnerActor; }

	UFUNCTION()
	void OnRep_OwningActor();
	void InitEquipmentActorInfo(AActor* InOwnerActor);

private:
	/** The actor that owns this component logically : Should be Controller */
	UPROPERTY(ReplicatedUsing = OnRep_OwningActor)
	TObjectPtr<AActor> OwnerActor;

public:
// Initializers
	UFUNCTION()
	void InitializeEquipmentComponent();

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = Equipment)
	void InitializeEquipmentList(bool IsMainEquipment, int32 EquipmentListSize);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly)
	void UnInitializeEquipmentList(FGASSEquipmentList& TargetList);
// Getters
	UFUNCTION()
	int32 GetEquipmentListSize(bool IsMain);

	UFUNCTION()
	int32 GetAvailableAdditionalEquipmentSlot();

	UFUNCTION()
	void GetItemDefFromList(bool IsMainEquipment, TArray<TSubclassOf<UGASSItemDefinition>>& ItemDefs) const;

	UFUNCTION(BlueprintCallable, Category = Inventory)
	const uint8 GetEquipmentIndexByItemTag(const FGameplayTag& ItemTag) const;

	UFUNCTION(BlueprintCallable, Category = Inventory)
	const uint8 GetEquipmentIndexByDefinition(TSubclassOf<UGASSEquipmentDefinition> EquipmentDefinition) const;

// Equip / Unequip
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly)
	UGASSEquipmentInstance* EquipItem(TSubclassOf<UGASSEquipmentDefinition> EquipmentDefinition);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly)
	UGASSEquipmentInstance* EquipOrSwapMainItem(TSubclassOf<UGASSEquipmentDefinition> EquipmentDefinition, int32 EquipmentIndex, TSubclassOf<UGASSEquipmentDefinition>& DettachedDefinition, UGASSEquipmentInstance* DettachedInstance);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly)
	bool CanEquipItem(TSubclassOf<UGASSEquipmentDefinition> EquipmentDefinition, UGASSInventoryManagerComponent* InIMC, int32& EquippableSlot, TSubclassOf<UGASSEquipmentDefinition>& DettachableDefinition);

	UFUNCTION()
	bool CanEquipItemWeightCheck(float NewEquipmentWeight, bool IsFromMyIMC, bool IsMainEquipment, int32 EquippableSlot);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly)
	void UnequipItem(TSubclassOf<UGASSEquipmentDefinition> EquipmentDefinition);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly)
	void UnequipItemAt(bool IsMainEquipment, int32 EquippedSlot);

// Arm / Disarm
	UFUNCTION(BlueprintCallable)
	int32 GetCurrentArmedSlot();

	UFUNCTION(BlueprintPure, Category = Equipment)
	UGASSEquipmentInstance* GetCurrentArmedInstance();

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly)
	void ArmItem(int32 EquippedSlot);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly)
	bool CanArmItem(int32 EquippedSlot);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly)
	void DisarmItem();

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly)
	void ChangeArmItem(int32 EquippedSlot);

// RPC Functions
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly)
	bool ServerCehckEMCEnoughItem(TSubclassOf<UGASSItemDefinition> ItemDef, bool IsMainEquipment, int32 EquippableSlot);

	UFUNCTION(Client, Reliable)
	void ServerCancelClientEquip(bool IsMainEquipment, int32 EquippableSlot);

	UFUNCTION(Client, Reliable)
	void ServerCancelClientUnequip(bool IsMainEquipment, int32 EquippableSlot);

	UFUNCTION(Server, Reliable, WithValidation)
	void ClientRequestEquipItem(TSubclassOf<UGASSEquipmentDefinition> EquipmentDefinition, UActorComponent* DepComponent, int32 DepIndex, bool DepIsBlackbox);

//~UObject interface
	virtual bool ReplicateSubobjects(class UActorChannel* Channel, class FOutBunch* Bunch, FReplicationFlags* RepFlags) override;
//~End of UObject interface

//~UActorComponent interface
	virtual void InitializeComponent() override;
	virtual void UninitializeComponent() override;
	virtual void ReadyForReplication() override;
//~End of UActorComponent interface

public:
	UPROPERTY(BlueprintAssignable)
	FGASSEquipment_Equipped OnEntryEquipped;

	UPROPERTY(BlueprintAssignable)
	FGASSEquipment_Unequipped OnEntryUnequipped;

	UPROPERTY(BlueprintAssignable)
	FGASSEquipment_EquipCanceled OnEquipCanceled;

	UPROPERTY(BlueprintAssignable)
	FGASSEquipment_UnequipCanceled OnUnequipCanceled;

private:
	UPROPERTY(Replicated)
	FGASSEquipmentList MainEquipmentList;

	UPROPERTY(Replicated)
	FGASSEquipmentList AdditionalEquipmentList;

	UPROPERTY(ReplicatedUsing = OnRep_ArmChanged)
	int32 CurrentArmedSlot = -1;

	UPROPERTY()
	int32 ClientLastArmedSlot = -1;

public:
	UFUNCTION()
	void OnRep_ArmChanged();
};
