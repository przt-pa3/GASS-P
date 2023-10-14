// Copyright Epic Games, Inc. All Rights Reserved.

#include "Equipment/GASSEquipmentManagerComponent.h"

#include "Character/GASSPawn.h"
#include "Player/GASSPlayerController.h"
#include "Equipment/GASSEquipmentDefinition.h"
#include "Equipment/GASSEquipmentInstance.h"
#include "Inventory/GASSInventoryManagerComponent.h"
#include "Inventory/GASSInventoryItemInstance.h"
#include "Item/GASSItemDefinition.h"
#include "AbilitySystem/GASSAbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "GASS/GASSGameplayTags.h"
#include "NativeGameplayTags.h"
#include "Engine/ActorChannel.h"
#include "Net/UnrealNetwork.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GASSEquipmentManagerComponent)

class FLifetimeProperty;
struct FReplicationFlags;

//////////////////////////////////////////////////////////////////////
// FGASSAppliedEquipmentEntry

FString FGASSAppliedEquipmentEntry::GetDebugString() const
{
	
	return FString::Printf(TEXT("%s of %s"), *GetNameSafe(Instance), *GetNameSafe(EquipmentDefinition.Get()));
}

//////////////////////////////////////////////////////////////////////
// FGASSEquipmentList

void FGASSEquipmentList::PreReplicatedRemove(const TArrayView<int32> RemovedIndices, int32 FinalSize)
{
	UGASSEquipmentManagerComponent* EMC = Cast<UGASSEquipmentManagerComponent>(OwnerComponent);
	check(EMC);

 	for (int32 Index : RemovedIndices)
 	{
 		const FGASSAppliedEquipmentEntry& Entry = Entries[Index];
		if (Entry.Instance != nullptr)
		{
			Entry.Instance->OnUnequipped();
			UE_LOG(LogTemp, Error, TEXT("In EMC :: Size - %d, Removed Index - %d"), Entries.Num(), Index);
			EMC->OnEntryUnequipped.Broadcast(false, Index);
		}
 	}
}

void FGASSEquipmentList::PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize)
{
	UE_LOG(LogTemp, Error, TEXT("  Added !!"));

	UGASSEquipmentManagerComponent* EMC = Cast<UGASSEquipmentManagerComponent>(OwnerComponent);
	check(EMC);

	for (int32 Index : AddedIndices)
	{
		const FGASSAppliedEquipmentEntry& Entry = Entries[Index];
		if (Entry.Instance != nullptr)
		{
			Entry.Instance->OnEquipped();
			const UGASSEquipmentDefinition* EquipmentCDO = GetDefault<UGASSEquipmentDefinition>(Entry.EquipmentDefinition);
			EMC->OnEntryEquipped.Broadcast(false, Index, EquipmentCDO->ItemDefinition);
		}
	}
}

void FGASSEquipmentList::PostReplicatedChange(const TArrayView<int32> ChangedIndices, int32 FinalSize)
{
	UE_LOG(LogTemp, Error, TEXT(" Changed !! %d"), ChangedIndices.Num());

	UGASSEquipmentManagerComponent* EMC = Cast<UGASSEquipmentManagerComponent>(OwnerComponent);
	check(EMC);

 	for (int32 Index : ChangedIndices)
 	{
		FGASSAppliedEquipmentEntry& Entry = Entries[Index];
		UE_LOG(LogTemp, Warning, TEXT("Index : %d"), Index);

		if (Entry.LastObservedInstance)
		{
			UE_LOG(LogTemp, Warning, TEXT("Unequipped"));
			Entry.LastObservedInstance->OnUnequipped();
			// TODO

			EMC->OnEntryUnequipped.Broadcast(bImMainList, Index);
		}
		else
		{
			//UE_LOG(LogTemp, Error, TEXT("LastObserved Is Null"));
		}

		if (Entry.Instance)
		{
			UE_LOG(LogTemp, Warning, TEXT("Equipped"));
			Entry.Instance->OnEquipped();
			const UGASSEquipmentDefinition* EquipmentCDO = GetDefault<UGASSEquipmentDefinition>(Entry.EquipmentDefinition);
			EMC->OnEntryEquipped.Broadcast(bImMainList, Index, EquipmentCDO->ItemDefinition);
		}
		else
		{
			//UE_LOG(LogTemp, Error, TEXT("Current Is Null"));
		}

		// Should Do Something With Instance?
		Entry.LastObservedInstance = Entry.Instance;
 	}
}

UGASSAbilitySystemComponent* FGASSEquipmentList::GetAbilitySystemComponent() const
{
	check(OwnerComponent);
	AActor* OwningActor = OwnerComponent->GetOwner();

	if (APawn* OwnerPawn = Cast<APawn>(OwningActor))
	{
		AGASSPlayerController* GASSPC = Cast<AGASSPlayerController>(OwnerPawn->GetController());
		check(GASSPC);

		return GASSPC->GetGASSAbilitySystemComponent();
	}
	else
	{
		return nullptr;
	}
}

