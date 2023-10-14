// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Interaction/InteractionOption.h"
#include "UI/Selector/GASSSelectorWidget.h"
#include "GASSSelectorWidget_Interact.generated.h"

/**
 * 
 */
UCLASS()
class GASS_API UGASSSelectorWidget_Interact : public UGASSSelectorWidget
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable)
	void InitializeSelectorWidget_Interact(const TArray<FInteractionOption>& InteractionOptions);

private:

};
