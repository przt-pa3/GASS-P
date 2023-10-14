// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GASSSelectorInfoConfig.generated.h"

USTRUCT(BlueprintType)
struct FGASSSelectorOptionInfo
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FName OptionName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UMaterialInterface> IconImage = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FText Description;
};

/**
 * 
 */
UCLASS()
class GASS_API UGASSSelectorInfoConfig : public UDataAsset
{
	GENERATED_BODY()
	
public:
	UGASSSelectorInfoConfig(const FObjectInitializer& ObjectInitializer);

	UFUNCTION(BlueprintCallable)
	const int32 GetOptionsIndexByOptionName(const FName& InOptionName);

	UFUNCTION(BlueprintCallable)
	FGASSSelectorOptionInfo GetSelectorOptionInfoByIndex(const int32& InIndex);

	UFUNCTION(BlueprintCallable)
	const FText GetDescriptionByIndex(const int32& InIndex);

	UFUNCTION(BlueprintCallable)
	const UMaterialInterface* GetIconImageByIndex(const int32& InIndex);

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Meta = (TitleProperty = "SelectorOption"))
	bool NeedToCount;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Meta = (TitleProperty = "SelectorOption"))
	TArray<FGASSSelectorOptionInfo> SelectorOptionsInfo;


};