bool FGASSEquipmentList::IsEntryEmpty(int32 EntryIndex)
{
	if (EntryIndex >= Entries.Num())
	{
		UE_LOG(LogTemp, Error, TEXT("Invalid Equipment Entry Index"));
		return false;
	}
	
	return !(Entries[EntryIndex].Instance);
}

void FGASSEquipmentList::AddEmptyEntry(int32 EntryNum)
{
	check(OwnerComponent);

	AActor* OwningActor = OwnerComponent->GetOwner();

	check(OwningActor->HasAuthority());

	for (auto i = 0; i < EntryNum; ++i)
	{
		FGASSAppliedEquipmentEntry& NewEntry = Entries.AddDefaulted_GetRef();
		//NewEntry.InitializeEquipmentEntry();
		MarkItemDirty(NewEntry);
	}
}

void FGASSEquipmentList::RemoveEntry(int32 EntryIndex)
{
	check(EntryIndex >= 0 && EntryIndex < Entries.Num());
	auto EntryIt = Entries.CreateIterator();
	EntryIt += EntryIndex;

	EntryIt.RemoveCurrent();
	MarkArrayDirty();
}

int32 FGASSEquipmentList::SearchAdditionalEntryWithDefinition(TSubclassOf<UGASSEquipmentDefinition> EquipmentDefinition)
{
	for (auto EntryIt = Entries.CreateIterator(); EntryIt; ++EntryIt)
	{
		FGASSAppliedEquipmentEntry& Entry = *EntryIt;
		if (Entry.EquipmentDefinition == EquipmentDefinition)
		{
			return EntryIt.GetIndex();
		}
	}
	return -1;
}

UGASSEquipmentInstance* FGASSEquipmentList::CreateInstanceByDefinition(TSubclassOf<UGASSEquipmentDefinition> EquipmentDefinition)
{
	UGASSEquipmentInstance* Result = nullptr;

	check(EquipmentDefinition != nullptr);
	check(OwnerComponent);
	check(OwnerComponent->GetOwner()->HasAuthority());

	const UGASSEquipmentDefinition* EquipmentCDO = GetDefault<UGASSEquipmentDefinition>(EquipmentDefinition);
	TSubclassOf<UGASSEquipmentInstance> InstanceType = EquipmentCDO->InstanceType;
	if (InstanceType == nullptr)
	{
		InstanceType = UGASSEquipmentInstance::StaticClass();
	}

	Result = NewObject<UGASSEquipmentInstance>(OwnerComponent->GetOwner(), InstanceType);

	return Result;
}

