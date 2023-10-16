// Copyright Epic Games, Inc. All Rights Reserved.

#include "Inventory/GASSInventoryManagerComponent.h"

#include "Equipment/GASSEquipmentManagerComponent.h"
#include "GameplayTagsManager.h"
#include "Engine/ActorChannel.h"
#include "Engine/World.h"
//#include "GameFramework/GameplayMessageSubsystem.h"
#include "Item/GASSItemDefinition.h"
#include "Inventory/GASSInventoryItemInstance.h"
#include "Inventory/IStorage.h"
#include "GASS/GASSGameplayTags.h"
#include "NativeGameplayTags.h"
#include "Net/UnrealNetwork.h"
#include "Player/GASSPlayerController.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GASSInventoryManagerComponent)

class FLifetimeProperty;
struct FReplicationFlags;

//UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_GASS_Inventory_Message_StackChanged, "GASS.Inventory.Message.StackChanged");

//////////////////////////////////////////////////////////////////////
// FGASSInventoryEntry

FString FGASSInventoryEntry::GetDebugString() const
{
	TSubclassOf<UGASSItemDefinition> ItemDef;
	if (Instance != nullptr)
	{
		ItemDef = Instance->GetItemDef();
	}

	return FString::Printf(TEXT("%s (%s)"), *GetNameSafe(Instance), *GetNameSafe(ItemDef));
}

void FGASSInventoryEntry::InitializeInventoryEntry()
{
	Instance = nullptr;
	ItemOffsetR = 0;
	ItemOffsetC = 0;
	Rotation = false;
	CurrStackCount = 0;
	LastObservedCount = 0;
	LastObservedOffset = 0;
}

void FGASSInventoryEntry::SetLastObservedAtInitialization()
{
	LastObservedOffset = ItemOffsetR + ItemOffsetC;
	LastObservedCount = CurrStackCount;
}

const uint8 FGASSInventoryEntry::GetCurrStackCount() const
{
	return CurrStackCount;
}

bool FGASSInventoryEntry::StackEntry(uint8 StackAmount)
{
	if (Instance == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("Cannot Stack Entry, Instance is nullptr"));
		return false;
	}
	if (Instance->MaxItemStackCount < CurrStackCount + StackAmount)
	{
		UE_LOG(LogTemp, Error, TEXT("Cannot Stack Entry, Amount Issue"));
		return false;
	}
	CurrStackCount += StackAmount;
	if (ItemOffsetC == 0 && ItemOffsetR == 0)
	{
		Instance->StackItem(StackAmount);
	}
		
	return true;
}

bool FGASSInventoryEntry::UnStackEntry(uint8 UnStackAmount)
{
	if (Instance == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("Cannot Stack Entry, Instance is nullptr"));
		return false;
	}
	if (CurrStackCount < UnStackAmount)
	{
		UE_LOG(LogTemp, Error, TEXT("Cannot Stack Entry, Amount Issue"));
		return false;
	}
	CurrStackCount -= UnStackAmount;

	if (ItemOffsetC == 0 && ItemOffsetR == 0)
	{
		Instance->UnStackItem(UnStackAmount);
	}

	return true;
}

//////////////////////////////////////////////////////////////////////
// FGASSInventoryList

void FGASSInventoryList::PostReplicatedChange(const TArrayView<int32> ChangedIndices, int32 FinalSize)
{
	if (OwnerComponent->GetOwner()->HasAuthority())
	{
		return;
	}

	for (int32 Index : ChangedIndices)
	{
		FGASSInventoryEntry& Stack = Entries[Index];

		UGASSInventoryManagerComponent* OwnerIMC = Cast<UGASSInventoryManagerComponent>(OwnerComponent);
		check(OwnerIMC);

		if (Stack.CurrStackCount > Stack.LastObservedCount)
		{
			if (Stack.LastObservedCount == 0)
			{
				if (Stack.ItemOffsetR == 0 && Stack.ItemOffsetC == 0)
				{
					OwnerIMC->OnEntryNewLink.Broadcast(OwnerIMC->IsBlackbox(*this), Index, Stack.CurrStackCount, Stack.Rotation, Stack.Instance);
				}
			}
			else
			{
				if (Stack.ItemOffsetR == 0 && Stack.ItemOffsetC == 0)
				{
					OwnerIMC->OnEntryStackChanged.Broadcast(OwnerIMC->IsBlackbox(*this), Index, Stack.CurrStackCount - Stack.LastObservedCount);
				}
			}
		}
		else if (Stack.CurrStackCount < Stack.LastObservedCount)
		{
			if (Stack.CurrStackCount == 0)
			{
				if (Stack.ItemOffsetR == 0 && Stack.ItemOffsetC == 0 && Stack.LastObservedOffset == 0)
				{
					OwnerIMC->OnEntryLinkDeleted.Broadcast(OwnerIMC->IsBlackbox(*this), Index);
				}
			}
			else
			{
				if (Stack.ItemOffsetR == 0 && Stack.ItemOffsetC == 0)
				{
					OwnerIMC->OnEntryStackChanged.Broadcast(OwnerIMC->IsBlackbox(*this), Index, Stack.CurrStackCount - Stack.LastObservedCount);
				}
			}
		}
		else
		{
			if (Stack.LastObservedOffset == 0)
			{
				if (Stack.ItemOffsetR == 0 && Stack.ItemOffsetC == 0)
				{
					// Swap
				}
				else
				{
					OwnerIMC->OnEntryLinkDeleted.Broadcast(OwnerIMC->IsBlackbox(*this), Index);
				}
			}
			else
			{
				if (Stack.ItemOffsetR == 0 && Stack.ItemOffsetC == 0)
				{
					OwnerIMC->OnEntryNewLink.Broadcast(OwnerIMC->IsBlackbox(*this), Index, Stack.CurrStackCount, Stack.Rotation, Stack.Instance);
				}
			}
		}
		Stack.LastObservedCount = Stack.CurrStackCount;
		Stack.LastObservedOffset = Stack.ItemOffsetR + Stack.ItemOffsetC;
	}
}

void FGASSInventoryList::SetLastObservedAtInitialization()
{
	UE_LOG(LogTemp, Error, TEXT("SetLastObserved Called"));
	for (FGASSInventoryEntry& Entry : Entries)
	{
		Entry.SetLastObservedAtInitialization();
	}
}

int32 FGASSInventoryList::GetNumOfEntry() const
{
	return Entries.Num();
}

void FGASSInventoryList::AddEntry(int32 EntryNum)
{
	check(OwnerComponent);

	AActor* OwningActor = OwnerComponent->GetOwner();

	check(OwningActor->HasAuthority());

	for (auto i = 0; i < EntryNum; ++i)
	{
		FGASSInventoryEntry& NewEntry = Entries.AddDefaulted_GetRef();
		NewEntry.InitializeInventoryEntry();
		MarkItemDirty(NewEntry);
	}
}

void FGASSInventoryList::DeleteEntry(int32 EntryNum)
{
	check(EntryNum <= Entries.Num());

	auto EntryIt = Entries.CreateIterator();
	EntryIt.SetToEnd();

	for (int32 i = 0; i < EntryNum; ++i)
	{
		EntryIt.RemoveCurrent();
		EntryIt--;
		MarkArrayDirty();
	}
}

UGASSInventoryItemInstance* FGASSInventoryList::CreateInstanceByDefinition(TSubclassOf<UGASSItemDefinition> ItemDef, uint8 StackCount)
{
	UGASSInventoryItemInstance* Result = nullptr;

	check(ItemDef != nullptr);
	check(OwnerComponent);

	AActor* OwningActor = OwnerComponent->GetOwner();

	check(OwningActor->HasAuthority());

	Result = NewObject<UGASSInventoryItemInstance>(OwnerComponent->GetOwner());
	Result->SetItemDef(ItemDef);
	Result->InitInventoryItemInstance(StackCount);
	for (UGASSItemFragment* Fragment : GetDefault<UGASSItemDefinition>(ItemDef)->Fragments)
	{
		if (Fragment != nullptr)
		{
			Fragment->OnInstanceCreated(Result);
		}
	}
		
	return Result;
}

void FGASSInventoryList::LinkInstanceWithEntry(UGASSInventoryItemInstance* Instance, uint16 Index, 
	uint8 OffsetR, uint8 OffsetC, bool Rotation, uint8 StackCount)
{
	if(Instance == nullptr) UE_LOG(LogTemp, Error, TEXT("Instance null"));

	FGASSInventoryEntry& TargetEntry = Entries[Index];

	TargetEntry.InitializeInventoryEntry();

	TargetEntry.Instance = Instance;
	TargetEntry.ItemOffsetR = OffsetR;
	TargetEntry.ItemOffsetC = OffsetC;
	TargetEntry.Rotation = Rotation;
	TargetEntry.CurrStackCount = StackCount;

	UGASSInventoryManagerComponent* OwnerIMC = Cast<UGASSInventoryManagerComponent>(OwnerComponent);

	if (OffsetR == 0 && OffsetC == 0)
	{
		if (OwnerIMC != nullptr)
		{
			OwnerIMC->AddWeight(TargetEntry.CurrStackCount * TargetEntry.Instance->IndItemWeight);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("OwnerIMC NULL"));
		}
	}

	MarkItemDirty(TargetEntry);

	return;
}

