// Copyright Epic Games, Inc. All Rights Reserved.

#include "Player/GASSPlayerSpawningManagerComponent.h"
#include "GameFramework/PlayerState.h"
#include "EngineUtils.h"
#include "Engine/PlayerStartPIE.h"
#include "Player/GASSPlayerStart.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GASSPlayerSpawningManagerComponent)

DEFINE_LOG_CATEGORY_STATIC(LogPlayerSpawning, Log, All);

UGASSPlayerSpawningManagerComponent::UGASSPlayerSpawningManagerComponent(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{
	SetIsReplicatedByDefault(false);
	bAutoRegister = true;
	bAutoActivate = true;
	bWantsInitializeComponent = true;
	PrimaryComponentTick.TickGroup = TG_PrePhysics;
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bAllowTickOnDedicatedServer = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

void UGASSPlayerSpawningManagerComponent::InitializeComponent()
{
	Super::InitializeComponent();

	FWorldDelegates::LevelAddedToWorld.AddUObject(this, &ThisClass::OnLevelAdded);

	UWorld* World = GetWorld();
	World->AddOnActorSpawnedHandler(FOnActorSpawned::FDelegate::CreateUObject(this, &ThisClass::HandleOnActorSpawned));

	for (TActorIterator<AGASSPlayerStart> It(World); It; ++It)
	{
		if (AGASSPlayerStart* PlayerStart = *It)
		{
			CachedPlayerStarts.Add(PlayerStart);
		}
	}
}

void UGASSPlayerSpawningManagerComponent::OnLevelAdded(ULevel* InLevel, UWorld* InWorld)
{
	if (InWorld == GetWorld())
	{
		for (AActor* Actor : InLevel->Actors)
		{
			if (AGASSPlayerStart* PlayerStart = Cast<AGASSPlayerStart>(Actor))
			{
				ensure(!CachedPlayerStarts.Contains(PlayerStart));
				CachedPlayerStarts.Add(PlayerStart);
			}
		}
	}
}

void UGASSPlayerSpawningManagerComponent::HandleOnActorSpawned(AActor* SpawnedActor)
{
	if (AGASSPlayerStart* PlayerStart = Cast<AGASSPlayerStart>(SpawnedActor))
	{
		CachedPlayerStarts.Add(PlayerStart);
	}
}

// AGASSGameMode Proxied Calls - Need to handle when someone chooses
// to restart a player the normal way in the engine.
//======================================================================

AActor* UGASSPlayerSpawningManagerComponent::ChoosePlayerStart(AController* Player)
{
	if (Player)
	{
#if WITH_EDITOR
		if (APlayerStart* PlayerStart = FindPlayFromHereStart(Player))
		{
			return PlayerStart;
		}
#endif

		TArray<AGASSPlayerStart*> StarterPoints;
		for (auto StartIt = CachedPlayerStarts.CreateIterator(); StartIt; ++StartIt)
		{
			if (AGASSPlayerStart* Start = (*StartIt).Get())
			{
				StarterPoints.Add(Start);
			}
			else
			{
				StartIt.RemoveCurrent();
			}
		}

		if (APlayerState* PlayerState = Player->GetPlayerState<APlayerState>())
		{
			// start dedicated spectators at any random starting location, but they do not claim it
			if (PlayerState->IsOnlyASpectator())
			{
				if (!StarterPoints.IsEmpty())
				{
					return StarterPoints[FMath::RandRange(0, StarterPoints.Num() - 1)];
				}

				return nullptr;
			}
		}

		AActor* PlayerStart = OnChoosePlayerStart(Player, StarterPoints);

		if (!PlayerStart)
		{
			PlayerStart = GetFirstRandomUnoccupiedPlayerStart(Player, StarterPoints);
		}

		if (AGASSPlayerStart* GASSStart = Cast<AGASSPlayerStart>(PlayerStart))
		{
			GASSStart->TryClaim(Player);
		}

		return PlayerStart;
	}

	return nullptr;
}

#if WITH_EDITOR
APlayerStart* UGASSPlayerSpawningManagerComponent::FindPlayFromHereStart(AController* Player)
{
	// Only 'Play From Here' for a player controller, bots etc. should all spawn from normal spawn points.
	if (Player->IsA<APlayerController>())
	{
		if (UWorld* World = GetWorld())
		{
			for (TActorIterator<APlayerStart> It(World); It; ++It)
			{
				if (APlayerStart* PlayerStart = *It)
				{
					if (PlayerStart->IsA<APlayerStartPIE>())
					{
						// Always prefer the first "Play from Here" PlayerStart, if we find one while in PIE mode
						return PlayerStart;
					}
				}
			}
		}
	}

	return nullptr;
}
#endif

bool UGASSPlayerSpawningManagerComponent::ControllerCanRestart(AController* Player)
{
	bool bCanRestart = true;

	// TODO Can they restart?

	return bCanRestart;
}

void UGASSPlayerSpawningManagerComponent::FinishRestartPlayer(AController* NewPlayer, const FRotator& StartRotation)
{
	OnFinishRestartPlayer(NewPlayer, StartRotation);
	K2_OnFinishRestartPlayer(NewPlayer, StartRotation);
}

//================================================================

void UGASSPlayerSpawningManagerComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

APlayerStart* UGASSPlayerSpawningManagerComponent::GetFirstRandomUnoccupiedPlayerStart(AController* Controller, const TArray<AGASSPlayerStart*>& StartPoints) const
{
	if (Controller)
	{
		TArray<AGASSPlayerStart*> UnOccupiedStartPoints;
		TArray<AGASSPlayerStart*> OccupiedStartPoints;

		for (AGASSPlayerStart* StartPoint : StartPoints)
		{
			EGASSPlayerStartLocationOccupancy State = StartPoint->GetLocationOccupancy(Controller);

			switch (State)
			{
				case EGASSPlayerStartLocationOccupancy::Empty:
					UnOccupiedStartPoints.Add(StartPoint);
					break;
				case EGASSPlayerStartLocationOccupancy::Partial:
					OccupiedStartPoints.Add(StartPoint);
					break;

			}
		}

		if (UnOccupiedStartPoints.Num() > 0)
		{
			return UnOccupiedStartPoints[FMath::RandRange(0, UnOccupiedStartPoints.Num() - 1)];
		}
		else if (OccupiedStartPoints.Num() > 0)
		{
			return OccupiedStartPoints[FMath::RandRange(0, OccupiedStartPoints.Num() - 1)];
		}
	}

	return nullptr;
}