UGASSEquipmentInstance* FGASSEquipmentList::AttachEquipmentWithEntry(int32 EntryIndex, TSubclassOf<UGASSEquipmentDefinition> EquipmentDefinition)
{
	UE_LOG(LogTemp, Error, TEXT("Attach Equipment Called"));
	UGASSEquipmentInstance* NewInstance = nullptr;
	if (EntryIndex >= Entries.Num())
	{
		UE_LOG(LogTemp, Error, TEXT("Invalid Equipment Entry Index"));
		return NewInstance;
	}

	NewInstance = CreateInstanceByDefinition(EquipmentDefinition);
	FGASSAppliedEquipmentEntry& TargetEntry = Entries[EntryIndex];

	TargetEntry.EquipmentDefinition = EquipmentDefinition;
	TargetEntry.Instance = NewInstance;
	const UGASSEquipmentDefinition* EquipmentCDO = GetDefault<UGASSEquipmentDefinition>(EquipmentDefinition);

	if (UGASSAbilitySystemComponent* ASC = GetAbilitySystemComponent())
	{
		for (TObjectPtr<const UGASSAbilitySet> AbilitySet : EquipmentCDO->AbilitySetsToGrant)
		{
			AbilitySet->GiveToAbilitySystem(ASC, /*inout*/ &TargetEntry.GrantedHandles, NewInstance);
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Cannot Find ASC"));
		TargetEntry.Instance = nullptr;
		MarkItemDirty(TargetEntry);
		return nullptr;
	}

	NewInstance->SpawnEquipmentActors(EquipmentCDO->ActorsToSpawn);
	MarkItemDirty(TargetEntry);

	return NewInstance;
}

TSubclassOf<UGASSEquipmentDefinition> FGASSEquipmentList::DettachEquipmentWithEntry(int32 EntryIndex, OUT UGASSEquipmentInstance*& DettachedInstance)
{
	TSubclassOf<UGASSEquipmentDefinition> DettachedEquipmentDefinition = UGASSEquipmentDefinition::StaticClass();
	DettachedInstance = nullptr;

	if (EntryIndex >= Entries.Num())
	{
		UE_LOG(LogTemp, Error, TEXT("Invalid Equipment Entry Index"));
		return DettachedEquipmentDefinition;
	}

	FGASSAppliedEquipmentEntry& TargetEntry = Entries[EntryIndex];
	if (UGASSAbilitySystemComponent* ASC = GetAbilitySystemComponent())
	{
		if (bImMainList)
		{
			UGASSEquipmentManagerComponent* OwnerEMC = Cast<UGASSEquipmentManagerComponent>(OwnerComponent);
			check(OwnerEMC);

			if (OwnerEMC->GetCurrentArmedSlot() == EntryIndex)
			{
				OwnerEMC->DisarmItem();
			}
		}

		TargetEntry.GrantedHandles.TakeFromAbilitySystem(ASC);
		TargetEntry.Instance->DestroyEquipmentActors();
		DettachedEquipmentDefinition = TargetEntry.EquipmentDefinition;

		DettachedInstance = TargetEntry.Instance;
		TargetEntry.Instance = nullptr;

		MarkItemDirty(TargetEntry);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Cannot Find ASC"));
	}

	return DettachedEquipmentDefinition;
}

UGASSEquipmentInstance* FGASSEquipmentList::EquipAdditionalEntry(TSubclassOf<UGASSEquipmentDefinition> EquipmentDefinition)
{
	int32 TargetIndex = -1;

	for (int32 i = 0; i < Entries.Num(); ++i)
	{
		if (Entries[i].Instance == nullptr)
		{
			TargetIndex = i;
			break;
		}
	}

	if (TargetIndex < 0)
	{
		UE_LOG(LogTemp, Error, TEXT("Additional Equipment List is Full"));
		return nullptr;
	}

	return AttachEquipmentWithEntry(TargetIndex, EquipmentDefinition);
}

UGASSEquipmentInstance* FGASSEquipmentList::UnequipAdditionalEntry(int32 Index)
{
	return nullptr;
}

UGASSEquipmentInstance* FGASSEquipmentList::UnequipAdditionalEntry(TSubclassOf<UGASSEquipmentDefinition> EquipmentDefinition)
{
	UGASSEquipmentInstance* RemovedInstance = nullptr;

	int32 TargetIndex = SearchAdditionalEntryWithDefinition(EquipmentDefinition);

	if (TargetIndex >= 0 && TargetIndex < Entries.Num())
	{
		FGASSAppliedEquipmentEntry& TargetEntry = Entries[TargetIndex];
		if (UGASSAbilitySystemComponent* ASC = GetAbilitySystemComponent())
		{
			TargetEntry.GrantedHandles.TakeFromAbilitySystem(ASC);
		}

		TargetEntry.Instance->DestroyEquipmentActors();
		RemovedInstance = TargetEntry.Instance;

		auto EntryIt = Entries.CreateIterator();
		EntryIt += TargetIndex;

		EntryIt.RemoveCurrent();
		MarkArrayDirty();
	}

	return RemovedInstance;
}

//////////////////////////////////////////////////////////////////////
// UGASSEquipmentManagerComponent

UGASSEquipmentManagerComponent::UGASSEquipmentManagerComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, MainEquipmentList(this, true)
	, AdditionalEquipmentList(this, false)
{
	SetIsReplicatedByDefault(true);
	bReplicateUsingRegisteredSubObjectList = true;
	bWantsInitializeComponent = true;
}

void UGASSEquipmentManagerComponent::Test_PrintEquip()
{
	UE_LOG(LogTemp, Warning, TEXT("=================    Main    ==============="));
	int32 Cnt = 0;
	for (auto Entry : MainEquipmentList.Entries)
	{
		FString EquipmentInfo;
		if (Entry.Instance)
		{
			const UGASSEquipmentDefinition* EquipCDO = GetDefault<UGASSEquipmentDefinition>(Entry.EquipmentDefinition);
			const UGASSItemDefinition* ItemCDO = GetDefault<UGASSItemDefinition>(EquipCDO->ItemDefinition);
			EquipmentInfo = ItemCDO->DisplayName.ToString();
		}
		else
		{
			EquipmentInfo = FString(TEXT("Not Equipped"));
		}

		UE_LOG(LogTemp, Warning, TEXT("Slot %d : %s"), Cnt, *EquipmentInfo);
		Cnt++;
	}
	UE_LOG(LogTemp, Warning, TEXT("=================    Additional    ==============="));
	Cnt = 0;

	for (auto Entry : AdditionalEquipmentList.Entries)
	{
		FString EquipmentInfo;
		if (Entry.Instance)
		{
			const UGASSEquipmentDefinition* EquipCDO = GetDefault<UGASSEquipmentDefinition>(Entry.EquipmentDefinition);
			const UGASSItemDefinition* ItemCDO = GetDefault<UGASSItemDefinition>(EquipCDO->ItemDefinition);
			EquipmentInfo = ItemCDO->DisplayName.ToString();
		}
		else
		{
			EquipmentInfo = FString(TEXT("Not Equipped"));
		}

		UE_LOG(LogTemp, Warning, TEXT("Slot %d : %s"), Cnt, *EquipmentInfo);
		Cnt++;
	}
}

void UGASSEquipmentManagerComponent::GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, MainEquipmentList);
	DOREPLIFETIME(ThisClass, AdditionalEquipmentList);
	DOREPLIFETIME(ThisClass, CurrentArmedSlot);
}