void FGASSInventoryList::CleanEntry(uint16 Index)
{
	UGASSInventoryManagerComponent* OwnerIMC = Cast<UGASSInventoryManagerComponent>(OwnerComponent);

	if (Entries[Index].ItemOffsetC == 0 && Entries[Index].ItemOffsetR == 0)
	{	
		if (Entries[Index].Instance != nullptr)
		{
			Entries[Index].Instance->CurrItemStackCount -= Entries[Index].CurrStackCount;
		}

		if (OwnerIMC != nullptr)
		{
			OwnerIMC->SubWeight((Entries[Index].Instance->IndItemWeight) * Entries[Index].CurrStackCount);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("OwnerIMC NULL"));
		}

	}

	Entries[Index].InitializeInventoryEntry();
	MarkItemDirty(Entries[Index]);
}

bool FGASSInventoryList::StackEntry(uint16 Index, uint8 StackAmount)
{
	if (Index > Entries.Num())
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot Stack Entry, Index is not Valid"));
		return false;
	}
	
	if (!(Entries[Index].StackEntry(StackAmount)))
	{
		return false;
	}

	MarkItemDirty(Entries[Index]);

	if (Entries[Index].ItemOffsetC == 0 && Entries[Index].ItemOffsetR == 0)
	{
		UGASSInventoryManagerComponent* OwnerIMC = Cast<UGASSInventoryManagerComponent>(OwnerComponent);

		if (OwnerIMC != nullptr)
		{
			OwnerIMC->AddWeight((Entries[Index].Instance->IndItemWeight) * StackAmount);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("OwnerIMC NULL"));
		}
	}

	return true;
}

bool FGASSInventoryList::UnStackEntry(uint16 Index, uint8 UnStackAmount)
{
	if (Index > Entries.Num())
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot Stack Entry, Index is not Valid"));
		return false;
	}

	if (!(Entries[Index].UnStackEntry(UnStackAmount)))
	{
		return false;
	}

	MarkItemDirty(Entries[Index]);

	if (Entries[Index].ItemOffsetC == 0 && Entries[Index].ItemOffsetR == 0)
	{
		UGASSInventoryManagerComponent* OwnerIMC = Cast<UGASSInventoryManagerComponent>(OwnerComponent);

		if (OwnerIMC != nullptr)
		{
			OwnerIMC->SubWeight((Entries[Index].Instance->IndItemWeight) * UnStackAmount);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("OwnerIMC NULL"));
		}
	}

	if (Entries[Index].CurrStackCount == 0)
	{
		Entries[Index].InitializeInventoryEntry();
	}

	return true;
}


//////////////////////////////////////////////////////////////////////
// FGASSEntryPQNode

bool FGASSEntryPQNode::operator<(const FGASSEntryPQNode Other) const
{
	return GetEntryStackCountByIndex() < Other.GetEntryStackCountByIndex();
}

uint8 FGASSEntryPQNode::GetEntryStackCountByIndex() const
{
	TObjectPtr<UGASSInventoryManagerComponent> MyIMC = Cast<UGASSInventoryManagerComponent>(OwnerComponent);
	if (MyIMC == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("PQNode Error"));
		return false;
	}
	return MyIMC->GetEntryStackCountByIndex(EntryIndex);
}

//////////////////////////////////////////////////////////////////////
// UGASSInventoryManagerComponent

UGASSInventoryManagerComponent::UGASSInventoryManagerComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, InventoryList(this)
	, Blackbox(this)
{
	SetIsReplicatedByDefault(true);
	bReplicateUsingRegisteredSubObjectList = true;
	UE_LOG(LogTemp, Warning, TEXT("IMC Const"));
}

void UGASSInventoryManagerComponent::GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, CounsumableFastAccess);
	DOREPLIFETIME(ThisClass, InventoryList);
	DOREPLIFETIME(ThisClass, Blackbox);
	DOREPLIFETIME(ThisClass, InventoryCurrWeight);
	DOREPLIFETIME(ThisClass, InventoryMaxWeight);
	DOREPLIFETIME(ThisClass, InventoryMaxCol);
	DOREPLIFETIME(ThisClass, InventoryMaxRow);
	DOREPLIFETIME(ThisClass, BlackboxMaxCol);
	DOREPLIFETIME(ThisClass, BlackboxMaxRow);
}

void UGASSInventoryManagerComponent::IMCTestFunc(TSubclassOf<UGASSItemDefinition> ItemDef)
{
	


}

void UGASSInventoryManagerComponent::PrintInventoryList(bool IsBlackbox)
{
	FGASSInventoryList& TargetList = IsBlackbox ? Blackbox : InventoryList;
	uint8 TargetMaxRow = IsBlackbox ? BlackboxMaxRow : InventoryMaxRow;
	uint8 TargetMaxCol = IsBlackbox ? BlackboxMaxCol : InventoryMaxCol;
	if (TargetList.Entries.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("Entries Num = 0"));
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("Print Inventory========================================="));
	for (auto i = 0; i < TargetMaxRow; ++i)
	{
		FString LogString = "";
		for (auto j = 0; j < TargetMaxCol; ++j)
		{
			int offset = TargetList.Entries[i * TargetMaxCol + j].CurrStackCount;
			char StackCount = '0';
			StackCount += offset;
			LogString += (StackCount);
		}
		UE_LOG(LogTemp, Warning, TEXT("%s"), *LogString);
	}
	UE_LOG(LogTemp, Warning, TEXT("Weight : %f"), InventoryCurrWeight);
	UE_LOG(LogTemp, Warning, TEXT("========================================================"));
}

//////////////////////////////////////////////////////////////////////
// Actor interaction

void UGASSInventoryManagerComponent::SetOwnerActor(AActor* NewOwnerActor)
{
	MARK_PROPERTY_DIRTY_FROM_NAME(UGASSInventoryManagerComponent, OwnerActor, this);
	if (OwnerActor)
	{

	}
	OwnerActor = NewOwnerActor;
	if (OwnerActor)
	{

	}
}

void UGASSInventoryManagerComponent::SetAvatarActor(AActor* NewAvatarActor)
{
	MARK_PROPERTY_DIRTY_FROM_NAME(UGASSInventoryManagerComponent, AvatarActor, this);
	if (AvatarActor)
	{

	}
	AvatarActor = NewAvatarActor;
	if (AvatarActor)
	{

	}
}

void UGASSInventoryManagerComponent::OnRep_OwningActor()
{

}

void UGASSInventoryManagerComponent::InitInventoryActorInfo(AActor* InOwnerActor, AActor* InAvatarActor)
{
	check(InOwnerActor);
	check(InAvatarActor);

	SetOwnerActor(InOwnerActor);
	SetAvatarActor(InAvatarActor);
}

//////////////////////////////////////////////////////////////////////
// Inventory interaction

//////////////////////////////////////////////////////////////////////
// Initialize Functions
int32 UGASSInventoryManagerComponent::GetInventorySize(bool bIsBlackbox) const
{
	if (bIsBlackbox)
	{
		return Blackbox.GetNumOfEntry();
	}
	else
	{
		return InventoryList.GetNumOfEntry();
	}
}

void UGASSInventoryManagerComponent::InitializeInventory(float MaxWeight, uint8 MaxCol, uint8 MaxRow)
{
	InventoryMaxWeight = MaxWeight;
	InventoryMaxCol = MaxCol;
	InventoryMaxRow = MaxRow;
	const FGASSEntryPQ dummyPQ;
	CounsumableFastAccess.Init(dummyPQ, 12);
	if (GetInventorySize(false))
	{
		UE_LOG(LogTemp, Warning, TEXT("InventoryComp. Inventory Already Exists!!"));
	}

	InventoryList.AddEntry(InventoryMaxCol * InventoryMaxRow);
}

void UGASSInventoryManagerComponent::InitializeBlackbox(uint8 MaxCol, uint8 MaxRow)
{
	BlackboxMaxCol = MaxCol;
	BlackboxMaxRow = MaxRow;

	if (GetInventorySize(true))
	{
		UE_LOG(LogTemp, Warning, TEXT("InventoryComp. Blackbox Already Exists!!"));
	}

	Blackbox.AddEntry(BlackboxMaxCol * BlackboxMaxRow);
}

bool UGASSInventoryManagerComponent::CanChangeInventory(uint8 NewMaxCol, uint8 NewMaxRow, float NewMaxWeight) const
{
	return (NewMaxWeight >= InventoryCurrWeight && NewMaxCol >= InventoryMaxCol && NewMaxRow >= InventoryMaxRow);
}

void UGASSInventoryManagerComponent::SetLastObservedAtInitialization()
{
	InventoryList.SetLastObservedAtInitialization();
	Blackbox.SetLastObservedAtInitialization();
}

