// Copyright Epic Games, Inc. All Rights Reserved.

#include "Weapons/GASSWeaponStateComponent.h"

#include "Abilities/GameplayAbilityTargetTypes.h"
#include "Equipment/GASSEquipmentManagerComponent.h"
#include "GameFramework/Pawn.h"
#include "GameplayEffectTypes.h"
#include "Kismet/GameplayStatics.h"
#include "NativeGameplayTags.h"
#include "Physics/PhysicalMaterialWithTags.h"
//#include "Teams/GASSTeamSubsystem.h"
#include "Weapons/GASSRangedWeaponInstance.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GASSWeaponStateComponent)

UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Gameplay_Zone, "Gameplay.Zone");

UGASSWeaponStateComponent::UGASSWeaponStateComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SetIsReplicatedByDefault(true);

	PrimaryComponentTick.bStartWithTickEnabled = true;
	PrimaryComponentTick.bCanEverTick = true;
}

void UGASSWeaponStateComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (APawn* Pawn = GetPawn<APawn>())
	{
		// @ TODO : Change
		// Find Proper UGASSWeaponInstance* from EMC
		if (UGASSEquipmentManagerComponent* EquipmentManager = Pawn->FindComponentByClass<UGASSEquipmentManagerComponent>())
		{
			//if (UGASSRangedWeaponInstance* CurrentWeapon = Cast<UGASSRangedWeaponInstance>(EquipmentManager->GetFirstInstanceOfType(UGASSRangedWeaponInstance::StaticClass())))
			//{
			//	CurrentWeapon->Tick(DeltaTime);
			//}
		}
	}
}

bool UGASSWeaponStateComponent::ShouldShowHitAsSuccess(const FHitResult& Hit) const
{
	AActor* HitActor = Hit.GetActor();

	//@TODO: Don't treat a hit that dealt no damage (due to invulnerability or similar) as a success
	UWorld* World = GetWorld();
	//if (UGASSTeamSubsystem* TeamSubsystem = UWorld::GetSubsystem<UGASSTeamSubsystem>(GetWorld()))
	//{
	//	return TeamSubsystem->CanCauseDamage(GetController<APlayerController>(), Hit.GetActor());
	//}

	return true;

	return false;
}

bool UGASSWeaponStateComponent::ShouldUpdateDamageInstigatedTime(const FGameplayEffectContextHandle& EffectContext) const
{
	//@TODO: Implement me, for the purposes of this component we really only care about damage caused by a weapon
	// or projectile fired from a weapon, and should filter to that
	// (or perhaps see if the causer is also the source of our active reticle config)
	return EffectContext.GetEffectCauser() != nullptr;
}

void UGASSWeaponStateComponent::ClientConfirmTargetData_Implementation(uint16 UniqueId, bool bSuccess, const TArray<uint8>& HitReplaces)
{
	for (int i = 0; i < UnconfirmedServerSideHitMarkers.Num(); i++)
	{
		FGASSServerSideHitMarkerBatch& Batch = UnconfirmedServerSideHitMarkers[i];
		if (Batch.UniqueId == UniqueId)
		{
			if (bSuccess && (HitReplaces.Num() != Batch.Markers.Num()))
			{
				UWorld* World = GetWorld();
				bool bFoundShowAsSuccessHit = false;

				int32 HitLocationIndex = 0;
				for (const FGASSScreenSpaceHitLocation& Entry : Batch.Markers)
				{
					if (!HitReplaces.Contains(HitLocationIndex) && Entry.bShowAsSuccess)
					{
						// Only need to do this once
						if (!bFoundShowAsSuccessHit)
						{
							ActuallyUpdateDamageInstigatedTime();
						}

						bFoundShowAsSuccessHit = true;

						LastWeaponDamageScreenLocations.Add(Entry);
					}
					++HitLocationIndex;
				}
			}

			UnconfirmedServerSideHitMarkers.RemoveAt(i);
			break;
		}
	}
}

void UGASSWeaponStateComponent::AddUnconfirmedServerSideHitMarkers(const FGameplayAbilityTargetDataHandle& InTargetData, const TArray<FHitResult>& FoundHits)
{
	FGASSServerSideHitMarkerBatch& NewUnconfirmedHitMarker = UnconfirmedServerSideHitMarkers.Emplace_GetRef(InTargetData.UniqueId);

	if (APlayerController* OwnerPC = GetController<APlayerController>())
	{
		for (const FHitResult& Hit : FoundHits)
		{
			FVector2D HitScreenLocation;
			if (UGameplayStatics::ProjectWorldToScreen(OwnerPC, Hit.Location, /*out*/ HitScreenLocation, /*bPlayerViewportRelative=*/ false))
			{
				FGASSScreenSpaceHitLocation& Entry = NewUnconfirmedHitMarker.Markers.AddDefaulted_GetRef();
				Entry.Location = HitScreenLocation;
				Entry.bShowAsSuccess = ShouldShowHitAsSuccess(Hit);

				// Determine the hit zone
				FGameplayTag HitZone;
				if (const UPhysicalMaterialWithTags* PhysMatWithTags = Cast<const UPhysicalMaterialWithTags>(Hit.PhysMaterial.Get()))
				{
					for (const FGameplayTag MaterialTag : PhysMatWithTags->Tags)
					{
						if (MaterialTag.MatchesTag(TAG_Gameplay_Zone))
						{
							Entry.HitZone = MaterialTag;
							break;
						}
					}
				}
			}
		}
	}
}

void UGASSWeaponStateComponent::UpdateDamageInstigatedTime(const FGameplayEffectContextHandle& EffectContext)
{
	if (ShouldUpdateDamageInstigatedTime(EffectContext))
	{
		ActuallyUpdateDamageInstigatedTime();
	}
}

void UGASSWeaponStateComponent::ActuallyUpdateDamageInstigatedTime()
{
	// If our LastWeaponDamageInstigatedTime was not very recent, clear our LastWeaponDamageScreenLocations array
	UWorld* World = GetWorld();
	if (World->GetTimeSeconds() - LastWeaponDamageInstigatedTime > 0.1)
	{
		LastWeaponDamageScreenLocations.Reset();
	}
	LastWeaponDamageInstigatedTime = World->GetTimeSeconds();
}

double UGASSWeaponStateComponent::GetTimeSinceLastHitNotification() const
{
	UWorld* World = GetWorld();
	return World->TimeSince(LastWeaponDamageInstigatedTime);
}

