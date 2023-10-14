// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/IUserObjectListEntry.h"
#include "GASSEquipmentSlot.generated.h"

class UGASSEquipmentManagerComponent;
class UGASSItemWidget;

/**
 * 
 */
UCLASS(Abstract)
class GASS_API UGASSEquipmentSlot : public UUserWidget, public IUserObjectListEntry
{
	GENERATED_BODY()

protected:
	

public:
	// Initializing Functions
	UFUNCTION(BlueprintCallable)
	void InitEquipmentSlot(UWidget* InManager, float InGridSize, float InEdgeThickness, int32 RowSize, int32 ColSize);

	UFUNCTION(BlueprintCallable)
	void SetEquipmentSlotSize();

	// Refresh
	UFUNCTION(BlueprintCallable)
	void Refresh();

	UFUNCTION(BlueprintCallable)
	void HideItemWidget();

	UFUNCTION(BlueprintCallable)
	void RemoveItemWidget();

	UFUNCTION(BlueprintCallable)
	void ShowHiddenItemWidget();

	UFUNCTION()
	void PlaceItemWidget(UGASSItemWidget* WidgetToPlace);

	UFUNCTION()
	void TemporaryPlaceItemWidget(UGASSItemWidget* WidgetToPlace);

	UFUNCTION()
	void ConfirmPlacedItemWidget();
public:
	// Child Widgets
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GASSEquipmentWidget", meta = (BindWidget))
	class UCanvasPanel* OuterCanvasPanel;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GASSEquipmentWidget", meta = (BindWidget))
	class UBorder* EdgeBorder;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GASSEquipmentWidget", meta = (BindWidget))
	class UBorder* SlotBorder;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GASSEquipmentWidget", meta = (BindWidget))
	class UCanvasPanel* SlotCanvasPanel;

	// Set in BP Properties
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GASSEquipment")
	bool bImMainEquipmentSlot = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GASSEquipment")
	int32 EquipmentIndex = -1;

	// Set in Parent
	UPROPERTY(BlueprintReadOnly, Category = "GASSEquipment")
	float GridSize = 64.0;

	UPROPERTY(BlueprintReadOnly, Category = "GASSEquipment")
	float EdgeThickness = 10.0;

	UPROPERTY(BlueprintReadOnly, Category = "GASSEquipment")
	FVector2D SlotBorderSize;

	// Size Information
	UPROPERTY(BlueprintReadOnly, Category = "GASSEquipment")
	int32 SlotMaxCol;

	UPROPERTY(BlueprintReadOnly, Category = "GASSEquipment")
	int32 SlotMaxRow;

public:
	UPROPERTY()
	UGASSItemWidget* EquippedItemWidget = nullptr;

	UPROPERTY()
	UWidget* EquipmentManager = nullptr;
};
