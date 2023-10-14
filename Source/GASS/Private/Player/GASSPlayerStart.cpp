// Copyright Epic Games, Inc. All Rights Reserved.

#include "Player/GASSPlayerStart.h"

#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"
#include "TimerManager.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GASSPlayerStart)

AGASSPlayerStart::AGASSPlayerStart(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

EGASSPlayerStartLocationOccupancy AGASSPlayerStart::GetLocationOccupancy(AController* const ControllerPawnToFit) const
{
	UWorld* const World = GetWorld();
	if (HasAuthority() && World)
	{
		if (AGameModeBase* AuthGameMode = World->GetAuthGameMode())
		{
			TSubclassOf<APawn> PawnClass = AuthGameMode->GetDefaultPawnClassForController(ControllerPawnToFit);
			const APawn* const PawnToFit = PawnClass ? GetDefault<APawn>(PawnClass) : nullptr;

			FVector ActorLocation = GetActorLocation();
			const FRotator ActorRotation = GetActorRotation();

			if (!World->EncroachingBlockingGeometry(PawnToFit, ActorLocation, ActorRotation, nullptr))
			{
				return EGASSPlayerStartLocationOccupancy::Empty;
			}
			else if (World->FindTeleportSpot(PawnToFit, ActorLocation, ActorRotation))
			{
				return EGASSPlayerStartLocationOccupancy::Partial;
			}
		}
	}

	return EGASSPlayerStartLocationOccupancy::Full;
}

bool AGASSPlayerStart::IsClaimed() const
{
	return ClaimingController != nullptr;
}

bool AGASSPlayerStart::TryClaim(AController* OccupyingController)
{
	if (OccupyingController != nullptr && !IsClaimed())
	{
		ClaimingController = OccupyingController;
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().SetTimer(ExpirationTimerHandle, FTimerDelegate::CreateUObject(this, &AGASSPlayerStart::CheckUnclaimed), ExpirationCheckInterval, true);
		}
		return true;
	}
	return false;
}

void AGASSPlayerStart::CheckUnclaimed()
{
	if (ClaimingController != nullptr && ClaimingController->GetPawn() != nullptr && GetLocationOccupancy(ClaimingController) == EGASSPlayerStartLocationOccupancy::Empty)
	{
		ClaimingController = nullptr;
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().ClearTimer(ExpirationTimerHandle);
		}
	}
}

