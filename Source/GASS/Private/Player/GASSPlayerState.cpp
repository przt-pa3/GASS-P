// Copyright Epic Games, Inc. All Rights Reserved.

#include "Player/GASSPlayerState.h"

#include "AbilitySystem/Attributes/GASSCombatSet.h"
#include "AbilitySystem/Attributes/GASSHealthSet.h"
#include "AbilitySystem/GASSAbilitySet.h"
#include "AbilitySystem/GASSAbilitySystemComponent.h"
#include "Character/GASSPawnData.h"
#include "Character/GASSPawnExtensionComponent.h"
#include "Components/GameFrameworkComponentManager.h"
#include "Engine/World.h"
//#include "GameFramework/GameplayMessageSubsystem.h"
#include "GameModes/GASSExperienceManagerComponent.h"
//@TODO: Would like to isolate this a bit better to get the pawn data in here without this having to know about other stuff
#include "GameModes/GASSGameMode.h"
#include "GASS/GASSLogChannels.h"
#include "Player/GASSPlayerController.h"
//#include "Messages/GASSVerbMessage.h"
#include "Net/UnrealNetwork.h"
#include "Net/Core/Misc/NetSubObjectRegistry.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GASSPlayerState)

class AController;
class APlayerState;
class FLifetimeProperty;

//struct FReplicatedComponentInfo;


const FName AGASSPlayerState::NAME_GASSAbilityReady("GASSAbilitiesReady");

AGASSPlayerState::AGASSPlayerState(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, MyPlayerConnectionType(EGASSPlayerConnectionType::Player)
{
	AbilitySystemComponent = ObjectInitializer.CreateDefaultSubobject<UGASSAbilitySystemComponent>(this, TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

	CreateDefaultSubobject<UGASSHealthSet>(TEXT("HealthSet"));
	CreateDefaultSubobject<UGASSCombatSet>(TEXT("CombatSet"));

	// AbilitySystemComponent needs to be updated at a high frequency.
	NetUpdateFrequency = 100.0f;

	//MyTeamID = FGenericTeamId::NoTeam;
	//MySquadID = INDEX_NONE;
}

void AGASSPlayerState::PreInitializeComponents()
{
	Super::PreInitializeComponents();
}

void AGASSPlayerState::Reset()
{
	Super::Reset();
}

void AGASSPlayerState::ClientInitialize(AController* C)
{
	Super::ClientInitialize(C);

	if (UGASSPawnExtensionComponent* PawnExtComp = UGASSPawnExtensionComponent::FindPawnExtensionComponent(GetPawn()))
	{
		PawnExtComp->CheckDefaultInitialization();
	}
}

void AGASSPlayerState::CopyProperties(APlayerState* PlayerState)
{
	Super::CopyProperties(PlayerState);

	//@TODO: Copy stats
}

void AGASSPlayerState::OnDeactivated()
{
	bool bDestroyDeactivatedPlayerState = false;

	switch (GetPlayerConnectionType())
	{
	case EGASSPlayerConnectionType::Player:
	case EGASSPlayerConnectionType::InactivePlayer:
		//@TODO: Ask the experience if we should destroy disconnecting players immediately or leave them around
		// (e.g., for long running servers where they might build up if lots of players cycle through)
		bDestroyDeactivatedPlayerState = true;
		break;
	default:
		bDestroyDeactivatedPlayerState = true;
		break;
	}

	SetPlayerConnectionType(EGASSPlayerConnectionType::InactivePlayer);

	if (bDestroyDeactivatedPlayerState)
	{
		Destroy();
	}
}

void AGASSPlayerState::OnReactivated()
{
	if (GetPlayerConnectionType() == EGASSPlayerConnectionType::InactivePlayer)
	{
		SetPlayerConnectionType(EGASSPlayerConnectionType::Player);
	}
}

void AGASSPlayerState::OnExperienceLoaded(const UGASSExperienceDefinition* /*CurrentExperience*/)
{
	if (AGASSGameMode* GASSGameMode = GetWorld()->GetAuthGameMode<AGASSGameMode>())
	{
		if (const UGASSPawnData* NewPawnData = GASSGameMode->GetPawnDataForController(GetOwningController()))
		{
			SetPawnData(NewPawnData);
			UE_LOG(LogTemp, Warning, TEXT("SetPawnData"));	
		}
		else
		{
			UE_LOG(LogGASS, Error, TEXT("AGASSPlayerState::OnExperienceLoaded(): Unable to find PawnData to initialize player state [%s]!"), *GetNameSafe(this));
		}
	}
}

void AGASSPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	FDoRepLifetimeParams SharedParams;
	SharedParams.bIsPushBased = true;

	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, PawnData, SharedParams);
	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, MyPlayerConnectionType, SharedParams);
	//DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, MyTeamID, SharedParams);
	//DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, MySquadID, SharedParams);

	DOREPLIFETIME(ThisClass, StatTags);
}

