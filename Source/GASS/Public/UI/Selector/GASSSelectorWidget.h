// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UI/Selector/GASSSelectorInfoConfig.h"
#include "AbilitySystem/Abilities/GASSGameplayAbility.h"

#include "GASSSelectorWidget.generated.h"

class UGASSSelectorInfoWidget;

/**
 * 
 */
UCLASS(Abstract)
class GASS_API UGASSSelectorWidget : public UUserWidget
{
	GENERATED_BODY()
	
protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;
	virtual FReply NativeOnKeyUp(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;

	FReply NativeOnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	void NativeOnMouseLeave(const FPointerEvent& InMouseEvent) override;

	void SetWidgetScreenPosition();
	void SetMousePosition(const FVector2f& InLocalMousePosition);

public:
	UFUNCTION(BlueprintCallable)
	void CreateInfoWidgets();

	UFUNCTION(BlueprintCallable)
	void SetNumberOfSections(int32 InNumber);

	UFUNCTION(BlueprintCallable)
	void SetWidgetsSize();

	UFUNCTION(BlueprintCallable)
	void SetMaterialInstance();

	UFUNCTION(BlueprintCallable)
	void ClearOptionsInfo();

	UFUNCTION(BlueprintCallable)
	void AddCancelOption();

	UFUNCTION(BlueprintCallable)
	void AddOptionInfo(const FName& InOptionName);

	UFUNCTION(BlueprintCallable)
	int32 GetSectionNumberByAngle(float InAngle);

	UFUNCTION(BlueprintCallable)
	void CurrentSectionChanged();

	UFUNCTION(BlueprintCallable)
	void ClearInfoWidgets();

	UFUNCTION(BlueprintImplementableEvent, Category = "GASSSelectorWidget")
	UUserWidget* CreateInfoWidgetInBP();

	UFUNCTION()
	void CreateInfoWidget(int OptionIndex);

	UFUNCTION()
	FVector2D GetPositionForInfoWidget(int Index);

	UFUNCTION()
	void SetInfoWidgetCount(int Index, int NewCount);

	UFUNCTION(BlueprintCallable)
	int32 GetCurrentSectionNumber();

	UFUNCTION(BlueprintCallable)
	void SetWidgetKey(const FName& KeyName);

	UFUNCTION(BlueprintCallable)
	virtual void OnWidgetKeyUp();
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GASSSelectorWidget", meta = (BindWidget))
	class UCanvasPanel* OuterCanvasPanel;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GASSSelectorWidget", meta = (BindWidget))
	class USizeBox* BackgroundSizeBox;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GASSSelectorWidget", meta = (BindWidget))
	class UBorder* BackgroundBorder;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GASSSelectorWidget", meta = (BindWidget))
	class UImage* SectionImage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GASSSelectorWidget", meta = (BindWidget))
	class UImage* HighlightImage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GASSSelectorWidget", meta = (BindWidget))
	class UBorder* SectionInfoBorder;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GASSSelectorWidget", meta = (BindWidget))
	class UCanvasPanel* SectionInfoCanvasPanel;

	// Color Settings
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GASSSelectorWidget")
	FLinearColor BackgroundColor;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GASSSelectorWidget")
	FLinearColor SectionColor;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GASSSelectorWidget")
	FLinearColor HighlightColor;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GASSSelectorWidget")
	FVector2D BorderSize = FVector2D(500.0f, 500.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GASSSelectorWidget")
	UMaterialInterface* SectionMI;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GASSSelectorWidget")
	UMaterialInterface* HighlightMI;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GASSSelectorWidget")
	TObjectPtr<UGASSSelectorInfoConfig> SelectorInfoConfig;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GASSSelectorWidget")
	bool ShouldAddCancelOption = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GASSSelectorWidget")
	bool ShouldRefreshEveryTime = false;
protected:
	UPROPERTY()
	TArray<FGASSSelectorOptionInfo> OptionsInfo;

	UPROPERTY()
	TArray<UGASSSelectorInfoWidget*> InfoWidgets;

	UPROPERTY()
	int32 NumberOfSections = 0;

	UPROPERTY()
	int32 CurrentSectionNumber = 1;

	UPROPERTY()
	bool ShouldUpdateNOSEveryConstruct = false;

	UPROPERTY()
	float InfoIconSize;

	UPROPERTY()
	float MouseMaxLength = 200;

	UPROPERTY()
	FVector2D WidgetScreenPosition = FVector2D(0,0);

	UPROPERTY()
	FVector2D BorderAbsoluteSize;

	UPROPERTY()
	bool ShouldUpdatePositionInfo = true;

	UPROPERTY()
	bool HasInitialized = false;

	UPROPERTY()
	FKey WidgetKey;

	UPROPERTY()
	UGASSGameplayAbility* InstigatorAbility = nullptr;
};

