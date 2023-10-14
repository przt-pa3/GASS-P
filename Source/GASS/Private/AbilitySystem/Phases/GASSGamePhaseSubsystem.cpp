// Copyright Epic Games, Inc. All Rights Reserved.

#include "AbilitySystem/Phases/GASSGamePhaseSubsystem.h"

#include "AbilitySystem/GASSAbilitySystemComponent.h"
#include "Engine/World.h"
#include "GameFramework/GameStateBase.h"
#include "AbilitySystem/Phases/GASSGamePhaseAbility.h"
#include "AbilitySystem/Phases/GASSGamePhaseLog.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GASSGamePhaseSubsystem)

class UGASSGameplayAbility;
class UObject;

DEFINE_LOG_CATEGORY(LogGASSGamePhase);

//////////////////////////////////////////////////////////////////////
// UGASSGamePhaseSubsystem

UGASSGamePhaseSubsystem::UGASSGamePhaseSubsystem()
{
	UE_LOG(LogTemp, Warning, TEXT("Phase Subsystem const."));
}

void UGASSGamePhaseSubsystem::PostInitialize()
{
	Super::PostInitialize();
}

bool UGASSGamePhaseSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	if (Super::ShouldCreateSubsystem(Outer))
	{
		//UWorld* World = Cast<UWorld>(Outer);
		//check(World);

		//return World->GetAuthGameMode() != nullptr;
		//return nullptr;
		UE_LOG(LogTemp, Warning, TEXT("Need to Create Subsystem"));
		return true;
	}

	return false;
}

bool UGASSGamePhaseSubsystem::DoesSupportWorldType(const EWorldType::Type WorldType) const
{
	return WorldType == EWorldType::Game || WorldType == EWorldType::PIE;
}

void UGASSGamePhaseSubsystem::StartPhase(TSubclassOf<UGASSGamePhaseAbility> PhaseAbility, FGASSGamePhaseDelegate PhaseEndedCallback)
{
	UWorld* World = GetWorld();
	UGASSAbilitySystemComponent* GameState_ASC = World->GetGameState()->FindComponentByClass<UGASSAbilitySystemComponent>();
	UE_LOG(LogTemp, Warning, TEXT("Start Phase _ Subsystem"));
	if (ensure(GameState_ASC))
	{
		UE_LOG(LogTemp, Warning, TEXT("Ensure ASC"));
		FGameplayAbilitySpec PhaseSpec(PhaseAbility, 1, 0, this);
		FGameplayAbilitySpecHandle SpecHandle = GameState_ASC->GiveAbilityAndActivateOnce(PhaseSpec);
		FGameplayAbilitySpec* FoundSpec = GameState_ASC->FindAbilitySpecFromHandle(SpecHandle);
		
		if (FoundSpec && FoundSpec->IsActive())
		{
			FGASSGamePhaseEntry& Entry = ActivePhaseMap.FindOrAdd(SpecHandle);
			Entry.PhaseEndedCallback = PhaseEndedCallback;
		}
		else
		{
			PhaseEndedCallback.ExecuteIfBound(nullptr);
		}
	}
}

void UGASSGamePhaseSubsystem::K2_StartPhase(TSubclassOf<UGASSGamePhaseAbility> PhaseAbility, const FGASSGamePhaseDynamicDelegate& PhaseEndedDelegate)
{
	const FGASSGamePhaseDelegate EndedDelegate = FGASSGamePhaseDelegate::CreateWeakLambda(const_cast<UObject*>(PhaseEndedDelegate.GetUObject()), [PhaseEndedDelegate](const UGASSGamePhaseAbility* PhaseAbility) {
		PhaseEndedDelegate.ExecuteIfBound(PhaseAbility);
	});

	StartPhase(PhaseAbility, EndedDelegate);
}

void UGASSGamePhaseSubsystem::K2_WhenPhaseStartsOrIsActive(FGameplayTag PhaseTag, EPhaseTagMatchType MatchType, FGASSGamePhaseTagDynamicDelegate WhenPhaseActive)
{
	const FGASSGamePhaseTagDelegate ActiveDelegate = FGASSGamePhaseTagDelegate::CreateWeakLambda(WhenPhaseActive.GetUObject(), [WhenPhaseActive](const FGameplayTag& PhaseTag) {
		WhenPhaseActive.ExecuteIfBound(PhaseTag);
	});

	WhenPhaseStartsOrIsActive(PhaseTag, MatchType, ActiveDelegate);
}

void UGASSGamePhaseSubsystem::K2_WhenPhaseEnds(FGameplayTag PhaseTag, EPhaseTagMatchType MatchType, FGASSGamePhaseTagDynamicDelegate WhenPhaseEnd)
{
	const FGASSGamePhaseTagDelegate EndedDelegate = FGASSGamePhaseTagDelegate::CreateWeakLambda(WhenPhaseEnd.GetUObject(), [WhenPhaseEnd](const FGameplayTag& PhaseTag) {
		WhenPhaseEnd.ExecuteIfBound(PhaseTag);
	});

	WhenPhaseEnds(PhaseTag, MatchType, EndedDelegate);
}

void UGASSGamePhaseSubsystem::WhenPhaseStartsOrIsActive(FGameplayTag PhaseTag, EPhaseTagMatchType MatchType, const FGASSGamePhaseTagDelegate& WhenPhaseActive)
{
	FPhaseObserver Observer;
	Observer.PhaseTag = PhaseTag;
	Observer.MatchType = MatchType;
	Observer.PhaseCallback = WhenPhaseActive;
	PhaseStartObservers.Add(Observer);

	if (IsPhaseActive(PhaseTag))
	{
		WhenPhaseActive.ExecuteIfBound(PhaseTag);
	}
}

