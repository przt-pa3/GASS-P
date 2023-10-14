// Copyright Epic Games, Inc. All Rights Reserved.

#include "Input/GASSInputComponent.h"

#include "EnhancedInputSubsystems.h"
#include "Player/GASSLocalPlayer.h"
#include "Settings/GASSSettingsLocal.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GASSInputComponent)

class UGASSInputConfig;

UGASSInputComponent::UGASSInputComponent(const FObjectInitializer& ObjectInitializer)
{
}

void UGASSInputComponent::AddInputMappings(const UGASSInputConfig* InputConfig, UEnhancedInputLocalPlayerSubsystem* InputSubsystem) const
{
	check(InputConfig);
	check(InputSubsystem);

	UGASSLocalPlayer* LocalPlayer = InputSubsystem->GetLocalPlayer<UGASSLocalPlayer>();
	check(LocalPlayer);

	// Add any registered input mappings from the settings!
	if (UGASSSettingsLocal* LocalSettings = UGASSSettingsLocal::Get())
	{
		//// Tell enhanced input about any custom keymappings that the player may have customized
		//for (const TPair<FName, FKey>& Pair : LocalSettings->GetCustomPlayerInputConfig())
		//{
		//	if (Pair.Key != NAME_None && Pair.Value.IsValid())
		//	{
		//		InputSubsystem->AddPlayerMappedKeyInSlot(Pair.Key, Pair.Value);
		//	}
		//}
	}
}

void UGASSInputComponent::RemoveInputMappings(const UGASSInputConfig* InputConfig, UEnhancedInputLocalPlayerSubsystem* InputSubsystem) const
{
	check(InputConfig);
	check(InputSubsystem);

	UGASSLocalPlayer* LocalPlayer = InputSubsystem->GetLocalPlayer<UGASSLocalPlayer>();
	check(LocalPlayer);

	if (UGASSSettingsLocal* LocalSettings = UGASSSettingsLocal::Get())
	{
		//// Remove any registered input contexts
		//const TArray<FLoadedMappableConfigPair>& Configs = LocalSettings->GetAllRegisteredInputConfigs();
		//for (const FLoadedMappableConfigPair& Pair : Configs)
		//{
		//	InputSubsystem->RemovePlayerMappableConfig(Pair.Config);
		//}

		//// Clear any player mapped keys from enhanced input
		//for (const TPair<FName, FKey>& Pair : LocalSettings->GetCustomPlayerInputConfig())
		//{
		//	InputSubsystem->RemovePlayerMappedKeyInSlot(Pair.Key);
		//}
	}
}

void UGASSInputComponent::RemoveBinds(TArray<uint32>& BindHandles)
{
	for (uint32 Handle : BindHandles)
	{
		RemoveBindingByHandle(Handle);
	}
	BindHandles.Reset();
}
