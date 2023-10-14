// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "GASSActivatableWidget.h"

#include "GASSHUDLayout.generated.h"

class UCommonActivatableWidget;
class UObject;


/**
 * UGASSHUDLayout
 *
 *	Widget used to lay out the player's HUD (typically specified by an Add Widgets action in the experience)
 */
UCLASS(Abstract, BlueprintType, Blueprintable, Meta = (DisplayName = "GASS HUD Layout", Category = "GASS|HUD"))
class UGASSHUDLayout : public UGASSActivatableWidget
{
	GENERATED_BODY()

public:

	UGASSHUDLayout(const FObjectInitializer& ObjectInitializer);

	void NativeOnInitialized() override;

protected:
	void HandleEscapeAction();

	UPROPERTY(EditDefaultsOnly)
	TSoftClassPtr<UCommonActivatableWidget> EscapeMenuClass;
};
