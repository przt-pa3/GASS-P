// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CommonGameViewportClient.h"

#include "GASSGameViewportClient.generated.h"

class UGameInstance;
class UObject;

UCLASS(BlueprintType)
class UGASSGameViewportClient : public UCommonGameViewportClient
{
	GENERATED_BODY()

public:
	UGASSGameViewportClient();

	virtual void Init(struct FWorldContext& WorldContext, UGameInstance* OwningGameInstance, bool bCreateNewAudioDevice = true) override;
};