UGASSInventoryManagerComponent* UGASSEquipmentManagerComponent::GetInventoryManagerComponent() const
{
	APawn* OwnerPawn = Cast<APawn>(GetOwner());

	if (OwnerPawn)
	{
		// TODO : EMC -> Character -> IMC
		AGASSPlayerController* OwnerPC = Cast<AGASSPlayerController>(OwnerPawn->GetController());

		if (OwnerPC)
		{
			return OwnerPC->GetGASSInventoryManagerComponent();
		}
	}

	return nullptr;
}
//////////////////////////////////////////////////////////////////////
// Actor interaction

void UGASSEquipmentManagerComponent::SetOwnerActor(AActor* NewOwnerActor)
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

void UGASSEquipmentManagerComponent::OnRep_OwningActor()
{

}

void UGASSEquipmentManagerComponent::InitEquipmentActorInfo(AActor* InOwnerActor)
{
	check(InOwnerActor);

	SetOwnerActor(InOwnerActor);
}
//////////////////////////////////////////////////////////////////////
// Initialize
void UGASSEquipmentManagerComponent::InitializeEquipmentComponent()
{
	int32 MainEquipmentListSize = GASSGameplayTags::CustomEquipmentTagMap.Num();
	InitializeEquipmentList(true, MainEquipmentListSize);
	InitializeEquipmentList(false, 10);
}

void UGASSEquipmentManagerComponent::InitializeEquipmentList(bool IsMainEquipment, int32 EquipmentListSize)
{
	if(IsMainEquipment)
	{
		MainEquipmentList.AddEmptyEntry(EquipmentListSize);
	}
	else
	{
		AdditionalEquipmentList.AddEmptyEntry(EquipmentListSize);
	}
}

void UGASSEquipmentManagerComponent::UnInitializeEquipmentList(FGASSEquipmentList& TargetList)
{
	for (int32 i = 0; i < TargetList.Entries.Num(); ++i)
	{
		UGASSEquipmentInstance* TargetInstance = TargetList.Entries[0].Instance;

		if (TargetInstance != nullptr)
		{
			TargetInstance->OnUnequipped();

			if (IsUsingRegisteredSubObjectList())
			{
				RemoveReplicatedSubObject(TargetInstance);
			}

			TargetList.RemoveEntry(0);
		}
	}
}

//////////////////////////////////////////////////////////////////////
// Getter
int32 UGASSEquipmentManagerComponent::GetEquipmentListSize(bool IsMain)
{
	return IsMain ? MainEquipmentList.Entries.Num() : AdditionalEquipmentList.Entries.Num();
}

int32 UGASSEquipmentManagerComponent::GetAvailableAdditionalEquipmentSlot()
{
	UE_LOG(LogTemp, Error, TEXT("Num %d"), AdditionalEquipmentList.Entries.Num());

	for (int32 i = 0; i < AdditionalEquipmentList.Entries.Num(); ++i)
	{
		if (AdditionalEquipmentList.Entries[i].Instance == nullptr)
		{
			UE_LOG(LogTemp, Error, TEXT("Return : %d"), i);
			return i;
		}
	}
	return -1;
}

void UGASSEquipmentManagerComponent::GetItemDefFromList(bool IsMainEquipment, TArray<TSubclassOf<UGASSItemDefinition>>& ItemDefs) const
{
	const FGASSEquipmentList& TargetList = IsMainEquipment ? MainEquipmentList : AdditionalEquipmentList;
	for (const FGASSAppliedEquipmentEntry& Entry : TargetList.Entries)
	{
		if (Entry.EquipmentDefinition)
		{
			const UGASSEquipmentDefinition* EquipmentCDO = GetDefault<UGASSEquipmentDefinition>(Entry.EquipmentDefinition);
			ItemDefs.Add(EquipmentCDO->ItemDefinition);
		}
		else
		{
			ItemDefs.Add(nullptr);
		}
	}
	return;
}

const uint8 UGASSEquipmentManagerComponent::GetEquipmentIndexByItemTag(const FGameplayTag& ItemTag) const
{
	for (const auto& Pair : GASSGameplayTags::CustomEquipmentTagMap)
	{
		if (ItemTag.MatchesTag(Pair.Key))
		{
			return uint8(Pair.Value);
		}
	}

	return uint8(255);
}

const uint8 UGASSEquipmentManagerComponent::GetEquipmentIndexByDefinition(TSubclassOf<UGASSEquipmentDefinition> EquipmentDefinition) const
{
	const UGASSEquipmentDefinition* EquipmentCDO = GetDefault<UGASSEquipmentDefinition>(EquipmentDefinition);
	const UGASSItemDefinition* ItemCDO = GetDefault<UGASSItemDefinition>(EquipmentCDO->ItemDefinition);
	FGameplayTag FullTag = ItemCDO->ItemTag;

	for (const auto& Pair : GASSGameplayTags::CustomEquipmentTagMap)
	{
		if (FullTag.MatchesTag(Pair.Key))
		{
			return uint8(Pair.Value);
		}
	}

	return uint8(255);
}

//////////////////////////////////////////////////////////////////////
// Equip / Unequip