//////////////////////////////////////////////////////////////////////
// Getter Functions

int32 UGASSInventoryManagerComponent::GetEntryStackCountByIndex(int32 Index) const
{
	return InventoryList.Entries[Index].GetCurrStackCount();
}

void UGASSInventoryManagerComponent::GetEveryBaseEntryInfo(bool IsBlackbox, TArray<uint32>& ItemsInfo, TArray<UGASSInventoryItemInstance*>& Instances) const
{
	const FGASSInventoryList& TargetList = IsBlackbox ? Blackbox : InventoryList;
	int32 MaxIndex = TargetList.Entries.Num();
	

	for (int32 i = 0; i < MaxIndex; ++i)
	{
		const FGASSInventoryEntry& TargetEntry = TargetList.Entries[i];
		if (TargetEntry.Instance && TargetEntry.CurrStackCount > 0 && TargetEntry.ItemOffsetR == 0 && TargetEntry.ItemOffsetC == 0)
		{
			uint32 ItemInfo = 0;
			int32 Amount = (int32)TargetEntry.CurrStackCount;

			ItemInfo += (i << 16);
			ItemInfo += (Amount << 8);
			if (TargetEntry.Rotation)
			{
				ItemInfo++;
			}

			ItemsInfo.Add(ItemInfo);
			Instances.Add(TargetEntry.Instance);

			//uint16 GetIndex = (uint16)(ItemInfo >> 16);
			//uint8 GetAmount = (uint8)(ItemInfo >> 8);
			//uint8 GetRotation = (uint8)(ItemInfo);

			//UE_LOG(LogTemp, Warning, TEXT("%d, %d, %d"), GetIndex, GetAmount, GetRotation);
		}
	}
}

UGASSInventoryItemInstance* UGASSInventoryManagerComponent::GetItemInstanceByIndex(bool IsBlackbox, int32 Index)
{
	FGASSInventoryList& TargetList = IsBlackbox ? Blackbox : InventoryList;
	check(Index < TargetList.Entries.Num());

	return TargetList.Entries[Index].Instance;
}

bool UGASSInventoryManagerComponent::IsBlackbox(FGASSInventoryList& List)
{
	if ((&List) == (&Blackbox))
		return true;
	return false;
}

//////////////////////////////////////////////////////////////////////
// Finding Functions

/*	Be AWARE : It should be Consumable, It will Return 255 if It is not Registered in map */
const uint8 UGASSInventoryManagerComponent::GetFastAccessIndexByTag(const FGameplayTag InTag) const
{
	UGameplayTagsManager& TagManager = UGameplayTagsManager::Get();
	const uint8* CustomIndex = nullptr;
	CustomIndex = GASSGameplayTags::CustomItemTagMap.Find(InTag);
	if (CustomIndex != nullptr)
	{
		return (*CustomIndex);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Tag is Not Registered in the Map"));
	}
	return (uint8)255;
}

uint16 UGASSInventoryManagerComponent::GetEntryBaseIndex(bool IsBlackbox, uint16 Index) const
{
	const FGASSInventoryList& TargetList = IsBlackbox ? Blackbox : InventoryList;
	uint8 MaxCol = IsBlackbox ? BlackboxMaxCol : InventoryMaxCol;

	TObjectPtr<UGASSInventoryItemInstance> Inst = TargetList.Entries[Index].Instance;
	uint8 ItemLonger(1), ItemShorter(1);
	if (Inst != nullptr)
	{
		ItemLonger = Inst->ItemSizeLonger;
		ItemShorter = Inst->ItemSizeShorter;

		uint8 Row = Index / MaxCol - TargetList.Entries[Index].ItemOffsetR;
		uint8 Col = Index % MaxCol - TargetList.Entries[Index].ItemOffsetC;

		return (uint16)Row * MaxCol + (uint16)Col;
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Error"));
		return 0;
	}	
}

bool UGASSInventoryManagerComponent::GetEveryEntryIndex(bool IsBlackbox, uint16 BaseIndex, TArray<uint16>& Indexes) const
{
	const FGASSInventoryList& TargetList = IsBlackbox ? Blackbox : InventoryList;
	TObjectPtr<UGASSInventoryItemInstance> Instance = TargetList.Entries[BaseIndex].Instance;
	if (Instance == nullptr) return false;

	uint8 RowMax(1), ColMax(1);

	uint16 BeforeNum = Indexes.Num();

	// True - 세로
	if (TargetList.Entries[BaseIndex].Rotation)
	{
		RowMax = Instance->ItemSizeLonger;
		ColMax = Instance->ItemSizeShorter;
	}
	else
	{
		RowMax = Instance->ItemSizeShorter;
		ColMax = Instance->ItemSizeLonger;
	}
	uint8 MaxCol = IsBlackbox ? BlackboxMaxCol : InventoryMaxCol;
	uint16 MaxIndex = IsBlackbox ? Blackbox.GetNumOfEntry() : InventoryList.GetNumOfEntry();
	uint8 BaseRow = BaseIndex / MaxCol;
	uint8 BaseCol = BaseIndex % MaxCol;

	for (auto i = 0; i < RowMax; ++i)
	{
		for (auto j = 0; j < ColMax; ++j)
		{
			uint16 TargetIndex = ((uint16)BaseRow + i) * (uint16)MaxCol + ((uint16)BaseCol + j);
			if (TargetIndex >= MaxIndex)
			{
				return false;
				UE_LOG(LogTemp, Warning, TEXT("Indexing Error"));
			}
			Indexes.Add(TargetIndex);
		}
	}
	
	return Indexes.Num() > BeforeNum;
}

bool UGASSInventoryManagerComponent::GetEveryEntryIndex(bool IsBlackbox, UGASSInventoryItemInstance* Instance,
	uint16 BaseIndex, bool Rotation, TArray<uint16>& Indexes, TArray<uint16>& OffsetInfo) const
{
	if (Instance == nullptr) return false;

	uint8 RowMax(1), ColMax(1);

	uint16 BeforeNum = Indexes.Num();

	// True - 세로
	if (Rotation)
	{
		RowMax = Instance->ItemSizeLonger;
		ColMax = Instance->ItemSizeShorter;
	}
	else
	{
		RowMax = Instance->ItemSizeShorter;
		ColMax = Instance->ItemSizeLonger;
	}

	uint8 MaxCol = IsBlackbox ? BlackboxMaxCol : InventoryMaxCol;
	uint16 MaxIndex = IsBlackbox ? Blackbox.GetNumOfEntry() : InventoryList.GetNumOfEntry();
	uint8 BaseRow = BaseIndex / MaxCol;
	uint8 BaseCol = BaseIndex % MaxCol;

	for (auto i = 0; i < RowMax; ++i)
	{
		for (auto j = 0; j < ColMax; ++j)
		{
			uint16 TargetIndex = ((uint16)BaseRow + i)* (uint16)MaxCol + ((uint16)BaseCol + j);
			if (TargetIndex >= MaxIndex)
			{
				return false;
				UE_LOG(LogTemp, Warning, TEXT("Indexing Error"));
			}
				
			Indexes.Add(TargetIndex);
			uint16 Offset = (i << 8);
			Offset += j;
			OffsetInfo.Add(Offset);
		}
	}

	return Indexes.Num() > BeforeNum;
}

bool UGASSInventoryManagerComponent::FindEntryByTagAndSize(bool bOnlyOne, bool IsBlackbox, TArray<uint16>& Indexes, const FGameplayTag InTag, const uint8 shorter, const uint8 longer) const
{

	uint8 MaxCol = IsBlackbox ? BlackboxMaxCol : InventoryMaxCol;
	uint8 MaxRow = IsBlackbox ? BlackboxMaxRow : InventoryMaxRow;

	uint16 MaxIndex = IsBlackbox ? Blackbox.GetNumOfEntry() : InventoryList.GetNumOfEntry();
	uint16 IndexToFind(0);
	
	const FGASSInventoryList& TargetList = IsBlackbox ? Blackbox : InventoryList;

	TArray<uint16> Visited;

	uint8 SearchMode;

	if (shorter == 1)
	{
		if (longer == 1) SearchMode = 0;
		else SearchMode = 1;
	}
	else
	{
		SearchMode = 2;
	}

	auto SetRCFromIndex = [MaxCol](uint16 Index, uint8& r, uint8& c)
	{
		r = Index / MaxCol;
		c = Index % MaxCol;
	};

	auto SetNextIndex = [SetRCFromIndex, SearchMode, longer, shorter, MaxCol](uint16& Index)
	{
		if (SearchMode == 0) Index++;
		else
		{
			uint8 PinR, PinC;
			SetRCFromIndex(Index, PinR, PinC);
			if (SearchMode == 1)
			{
				PinC += longer;
				if (PinC >= MaxCol)
				{
					PinR++;
					PinC = PinR % longer;
				}
				Index = (uint16)PinR * MaxCol + (uint16)PinC;
			}
			else if (SearchMode == 2)
			{
				PinC += shorter;
				if (PinC >= MaxCol)
				{
					PinR += shorter;
					PinC = (PinR / shorter) % shorter;
				}
				Index = (uint16)PinR * MaxCol + (uint16)PinC;
			}
		}
	};

	while (IndexToFind < MaxIndex)
	{
		TObjectPtr<UGASSInventoryItemInstance> InstanceToCheck = TargetList.Entries[IndexToFind].Instance;
		if (InstanceToCheck != nullptr)
		{
			if (InstanceToCheck->ItemTag == InTag)
			{
				bool NeedSave = true;

				if (Visited.Num())
					if (Visited.Find(IndexToFind) != INDEX_NONE)
						NeedSave = false;

				if (NeedSave)
				{
					uint16 idx = GetEntryBaseIndex(IsBlackbox, IndexToFind);
					Indexes.Add(idx);
					GetEveryEntryIndex(IsBlackbox, idx, OUT Visited);
					if (bOnlyOne) return true;
				}
			}
		}
		
		SetNextIndex(IndexToFind);
	}
	if (Indexes.Num()) return true;
	else return false;
}

