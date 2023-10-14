// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GASSItemWidget.generated.h"

class UGASSItemDefinition;

/**
 * 
 */
UCLASS()
class GASS_API UGASSItemWidget : public UUserWidget
{
	GENERATED_BODY()
	
protected:
	void NativeConstruct() override;
	void NativeDestruct() override;
	FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

	void NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation) override;
	void NativeOnDragCancelled(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
	bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation);

	FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;

	void NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	void NativeOnMouseLeave(const FPointerEvent& InMouseEvent) override;

	void NativeOnDragEnter(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
	void NativeOnDragLeave(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
	
public:
	UFUNCTION()
	void InitItemWidget(float InGridSize, uint16 Index, int32 NewAmount, bool InRotation, TSubclassOf<UGASSItemDefinition> ItemDef);
	
	UFUNCTION(BlueprintCallable)
	void SetIconInfo();

	UFUNCTION(BlueprintCallable)
	void SetIcon();

	UFUNCTION()
	void SetParentWidget(UUserWidget* InParentWidget);

	UFUNCTION()
	void RotateItemWidget();

	// Child Widgets
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GASSItemWidget", meta = (BindWidget))
	class UCanvasPanel* OuterCanvasPanel;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GASSItemWidget", meta = (BindWidget))
	class USizeBox* BackgroundSizeBox;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GASSItemWidget", meta = (BindWidget))
	class UBorder* BackgroundBorder;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GASSItemWidget", meta = (BindWidget))
	class UImage* ItemImage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GASSItemWidget", meta = (BindWidget))
	class UTextBlock* CountText;

	// Color Settings
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GASSItemWidget")
	FLinearColor NormalBackgroundColor;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GASSItemWidget")
	FLinearColor MouseEnterBackgroundColor;

	UPROPERTY(BlueprintReadOnly)
	float GridSize = 64.0;

	// Current Informations
	UPROPERTY(BlueprintReadOnly)
	uint8 CurrentStackCount;

	UPROPERTY(BlueprintReadOnly)
	bool Rotation;

	UPROPERTY(BlueprintReadOnly)
	int32 CurrIndex;

	// Initialized Once
	UPROPERTY(BlueprintReadOnly)
	TSubclassOf<UGASSItemDefinition> MyItemDef;

	UPROPERTY(BlueprintReadOnly)
	UMaterialInterface* IconRotateTrue;

	UPROPERTY(BlueprintReadOnly)
	UMaterialInterface* IconRotateFalse;

	UPROPERTY(BlueprintReadOnly)
	bool InitialRotation;

	// X : Shorter, Y : Longer
	UPROPERTY(BlueprintReadOnly)
	FVector2D ItemSize;

	// Caches
	UPROPERTY()
	UUserWidget* ParentWidget = nullptr;

	// Move 후 -> ParentWidget, CurrIndex, Rotation 갱신 해줘야함 (Rotation도?)
private:

};