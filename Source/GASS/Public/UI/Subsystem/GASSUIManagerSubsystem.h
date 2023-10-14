// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Containers/Ticker.h"
#include "GameUIManagerSubsystem.h"

#include "GASSUIManagerSubsystem.generated.h"

class FSubsystemCollectionBase;
class UObject;

UCLASS()
class UGASSUIManagerSubsystem : public UGameUIManagerSubsystem
{
	GENERATED_BODY()

public:

	UGASSUIManagerSubsystem();

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

private:
	bool Tick(float DeltaTime);
	void SyncRootLayoutVisibilityToShowHUD();
	
	FTSTicker::FDelegateHandle TickHandle;
};