bool UGASSInventoryManagerComponent::GetPossibleLocation(uint8& PositionR, uint8& PositionC, bool& Rotation, const uint8 shorter, const uint8 longer) const
{
	uint8 PinR = 0;
	uint8 PinC = 0;
	uint8 CJumpTo = 0;

	uint8 MaxCol = InventoryMaxCol;
	uint8 MaxRow = InventoryMaxRow;
	const TArray<FGASSInventoryEntry>* const Inv = &(InventoryList.Entries);

	auto Get_Index = [MaxCol](uint8 r, uint8 c) -> uint16
	{
		return MaxCol * (uint16)r + (uint16)c;
	};

	auto Check_Grid = [Get_Index, &CJumpTo, Inv](uint8 pinR, uint8 pinC, uint8 row, uint8 col) -> bool
	{
		for (auto i = 0; i < col; ++i)
		{
			for (auto j = 0; j < row; ++j)
			{
				if ((*Inv)[Get_Index(pinR + j, pinC + i)].Instance != nullptr)
				{
					CJumpTo = pinC + i + 1;
					return false;
				}

			}
		}
		return true;
	};


	if (MaxRow >= longer)
	{
		while (PinR <= MaxRow - longer)
		{
			if (PinC <= MaxCol - longer)
			{
				if (Check_Grid(PinR, PinC, shorter, shorter))
				{
					if (Check_Grid(PinR + shorter, PinC, longer - shorter, shorter))
					{
						PositionR = PinR;	PositionC = PinC;	Rotation = true;
						return true;
					}
					if (Check_Grid(PinR, PinC + shorter, shorter, longer - shorter))
					{
						PositionR = PinR;	PositionC = PinC;	Rotation = false;
						return true;
					}
					PinC++;
				}
				else
				{
					PinC = CJumpTo;
				}
			}
			else if (PinC <= MaxCol - shorter)
			{
				if (Check_Grid(PinR, PinC, longer, shorter))
				{
					PositionR = PinR;	PositionC = PinC;	Rotation = true;
					return true;
				}
				PinC++;
			}
			else
			{
				PinC = 0;
				PinR++;
			}

		}
	}
	else PinR = 0;
	while (PinR <= MaxRow - shorter)
	{
		PinC = 0;
		while (PinC <= MaxCol - longer)
		{
			if (Check_Grid(PinR, PinC, shorter, longer))
			{
				PositionR = PinR;	PositionC = PinC;	Rotation = false;
				return true;
			}
			else
			{
				PinC = CJumpTo;
			}
		}
		PinR++;
	}
	return false;
}

//////////////////////////////////////////////////////////////////////
// Instance Functions

UGASSInventoryItemInstance* UGASSInventoryManagerComponent::CreateInstanceByDefinition(TSubclassOf<UGASSItemDefinition> ItemDef, uint8 StackCount)
{
	check(OwnerActor->HasAuthority());
	UGASSInventoryItemInstance* Result = nullptr;
	if (ItemDef != nullptr)
	{
		Result = InventoryList.CreateInstanceByDefinition(ItemDef, StackCount);

		if (IsUsingRegisteredSubObjectList() && IsReadyForReplication() && Result)
		{
			AddReplicatedSubObject(Result);
		}
	}

	return Result;
}

UGASSInventoryItemInstance* UGASSInventoryManagerComponent::FindInstanceByRegisteredList(TSubclassOf<UGASSItemDefinition> ItemDef)
{
	AGASSPlayerController* GASSPC = Cast<AGASSPlayerController>(OwnerActor);
	
	// Character's IMC
	UGASSInventoryItemInstance* TargetInstance = nullptr;
	if (GASSPC != nullptr)
	{
		TargetInstance = const_cast<UGASSInventoryItemInstance*>(GASSPC->GetInventoryInstance(ItemDef));
	}
	// Storage's IMC
	else
	{
		TargetInstance = const_cast<UGASSInventoryItemInstance*>(UStorageStatics::GetInventoryInstance(OwnerActor, ItemDef));
	}

	return TargetInstance;
}

void UGASSInventoryManagerComponent::RemoveInstanceFromRegisteredList(UGASSInventoryItemInstance* InItemInstance)
{
	if (InItemInstance && IsUsingRegisteredSubObjectList())
	{
		RemoveReplicatedSubObject(InItemInstance);
	}
}

void UGASSInventoryManagerComponent::LinkInstanceWithEntries(UGASSInventoryItemInstance* Instance, uint8 StackCount,
	uint8 BasePositionR, uint8 BasePositionC, bool Rotation, bool IsBlackbox)
{
	uint8 MaxCol = IsBlackbox ? BlackboxMaxCol : InventoryMaxCol;
	uint16 MaxIndex = IsBlackbox ? Blackbox.GetNumOfEntry() : InventoryList.GetNumOfEntry();
	FGASSInventoryList& TargetList = IsBlackbox ? Blackbox : InventoryList;

	uint16 BaseIndex = MaxCol * (uint16)BasePositionR + (uint16)BasePositionC;
	TArray<uint16> IndexesToLink;
	TArray<uint16> Offsets;


	if (BaseIndex < MaxIndex && Instance)
	{
		if (!GetEveryEntryIndex(IsBlackbox, Instance, BaseIndex, Rotation, OUT IndexesToLink, OUT Offsets))
		{
			UE_LOG(LogTemp, Error, TEXT("UGASSInventoryManagerComponent::LinkInstanceWithEntries Error"));
			return;
		}
	}
	if (IndexesToLink.Num() == 0)
	{
		UE_LOG(LogTemp, Error, TEXT("UGASSInventoryManagerComponent::LinkInstanceWithEntries Error"));
		return;
	}
	
	for (auto i = 0; i < IndexesToLink.Num(); ++i)
	{
		if (IndexesToLink[i] < MaxIndex)
		{
			TargetList.LinkInstanceWithEntry(Instance, IndexesToLink[i], (Offsets[i] >> 8), Offsets[i] % (1 << 8), Rotation, StackCount);
		}
	}
}

//////////////////////////////////////////////////////////////////////
// Adding Functions

