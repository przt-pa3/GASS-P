// Copyright Epic Games, Inc. All Rights Reserved.

#include "Character/GASSPawnData.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GASSPawnData)

UGASSPawnData::UGASSPawnData(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PawnClass = nullptr;
	InputConfig = nullptr;
	DefaultCameraMode = nullptr;
	UE_LOG(LogTemp, Warning, TEXT("PawnData Const."));
}

