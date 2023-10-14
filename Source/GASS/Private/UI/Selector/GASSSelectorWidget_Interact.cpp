// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Selector/GASSSelectorWidget_Interact.h"



void UGASSSelectorWidget_Interact::InitializeSelectorWidget_Interact(const TArray<FInteractionOption>& InteractionOptions)
{
	if (!HasInitialized || ShouldRefreshEveryTime)
	{
		ClearOptionsInfo();

		for (const FInteractionOption& InteractionOption : InteractionOptions)
		{
			AddOptionInfo(InteractionOption.OptionName);
		}

		AddCancelOption();

		SetMaterialInstance();

		ClearInfoWidgets();

		CreateInfoWidgets();

		//SetWidgetKey(KeyName);
	}
	
	HasInitialized = true;
}