// Fill out your copyright notice in the Description page of Project Settings.


#include "Interactables/CurveFloatsManager.h"
#include "Kismet/KismetStringLibrary.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(CurveFloatsManager)

UCurveFloatsManager* UCurveFloatsManager::SingletonManager = nullptr;

UCurveFloatsManager::UCurveFloatsManager(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
	static ConstructorHelpers::FObjectFinder<UCurveFloat> BaseCurveFloat(TEXT("/Game/Interactables/CurveBase.CurveBase"));
	UCurveFloat* BaseCurve = BaseCurveFloat.Object;
	if (BaseCurve)
	{
		BaseCurveInfo = BaseCurve->ExportAsJSONString();
		SetStringParts();
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Cannot Find BaseCurveInfo"));
	}
}

UCurveFloatsManager::~UCurveFloatsManager()
{
	SingletonManager = nullptr;
}

void UCurveFloatsManager::InitializeManager()
{
	check(!SingletonManager);
	SCOPED_BOOT_TIMING("UCurveFloatsManager::InitializeManager");

	SingletonManager = NewObject<UCurveFloatsManager>();
}

void UCurveFloatsManager::LoadBaseCurveFloat()
{
	//UGASSAssetManager& AssetManager = UGASSAssetManager::Get();
	//AssetManager.GetAsset(Pair.Config);
}

void UCurveFloatsManager::SetStringParts()
{
	if (BaseCurveInfo.Len() == 0)
	{
		UE_LOG(LogTemp, Error, TEXT("BaseCurveInfo null"));
		return;
	}
	if (StringParts.Num() > 0)
	{
		UE_LOG(LogTemp, Error, TEXT("StringParts Already Initialized"));
		return;
	}

	FString TimeString = "time\": ";
	const TCHAR* TimeTCHAR = &(TimeString)[0];

	FString CommaString = ",";
	const TCHAR* CommaTCHAR = &(CommaString)[0];

	if (BaseCurveInfo.Contains(TimeString))
	{
		FString Copy(MoveTemp(BaseCurveInfo));
		TCHAR* WritePosition = (TCHAR*)Copy.GetCharArray().GetData();
		TCHAR* TimeSearchPosition = FCString::Stristr(WritePosition, TimeTCHAR);
		TCHAR* CommaSearchPosition;

		while (TimeSearchPosition != nullptr)
		{
			// replace the first letter of the From with 0 so we can do a strcpy (FString +=)
			*TimeSearchPosition = TEXT('\0');

			FString StringPart;
			StringPart += WritePosition;
			StringPart += TimeString;
			
			StringParts.Add(StringPart);
			
			// restore the letter, just so we don't have 0's in the string
			*TimeSearchPosition = *TimeTCHAR;

			CommaSearchPosition = FCString::Stristr(TimeSearchPosition, CommaTCHAR);

			WritePosition = CommaSearchPosition;

			TimeSearchPosition = FCString::Stristr(WritePosition, TimeTCHAR);
		}

		FString LastStringPart;
		LastStringPart += WritePosition;
		StringParts.Add(LastStringPart);
	}

	//UE_LOG(LogTemp, Error, TEXT("String Handle Result : "));
	//for (int32 i = 0; i < 5; ++i)
	//{
	//	UE_LOG(LogTemp, Error, TEXT("%d"), i);
	//	UE_LOG(LogTemp, Warning, TEXT("%s"), *StringParts[i]);
	//}

}

const UCurveFloat* UCurveFloatsManager::GetCurveFloat(float TimeLength, float TimeOffset)
{
	TTuple<float, float> Key = MakeTuple(TimeLength, TimeOffset);

	UCurveFloat** Target = CurveFloatMap.Find(Key);
	if (Target != nullptr)
	{
		return (*Target);
	}

	const UCurveFloat* NewCurveFloat = GenerateCurveFloat(TimeLength, TimeOffset);

	return NewCurveFloat;
}

const UCurveFloat* UCurveFloatsManager::GenerateCurveFloat(float TimeLength, float TimeOffset)
{
	FString NewCurveInfo;

	for (int32 i = 0; i < 4; ++i)
	{
		NewCurveInfo += StringParts[i];

		float KeyTime = i < 2 ? 0 : TimeLength;
		if (i == 1)
		{
			KeyTime += TimeOffset;
		}
		if (i == 2)
		{
			KeyTime -= TimeOffset;
		}

		NewCurveInfo += UKismetStringLibrary::Conv_FloatToString(KeyTime);
	}
	NewCurveInfo += StringParts[4];

	FJsonSerializableArray JsonProblems;
	UCurveFloat* NewCurveFloat = NewObject<UCurveFloat>();
	NewCurveFloat->ImportFromJSONString(NewCurveInfo, OUT JsonProblems);
	
	TTuple<float, float> Key = MakeTuple(TimeLength, TimeOffset);
	CurveFloatMap.Add(Key, NewCurveFloat);

	return NewCurveFloat;
}




