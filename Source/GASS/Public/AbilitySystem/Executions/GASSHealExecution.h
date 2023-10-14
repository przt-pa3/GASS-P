// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameplayEffectExecutionCalculation.h"
#include "GASSHealExecution.generated.h"

class UObject;

/**
 * 
 */
UCLASS()
class GASS_API UGASSHealExecution : public UGameplayEffectExecutionCalculation
{
	GENERATED_BODY()
	
public:
	UGASSHealExecution();

protected:
	virtual void Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams, FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const override;
};
