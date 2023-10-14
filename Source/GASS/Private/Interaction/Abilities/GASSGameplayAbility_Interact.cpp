// Copyright Epic Games, Inc. All Rights Reserved.

#include "Interaction/Abilities/GASSGameplayAbility_Interact.h"

#include "AbilitySystemComponent.h"
#include "Interaction/IInteractableTarget.h"
#include "Interaction/InteractionStatics.h"
#include "Interaction/Tasks/AbilityTask_GrantNearbyInteraction.h"
#include "NativeGameplayTags.h"
#include "Player/GASSPlayerController.h"
//#include "UI/IndicatorSystem/IndicatorDescriptor.h"
//#include "UI/IndicatorSystem/GASSIndicatorManagerComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GASSGameplayAbility_Interact)

class UUserWidget;

UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Ability_Interaction_Activate, "Ability.Interaction.Activate");
UE_DEFINE_GAMEPLAY_TAG(TAG_INTERACTION_DURATION_MESSAGE, "Ability.Interaction.Duration.Message");

UGASSGameplayAbility_Interact::UGASSGameplayAbility_Interact(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ActivationPolicy = EGASSAbilityActivationPolicy::OnSpawn;
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
}

void UGASSGameplayAbility_Interact::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	UAbilitySystemComponent* AbilitySystem = GetAbilitySystemComponentFromActorInfo();
	if (AbilitySystem && AbilitySystem->GetOwnerRole() == ROLE_Authority)
	{
		UAbilityTask_GrantNearbyInteraction* Task = UAbilityTask_GrantNearbyInteraction::GrantAbilitiesForNearbyInteractors(this, InteractionScanRange, InteractionScanRate);
		Task->ReadyForActivation();
	}
}

void UGASSGameplayAbility_Interact::UpdateInteractions(const TArray<FInteractionOption>& InteractiveOptions)
{
	if (AGASSPlayerController* PC = GetGASSPlayerControllerFromActorInfo())
	{
		//if (UGASSIndicatorManagerComponent* IndicatorManager = UGASSIndicatorManagerComponent::GetComponent(PC))
		//{
		//	for (UIndicatorDescriptor* Indicator : Indicators)
		//	{
		//		IndicatorManager->RemoveIndicator(Indicator);
		//	}
		//	Indicators.Reset();

		//	for (const FInteractionOption& InteractionOption : InteractiveOptions)
		//	{
		//		AActor* InteractableTargetActor = UInteractionStatics::GetActorFromInteractableTarget(InteractionOption.InteractableTarget);

		//		TSoftClassPtr<UUserWidget> InteractionWidgetClass = 
		//			InteractionOption.InteractionWidgetClass.IsNull() ? DefaultInteractionWidgetClass : InteractionOption.InteractionWidgetClass;

		//		UIndicatorDescriptor* Indicator = NewObject<UIndicatorDescriptor>();
		//		Indicator->SetDataObject(InteractableTargetActor);
		//		Indicator->SetSceneComponent(InteractableTargetActor->GetRootComponent());
		//		Indicator->SetIndicatorClass(InteractionWidgetClass);
		//		IndicatorManager->AddIndicator(Indicator);

		//		Indicators.Add(Indicator);
		//	}
		//}
		//else
		//{
		//	//TODO This should probably be a noisy warning.  Why are we updating interactions on a PC that can never do anything with them?
		//}
	}

	CurrentOptions = InteractiveOptions;
}

void UGASSGameplayAbility_Interact::TriggerFirstInteraction()
{
	if (CurrentOptions.Num() == 0)
	{
		return;
	}
	const FInteractionOption& InteractionOption = CurrentOptions[0];

	HandleTriggerInteraction(InteractionOption);
}

void UGASSGameplayAbility_Interact::TriggerInteractionNum()
{
	if (CurrentOptions.Num() < InteractOptionNumber)
	{
		return;
	}
	else if (CurrentOptions.Num() == InteractOptionNumber)
	{
		UE_LOG(LogTemp, Warning, TEXT("Cancel Interaction"));
		return;
	}
	const FInteractionOption& InteractionOption = CurrentOptions[InteractOptionNumber];
	HandleTriggerInteraction(InteractionOption);
}

void UGASSGameplayAbility_Interact::HandleTriggerInteraction(const FInteractionOption& InteractionOption)
{
	UAbilitySystemComponent* AbilitySystem = GetAbilitySystemComponentFromActorInfo();
	if (AbilitySystem)
	{
		AActor* Instigator = GetAvatarActorFromActorInfo();
		AActor* InteractableTargetActor = UInteractionStatics::GetActorFromInteractableTarget(InteractionOption.InteractableTarget);

		// Allow the target to customize the event data we're about to pass in, in case the ability needs custom data
		// that only the actor knows.
		FGameplayEventData Payload;
		Payload.EventTag = TAG_Ability_Interaction_Activate;
		Payload.Instigator = Instigator;
		Payload.Target = InteractableTargetActor;

		if (InteractionOption.OptionName == FName(TEXT("Hi")))
		{
			// Do Something
			Payload.EventMagnitude = 1.0;
		}

		// If needed we allow the interactable target to manipulate the event data so that for example, a button on the wall
		// may want to specify a door actor to execute the ability on, so it might choose to override Target to be the
		// door actor.
		InteractionOption.InteractableTarget->CustomizeInteractionEventData(TAG_Ability_Interaction_Activate, Payload);

		// Grab the target actor off the payload we're going to use it as the 'avatar' for the interaction, and the
		// source InteractableTarget actor as the owner actor.
		AActor* TargetActor = const_cast<AActor*>(ToRawPtr(Payload.Target));

		// The actor info needed for the interaction.
		FGameplayAbilityActorInfo ActorInfo;
		ActorInfo.InitFromActor(InteractableTargetActor, TargetActor, InteractionOption.TargetAbilitySystem);

		// Trigger the ability using event tag.
		const bool bSuccess = InteractionOption.TargetAbilitySystem->TriggerAbilityFromGameplayEvent(
			InteractionOption.TargetInteractionAbilityHandle,
			&ActorInfo,
			TAG_Ability_Interaction_Activate,
			&Payload,
			*InteractionOption.TargetAbilitySystem
		);

		if (bSuccess)
		{
			UE_LOG(LogTemp, Warning, TEXT(" Trigger By Event Success "));
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT(" Trigger By Event Fail "));
		}
	}
}

int UGASSGameplayAbility_Interact::GetCurrentOptionsNum()
{
	return CurrentOptions.Num();
}

bool UGASSGameplayAbility_Interact::SetInteractOptionNumber(uint8 InOptionNumber)
{
	if (HasAuthority(&CurrentActivationInfo))
		return false;

	UE_LOG(LogTemp, Warning, TEXT("In Option Number : %d, CurrenOptions Num : %d"), InOptionNumber, CurrentOptions.Num());

	if (InOptionNumber <= CurrentOptions.Num() + 1)
	{
		UE_LOG(LogTemp, Warning, TEXT("Valid :: In Option Number : %d"), InOptionNumber);
		InteractOptionNumber = InOptionNumber - 1;
		return true;
	}
	else
	{
		return false;
	}
}


