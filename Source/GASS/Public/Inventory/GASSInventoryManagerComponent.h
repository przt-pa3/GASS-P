// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Components/ActorComponent.h"
#include "Net/Serialization/FastArraySerializer.h"

#include "GASSInventoryManagerComponent.generated.h"

class UGASSItemDefinition;
class UGASSInventoryItemInstance;
class UGASSInventoryManagerComponent;
class UObject;
struct FFrame;
struct FGASSInventoryList;
struct FNetDeltaSerializeInfo;
struct FReplicationFlags;

// Might Need To Add IsBlackBox
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FiveParams(FGASSInventory_EntryNewLink, bool, IsBlackbox, uint16, Index, int32, NewAmount, bool, Rotation, UGASSInventoryItemInstance*, ItemInstance);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FGASSInventory_EntryStackChanged, bool, IsBlackbox, uint16, Index, int32, ChangeAmount);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FGASSInventory_EntryLinkDeleted, bool, IsBlackbox, uint16, Index);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FGASSInventory_MoveCanceled_Dep, bool, IsBlackbox, uint16, Index);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FGASSInventory_MoveCanceled_Arr, bool, IsBlackbox);

/** A message when an item is added to the inventory */
USTRUCT(BlueprintType)
struct FGASSInventoryChangeMessage
{
	GENERATED_BODY()

	//@TODO: Tag based names+owning actors for inventories instead of directly exposing the component?
	UPROPERTY(BlueprintReadOnly, Category = Inventory)
	TObjectPtr<UActorComponent> InventoryOwner = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = Inventory)
	TObjectPtr<UGASSInventoryItemInstance> Instance = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = Inventory)
	int32 NewCount = 0;

	UPROPERTY(BlueprintReadOnly, Category = Inventory)
	int32 Delta = 0;
};

/** A single entry in an inventory */
USTRUCT(BlueprintType)
struct FGASSInventoryEntry : public FFastArraySerializerItem
{
	GENERATED_BODY()

	FGASSInventoryEntry()
	{}

	FString GetDebugString() const;

public:
	void InitializeInventoryEntry();
	void SetLastObservedAtInitialization();

	const uint8 GetCurrStackCount() const;
	const TObjectPtr<UGASSInventoryItemInstance> GetEntryInstance() const { return Instance; }
	bool StackEntry(uint8 StackAmount);
	bool UnStackEntry(uint8 UnStackAmount);	
private:
	friend FGASSInventoryList;
	friend UGASSInventoryManagerComponent;

	UPROPERTY()
	TObjectPtr<UGASSInventoryItemInstance> Instance = nullptr;

	UPROPERTY()
	uint8 ItemOffsetR = 0;

	UPROPERTY()
	uint8 ItemOffsetC = 0;

	UPROPERTY()
	bool Rotation = false;	// Rotate=false -> 세로

	UPROPERTY()
	uint8 CurrStackCount = 0;

	UPROPERTY(NotReplicated)
	uint8 LastObservedCount = 0;

	UPROPERTY(NotReplicated)
	uint16 LastObservedOffset = 0;
};

/** List of inventory items */
USTRUCT(BlueprintType)
struct FGASSInventoryList : public FFastArraySerializer
{
	GENERATED_BODY()

	FGASSInventoryList()
	: OwnerComponent(nullptr)
	{
	}

	FGASSInventoryList(UActorComponent* InOwnerComponent)
	: OwnerComponent(InOwnerComponent)
	{
	}

public:

	void PostReplicatedChange(const TArrayView<int32> ChangedIndices, int32 FinalSize);

	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms)
	{
		return FFastArraySerializer::FastArrayDeltaSerialize<FGASSInventoryEntry, FGASSInventoryList>(Entries, DeltaParms, *this);
	}

	void SetLastObservedAtInitialization();

	int32 GetNumOfEntry() const;
	void AddEntry(int32 EntryNum);
	void DeleteEntry(int32 EntryNum);

	UGASSInventoryItemInstance* CreateInstanceByDefinition(TSubclassOf<UGASSItemDefinition> ItemDef, uint8 StackCount);

	void LinkInstanceWithEntry(UGASSInventoryItemInstance* Instance, uint16 Index, uint8 OffsetR, uint8 OffsetC, bool Rotation, uint8 StackCount);
	void CleanEntry(uint16 Index);
	bool StackEntry(uint16 Index, uint8 StackAmount);
	bool UnStackEntry(uint16 Index, uint8 UnStackAmount);