// Why Return is uint8?
/*	Assure That Input to this Function should be in ItemCondition (StackCount <= ItemDef->MaxStackCount)	*/
uint8 UGASSInventoryManagerComponent::AutoAddItemDefinition(const TSubclassOf<UGASSItemDefinition> ItemDef, uint8 StackCount)
{
	check(OwnerActor->HasAuthority());

	const UGASSItemDefinition* ItemCDO = GetDefault<UGASSItemDefinition>(ItemDef);
	
	const FGameplayTag ItemCDOTag = ItemCDO->ItemTag;
	UGameplayTagsManager& TagManager = UGameplayTagsManager::Get();

	uint8 ItemSizeLonger = ItemCDO->ItemSizeLonger;
	uint8 ItemSizeShorter = ItemCDO->ItemSizeShorter;
	float ItemWeight = ItemCDO->ItemWeight;

	bool bIsFullStack = false;
	uint8 StackAmount = StackCount;

	TObjectPtr<UGASSInventoryItemInstance> TargetInstance = nullptr;

	// Weight Check
	if (ItemWeight * StackCount + InventoryCurrWeight > InventoryMaxWeight)
	{
		StackAmount = (uint8)(floor)((InventoryMaxWeight - InventoryCurrWeight) / ItemWeight);
		if (StackAmount == 0)
		{
			UE_LOG(LogTemp, Error, TEXT("Weight Over"));
			return 0;
		}
	}

	// FullStack Check
	if (!(ItemCDO->bCanStack) || StackAmount == ItemCDO->MaxStackCount)
	{
		bIsFullStack = true;
	}

	bool NeedToUpdateFastAccess = false;
	uint8 ProcessedAmount = StackAmount;
	uint8 FastAccessIndex = -1;
	TArray<uint16> EntryIndexesToStack;
	TArray<uint8> EntriesToStackAmount;

	// Check Existing Entries to Stack
	if (!bIsFullStack)
	{
		// Check Stackable Entry & Instance
		const FName ItemConsumable = FName(TEXT("Item.Consumable"));
		const FGameplayTag ConsumableTag = TagManager.RequestGameplayTag(ItemConsumable);
		
		if (ItemCDOTag.MatchesTag(ConsumableTag))
		{
			FastAccessIndex =  GetFastAccessIndexByTag(ItemCDOTag);
			if (FastAccessIndex < CounsumableFastAccess.Num())
			{
				NeedToUpdateFastAccess = true;
				FGASSEntryPQ faPQ = CounsumableFastAccess[FastAccessIndex];
				faPQ.UpdateSelf();
				if (!faPQ.IsEmpty())
				{
					//	There is Instance, Check How many can stack to Entry
					TargetInstance = InventoryList.Entries[faPQ.Top().GetEntryIndex()].Instance;
					//TargetInstance = faPQ.Top().GetEntryInstance();

					// 무슨 Entry에 몇개씩 들어갈 수 있는지, 남은 개수는 몇개인지
					uint16 FastAccessPQSize = faPQ.GetSize();
					for (int32 i = FastAccessPQSize - 1; i >=0; i--)
					{
						uint8 Stackable = ItemCDO->MaxStackCount - faPQ.GetNodeByIndexing(i).GetEntryStackCountByIndex();
						if (Stackable == 0) continue;
						if (StackAmount - Stackable > 0)
						{
							StackAmount -= Stackable;
							EntriesToStackAmount.Add(Stackable);
							EntryIndexesToStack.Add(faPQ.GetNodeByIndexing(i).GetEntryIndex());
						}
						else
						{
							EntriesToStackAmount.Add(StackAmount);
							EntryIndexesToStack.Add(faPQ.GetNodeByIndexing(i).GetEntryIndex());
							StackAmount = 0;
							break;
						}
					}
				}
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("ERROR : Consumable But Not In Map"));
			}		
		}
		else
		{
			TArray<uint16> EveryEntriesBasePoint;
			FindEntryByTagAndSize(false, false, OUT EveryEntriesBasePoint, ItemCDOTag, ItemSizeShorter, ItemSizeLonger);
			uint16 EntriesSize = EveryEntriesBasePoint.Num();
			if (EntriesSize > 0)
			{
				FGASSEntryPQ PQ;
				for (uint16 i = 0; i < EntriesSize; ++i)
				{
					FGASSEntryPQNode NewNode(EveryEntriesBasePoint[i], this);
					PQ.Push(NewNode);					
				}
				TargetInstance = InventoryList.Entries[PQ.Top().GetEntryIndex()].Instance;
				for (int32 i = EntriesSize - 1; i >= 0; --i)
				{
					uint8 Stackable = ItemCDO->MaxStackCount - PQ.GetNodeByIndexing(i).GetEntryStackCountByIndex();
					// Entry
					if (Stackable == 0) continue;
					if (StackAmount - Stackable > 0)
					{
						StackAmount -= Stackable;
						EntriesToStackAmount.Add(Stackable);
						EntryIndexesToStack.Add(PQ.GetNodeByIndexing(i).GetEntryIndex());
					}
					else
					{
						EntriesToStackAmount.Add(StackAmount);
						EntryIndexesToStack.Add(PQ.GetNodeByIndexing(i).GetEntryIndex());
						StackAmount = 0;
						break;
					}
					
				}
			}
		}
	}
	
	// Don't need to New Link Entry
	if (StackAmount == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("Stacking Only"));
		for (auto i = 0; i < EntryIndexesToStack.Num(); ++i)
		{
			
			if (InventoryList.StackEntry(EntryIndexesToStack[i], EntriesToStackAmount[i]))
			{
				TArray<uint16> OtherEntries;
				if (GetEveryEntryIndex(false, EntryIndexesToStack[i], OUT OtherEntries))
				{
					for (uint16 OtherIndex : OtherEntries)
					{
						if (OtherIndex == EntryIndexesToStack[i])
							continue;

						// TODO : Do i have to maintain StackCount for Non-Base Indexes?
						InventoryList.StackEntry(OtherIndex, EntriesToStackAmount[i]);
					}
				}
			}
		}
		
		// FastAccess update
		if (NeedToUpdateFastAccess)
		{
			check(FastAccessIndex >= 0 && FastAccessIndex < CounsumableFastAccess.Num());
			CounsumableFastAccess[FastAccessIndex].UpdateSelf();
		}
			

		return ProcessedAmount;
	}

	// Potential Entry Position Check
	uint8 NewPositionR(0), NewPositionC(0);
	bool NewRotation(false);
	if (GetPossibleLocation(OUT NewPositionR, OUT NewPositionC, OUT NewRotation, ItemSizeShorter, ItemSizeLonger))
	{
		// Create Instance
		if (TargetInstance == nullptr)
		{
			TargetInstance = FindInstanceByRegisteredList(ItemDef);

			if (TargetInstance != nullptr)
			{
				if (!(TargetInstance->bCanItemStack))
				{
					TargetInstance = nullptr;
				}
			}

			if (TargetInstance == nullptr)
			{
				UE_LOG(LogTemp, Warning, TEXT("Create TargetInstance"));
				TargetInstance = CreateInstanceByDefinition(ItemDef, StackAmount);
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("Found TargetInstance"));
				TargetInstance->StackItem(StackAmount);
			}
		}
		
		// Entry - Instance 연결
		LinkInstanceWithEntries(TargetInstance, StackAmount, NewPositionR, NewPositionC, NewRotation, false);
		//InventoryCurrWeight += ItemWeight * StackAmount;

		for (auto i = 0; i < EntryIndexesToStack.Num(); ++i)
		{
			if (InventoryList.StackEntry(EntryIndexesToStack[i], EntriesToStackAmount[i]))
			{
				//InventoryCurrWeight += ItemWeight * EntriesToStackAmount[i];
				TArray<uint16> OtherEntries;
				if (GetEveryEntryIndex(false, EntryIndexesToStack[i], OUT OtherEntries))
				{
					for (uint16 OtherIndex : OtherEntries)
					{
						if (OtherIndex == EntryIndexesToStack[i])
							continue;
						// TODO : Do i have to maintain StackCount for Non-Base Indexes?
						InventoryList.StackEntry(OtherIndex, EntriesToStackAmount[i]);
					}
				}
			}
		}

		//// FastAccess update
		if (NeedToUpdateFastAccess)
		{
			if (FastAccessIndex < CounsumableFastAccess.Num())
			{
				FGASSEntryPQNode NewNode((uint16)NewPositionR * (uint16)InventoryMaxCol + (uint16)NewPositionC, this);
				CounsumableFastAccess[FastAccessIndex].Push(NewNode);
			}
		}
		return ProcessedAmount;
	}

	UE_LOG(LogTemp, Warning, TEXT("Position Not Found"));
	// 지금은 일단 다 처리 못하면 아예 안함
	return 0;
}

void UGASSInventoryManagerComponent::AddItemInstanceAt(UGASSInventoryItemInstance* Instance, uint8 StackCount, uint8 BasePositionR, uint8 BasePositionC, bool Rotation, bool IsBlackbox)
{
	check(OwnerActor->HasAuthority());
	uint8 TargetMaxCol = IsBlackbox ? BlackboxMaxCol : InventoryMaxCol;
	uint8 TargetMaxRow = IsBlackbox ? BlackboxMaxRow : InventoryMaxRow;
	FGASSInventoryList& TargetList = IsBlackbox ? Blackbox : InventoryList;
	
	uint8 StackAmount = StackCount;
	float ItemWeight = Instance->IndItemWeight;

	TObjectPtr<UGASSInventoryItemInstance> TargetInstance = nullptr;

	// Weight Check
	if (ItemWeight * StackAmount + InventoryCurrWeight > InventoryMaxWeight)
	{
		StackAmount = (uint8)(floor)((InventoryMaxWeight - InventoryCurrWeight) / ItemWeight);
		if (StackAmount == 0)
		{
			UE_LOG(LogTemp, Warning, TEXT("Weight Over"));
			return;
		}
	}

	TArray<uint16> TargetIndex;
	if (FindEntryByTagAndSize(true, IsBlackbox, OUT TargetIndex, Instance->ItemTag, Instance->ItemSizeShorter, Instance->ItemSizeLonger))
	{
		TargetInstance = TargetList.Entries[TargetIndex[0]].Instance;
	}

	if (TargetInstance != nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("Found TargetInstance"));
		TargetInstance->StackInstance(Instance, StackAmount);
	}
	else
	{
		TargetInstance = FindInstanceByRegisteredList(Instance->GetItemDef());

		if (TargetInstance != nullptr)
		{
			if (!(TargetInstance->bCanItemStack))
			{
				TargetInstance = nullptr;
			}
		}

		if (TargetInstance == nullptr)
		{
			UE_LOG(LogTemp, Warning, TEXT("Create TargetInstance"));
			TargetInstance = CreateInstanceByDefinition(Instance->GetItemDef(), StackAmount);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Found TargetInstance"));
			TargetInstance->StackItem(StackAmount);
		}
	}

	LinkInstanceWithEntries(TargetInstance, StackAmount, BasePositionR, BasePositionC, Rotation, IsBlackbox);
	
	if (IsBlackbox)
		return;

	UGameplayTagsManager& TagManager = UGameplayTagsManager::Get();
	const FName ItemConsumable = FName(TEXT("Item.Consumable"));
	const FGameplayTag ConsumableTag = TagManager.RequestGameplayTag(ItemConsumable);
	uint8 FastAccessIndex;
	FGameplayTag ItemCDOTag = Instance->ItemTag;

	if (ItemCDOTag.MatchesTag(ConsumableTag))
	{
		FastAccessIndex = GetFastAccessIndexByTag(ItemCDOTag);
		if (FastAccessIndex < CounsumableFastAccess.Num())
		{
			FGASSEntryPQNode NewNode((uint16)BasePositionR * (uint16)TargetMaxCol + (uint16)BasePositionC, this);
			CounsumableFastAccess[FastAccessIndex].Push(NewNode);
		}
	}
}

