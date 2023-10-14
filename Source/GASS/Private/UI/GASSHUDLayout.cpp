// Copyright Epic Games, Inc. All Rights Reserved.

#include "UI/GASSHUDLayout.h"

#include "CommonUIExtensions.h"
#include "Input/CommonUIInputTypes.h"
#include "NativeGameplayTags.h"
#include "UI/GASSActivatableWidget.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GASSHUDLayout)

UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_UI_LAYER_MENU, "UI.Layer.Menu");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_UI_ACTION_ESCAPE, "UI.Action.Escape");

UGASSHUDLayout::UGASSHUDLayout(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UGASSHUDLayout::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	// TODO : Crash Here, Check Later
	//RegisterUIActionBinding(FBindUIActionArgs(FUIActionTag::ConvertChecked(TAG_UI_ACTION_ESCAPE), false, FSimpleDelegate::CreateUObject(this, &ThisClass::HandleEscapeAction)));
}

void UGASSHUDLayout::HandleEscapeAction()
{
	//if (ensure(!EscapeMenuClass.IsNull()))
	//{
	//	UCommonUIExtensions::PushStreamedContentToLayer_ForPlayer(GetOwningLocalPlayer(), TAG_UI_LAYER_MENU, EscapeMenuClass);
	//}
}