UGASSEquipmentInstance* UGASSEquipmentManagerComponent::EquipItem(TSubclassOf<UGASSEquipmentDefinition> EquipmentClass)
{
	UGASSEquipmentInstance* EquippedInstance = nullptr;
	if (EquipmentClass != nullptr)
	{
		TSubclassOf<UGASSEquipmentDefinition> DettachedDefinition;
		UGASSEquipmentInstance* DettachedInstance = nullptr;

		const uint8 EquipmentIndex = GetEquipmentIndexByDefinition(EquipmentClass);

		if (EquipmentIndex < MainEquipmentList.Entries.Num())
		{
			// Main Slot
			EquippedInstance = EquipOrSwapMainItem(EquipmentClass, EquipmentIndex, OUT DettachedDefinition, OUT DettachedInstance);
		}
		else
		{
			// Additional Slot
			UE_LOG(LogTemp, Error, TEXT("Equip Additional!"));
			EquippedInstance = AdditionalEquipmentList.EquipAdditionalEntry(EquipmentClass);
		}

		if (DettachedInstance)
		{
			DettachedInstance->OnUnequipped();

			if (IsUsingRegisteredSubObjectList())
			{
				RemoveReplicatedSubObject(DettachedInstance);
			}

			// Do Something With DettachedInstance
		}

		if (EquippedInstance)
		{
			EquippedInstance->OnEquipped();

			if (IsUsingRegisteredSubObjectList() && IsReadyForReplication())
			{
				AddReplicatedSubObject(EquippedInstance);
			}

		}

		// @ TODO : Do Something With DettachedDefinition, IMC (Create Item to Inventory)

	}
	return EquippedInstance;
}

UGASSEquipmentInstance* UGASSEquipmentManagerComponent::EquipOrSwapMainItem(TSubclassOf<UGASSEquipmentDefinition> EquipmentDefinition, int32 EquipmentIndex, TSubclassOf<UGASSEquipmentDefinition>& DettachedDefinition, UGASSEquipmentInstance* DettachedInstance)
{
	UGASSEquipmentInstance* NewInstance = nullptr;

	if (!(MainEquipmentList.IsEntryEmpty(EquipmentIndex)))
	{
		// Dettach and Equip
		DettachedDefinition = MainEquipmentList.DettachEquipmentWithEntry(EquipmentIndex, OUT DettachedInstance);
		check(DettachedInstance && DettachedDefinition); // Error
	}

	NewInstance = MainEquipmentList.AttachEquipmentWithEntry(EquipmentIndex, EquipmentDefinition);

	return NewInstance;
}

bool UGASSEquipmentManagerComponent::CanEquipItem(TSubclassOf<UGASSEquipmentDefinition> EquipmentDefinition, UGASSInventoryManagerComponent* InIMC, int32& EquippableSlot, TSubclassOf<UGASSEquipmentDefinition>& DettachableDefinition)
{
	const uint8 EquipmentIndex = GetEquipmentIndexByDefinition(EquipmentDefinition);
	UGASSInventoryManagerComponent* MyIMC = GetInventoryManagerComponent();
	check(MyIMC && InIMC);

	const UGASSEquipmentDefinition* EquipmentCDO = GetDefault<UGASSEquipmentDefinition>(EquipmentDefinition);
	const UGASSItemDefinition* ItemCDO = GetDefault<UGASSItemDefinition>(EquipmentCDO->ItemDefinition);

	const UGASSEquipmentDefinition* EquippedEquipmentCDO = nullptr;
	const UGASSItemDefinition* EquippedItemCDO = nullptr;

	float NewEquipWeight = 0;
	float AlreadyEquippedWeight = 0;

	EquippableSlot = GetEquipmentIndexByDefinition(EquipmentDefinition);

	if (EquippableSlot >= MainEquipmentList.Entries.Num())
	{
		EquippableSlot = -1;
		//int32 Index = AdditionalEquipmentList.SearchAdditionalEntryWithDefinition(EquipmentDefinition);
		//if(Index )
	}
	else
	{
		if (MainEquipmentList.Entries[EquippableSlot].Instance)
		{
			EquippedEquipmentCDO = GetDefault<UGASSEquipmentDefinition>(MainEquipmentList.Entries[EquippableSlot].EquipmentDefinition);
			EquippedItemCDO = GetDefault<UGASSItemDefinition>(EquippedEquipmentCDO->ItemDefinition);
			AlreadyEquippedWeight = EquippedItemCDO->ItemWeight;
			DettachableDefinition = EquippedEquipmentCDO->ItemDefinition;
		}
	}

	if (MyIMC != InIMC)
	{
		NewEquipWeight = ItemCDO->ItemWeight;
	}

	if (NewEquipWeight > AlreadyEquippedWeight)
	{
		// Weight Check
		if (!(MyIMC->CanAddWeight(NewEquipWeight - AlreadyEquippedWeight)))
		{
			return false;
		}
	}

	return true;
}