void UGASSInventoryManagerComponent::AddItemAt(TSubclassOf<UGASSItemDefinition> ItemDef, uint8 StackCount, int32 Index, bool Rotation, bool IsBlackbox)
{
	check(OwnerActor->HasAuthority());
	uint8 TargetMaxCol = IsBlackbox ? BlackboxMaxCol : InventoryMaxCol;
	uint8 TargetMaxRow = IsBlackbox ? BlackboxMaxRow : InventoryMaxRow;
	FGASSInventoryList& TargetList = IsBlackbox ? Blackbox : InventoryList;

	const UGASSItemDefinition* ItemCDO = GetDefault<UGASSItemDefinition>(ItemDef);

	uint8 StackAmount = StackCount;
	float ItemWeight = ItemCDO->ItemWeight;

	TObjectPtr<UGASSInventoryItemInstance> TargetInstance = nullptr;

	if (ItemWeight * StackAmount + InventoryCurrWeight > InventoryMaxWeight)
	{
		StackAmount = (uint8)(floor)((InventoryMaxWeight - InventoryCurrWeight) / ItemWeight);
		if (StackAmount == 0)
		{
			UE_LOG(LogTemp, Warning, TEXT("Weight Over"));
			return;
		}
	}

	TargetInstance = CreateInstanceByDefinition(ItemDef, StackAmount);

	uint8 BasePositionR = Index / TargetMaxCol;
	uint8 BasePositionC = Index % TargetMaxCol;
	LinkInstanceWithEntries(TargetInstance, StackAmount, BasePositionR, BasePositionC, Rotation, IsBlackbox);

	return;
}
//////////////////////////////////////////////////////////////////////
// Deleting Functions

void UGASSInventoryManagerComponent::DetachInstance(uint16 BaseIndex, bool IsBlackbox)
{
	UE_LOG(LogTemp, Warning, TEXT("Detach Instance Called"));
	TArray<uint16> IndexesToDetach;
	if (!GetEveryEntryIndex(IsBlackbox, BaseIndex, OUT IndexesToDetach))
	{
		UE_LOG(LogTemp, Error, TEXT("Cannot GetEveryEntryIndex"));
		return;
	}
	
	FGASSInventoryList& TargetList = IsBlackbox ? Blackbox : InventoryList;
	
	for (uint16 Index : IndexesToDetach)
	{
		TargetList.CleanEntry(Index);
	}
}

//////////////////////////////////////////////////////////////////////
// Stack / UnStack Functions

bool UGASSInventoryManagerComponent::StackEntries(uint16 BaseIndex, uint8 StackCount, bool IsBlackbox)
{
	FGASSInventoryList& TargetList = IsBlackbox ? Blackbox : InventoryList;
	TArray<uint16> TargetIndexes;
	if (!GetEveryEntryIndex(IsBlackbox, BaseIndex, OUT TargetIndexes))
	{
		return false;
	}

	for (uint16 TargetIndex : TargetIndexes)
	{
		if (!(TargetList.StackEntry(TargetIndex, StackCount)))
		{
			return false;
		}
	}

	return true;
}

bool UGASSInventoryManagerComponent::UnstackEntries(uint16 BaseIndex, uint8 StackCount, bool IsBlackbox)
{
	FGASSInventoryList& TargetList = IsBlackbox ? Blackbox : InventoryList;
	TArray<uint16> TargetIndexes;
	if (!GetEveryEntryIndex(IsBlackbox, BaseIndex, OUT TargetIndexes))
	{
		return false;
	}

	for (uint16 TargetIndex : TargetIndexes)
	{
		if (!(TargetList.UnStackEntry(TargetIndex, StackCount)))
		{
			return false;
		}
	}

	return true;
}

//////////////////////////////////////////////////////////////////////
// Weight Management Functions

bool UGASSInventoryManagerComponent::CanAddWeight(float AddWeight)
{
	return InventoryCurrWeight + AddWeight <= InventoryMaxWeight;
}

bool UGASSInventoryManagerComponent::AddWeight(float AddWeight)
{
	if (InventoryCurrWeight + AddWeight <= InventoryMaxWeight)
	{
		InventoryCurrWeight += AddWeight;
		return true;
	}
	return false;
}

bool UGASSInventoryManagerComponent::SubWeight(float SubWeight)
{
	if (InventoryCurrWeight - SubWeight >= 0)
	{
		InventoryCurrWeight -= SubWeight;
		return true;
	}
	return false;
}

//////////////////////////////////////////////////////////////////////
// Interaction with Widget

uint8 UGASSInventoryManagerComponent::CanPlaceItem(bool NeedToWeightCheck, TSubclassOf<UGASSItemDefinition> ItemDef, int32 StackAmount, bool IsBlackbox)
{
	const UGASSItemDefinition* ItemCDO = GetDefault<UGASSItemDefinition>(ItemDef);

	const FGameplayTag ItemCDOTag = ItemCDO->ItemTag;
	UGameplayTagsManager& TagManager = UGameplayTagsManager::Get();

	uint8 ItemSizeLonger = ItemCDO->ItemSizeLonger;
	uint8 ItemSizeShorter = ItemCDO->ItemSizeShorter;
	float ItemWeight = ItemCDO->ItemWeight;

	uint8 StackableAmount = StackAmount;
	bool bIsFullStack = false;

	TObjectPtr<UGASSInventoryItemInstance> TargetInstance = nullptr;

	// Weight Check
	if (NeedToWeightCheck)
	{
		if (ItemWeight * StackAmount + InventoryCurrWeight > InventoryMaxWeight)
		{
			StackableAmount = (uint8)(floor)((InventoryMaxWeight - InventoryCurrWeight) / ItemWeight);
			if (StackableAmount == 0)
			{
				UE_LOG(LogTemp, Warning, TEXT("Weight Over"));
				return 0;
			}
		}
	}

	// FullStack Check
	if (!(ItemCDO->bCanStack) || StackAmount == ItemCDO->MaxStackCount)
	{
		bIsFullStack = true;
	}

	uint8 NeedToStackAmount = StackableAmount;
	TArray<uint16> EntryIndexesToStack;
	TArray<uint8> EntriesToStackAmount;

	if (!bIsFullStack)
	{
		TArray<uint16> EveryEntriesBasePoint;
		FindEntryByTagAndSize(false, false, OUT EveryEntriesBasePoint, ItemCDOTag, ItemSizeShorter, ItemSizeLonger);
		uint16 EntriesSize = EveryEntriesBasePoint.Num();
		if (EntriesSize > 0)
		{
			FGASSEntryPQ PQ;
			for (uint16 i = 0; i < EntriesSize; ++i)
			{
				FGASSEntryPQNode NewNode(EveryEntriesBasePoint[i], this);
				PQ.Push(NewNode);
			}
			TargetInstance = InventoryList.Entries[PQ.Top().GetEntryIndex()].Instance;
			for (int32 i = EntriesSize - 1; i >= 0; --i)
			{
				uint8 Stackable = ItemCDO->MaxStackCount - PQ.GetNodeByIndexing(i).GetEntryStackCountByIndex();
				// Entry
				if (Stackable == 0) continue;
				if (NeedToStackAmount - Stackable > 0)
				{
					NeedToStackAmount -= Stackable;
					EntriesToStackAmount.Add(Stackable);
					EntryIndexesToStack.Add(PQ.GetNodeByIndexing(i).GetEntryIndex());
				}
				else
				{
					EntriesToStackAmount.Add(NeedToStackAmount);
					EntryIndexesToStack.Add(PQ.GetNodeByIndexing(i).GetEntryIndex());
					NeedToStackAmount = 0;
					break;
				}

			}
		}
	}

	if (NeedToStackAmount == 0)
	{
		return StackableAmount;
	}

	uint8 NewPositionR(0), NewPositionC(0);
	bool NewRotation(false);
	if (GetPossibleLocation(OUT NewPositionR, OUT NewPositionC, OUT NewRotation, ItemSizeShorter, ItemSizeLonger))
	{
		return StackableAmount;
	}
	return StackableAmount - NeedToStackAmount;
}

