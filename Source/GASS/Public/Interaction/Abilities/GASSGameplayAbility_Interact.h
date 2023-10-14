// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "AbilitySystem/Abilities/GASSGameplayAbility.h"
#include "Interaction/InteractionOption.h"

#include "GASSGameplayAbility_Interact.generated.h"

//class UIndicatorDescriptor;
class UObject;
class UUserWidget;
struct FFrame;
struct FGameplayAbilityActorInfo;
struct FGameplayEventData;

/**
 * UGASSGameplayAbility_Interact
 *
 * Gameplay ability used for character interacting
 */
UCLASS(Abstract)
class UGASSGameplayAbility_Interact : public UGASSGameplayAbility
{
	GENERATED_BODY()

public:
	UGASSGameplayAbility_Interact(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	UFUNCTION(BlueprintCallable)
	void UpdateInteractions(const TArray<FInteractionOption>& InteractiveOptions);

	UFUNCTION(BlueprintCallable)
	void TriggerFirstInteraction();

	UFUNCTION(BlueprintCallable)
	void TriggerInteractionNum();

	UFUNCTION()
	void HandleTriggerInteraction(const FInteractionOption& InteractionOption);

	UFUNCTION(BlueprintCallable)
	int GetCurrentOptionsNum();

	UFUNCTION(BlueprintCallable)
	bool SetInteractOptionNumber(uint8 InOptionNumber);
protected:
	UPROPERTY(BlueprintReadWrite)
	TArray<FInteractionOption> CurrentOptions;

	//UPROPERTY()
	//TArray<TObjectPtr<UIndicatorDescriptor>> Indicators;

protected:

	UPROPERTY(EditDefaultsOnly)
	float InteractionScanRate = 0.1f;

	UPROPERTY(EditDefaultsOnly)
	float InteractionScanRange = 500;

	UPROPERTY(EditDefaultsOnly)
	TSoftClassPtr<UUserWidget> DefaultInteractionWidgetClass;

	UPROPERTY(BlueprintReadOnly)
	uint8 InteractOptionNumber = 0;
};