void UGASSGamePhaseSubsystem::WhenPhaseEnds(FGameplayTag PhaseTag, EPhaseTagMatchType MatchType, const FGASSGamePhaseTagDelegate& WhenPhaseEnd)
{
	FPhaseObserver Observer;
	Observer.PhaseTag = PhaseTag;
	Observer.MatchType = MatchType;
	Observer.PhaseCallback = WhenPhaseEnd;
	PhaseEndObservers.Add(Observer);
}

bool UGASSGamePhaseSubsystem::IsPhaseActive(const FGameplayTag& PhaseTag) const
{
	for (const auto& KVP : ActivePhaseMap)
	{
		const FGASSGamePhaseEntry& PhaseEntry = KVP.Value;
		if (PhaseEntry.PhaseTag.MatchesTag(PhaseTag))
		{
			return true;
		}
	}

	return false;
}

void UGASSGamePhaseSubsystem::OnBeginPhase(const UGASSGamePhaseAbility* PhaseAbility, const FGameplayAbilitySpecHandle PhaseAbilityHandle)
{
	const FGameplayTag IncomingPhaseTag = PhaseAbility->GetGamePhaseTag();

	UE_LOG(LogGASSGamePhase, Log, TEXT("Beginning Phase '%s' (%s)"), *IncomingPhaseTag.ToString(), *GetNameSafe(PhaseAbility));

	const UWorld* World = GetWorld();
	UGASSAbilitySystemComponent* GameState_ASC = World->GetGameState()->FindComponentByClass<UGASSAbilitySystemComponent>();
	if (ensure(GameState_ASC))
	{
		TArray<FGameplayAbilitySpec*> ActivePhases;
		for (const auto& KVP : ActivePhaseMap)
		{
			const FGameplayAbilitySpecHandle ActiveAbilityHandle = KVP.Key;
			if (FGameplayAbilitySpec* Spec = GameState_ASC->FindAbilitySpecFromHandle(ActiveAbilityHandle))
			{
				ActivePhases.Add(Spec);
			}
		}

		for (const FGameplayAbilitySpec* ActivePhase : ActivePhases)
		{
			const UGASSGamePhaseAbility* ActivePhaseAbility = CastChecked<UGASSGamePhaseAbility>(ActivePhase->Ability);
			const FGameplayTag ActivePhaseTag = ActivePhaseAbility->GetGamePhaseTag();
			
			// So if the active phase currently matches the incoming phase tag, we allow it.
			// i.e. multiple gameplay abilities can all be associated with the same phase tag.
			// For example,
			// You can be in the, Game.Playing, phase, and then start a sub-phase, like Game.Playing.SuddenDeath
			// Game.Playing phase will still be active, and if someone were to push another one, like,
			// Game.Playing.ActualSuddenDeath, it would end Game.Playing.SuddenDeath phase, but Game.Playing would
			// continue.  Similarly if we activated Game.GameOver, all the Game.Playing* phases would end.
			if (!IncomingPhaseTag.MatchesTag(ActivePhaseTag))
			{
				UE_LOG(LogGASSGamePhase, Log, TEXT("\tEnding Phase '%s' (%s)"), *ActivePhaseTag.ToString(), *GetNameSafe(ActivePhaseAbility));

				FGameplayAbilitySpecHandle HandleToEnd = ActivePhase->Handle;
				GameState_ASC->CancelAbilitiesByFunc([HandleToEnd](const UGASSGameplayAbility* GASSAbility, FGameplayAbilitySpecHandle Handle) {
					return Handle == HandleToEnd;
				}, true);
			}
		}

		FGASSGamePhaseEntry& Entry = ActivePhaseMap.FindOrAdd(PhaseAbilityHandle);
		Entry.PhaseTag = IncomingPhaseTag;

		// Notify all observers of this phase that it has started.
		for (const FPhaseObserver& Observer : PhaseStartObservers)
		{
			if (Observer.IsMatch(IncomingPhaseTag))
			{
				Observer.PhaseCallback.ExecuteIfBound(IncomingPhaseTag);
			}
		}
	}
}

void UGASSGamePhaseSubsystem::OnEndPhase(const UGASSGamePhaseAbility* PhaseAbility, const FGameplayAbilitySpecHandle PhaseAbilityHandle)
{
	const FGameplayTag EndedPhaseTag = PhaseAbility->GetGamePhaseTag();
	UE_LOG(LogGASSGamePhase, Log, TEXT("Ended Phase '%s' (%s)"), *EndedPhaseTag.ToString(), *GetNameSafe(PhaseAbility));

	const FGASSGamePhaseEntry& Entry = ActivePhaseMap.FindChecked(PhaseAbilityHandle);
	Entry.PhaseEndedCallback.ExecuteIfBound(PhaseAbility);

	ActivePhaseMap.Remove(PhaseAbilityHandle);

	// Notify all observers of this phase that it has ended.
	for (const FPhaseObserver& Observer : PhaseEndObservers)
	{
		if (Observer.IsMatch(EndedPhaseTag))
		{
			Observer.PhaseCallback.ExecuteIfBound(EndedPhaseTag);
		}
	}
}

bool UGASSGamePhaseSubsystem::FPhaseObserver::IsMatch(const FGameplayTag& ComparePhaseTag) const
{
	switch(MatchType)
	{
	case EPhaseTagMatchType::ExactMatch:
		return ComparePhaseTag == PhaseTag;
	case EPhaseTagMatchType::PartialMatch:
		return ComparePhaseTag.MatchesTag(PhaseTag);
	}

	return false;
}