bool UGASSInventoryManagerComponent::CanPlaceItemAt(bool NeedToWeightCheck, int32 ColStart, int32 RowStart, int32 ColSize, int32 RowSize,
	TSubclassOf<UGASSItemDefinition> ItemDef, int32 StackAmount, bool IsBlackbox)
{
	uint8 TargetMaxCol = IsBlackbox ? BlackboxMaxCol : InventoryMaxCol;
	FGASSInventoryList& TargetList = IsBlackbox ? Blackbox : InventoryList;

	const UGASSItemDefinition* ItemCDO = GetDefault<UGASSItemDefinition>(ItemDef);

	if (NeedToWeightCheck)
	{
		if ((ItemCDO->ItemWeight * StackAmount) + InventoryCurrWeight > InventoryMaxWeight)
			return false;
	}
	
	for (auto r = RowStart; r < RowStart + RowSize; ++r)
	{
		for (auto c = ColStart; c < ColStart + ColSize; ++c)
		{
			uint16 TargetIndex = TargetMaxCol * r + c;
			if (TargetList.Entries[TargetIndex].Instance != nullptr)
				return false;
		}
	}

	return true;
}

bool UGASSInventoryManagerComponent::CanMoveMyItemAt(int32 InitialIndex, int32 NewIndex, bool NewRotation, bool IsBlackbox)
{
	uint8 TargetMaxCol = IsBlackbox ? BlackboxMaxCol : InventoryMaxCol;
	FGASSInventoryList& TargetList = IsBlackbox ? Blackbox : InventoryList;
	
	if (TargetList.Entries[InitialIndex].Instance == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("CanMoveMyItemAt :: Instance Nullptr"));
		return false;
	}

	TArray<uint16> InitialIndexes;
	check(GetEveryEntryIndex(IsBlackbox, InitialIndex, OUT InitialIndexes));

	uint8 ItemSizeLonger = TargetList.Entries[InitialIndex].Instance->ItemSizeLonger;
	uint8 ItemSizeShorter = TargetList.Entries[InitialIndex].Instance->ItemSizeShorter;

	uint8 RowStart = NewIndex / TargetMaxCol;
	uint8 ColStart = NewIndex % TargetMaxCol;
	uint8 RowSize = NewRotation ? ItemSizeLonger : ItemSizeShorter;
	uint8 ColSize = NewRotation ? ItemSizeShorter : ItemSizeLonger;

	for (auto r = RowStart; r < RowStart + RowSize; ++r)
	{
		for (auto c = ColStart; c < ColStart + ColSize; ++c)
		{
			uint16 TargetIndex = TargetMaxCol * r + c;
			if (TargetList.Entries[TargetIndex].Instance != nullptr)
			{
				int32 IndexFound = 0;
				if (InitialIndexes.Find(TargetIndex, OUT IndexFound))
				{
					InitialIndexes.RemoveAt(IndexFound);
				}
				else
				{
					return false;
				}
			}
		}
	}

	return true;
}

//////////////////////////////////////////////////////////////////////
// Server Check for RPC

bool UGASSInventoryManagerComponent::ServerCheckIMCEnoughItem(TSubclassOf<UGASSItemDefinition> ItemDef, int32 Index, int32 StackAmount, bool IsBlackbox)
{
	check(OwnerActor->HasAuthority());
	FGASSInventoryList& TargetList = IsBlackbox ? Blackbox : InventoryList;
	
	if (Index < 0 || Index >= TargetList.GetNumOfEntry())
	{
		return false;
	}
	
	if (TargetList.Entries[Index].CurrStackCount < StackAmount)
	{
		return false;
	}

	if (TargetList.Entries[Index].Instance == nullptr)
	{
		return false;
	}

	if (TargetList.Entries[Index].Instance->GetItemDef() != ItemDef)
	{
		return false;
	}

	return true;
}

bool UGASSInventoryManagerComponent::ServerCheckIMCCanPlaceItem(bool NeedToWeightCheck, TSubclassOf<UGASSItemDefinition> ItemDef, int32 Index, int32 StackAmount, bool Rotation, bool IsBlackbox)
{
	check(OwnerActor->HasAuthority());
	const UGASSItemDefinition* ItemCDO = GetDefault<UGASSItemDefinition>(ItemDef);
	int32 TargetMaxCol = IsBlackbox ? (BlackboxMaxCol) : (InventoryMaxCol);
	int32 ColSize = Rotation ? (ItemCDO->ItemSizeShorter) : (ItemCDO->ItemSizeLonger);
	int32 RowSize = Rotation ? (ItemCDO->ItemSizeLonger) : (ItemCDO->ItemSizeShorter);
	
	return CanPlaceItemAt(NeedToWeightCheck, Index % TargetMaxCol, Index / TargetMaxCol, ColSize, RowSize, ItemDef, StackAmount, IsBlackbox);
}

//////////////////////////////////////////////////////////////////////
// RPC

bool UGASSInventoryManagerComponent::ClientRequestMoveItem_Validate(TSubclassOf<UGASSItemDefinition> ItemDef, UActorComponent* DepComponent, UActorComponent* ArrComponent, int32 DepIndex, int32 ArrIndex, int32 StackAmount, bool Rotation, bool DepIsBlackbox, bool ArrIsBlackbox)
{
	UE_LOG(LogTemp, Warning, TEXT("Client Request Move Item !"));
	UGASSInventoryManagerComponent* DepIMC = Cast<UGASSInventoryManagerComponent>(DepComponent);
	UGASSInventoryManagerComponent* ArrIMC = Cast<UGASSInventoryManagerComponent>(ArrComponent);

	UGASSEquipmentManagerComponent* DepEMC = Cast<UGASSEquipmentManagerComponent>(DepComponent);

	if (OwnerActor->HasAuthority() && (DepIMC || DepEMC) && ArrIMC)
	{
		return true;
	}
	else
	{
		ServerCancelClientItemMove(DepComponent, ArrComponent, DepIndex, ArrIndex, StackAmount, Rotation, DepIsBlackbox, ArrIsBlackbox);
		return false;
	}
}

