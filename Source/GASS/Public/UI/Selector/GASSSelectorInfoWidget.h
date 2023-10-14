// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GASSSelectorInfoWidget.generated.h"

/**
 * 
 */
UCLASS()
class GASS_API UGASSSelectorInfoWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	void NativeConstruct() override;

public:
	UFUNCTION(BlueprintCallable)
	void InitInfoWidget(UMaterialInterface* InMI, const FText& InDescription, float IconSize);

	UFUNCTION(BlueprintCallable)
	void SetIcon();

public:
	// Child Widgets
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GASSSelectorInfoWidget", meta = (BindWidget))
	class UCanvasPanel* OuterCanvasPanel;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GASSSelectorInfoWidget", meta = (BindWidget))
	class UCanvasPanel* ImageCanvasPanel;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GASSSelectorInfoWidget", meta = (BindWidget))
	class UVerticalBox* InfoVerticalBox;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GASSSelectorInfoWidget", meta = (BindWidget))
	class UImage* IconImage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GASSSelectorInfoWidget", meta = (BindWidget))
	class UTextBlock* DescriptionText;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GASSSelectorInfoWidget", meta = (BindWidget))
	class UTextBlock* CountText;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GASSSelectorInfoWidget", meta = (BindWidget))
	float InfoIconSize = 64.0;

	UPROPERTY(BlueprintReadOnly)
	UMaterialInterface* InfoIcon;

	UPROPERTY(BlueprintReadOnly)
	uint8 CurrentStackCount;

	UPROPERTY(BlueprintReadOnly)
	FText DescriptionToShow;
};
