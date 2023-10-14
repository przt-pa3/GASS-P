// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "IAbilitySelectorInterface.generated.h"

struct FInteractionQuery;

/**  */
UINTERFACE(MinimalAPI, meta = (CannotImplementInterfaceInBlueprint))
class UAbilitySelectorInterface : public UInterface
{
	GENERATED_BODY()
};

class IAbilitySelectorInterface
{
	GENERATED_BODY()

public:
	virtual void DeactivateWidget() = 0;
};