void UGASSInventoryManagerComponent::ClientRequestMoveItem_Implementation(TSubclassOf<UGASSItemDefinition> ItemDef, UActorComponent* DepComponent, UActorComponent* ArrComponent, int32 DepIndex, int32 ArrIndex, int32 StackAmount, bool Rotation, bool DepIsBlackbox, bool ArrIsBlackbox)
{
	UGASSInventoryManagerComponent* DepIMC = Cast<UGASSInventoryManagerComponent>(DepComponent);
	UGASSEquipmentManagerComponent* DepEMC = Cast<UGASSEquipmentManagerComponent>(DepComponent);
	UGASSInventoryManagerComponent* ArrIMC = Cast<UGASSInventoryManagerComponent>(ArrComponent);

	check(DepIMC || DepEMC);
	check(ArrIMC);

	UE_LOG(LogTemp, Warning, TEXT("Client -> Server RPC Called"));
	if (DepIMC)
	{
		if (DepIMC->ServerCheckIMCEnoughItem(ItemDef, DepIndex, StackAmount, DepIsBlackbox))
		{
			if (DepIMC == ArrIMC && DepIsBlackbox == ArrIsBlackbox)
			{
				if (ArrIMC->CanMoveMyItemAt(DepIndex, ArrIndex, Rotation, DepIsBlackbox))
				{
					// SUCCESS
					UE_LOG(LogTemp, Warning, TEXT(" Server :: Success to Move "));
					UGASSInventoryItemInstance* TargetInstance = DepIsBlackbox ? ArrIMC->Blackbox.Entries[DepIndex].Instance : ArrIMC->InventoryList.Entries[DepIndex].Instance;
					uint8 MaxCol = DepIsBlackbox ? ArrIMC->BlackboxMaxCol : ArrIMC->InventoryMaxCol;
					ArrIMC->DetachInstance(DepIndex, DepIsBlackbox);
					ArrIMC->AddItemInstanceAt(TargetInstance, StackAmount, ArrIndex / MaxCol, ArrIndex % MaxCol, Rotation, ArrIsBlackbox);

					if (TargetInstance->GetCurrStackCount() == 0)
					{
						ArrIMC->RemoveInstanceFromRegisteredList(TargetInstance);
					}
				}
				else
				{
					UE_LOG(LogTemp, Warning, TEXT(" Server :: Fail to Move "));
					// Fail
					ServerCancelClientItemMove(DepComponent, ArrComponent, DepIndex, ArrIndex, StackAmount, Rotation, DepIsBlackbox, ArrIsBlackbox);
					return;
				}
			}
			else
			{
				// TODO : Add Auto
				bool NeedToWeightCheck = true;
				if (DepIMC == ArrIMC)
				{
					NeedToWeightCheck = false;
				}

				if (ArrIMC->ServerCheckIMCCanPlaceItem(NeedToWeightCheck, ItemDef, ArrIndex, StackAmount, Rotation, ArrIsBlackbox))
				{
					// SUCCESS
					UGASSInventoryItemInstance* TargetInstance = DepIsBlackbox ? DepIMC->Blackbox.Entries[DepIndex].Instance : DepIMC->InventoryList.Entries[DepIndex].Instance;
					uint8 MaxCol = ArrIsBlackbox ? ArrIMC->BlackboxMaxCol : ArrIMC->InventoryMaxCol;
					DepIMC->DetachInstance(DepIndex, DepIsBlackbox);
					ArrIMC->AddItemInstanceAt(TargetInstance, StackAmount, ArrIndex / MaxCol, ArrIndex % MaxCol, Rotation, ArrIsBlackbox);
					if (TargetInstance->GetCurrStackCount() == 0)
					{
						DepIMC->RemoveInstanceFromRegisteredList(TargetInstance);
					}
				}
				else
				{
					// Fail
					UE_LOG(LogTemp, Warning, TEXT(" Server :: Fail to Move "));
					ArrIMC->ServerCancelClientItemMove(DepComponent, ArrComponent, DepIndex, ArrIndex, StackAmount, Rotation, DepIsBlackbox, ArrIsBlackbox);
					if (ArrIMC != DepIMC)
					{
						DepIMC->ServerCancelClientItemMove(DepComponent, ArrComponent, DepIndex, ArrIndex, StackAmount, Rotation, DepIsBlackbox, ArrIsBlackbox);
					}
					return;
				}
			}
		}
		else
		{
			// Fail
			UE_LOG(LogTemp, Error, TEXT(" Server :: Fail to Move "));
			ArrIMC->ServerCancelClientItemMove(DepComponent, ArrComponent, DepIndex, ArrIndex, StackAmount, Rotation, DepIsBlackbox, ArrIsBlackbox);
			if (ArrIMC != DepIMC)
			{
				DepIMC->ServerCancelClientItemMove(DepComponent, ArrComponent, DepIndex, ArrIndex, StackAmount, Rotation, DepIsBlackbox, ArrIsBlackbox);
			}
		}
		return;
	}
	else
	{
		check(DepEMC);

		if(DepEMC->ServerCehckEMCEnoughItem(ItemDef, DepIsBlackbox, DepIndex))
		{
			bool NeedToWeightCheck = true;
			if (DepEMC->GetOwnerActor() == OwnerActor)
			{
				NeedToWeightCheck = false;
			}

			if (ArrIMC->ServerCheckIMCCanPlaceItem(NeedToWeightCheck, ItemDef, ArrIndex, StackAmount, Rotation, ArrIsBlackbox))
			{
				//SUCCESS
				DepEMC->UnequipItemAt(DepIsBlackbox, DepIndex);
				ArrIMC->AddItemAt(ItemDef, 1, ArrIndex, Rotation, ArrIsBlackbox);
			}
			else
			{
				//FAIL
				UE_LOG(LogTemp, Warning, TEXT(" Server :: Fail to Move "));
				ArrIMC->ServerCancelClientItemMove(DepComponent, ArrComponent, DepIndex, ArrIndex, StackAmount, Rotation, DepIsBlackbox, ArrIsBlackbox);
				DepEMC->ServerCancelClientUnequip(DepIsBlackbox, DepIndex);
			}
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT(" Server :: Fail to Move "));
			ArrIMC->ServerCancelClientItemMove(DepComponent, ArrComponent, DepIndex, ArrIndex, StackAmount, Rotation, DepIsBlackbox, ArrIsBlackbox);
			DepEMC->ServerCancelClientUnequip(DepIsBlackbox, DepIndex);
		}
		return;

	}
}

void UGASSInventoryManagerComponent::ServerCancelClientItemMove_Implementation(UActorComponent* DepComponent, UActorComponent* ArrComponent, int32 DepIndex, int32 ArrIndex, int32 StackAmount, bool Rotation, bool DepIsBlackbox, bool ArrIsBlackbox)
{
	UGASSInventoryManagerComponent* DepIMC = Cast<UGASSInventoryManagerComponent>(DepComponent);
	UGASSInventoryManagerComponent* ArrIMC = Cast<UGASSInventoryManagerComponent>(ArrComponent);

	if (ArrIMC == this)
	{
		ArrIMC->OnInventoryMoveCanceled_Arr.Broadcast(ArrIsBlackbox);
	}

	if (DepIMC == this)
	{
		DepIMC->OnInventoryMoveCanceled_Dep.Broadcast(DepIsBlackbox, DepIndex);
	}
}

bool UGASSInventoryManagerComponent::ClientRequestStackItem_IMCtoIMC_Validate(TSubclassOf<UGASSItemDefinition> ItemDef, UActorComponent* DepComponent, UActorComponent* ArrComponent, int32 DepIndex, int32 ArrIndex, int32 StackAmount, bool DepIsBlackbox, bool ArrIsBlackbox)
{
	UGASSInventoryManagerComponent* DepIMC = Cast<UGASSInventoryManagerComponent>(DepComponent);
	UGASSInventoryManagerComponent* ArrIMC = Cast<UGASSInventoryManagerComponent>(ArrComponent);

	if (OwnerActor->HasAuthority() && DepIMC && ArrIMC)
	{
		return true;
	}
	else
	{
		DepIMC->ServerCancelClientItemMove(DepIMC, nullptr, DepIndex, 0, StackAmount, true, DepIsBlackbox, true);
		return false;
	}
}

void UGASSInventoryManagerComponent::ClientRequestStackItem_IMCtoIMC_Implementation(TSubclassOf<UGASSItemDefinition> ItemDef, UActorComponent* DepComponent, UActorComponent* ArrComponent, int32 DepIndex, int32 ArrIndex, int32 StackAmount, bool DepIsBlackbox, bool ArrIsBlackbox)
{
	UGASSInventoryManagerComponent* DepIMC = Cast<UGASSInventoryManagerComponent>(DepComponent);
	UGASSInventoryManagerComponent* ArrIMC = Cast<UGASSInventoryManagerComponent>(ArrComponent);

	check(DepIMC);
	check(ArrIMC);

	if (DepIMC->ServerCheckIMCEnoughItem(ItemDef, DepIndex, StackAmount, DepIsBlackbox))
	{
		if (ArrIMC->ServerCheckIMCEnoughItem(ItemDef, ArrIndex, 1, ArrIsBlackbox))
		{
			bool WeightCheck = false;
			if (DepIMC == ArrIMC)
			{
				// No Weight Check
				WeightCheck = true;
			}
			else
			{
				// Weight Check
				const UGASSItemDefinition* ItemCDO = GetDefault<UGASSItemDefinition>(ItemDef);
				WeightCheck = DepIMC->InventoryCurrWeight + StackAmount * (ItemCDO->ItemWeight) <= DepIMC->InventoryMaxWeight;
			}

			if (!WeightCheck)
			{
				DepIMC->ServerCancelClientItemMove(DepIMC, nullptr, DepIndex, 0, StackAmount, true, DepIsBlackbox, true);
				return;
			}

			DepIMC->UnstackEntries(DepIndex, StackAmount, DepIsBlackbox);
			ArrIMC->StackEntries(ArrIndex, StackAmount, ArrIsBlackbox);

		}
		else
		{
			// Cancel
			DepIMC->ServerCancelClientItemMove(DepIMC, nullptr, DepIndex, 0, StackAmount, true, DepIsBlackbox, true);
			return;
		}
	}
	else
	{
		// Cancel
		DepIMC->ServerCancelClientItemMove(DepIMC, nullptr, DepIndex, 0, StackAmount, true, DepIsBlackbox, true);
		return;
	}
}

//////////////////////////////////////////////////////////////////////
// Replications (Not Used)

void UGASSInventoryManagerComponent::ReadyForReplication()
{
	Super::ReadyForReplication();

	// Register existing UGASSInventoryItemInstance
	if (IsUsingRegisteredSubObjectList())
	{
		for (const FGASSInventoryEntry& Entry : InventoryList.Entries)
		{
			UGASSInventoryItemInstance* Instance = Entry.Instance;

			if (IsValid(Instance))
			{
				AddReplicatedSubObject(Instance);
			}
		}
	}
}

bool UGASSInventoryManagerComponent::ReplicateSubobjects(UActorChannel* Channel, class FOutBunch* Bunch, FReplicationFlags* RepFlags)
{
	bool WroteSomething = Super::ReplicateSubobjects(Channel, Bunch, RepFlags);

	for (FGASSInventoryEntry& Entry : InventoryList.Entries)
	{
		UGASSInventoryItemInstance* Instance = Entry.Instance;

		if (Instance && IsValid(Instance))
		{
			WroteSomething |= Channel->ReplicateSubobject(Instance, *Bunch, *RepFlags);
		}
	}

	return WroteSomething;
}