// Copyright Epic Games, Inc. All Rights Reserved.

#include "UI/GASSGameViewportClient.h"

#include "CommonUISettings.h"
#include "ICommonUIModule.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GASSGameViewportClient)

class UGameInstance;

namespace GameViewportTags
{
	//UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Platform_Trait_Input_HardwareCursor, "Platform.Trait.Input.HardwareCursor");
}

UGASSGameViewportClient::UGASSGameViewportClient()
	: Super(FObjectInitializer::Get())
{
}

void UGASSGameViewportClient::Init(struct FWorldContext& WorldContext, UGameInstance* OwningGameInstance, bool bCreateNewAudioDevice)
{
	Super::Init(WorldContext, OwningGameInstance, bCreateNewAudioDevice);
	
	//// We have software cursors set up in our project settings for console/mobile use, but on desktop we're fine with
	//// the standard hardware cursors
	//const bool UseHardwareCursor = ICommonUIModule::GetSettings().GetPlatformTraits().HasTag(GameViewportTags::TAG_Platform_Trait_Input_HardwareCursor);
	//SetUseSoftwareCursorWidgets(!UseHardwareCursor);
}