AGASSPlayerController* AGASSPlayerState::GetGASSPlayerController() const
{
	return Cast<AGASSPlayerController>(GetOwner());
}

UAbilitySystemComponent* AGASSPlayerState::GetAbilitySystemComponent() const
{
	return GetGASSAbilitySystemComponent();
}

void AGASSPlayerState::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	check(AbilitySystemComponent);
	AbilitySystemComponent->InitAbilityActorInfo(this, GetPawn());

	UWorld* World = GetWorld();
	if (World && World->IsGameWorld() && World->GetNetMode() != NM_Client)
	{
		AGameStateBase* GameState = GetWorld()->GetGameState();
		check(GameState);
		UGASSExperienceManagerComponent* ExperienceComponent = GameState->FindComponentByClass<UGASSExperienceManagerComponent>();
		check(ExperienceComponent);
		ExperienceComponent->CallOrRegister_OnExperienceLoaded(FOnGASSExperienceLoaded::FDelegate::CreateUObject(this, &ThisClass::OnExperienceLoaded));
	}
}

void AGASSPlayerState::SetPawnData(const UGASSPawnData* InPawnData)
{
	check(InPawnData);

	if (GetLocalRole() != ROLE_Authority)
	{
		return;
	}

	if (PawnData)
	{
		UE_LOG(LogGASS, Error, TEXT("Trying to set PawnData [%s] on player state [%s] that already has valid PawnData [%s]."), *GetNameSafe(InPawnData), *GetNameSafe(this), *GetNameSafe(PawnData));
		return;
	}

	MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, PawnData, this);
	PawnData = InPawnData;

	for (const UGASSAbilitySet* AbilitySet : PawnData->AbilitySets)
	{
		if (AbilitySet)
		{
			AbilitySet->GiveToAbilitySystem(AbilitySystemComponent, nullptr);
		}
	}

	UGameFrameworkComponentManager::SendGameFrameworkComponentExtensionEvent(this, NAME_GASSAbilityReady);
	ForceNetUpdate();
}

void AGASSPlayerState::OnRep_PawnData()
{
}

void AGASSPlayerState::SetPlayerConnectionType(EGASSPlayerConnectionType NewType)
{
	MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, MyPlayerConnectionType, this);
	MyPlayerConnectionType = NewType;
}

//void AGASSPlayerState::SetSquadID(int32 NewSquadId)
//{
//	if (HasAuthority())
//	{
//		MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, MySquadID, this);
//
//		MySquadID = NewSquadId;
//	}
//}

//void AGASSPlayerState::SetGenericTeamId(const FGenericTeamId& NewTeamID)
//{
//	if (HasAuthority())
//	{
//		const FGenericTeamId OldTeamID = MyTeamID;
//
//		MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, MyTeamID, this);
//		MyTeamID = NewTeamID;
//		ConditionalBroadcastTeamChanged(this, OldTeamID, NewTeamID);
//	}
//	else
//	{
//		UE_LOG(LogGASSTeams, Error, TEXT("Cannot set team for %s on non-authority"), *GetPathName(this));
//	}
//}

//FGenericTeamId AGASSPlayerState::GetGenericTeamId() const
//{
//	return MyTeamID;
//}

//FOnGASSTeamIndexChangedDelegate* AGASSPlayerState::GetOnTeamIndexChangedDelegate()
//{
//	return &OnTeamChangedDelegate;
//}

//void AGASSPlayerState::OnRep_MyTeamID(FGenericTeamId OldTeamID)
//{
//	ConditionalBroadcastTeamChanged(this, OldTeamID, MyTeamID);
//}
//
//void AGASSPlayerState::OnRep_MySquadID()
//{
//	//@TODO: Let the squad subsystem know (once that exists)
//}

//void AGASSPlayerState::AddStatTagStack(FGameplayTag Tag, int32 StackCount)
//{
//	StatTags.AddStack(Tag, StackCount);
//}
//
//void AGASSPlayerState::RemoveStatTagStack(FGameplayTag Tag, int32 StackCount)
//{
//	StatTags.RemoveStack(Tag, StackCount);
//}
//
//int32 AGASSPlayerState::GetStatTagStackCount(FGameplayTag Tag) const
//{
//	return StatTags.GetStackCount(Tag);
//}
//
//bool AGASSPlayerState::HasStatTag(FGameplayTag Tag) const
//{
//	return StatTags.ContainsTag(Tag);
//}
//
//void AGASSPlayerState::ClientBroadcastMessage_Implementation(const FGASSVerbMessage Message)
//{
//	// This check is needed to prevent running the action when in standalone mode
//	if (GetNetMode() == NM_Client)
//	{
//		UGameplayMessageSubsystem::Get(this).BroadcastMessage(Message.Verb, Message);
//	}
//}

