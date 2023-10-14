// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "InputModifiers.h"

#include "UObject/UnrealType.h"
#include "GASSInputModifiers.generated.h"

struct FInputActionValue;

class FProperty;
class UEnhancedPlayerInput;
class UGASSAimSensitivityData;
class UObject;

/** 
*  Scales input basedon a double property in the SharedUserSettings
*/
UCLASS(NotBlueprintable, MinimalAPI, meta = (DisplayName = "Setting Based Scalar"))
class UGASSSettingBasedScalar : public UInputModifier
{
	GENERATED_BODY()

public:

	/** Name of the property that will be used to clamp the X Axis of this value */
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category=Settings)
	FName XAxisScalarSettingName = NAME_None;

	/** Name of the property that will be used to clamp the Y Axis of this value */
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category=Settings)
	FName YAxisScalarSettingName = NAME_None;

	/** Name of the property that will be used to clamp the Z Axis of this value */
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category=Settings)
	FName ZAxisScalarSettingName = NAME_None;
	
	/** Set the maximium value of this setting on each axis. */
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category=Settings)
	FVector MaxValueClamp = FVector(10.0, 10.0, 10.0);
	
	/** Set the minimum value of this setting on each axis. */
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category=Settings)
	FVector MinValueClamp = FVector::ZeroVector;

protected:
	virtual FInputActionValue ModifyRaw_Implementation(const UEnhancedPlayerInput* PlayerInput, FInputActionValue CurrentValue, float DeltaTime) override;

	/** FProperty Cache that will be populated with any found FProperty's on the settings class so that we don't need to look them up each frame */
	TArray<const FProperty*> PropertyCache;
};


/** The type of targeting sensitity that should be considered */
UENUM()
enum class EGASSTargetingType : uint8
{
	/** Sensitivity to be applied why normally looking around */
	Normal = 0,

	/** The sensitivity that should be applied while Aiming Down Sights */
	ADS = 1,
};

/** Applies an inversion of axis values based on a setting in the GASS Shared game settings */
UCLASS(NotBlueprintable, MinimalAPI, meta = (DisplayName = "GASS Aim Inversion Setting"))
class UGASSInputModifierAimInversion : public UInputModifier
{
	GENERATED_BODY()
	
protected:
	virtual FInputActionValue ModifyRaw_Implementation(const UEnhancedPlayerInput* PlayerInput, FInputActionValue CurrentValue, float DeltaTime) override;	
};
