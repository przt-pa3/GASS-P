// Copyright Epic Games, Inc. All Rights Reserved.

#include "GameModes/GASSGameState.h"

#include "AbilitySystem/GASSAbilitySystemComponent.h"
#include "Async/TaskGraphInterfaces.h"
//#include "GameFramework/GameplayMessageSubsystem.h"
#include "GameModes/GASSExperienceManagerComponent.h"
//#include "Messages/GASSVerbMessage.h"
#include "Player/GASSPlayerState.h"
#include "Net/UnrealNetwork.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GASSGameState)

class APlayerState;
class FLifetimeProperty;

extern ENGINE_API float GAverageFPS;


AGASSGameState::AGASSGameState(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	AbilitySystemComponent = ObjectInitializer.CreateDefaultSubobject<UGASSAbilitySystemComponent>(this, TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

	ExperienceManagerComponent = CreateDefaultSubobject<UGASSExperienceManagerComponent>(TEXT("ExperienceManagerComponent"));

	ServerFPS = 0.0f;
}

void AGASSGameState::PreInitializeComponents()
{
	Super::PreInitializeComponents();
}

void AGASSGameState::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	check(AbilitySystemComponent);
	AbilitySystemComponent->InitAbilityActorInfo(/*Owner=*/ this, /*Avatar=*/ this);
}

UAbilitySystemComponent* AGASSGameState::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void AGASSGameState::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
}

void AGASSGameState::AddPlayerState(APlayerState* PlayerState)
{
	Super::AddPlayerState(PlayerState);
}

void AGASSGameState::RemovePlayerState(APlayerState* PlayerState)
{
	//@TODO: This isn't getting called right now (only the 'rich' AGameMode uses it, not AGameModeBase)
	// Need to at least comment the engine code, and possibly move things around
	Super::RemovePlayerState(PlayerState);
}

void AGASSGameState::SeamlessTravelTransitionCheckpoint(bool bToTransitionMap)
{
	// Remove inactive and bots
	for (int32 i = PlayerArray.Num() - 1; i >= 0; i--)
	{
		APlayerState* PlayerState = PlayerArray[i];
		if (PlayerState && (PlayerState->IsABot() || PlayerState->IsInactive()))
		{
			RemovePlayerState(PlayerState);
		}
	}
}

void AGASSGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, ServerFPS);
}

void AGASSGameState::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (GetLocalRole() == ROLE_Authority)
	{
		ServerFPS = GAverageFPS;
	}
}

//void AGASSGameState::MulticastMessageToClients_Implementation(const FGASSVerbMessage Message)
//{
//	if (GetNetMode() == NM_Client)
//	{
//		UGameplayMessageSubsystem::Get(this).BroadcastMessage(Message.Verb, Message);
//	}
//}
//
//void AGASSGameState::MulticastReliableMessageToClients_Implementation(const FGASSVerbMessage Message)
//{
//	MulticastMessageToClients_Implementation(Message);
//}