bool UGASSEquipmentManagerComponent::CanEquipItemWeightCheck(float NewEquipmentWeight, bool IsFromMyIMC, bool IsMainEquipment, int32 EquippableSlot)
{
	float AlreadyEquippedWeight = 0;

	if (IsFromMyIMC)
	{
		return true;
	}

	if (IsMainEquipment)
	{
		const UGASSEquipmentDefinition* EquipmentCDO = GetDefault<UGASSEquipmentDefinition>(MainEquipmentList.Entries[EquippableSlot].EquipmentDefinition);
		const UGASSItemDefinition* ItemCDO = GetDefault<UGASSItemDefinition>(EquipmentCDO->ItemDefinition);
		AlreadyEquippedWeight = ItemCDO->ItemWeight;
	}

	UGASSInventoryManagerComponent* MyIMC = GetInventoryManagerComponent();
	check(MyIMC);

	return MyIMC->CanAddWeight(NewEquipmentWeight - AlreadyEquippedWeight);
}

void UGASSEquipmentManagerComponent::UnequipItem(TSubclassOf<UGASSEquipmentDefinition> EquipmentDefinition)
{
	const uint8 EquipmentIndex = GetEquipmentIndexByDefinition(EquipmentDefinition);
	UGASSEquipmentInstance* RemovedInstance = nullptr;

	if (EquipmentIndex < MainEquipmentList.Entries.Num())
	{
		MainEquipmentList.DettachEquipmentWithEntry(EquipmentIndex, OUT RemovedInstance);
	}
	else
	{
		RemovedInstance = AdditionalEquipmentList.UnequipAdditionalEntry(EquipmentDefinition);
	}

	if (RemovedInstance)
	{
		RemovedInstance->OnUnequipped();

		if (IsUsingRegisteredSubObjectList())
		{
			RemoveReplicatedSubObject(RemovedInstance);
		}
	}
}

void UGASSEquipmentManagerComponent::UnequipItemAt(bool IsMainEquipment, int32 EquippedSlot)
{
	UE_LOG(LogTemp, Error, TEXT("UnEquipItemAt Called"));

	UGASSEquipmentInstance* RemovedInstance = nullptr;
	FGASSEquipmentList& TargetList = IsMainEquipment ? MainEquipmentList : AdditionalEquipmentList;

	if (EquippedSlot >= TargetList.Entries.Num())
	{
		UE_LOG(LogTemp, Error, TEXT("Unequip Invalid Index"));
		return;
	}

	TargetList.DettachEquipmentWithEntry(EquippedSlot, OUT RemovedInstance);

	if (RemovedInstance)
	{
		UE_LOG(LogTemp, Error, TEXT("Calling OnUnequipped"));
		RemovedInstance->OnUnequipped();
		if (EquippedSlot == CurrentArmedSlot)
		{
			UE_LOG(LogTemp, Error, TEXT("Calling Disarm"));
			RemovedInstance->OnDisarmed();
		}

		if (IsUsingRegisteredSubObjectList())
		{
			RemoveReplicatedSubObject(RemovedInstance);
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("RemovedInstance Null"));
	}
}

//////////////////////////////////////////////////////////////////////
// Arm / Disarm

int32 UGASSEquipmentManagerComponent::GetCurrentArmedSlot()
{
	return CurrentArmedSlot;
}

UGASSEquipmentInstance* UGASSEquipmentManagerComponent::GetCurrentArmedInstance()
{
	if (!(MainEquipmentList.Entries.IsValidIndex(CurrentArmedSlot)))
	{
		return nullptr;
	}

	return MainEquipmentList.Entries[CurrentArmedSlot].Instance;
}

void UGASSEquipmentManagerComponent::ArmItem(int32 EquippedSlot)
{
	UE_LOG(LogTemp, Warning, TEXT("Arm Called"));

	if (MainEquipmentList.Entries.IsValidIndex(EquippedSlot))
	{
		if (MainEquipmentList.Entries[EquippedSlot].Instance)
		{
			FGASSAppliedEquipmentEntry& TargetEntry = MainEquipmentList.Entries[EquippedSlot];
			const UGASSEquipmentDefinition* EquipmentCDO = GetDefault<UGASSEquipmentDefinition>(TargetEntry.EquipmentDefinition);
			
			if (UGASSAbilitySystemComponent* ASC = MainEquipmentList.GetAbilitySystemComponent())
			{
				for (TObjectPtr<const UGASSAbilitySet> AbilitySet : EquipmentCDO->AbilitySetsToGrantWhenArmed)
				{
					AbilitySet->GiveToAbilitySystem(ASC, /*inout*/ &TargetEntry.GrantedHandlesWhenArmed, TargetEntry.Instance);
				}
				TargetEntry.Instance->SpawnEquipmentActorsWhenArmed(EquipmentCDO->ActorsToSpawn);
				TargetEntry.Instance->OnArmed();
				CurrentArmedSlot = EquippedSlot;
				UE_LOG(LogTemp, Error, TEXT("Arm Complete"));
				return;
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("Cannot Find ASC"));
				return;
			}
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT(" Cannot Arm, Target EquipSlot is Empty"));
			return;
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT(" Invalid EquipSlot"));
		return;
	}
}

