// Copyright Epic Games, Inc. All Rights Reserved.

#include "Equipment/GASSEquipmentInstance.h"

#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Character.h"
#include "Equipment/GASSEquipmentDefinition.h"
#include "Net/UnrealNetwork.h"

#if UE_WITH_IRIS
#include "Iris/ReplicationSystem/ReplicationFragmentUtil.h"
#endif // UE_WITH_IRIS

#include UE_INLINE_GENERATED_CPP_BY_NAME(GASSEquipmentInstance)

class FLifetimeProperty;
class UClass;
class USceneComponent;

UGASSEquipmentInstance::UGASSEquipmentInstance(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UWorld* UGASSEquipmentInstance::GetWorld() const
{
	if (APawn* OwnerPawn = GetPawn())
	{
		return OwnerPawn->GetWorld();
	}
	else
	{
		return nullptr;
	}
}

void UGASSEquipmentInstance::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, Instigator);
	DOREPLIFETIME(ThisClass, SpawnedActors);
	DOREPLIFETIME(ThisClass, SpawnedActorsWhenArmed);
}

#if UE_WITH_IRIS
void UGASSEquipmentInstance::RegisterReplicationFragments(UE::Net::FFragmentRegistrationContext& Context, UE::Net::EFragmentRegistrationFlags RegistrationFlags)
{
	using namespace UE::Net;

	Super::RegisterReplicationFragments(Context, RegistrationFlags);

	// Build descriptors and allocate PropertyReplicationFragments for this object
	FReplicationFragmentUtil::CreateAndRegisterFragmentsForObject(this, Context, RegistrationFlags);
}
#endif // UE_WITH_IRIS

APawn* UGASSEquipmentInstance::GetPawn() const
{
	APawn* OwnerPawn = Cast<APawn>(GetOuter());

	if (OwnerPawn)
	{
		return OwnerPawn;
	}
	else
	{
		return nullptr;
	}
}

APawn* UGASSEquipmentInstance::GetTypedPawn(TSubclassOf<APawn> PawnType) const
{
	APawn* Result = nullptr;
	if (UClass* ActualPawnType = PawnType)
	{
		if (GetPawn()->IsA(ActualPawnType))
		{
			Result = Cast<APawn>(GetPawn());
		}
	}
	return Result;
}

void UGASSEquipmentInstance::SpawnEquipmentActors(const TArray<FGASSEquipmentActorToSpawn>& ActorsToSpawn)
{
	if (APawn* OwningPawn = GetPawn())
	{
		USceneComponent* AttachTarget = OwningPawn->GetRootComponent();
		if (ACharacter* Char = Cast<ACharacter>(OwningPawn))
		{
			AttachTarget = Char->GetMesh();
		}

		for (const FGASSEquipmentActorToSpawn& SpawnInfo : ActorsToSpawn)
		{
			if (!SpawnInfo.bSpawnWhenEquipped)
			{
				continue;
			}

			AActor* NewActor = GetWorld()->SpawnActorDeferred<AActor>(SpawnInfo.ActorToSpawn, FTransform::Identity, OwningPawn);
			NewActor->FinishSpawning(FTransform::Identity, /*bIsDefaultTransform=*/ true);
			NewActor->SetActorRelativeTransform(SpawnInfo.AttachTransform);
			NewActor->AttachToComponent(AttachTarget, FAttachmentTransformRules::KeepRelativeTransform, SpawnInfo.AttachSocket);

			SpawnedActors.Add(NewActor);
		}
	}
}

void UGASSEquipmentInstance::DestroyEquipmentActors()
{
	for (AActor* Actor : SpawnedActors)
	{
		if (Actor)
		{
			Actor->Destroy();
		}
	}
	SpawnedActors.Empty();
}

void UGASSEquipmentInstance::SpawnEquipmentActorsWhenArmed(const TArray<FGASSEquipmentActorToSpawn>& ActorsToSpawn)
{
	if (APawn* OwningPawn = GetPawn())
	{
		USceneComponent* AttachTarget = OwningPawn->GetRootComponent();
		if (ACharacter* Char = Cast<ACharacter>(OwningPawn))
		{
			AttachTarget = Char->GetMesh();
		}

		for (const FGASSEquipmentActorToSpawn& SpawnInfo : ActorsToSpawn)
		{
			if (SpawnInfo.bSpawnWhenEquipped)
			{
				continue;
			}

			AActor* NewActor = GetWorld()->SpawnActorDeferred<AActor>(SpawnInfo.ActorToSpawn, FTransform::Identity, OwningPawn);
			NewActor->FinishSpawning(FTransform::Identity, /*bIsDefaultTransform=*/ true);
			NewActor->SetActorRelativeTransform(SpawnInfo.AttachTransform);
			NewActor->AttachToComponent(AttachTarget, FAttachmentTransformRules::KeepRelativeTransform, SpawnInfo.AttachSocket);

			SpawnedActorsWhenArmed.Add(NewActor);
		}
	}
}

void UGASSEquipmentInstance::DestroyEquipmentActorsWhenDisarmed()
{
	for (AActor* Actor : SpawnedActorsWhenArmed)
	{
		if (Actor)
		{
			Actor->Destroy();
		}
	}
	SpawnedActorsWhenArmed.Empty();
}

void UGASSEquipmentInstance::OnEquipped()
{
	K2_OnEquipped();
}

void UGASSEquipmentInstance::OnUnequipped()
{
	K2_OnUnequipped();
}

void UGASSEquipmentInstance::OnArmed()
{
	K2_OnArmed();
}

void UGASSEquipmentInstance::OnDisarmed()
{
	K2_OnDisarmed();
}

void UGASSEquipmentInstance::OnRep_Instigator()
{
}