private:

	friend UGASSInventoryManagerComponent;

	// Replicated list of items
	UPROPERTY()
	TArray<FGASSInventoryEntry> Entries;

	UPROPERTY(NotReplicated)
	TObjectPtr<UActorComponent> OwnerComponent;
};

template<>
struct TStructOpsTypeTraits<FGASSInventoryList> : public TStructOpsTypeTraitsBase2<FGASSInventoryList>
{
	enum { WithNetDeltaSerializer = true };
};

USTRUCT()
struct FGASSEntryPQNode : public FFastArraySerializerItem
{
	GENERATED_BODY()

public:
	FGASSEntryPQNode()
		:EntryIndex(0), OwnerComponent(nullptr) {}
	FGASSEntryPQNode(uint16 NewEntryIndex, UActorComponent* InActorComponent)
		:EntryIndex(NewEntryIndex), OwnerComponent(InActorComponent) {}

	const uint16 GetEntryIndex()
	{
		return (EntryIndex);
	}
	void SetEntryIndex(uint16 NewIndex)
	{
		EntryIndex = NewIndex;
	}
	bool operator<(const FGASSEntryPQNode Other) const;
	uint8 GetEntryStackCountByIndex() const;
private:
	friend UGASSInventoryManagerComponent;

	UPROPERTY()
	uint16 EntryIndex;

	UPROPERTY()
	TObjectPtr<UActorComponent> OwnerComponent;
};

USTRUCT()
struct FGASSEntryPQ : public FFastArraySerializer
{
	GENERATED_BODY()
public:
	FGASSEntryPQ() { Array.Heapify(); }

	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms)
	{
		return FFastArraySerializer::FastArrayDeltaSerialize<FGASSEntryPQNode, FGASSEntryPQ>(Array, DeltaParms, *this);
	}
	bool IsEmpty() const { return Array.Num() == 0; }
	uint16 GetSize() const { return Array.Num(); }
	void PopDiscard() 
	{ 
		Array.HeapPopDiscard(); 
		MarkArrayDirty();
	}
	void Push(FGASSEntryPQNode InNode)
	{
		int32 NewIdx = Array.HeapPush(InNode);
		FGASSEntryPQNode& NewNode = Array[NewIdx];
		MarkItemDirty(NewNode);
	}
	FGASSEntryPQNode Top()
	{
		return Array.HeapTop();
	}
	void UpdateSelf()
	{
		if (IsEmpty())
			return;

		Array.HeapSort();

		while (!IsEmpty())
		{
			if (Top().GetEntryStackCountByIndex() == 0)
				PopDiscard();
			else
				break;
		}
		MarkArrayDirty();
		return;
	}
	FGASSEntryPQNode GetNodeByIndexing(uint16 Index) const
	{
		return Array[Index];
	}
	void ChangeNodeEntry(uint16 BeforeIndex, uint16 NewIndex)
	{
		for (FGASSEntryPQNode& Node : Array)
		{
			if (Node.GetEntryIndex() == BeforeIndex)
			{
				Node.SetEntryIndex(NewIndex);
				MarkItemDirty(Node);
				return;
			}
		}
	}

private:
	UPROPERTY()
	TArray<FGASSEntryPQNode> Array;
};

template<>
struct TStructOpsTypeTraits<FGASSEntryPQ> : public TStructOpsTypeTraitsBase2<FGASSEntryPQ>
{
	enum { WithNetDeltaSerializer = true };
};

/**
 * Manages an inventory
 */
UCLASS(BlueprintType)
class GASS_API UGASSInventoryManagerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UGASSInventoryManagerComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	UFUNCTION(BlueprintCallable, Category = Inventory)
	void IMCTestFunc(TSubclassOf<UGASSItemDefinition> ItemDef);

	UFUNCTION(BlueprintCallable, Category = Inventory)
	void PrintInventoryList(bool IsBlackbox);

// ----------------------------------------------------------------------------------------------------------------
//	Actor interaction
// ----------------------------------------------------------------------------------------------------------------	

public:
	void SetOwnerActor(AActor* NewOwnerActor);
	AActor* GetOwnerActor() const { return OwnerActor; }

	void SetAvatarActor(AActor* NewAvatarActor);
	AActor* GetAvatarActor() const { return AvatarActor; }

	UFUNCTION()
	void OnRep_OwningActor();

	void InitInventoryActorInfo(AActor* InOwnerActor, AActor* InAvatarActor);

