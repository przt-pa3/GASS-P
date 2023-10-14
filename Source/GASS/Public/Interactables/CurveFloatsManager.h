// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CurveFloatsManager.generated.h"

/**
 * 
 */
UCLASS(config=Engine)
class GASS_API UCurveFloatsManager : public UObject
{
	GENERATED_UCLASS_BODY()

	~UCurveFloatsManager();

	FORCEINLINE static UCurveFloatsManager& Get()
	{
		if (SingletonManager == nullptr)
		{
			InitializeManager();
		}

		return *SingletonManager;
	}

private:
	static void InitializeManager();

	static UCurveFloatsManager* SingletonManager;

public:
	void LoadBaseCurveFloat();

	void SetStringParts();

	const UCurveFloat* GetCurveFloat(float TimeLength, float TimeOffset);

private:
	const UCurveFloat* GenerateCurveFloat(float TimeLength, float TimeOffset);

private:
	FString BaseCurveInfo;

	TArray<FString> StringParts;

	TMap<TTuple<float,float>, UCurveFloat*> CurveFloatMap;
};