bool UGASSEquipmentManagerComponent::CanArmItem(int32 EquippedSlot)
{
	if (CurrentArmedSlot == EquippedSlot)
	{
		return false;
	}

	if (!MainEquipmentList.Entries.IsValidIndex(EquippedSlot))
	{
		return false;
	}

	if (!MainEquipmentList.Entries[EquippedSlot].Instance)
	{
		return false;
	}

	return true;
}

void UGASSEquipmentManagerComponent::DisarmItem()
{
	UE_LOG(LogTemp, Error, TEXT("Disarm Called"));
	if(MainEquipmentList.Entries.IsValidIndex(CurrentArmedSlot))
	{
		if (MainEquipmentList.Entries[CurrentArmedSlot].Instance)
		{
			FGASSAppliedEquipmentEntry& TargetEntry = MainEquipmentList.Entries[CurrentArmedSlot];
			const UGASSEquipmentDefinition* EquipmentCDO = GetDefault<UGASSEquipmentDefinition>(TargetEntry.EquipmentDefinition);

			if (UGASSAbilitySystemComponent* ASC = MainEquipmentList.GetAbilitySystemComponent())
			{
				TargetEntry.GrantedHandlesWhenArmed.TakeFromAbilitySystem(ASC);
				TargetEntry.Instance->OnDisarmed();
				// TODO : Destory Actor After Disarm Anim Finish??
				TargetEntry.Instance->DestroyEquipmentActorsWhenDisarmed();
				CurrentArmedSlot = -1;
				return;
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("Cannot Find ASC"));
				return;
			}
		}
		else
		{
			CurrentArmedSlot = -1;
			return;
		}
	}
	else
	{
		CurrentArmedSlot = -1;
		return;
	}
}

void UGASSEquipmentManagerComponent::ChangeArmItem(int32 EquippedSlot)
{
	UE_LOG(LogTemp, Warning, TEXT("Change Arm Item Called : Index %d"), EquippedSlot);

	if (!CanArmItem(EquippedSlot))
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot Arm"));
		return;
	}

	if (CurrentArmedSlot >= 0)
	{
		DisarmItem();
	}

	ArmItem(EquippedSlot);
}

//////////////////////////////////////////////////////////////////////
// RPC Functions
bool UGASSEquipmentManagerComponent::ServerCehckEMCEnoughItem(TSubclassOf<UGASSItemDefinition> ItemDef, bool IsMainEquipment, int32 EquippableSlot)
{
	if (!(OwnerActor->HasAuthority()))
	{
		return false;
	}

	FGASSEquipmentList& TargetList = IsMainEquipment ? MainEquipmentList : AdditionalEquipmentList;

	if (TargetList.Entries.Num() <= EquippableSlot)
	{
		return false;
	}

	if (TargetList.Entries[EquippableSlot].EquipmentDefinition)
	{
		const UGASSEquipmentDefinition* EquipCDO = GetDefault<UGASSEquipmentDefinition>(TargetList.Entries[EquippableSlot].EquipmentDefinition);
		if (EquipCDO->ItemDefinition == ItemDef)
		{
			return true;
		}
	}

	return false;
}

void UGASSEquipmentManagerComponent::ServerCancelClientEquip_Implementation(bool IsMainEquipment, int32 EquippableSlot)
{
	OnEquipCanceled.Broadcast(IsMainEquipment, EquippableSlot);
}

void UGASSEquipmentManagerComponent::ServerCancelClientUnequip_Implementation(bool IsMainEquipment, int32 EquippableSlot)
{
	OnUnequipCanceled.Broadcast(IsMainEquipment, EquippableSlot);
}

bool UGASSEquipmentManagerComponent::ClientRequestEquipItem_Validate(TSubclassOf<UGASSEquipmentDefinition> EquipmentDefinition, UActorComponent* DepComponent, int32 DepIndex, bool DepIsBlackbox)
{
	if (OwnerActor->HasAuthority())
	{
		return true;
	}
	else
	{
		return false;
	}
	
}