private:
	/** The actor that owns this component logically */
	UPROPERTY(ReplicatedUsing = OnRep_OwningActor)
	TObjectPtr<AActor> OwnerActor;

	/** The actor that is the physical representation used for Inventories. Can be NULL */
	UPROPERTY(ReplicatedUsing = OnRep_OwningActor)
	TObjectPtr<AActor> AvatarActor;

// ----------------------------------------------------------------------------------------------------------------
//	Inventory interaction
// ----------------------------------------------------------------------------------------------------------------	
public:

	UFUNCTION(BlueprintCallable, Category = Inventory)
	int32 GetInventorySize(bool bIsBlackbox) const;

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = Inventory)
	void InitializeInventory(float MaxWeight, uint8 MaxCol, uint8 MaxRow);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = Inventory)
	void InitializeBlackbox(uint8 MaxCol, uint8 MaxRow);

	UFUNCTION(BlueprintCallable, Category = Inventory)
	bool CanChangeInventory(uint8 NewMaxCol, uint8 NewMaxRow, float NewMaxWeight) const;

	UFUNCTION()
	void SetLastObservedAtInitialization();

	//Getters
	UFUNCTION(BlueprintCallable, Category = Inventory)
	int32 GetEntryStackCountByIndex(int32 Index) const;

	UFUNCTION()
	void GetEveryBaseEntryInfo(bool IsBlackbox, TArray<uint32>& ItemsInfo, TArray<UGASSInventoryItemInstance*>& Instances) const;

	UFUNCTION()
	UGASSInventoryItemInstance* GetItemInstanceByIndex(bool IsBlackbox, int32 Index);

	UFUNCTION()
	bool IsBlackbox(FGASSInventoryList& List);

	// Finding Functions
	UFUNCTION(BlueprintCallable, Category = Inventory)
	const uint8 GetFastAccessIndexByTag(const FGameplayTag InTag) const;

	uint16 GetEntryBaseIndex(bool IsBlackbox, uint16 Index) const;
	bool GetEveryEntryIndex(bool IsBlackbox, uint16 BaseIndex, TArray<uint16>& Indexes) const;
	bool GetEveryEntryIndex(bool IsBlackbox, UGASSInventoryItemInstance* Instance, uint16 BaseIndex, bool Rotation, TArray<uint16>& Indexes, TArray<uint16>& OffsetInfo) const;
	bool FindEntryByTagAndSize(bool bOnlyOne, bool IsBlackbox, TArray<uint16>& Positions, const FGameplayTag InTag, const uint8 shorter, const uint8 longer) const;

	UFUNCTION(BlueprintCallable, Category = Inventory)
	bool GetPossibleLocation(uint8& PositionR, uint8& PositionC, bool& Rotation, const uint8 shorter, const uint8 longer) const;

	// Instance
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = Inventory)
	UGASSInventoryItemInstance* CreateInstanceByDefinition(TSubclassOf<UGASSItemDefinition> ItemDef, uint8 StackCount);
	
	UGASSInventoryItemInstance* FindInstanceByRegisteredList(TSubclassOf<UGASSItemDefinition> ItemDef);

	void RemoveInstanceFromRegisteredList(UGASSInventoryItemInstance* InItemInstance);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = Inventory)
	void LinkInstanceWithEntries(UGASSInventoryItemInstance* Instance, uint8 StackCount, uint8 BasePositionR, uint8 BasePositionC, bool Rotation, bool IsBlackbox);

	// Adding Items
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = Inventory)
	uint8 AutoAddItemDefinition(const TSubclassOf<UGASSItemDefinition> ItemDef, uint8 StackCount);

	// 바로 Instance를 연결하는게 아님. Instance가지고 List 내에 존재하는지 확인, 없으면 새로 Create Instance 해서 만듬.
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = Inventory)
	void AddItemInstanceAt(UGASSInventoryItemInstance* Instance, uint8 StackCount, uint8 BasePositionR, uint8 BasePositionC, bool Rotation, bool IsBlackbox);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = Inventory)
	void AddItemAt(TSubclassOf<UGASSItemDefinition> ItemDef, uint8 StackCount, int32 Index, bool Rotation, bool IsBlackbox);

	// Deleting Items
	UFUNCTION()
	void DetachInstance(uint16 BaseIndex, bool IsBlackbox);

	// Stack/UnStack Items
	UFUNCTION()
	bool StackEntries(uint16 BaseIndex, uint8 StackCount, bool IsBlackbox);

	UFUNCTION()
	bool UnstackEntries(uint16 BaseIndex, uint8 StackCount, bool IsBlackbox);

	// Weight Management
	UFUNCTION()
	bool CanAddWeight(float AddWeight);

	UFUNCTION()
	bool AddWeight(float AddWeight);

	UFUNCTION()
	bool SubWeight(float SubWeight);

	// Interact With Widget
	UFUNCTION()
	uint8 CanPlaceItem(bool NeedToWeightCheck, TSubclassOf<UGASSItemDefinition> ItemDef, int32 StackAmount, bool IsBlackbox);

	UFUNCTION(BlueprintCallable, Category = Inventory)
	bool CanPlaceItemAt(bool NeedToWeightCheck, int32 ColStart, int32 RowStart, int32 ColSize, int32 RowSize, TSubclassOf<UGASSItemDefinition> ItemDef, int32 StackAmount, bool IsBlackbox);
	
	UFUNCTION(BlueprintCallable, Category = Inventory)
	bool CanMoveMyItemAt(int32 InitialIndex, int32 NewIndex, bool NewRotation, bool IsBlackbox);

	UFUNCTION()
	bool ServerCheckIMCEnoughItem(TSubclassOf<UGASSItemDefinition> ItemDef, int32 Index, int32 StackAmount, bool IsBlackbox);

	UFUNCTION()
	bool ServerCheckIMCCanPlaceItem(bool NeedToWeightCheck, TSubclassOf<UGASSItemDefinition> ItemDef, int32 Index, int32 StackAmount, bool Rotation, bool IsBlackbox);

	// RPC Function
	UFUNCTION(Server, Reliable, WithValidation)
	void ClientRequestMoveItem(TSubclassOf<UGASSItemDefinition> ItemDef, UActorComponent* DepComponent, UActorComponent* ArrComponent, int32 DepIndex, int32 ArrIndex, int32 StackAmount, bool Rotation, bool DepIsBlackbox, bool ArrIsBlackbox);

	UFUNCTION(Client, Reliable)
	void ServerCancelClientItemMove(UActorComponent* DepComponent, UActorComponent* ArrComponent, int32 DepIndex, int32 ArrIndex, int32 StackAmount, bool Rotation, bool DepIsBlackbox, bool ArrIsBlackbox);

	UFUNCTION(Server, Reliable, WithValidation)
	void ClientRequestStackItem_IMCtoIMC(TSubclassOf<UGASSItemDefinition> ItemDef, UActorComponent* DepComponent, UActorComponent* ArrComponent, int32 DepIndex, int32 ArrIndex, int32 StackAmount, bool DepIsBlackbox, bool ArrIsBlackbox);

	//~UObject interface
	virtual bool ReplicateSubobjects(class UActorChannel* Channel, class FOutBunch* Bunch, FReplicationFlags* RepFlags) override;
	virtual void ReadyForReplication() override;
	//~End of UObject interface
	
