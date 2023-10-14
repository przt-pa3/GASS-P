// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameplayEffectExecutionCalculation.h"
#include "GASSDamageExecution.generated.h"

class UObject;

/**
 * 
 */
UCLASS()
class GASS_API UGASSDamageExecution : public UGameplayEffectExecutionCalculation
{
	GENERATED_BODY()
	
public:
	UGASSDamageExecution();

protected:
	virtual void Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams, FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const override;

};