void UGASSEquipmentManagerComponent::ClientRequestEquipItem_Implementation(TSubclassOf<UGASSEquipmentDefinition> EquipmentDefinition, UActorComponent* DepComponent, int32 DepIndex, bool DepIsBlackbox)
{
	UGASSInventoryManagerComponent* DepIMC = Cast<UGASSInventoryManagerComponent>(DepComponent);

	if (DepIMC)
	{
		const UGASSEquipmentDefinition* EquipmentCDO = GetDefault<UGASSEquipmentDefinition>(EquipmentDefinition);
		//const UGASSItemDefinition* ItemCDO = GetDefault<UGASSItemDefinition>(EquipmentCDO->ItemDefinition);

		if (DepIMC->ServerCheckIMCEnoughItem(EquipmentCDO->ItemDefinition, DepIndex, 1, DepIsBlackbox))
		{
			int32 EquippableSlot = -1;
			TSubclassOf<UGASSEquipmentDefinition> DettachableEquipmentDef;
			if (CanEquipItem(EquipmentDefinition, DepIMC, OUT EquippableSlot, OUT DettachableEquipmentDef))
			{
				if (DettachableEquipmentDef)
				{
					// Have To Handle DettachedEquipment

				}
				else
				{
					// TODO : Move Removing Instance to Handle Inside IMC
					UGASSInventoryItemInstance* TargetInstance = DepIMC->GetItemInstanceByIndex(DepIsBlackbox, DepIndex);
					DepIMC->DetachInstance(DepIndex, DepIsBlackbox);
					EquipItem(EquipmentDefinition);
					if (TargetInstance->GetCurrStackCount() == 0)
					{
						DepIMC->RemoveInstanceFromRegisteredList(TargetInstance);
					}
					return;
				}
			}
			else
			{
				// Fail
				bool IsMainEquipment = true;
				if (EquippableSlot < 0)
				{
					IsMainEquipment = false;
					EquippableSlot = AdditionalEquipmentList.Entries.Num();
				}

				ServerCancelClientEquip(IsMainEquipment, EquippableSlot);
				DepIMC->ServerCancelClientItemMove(DepComponent, nullptr, DepIndex, -1, 1, false, DepIsBlackbox, false);
				return;
			}
		}
		else
		{
			// Fail
			UE_LOG(LogTemp, Error, TEXT(" Server :: Fail to Equip "));
			ServerCancelClientEquip(false, -1); // Need to Refresh
			DepIMC->ServerCancelClientItemMove(DepComponent, nullptr, DepIndex, -1, 1, false, DepIsBlackbox, false);
			return;
		}
	}
	else
	{
		// Directly From Pickup??
	}
	return;
}

//////////////////////////////////////////////////////////////////////
// UObject, UActorComponent Interface

bool UGASSEquipmentManagerComponent::ReplicateSubobjects(UActorChannel* Channel, class FOutBunch* Bunch, FReplicationFlags* RepFlags)
{
	bool WroteSomething = Super::ReplicateSubobjects(Channel, Bunch, RepFlags);

	for (FGASSAppliedEquipmentEntry& Entry : MainEquipmentList.Entries)
	{
		UGASSEquipmentInstance* Instance = Entry.Instance;

		if (IsValid(Instance))
		{
			WroteSomething |= Channel->ReplicateSubobject(Instance, *Bunch, *RepFlags);
		}
	}

	for (FGASSAppliedEquipmentEntry& Entry : AdditionalEquipmentList.Entries)
	{
		UGASSEquipmentInstance* Instance = Entry.Instance;

		if (IsValid(Instance))
		{
			WroteSomething |= Channel->ReplicateSubobject(Instance, *Bunch, *RepFlags);
		}
	}

	return WroteSomething;
}

void UGASSEquipmentManagerComponent::InitializeComponent()
{
	Super::InitializeComponent();
}

void UGASSEquipmentManagerComponent::UninitializeComponent()
{
	

	// gathering all instances before removal to avoid side effects affecting the equipment list iterator	

	UnInitializeEquipmentList(MainEquipmentList);
	UnInitializeEquipmentList(AdditionalEquipmentList);

	Super::UninitializeComponent();
}

void UGASSEquipmentManagerComponent::ReadyForReplication()
{
	Super::ReadyForReplication();

	// Register existing GASSEquipmentInstances
	if (IsUsingRegisteredSubObjectList())
	{
		TArray<UGASSEquipmentInstance*> AllEquipmentInstances;

		for (const FGASSAppliedEquipmentEntry& Entry : MainEquipmentList.Entries)
		{
			AllEquipmentInstances.Add(Entry.Instance);	
		}
		for (const FGASSAppliedEquipmentEntry& Entry : AdditionalEquipmentList.Entries)
		{
			AllEquipmentInstances.Add(Entry.Instance);
		}

		for (auto Instance : AllEquipmentInstances)
		{
			if (IsValid(Instance))
			{
				AddReplicatedSubObject(Instance);
			}
		}
	}
}

void UGASSEquipmentManagerComponent::OnRep_ArmChanged()
{
	check(!HasAuthority());
	UE_LOG(LogTemp, Error, TEXT("Client :: OnRepArmChanged !!, Last : %d || Current : %d"), ClientLastArmedSlot, CurrentArmedSlot);

	if (ClientLastArmedSlot == CurrentArmedSlot)
	{
		return;
	}


	if (MainEquipmentList.Entries.IsValidIndex(ClientLastArmedSlot))
	{
		UGASSEquipmentInstance* TargetInstance = MainEquipmentList.Entries[ClientLastArmedSlot].Instance;
		if (TargetInstance)
		{
			TargetInstance->OnDisarmed();
		}
		else
		{
			// Last Arm has Unequipped!!

			UE_LOG(LogTemp, Error, TEXT("Should Do Something!!"));
		}
	}
	
	if (MainEquipmentList.Entries.IsValidIndex(CurrentArmedSlot))
	{
		UGASSEquipmentInstance* TargetInstance = MainEquipmentList.Entries[CurrentArmedSlot].Instance;
		if (TargetInstance)
		{
			TargetInstance->OnArmed();
		}
	}
	else if (CurrentArmedSlot == -1)
	{

	}

	ClientLastArmedSlot = CurrentArmedSlot;
}