public:

	UPROPERTY(BlueprintAssignable)
	FGASSInventory_EntryNewLink OnEntryNewLink;

	UPROPERTY(BlueprintAssignable)
	FGASSInventory_EntryStackChanged OnEntryStackChanged;

	UPROPERTY(BlueprintAssignable)
	FGASSInventory_EntryLinkDeleted OnEntryLinkDeleted;

	UPROPERTY(BlueprintAssignable)
	FGASSInventory_MoveCanceled_Dep OnInventoryMoveCanceled_Dep;

	UPROPERTY(BlueprintAssignable)
	FGASSInventory_MoveCanceled_Arr OnInventoryMoveCanceled_Arr;

	UPROPERTY(Replicated)
	uint8 InventoryMaxCol = 1;

	UPROPERTY(Replicated)
	uint8 InventoryMaxRow = 1;

	UPROPERTY(Replicated)
	uint8 BlackboxMaxCol = 0;

	UPROPERTY(Replicated)
	uint8 BlackboxMaxRow = 0;

private:
	UPROPERTY(Replicated) // Does it have to replicated? Need to check Later
	TArray<FGASSEntryPQ> CounsumableFastAccess;

	UPROPERTY(Replicated)
	FGASSInventoryList InventoryList;

	UPROPERTY(Replicated)
	FGASSInventoryList Blackbox;

	UPROPERTY(Replicated)
	float InventoryCurrWeight = 0.0;

	UPROPERTY(Replicated)
	float InventoryMaxWeight = 0.0;

public:
};
