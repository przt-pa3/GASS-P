// Copyright Epic Games, Inc. All Rights Reserved.

#include "Input/GASSInputModifiers.h"

#include "EnhancedPlayerInput.h"
#include "GameFramework/PlayerController.h"
//#include "Input/GASSAimSensitivityData.h"
#include "Player/GASSLocalPlayer.h"
//#include "Settings/GASSSettingsShared.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GASSInputModifiers)

DEFINE_LOG_CATEGORY_STATIC(LogGASSInputModifiers, Log, All);

//////////////////////////////////////////////////////////////////////
// GASSInputModifiersHelpers

namespace GASSInputModifiersHelpers
{
	/** Returns the owning GASSLocalPlayer of an Enhanced Player Input pointer */
	static UGASSLocalPlayer* GetLocalPlayer(const UEnhancedPlayerInput* PlayerInput)
	{
		if (PlayerInput)
		{
			if (APlayerController* PC = Cast<APlayerController>(PlayerInput->GetOuter()))
			{
				return Cast<UGASSLocalPlayer>(PC->GetLocalPlayer());
			}
		}
		return nullptr;
	}
	
}

//////////////////////////////////////////////////////////////////////
// UGASSSettingBasedScalar

FInputActionValue UGASSSettingBasedScalar::ModifyRaw_Implementation(const UEnhancedPlayerInput* PlayerInput, FInputActionValue CurrentValue, float DeltaTime)
{
	if (ensureMsgf(CurrentValue.GetValueType() != EInputActionValueType::Boolean, TEXT("Setting Based Scalar modifier doesn't support boolean values.")))
	{
		if (UGASSLocalPlayer* LocalPlayer = GASSInputModifiersHelpers::GetLocalPlayer(PlayerInput))
		{
			//const UClass* SettingsClass = UGASSSettingsShared::StaticClass();
			//UGASSSettingsShared* SharedSettings = LocalPlayer->GetSharedSettings();
			
			const bool bHasCachedProperty = PropertyCache.Num() == 3;
			
			//const FProperty* XAxisValue = bHasCachedProperty ? PropertyCache[0] : SettingsClass->FindPropertyByName(XAxisScalarSettingName);
			//const FProperty* YAxisValue = bHasCachedProperty ? PropertyCache[1] : SettingsClass->FindPropertyByName(YAxisScalarSettingName);
			//const FProperty* ZAxisValue = bHasCachedProperty ? PropertyCache[2] : SettingsClass->FindPropertyByName(ZAxisScalarSettingName);

			const FProperty* XAxisValue = bHasCachedProperty ? PropertyCache[0] : nullptr;
			const FProperty* YAxisValue = bHasCachedProperty ? PropertyCache[1] : nullptr;
			const FProperty* ZAxisValue = bHasCachedProperty ? PropertyCache[2] : nullptr;

			if (PropertyCache.IsEmpty())
			{
				PropertyCache.Emplace(XAxisValue);
				PropertyCache.Emplace(YAxisValue);
				PropertyCache.Emplace(ZAxisValue);
			}

			FVector ScalarToUse = FVector(1.0, 1.0, 1.0);

			switch (CurrentValue.GetValueType())
			{
			case EInputActionValueType::Axis3D:
				ScalarToUse.Z = 1.0;
				/*ScalarToUse.Z = ZAxisValue ? *ZAxisValue->ContainerPtrToValuePtr<double>(SharedSettings) : 1.0;*/
				//[[fallthrough]];
			case EInputActionValueType::Axis2D:
				ScalarToUse.Y = 1.0;
				/*ScalarToUse.Y = YAxisValue ? *YAxisValue->ContainerPtrToValuePtr<double>(SharedSettings) : 1.0;*/
				//[[fallthrough]];
			case EInputActionValueType::Axis1D:
				ScalarToUse.X = 1.0;
				/*ScalarToUse.X = XAxisValue ? *XAxisValue->ContainerPtrToValuePtr<double>(SharedSettings) : 1.0;*/
				break;
			}

			ScalarToUse.X = FMath::Clamp(ScalarToUse.X, MinValueClamp.X, MaxValueClamp.X);
			ScalarToUse.Y = FMath::Clamp(ScalarToUse.Y, MinValueClamp.Y, MaxValueClamp.Y);
			ScalarToUse.Z = FMath::Clamp(ScalarToUse.Z, MinValueClamp.Z, MaxValueClamp.Z);
			
			return CurrentValue.Get<FVector>() * ScalarToUse;
		}
	}
	
	return CurrentValue;	
}



//////////////////////////////////////////////////////////////////////
// UGASSInputModifierAimInversion

FInputActionValue UGASSInputModifierAimInversion::ModifyRaw_Implementation(const UEnhancedPlayerInput* PlayerInput, FInputActionValue CurrentValue, float DeltaTime)
{
	UGASSLocalPlayer* LocalPlayer = GASSInputModifiersHelpers::GetLocalPlayer(PlayerInput);
	if (!LocalPlayer)
	{
		return CurrentValue;
	}
	
	//UGASSSettingsShared* Settings = LocalPlayer->GetSharedSettings();
	//ensure(Settings);

	FVector NewValue = CurrentValue.Get<FVector>();
	
	//if (Settings->GetInvertVerticalAxis())
	if( 1 )
	{
		NewValue.Y *= -1.0f;
	}
	
	//if (Settings->GetInvertHorizontalAxis())
	if ( 0 )
	{
		NewValue.X *= -1.0f;
	}
	
	return NewValue;
}
