// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Engine/DataAsset.h"
#include "UI/Equipment/EquipmentSlotSizeConfig.h"
#include "GASSEquipmentManagerWidget.generated.h"

class UGASSInventoryManagerComponent;
class UGASSEquipmentManagerComponent;
class UGASSEquipmentSlot;
class UGASSItemWidget;

/**
 * 
 */
UCLASS(Abstract)
class GASS_API UGASSEquipmentManagerWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;
	virtual bool NativeOnDragOver(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
	virtual void NativeOnDragLeave(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation);
	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;

	bool TrySetEMC();
	bool RegisterEMCDelegate();

	void InitMainEquipmentSlots();
	void InitAdditionalSlots();

	void SetBoxInfoWithCache();
	void ClearCache();

	UGASSInventoryManagerComponent* GetInventoryManagerComponent();
public:
	UFUNCTION(BlueprintCallable)
	void Refresh();

// Non Use Now
	UFUNCTION(BlueprintImplementableEvent, Category = "GASSItemWidget")
	UGASSItemWidget* CreateItemWidgetInBP();

	UFUNCTION(BlueprintCallable)
	void CreateItemWidget();
// ~ Non Use Now

	UFUNCTION(BlueprintImplementableEvent, Category = "GASSItemWidget")
	UGASSEquipmentSlot* CreateSlotWidgetInBP();

	UFUNCTION(BlueprintCallable)
	UGASSEquipmentSlot* CreateSlotWidget(int32 TargetIndex);

	UFUNCTION()
	void DestroySlotWidget(int32 Index);

	UFUNCTION()
	FVector2D GetNextAdditionalSlotPosition();

	UFUNCTION()
	FVector2D GetAdditionalSlotPositionByIndex(int32 Index);

	UFUNCTION(BlueprintCallable)
	void PlaceItemWidget(UGASSItemWidget* WidgetToPlace, bool IsMainEquipment, int32 Index);

	UFUNCTION(BlueprintCallable)
	void TemporaryPlaceItemWidget(UGASSItemWidget* WidgetToPlace, bool IsMainEquipment, int32 Index);

	UFUNCTION()
	void ConfirmMovedItemWidget();

	//DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FGASSEquipment_Equipped, bool, IsMainEquipment, uint16, Index, TSubclassOf<UGASSItemDefinition>, ItemDef);
	//DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FGASSEquipment_Unequipped, bool, IsMainEquipment, uint16, Index);

	// EMC Delegates
	UFUNCTION()
	void OnEntryEquipped(bool IsMainEquipment, uint16 Index, TSubclassOf<UGASSItemDefinition> ItemDef);

	UFUNCTION()
	void OnEntryUnequipped(bool IsMainEquipment, uint16 Index);

	UFUNCTION()
	void OnEntryEquipCanceled(bool IsMainEquipment, uint16 Index);

	UFUNCTION()
	void OnEntryUnequipCanceled(bool IsMainEquipment, uint16 Index);

	
public:
	// Child Widgets
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GASSEquipmentWidget", meta = (BindWidget))
	class UCanvasPanel* OuterCanvasPanel;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GASSEquipmentWidget", meta = (BindWidget))
	class UBorder* BackGroundBorder;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GASSEquipmentWidget", meta = (BindWidget))
	class UCanvasPanel* SlotCanvasPanel;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GASSEquipmentWidget", meta = (BindWidget))
	class UBorder* AdditionalSlotBackGroundBorder;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GASSEquipmentWidget", meta = (BindWidget))
	class UCanvasPanel* AdditionalSlotCanvasPanel;

	// Set in BP Properties
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GASSEquipment")
	TObjectPtr<UDataAsset> SlotSizeConfig;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GASSEquipment")
	float SlotGridSize = 64.0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GASSEquipment")
	float MainSlotEdgeThickness = 10.0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GASSEquipment")
	float AdditionalSlotEdgeThickness = 5.0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GASSEquipment")
	FVector2D AdditionalSlotSize;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GASSEquipment")
	FLinearColor AbleToPlaceColor;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GASSEquipment")
	FLinearColor UnableToPlaceColor;

public:
	UPROPERTY()
	UGASSEquipmentManagerComponent* MyEMC = nullptr;

protected:
	UPROPERTY()
	TArray<UGASSEquipmentSlot*> MainEquipmentSlots;

	UPROPERTY()
	TArray<UGASSEquipmentSlot*> AdditionalEquipmentSlots;

	UPROPERTY()
	FVector2D AdditionalSlotsBasePosition;

	UPROPERTY(BlueprintReadOnly, Category = "GASSEquipmentWidget")
	FVector4f BoxInfo = FVector4f(0, 0, 0, 0);

	UPROPERTY(BlueprintReadOnly, Category = "GASSEquipmentWidget")
	bool DrawBox = false;

	UPROPERTY(BlueprintReadOnly, Category = "GASSEquipmentWidget")
	bool AbleToEquip = false;

protected:
	UPROPERTY()
	UGASSItemWidget* ItemWidgetCache = nullptr;

	UPROPERTY()
	bool IsCachedWidgetMain = false;

	UPROPERTY()
	int32 CachedWidgetIndex = -1;

	UPROPERTY()
	bool CanEquipItem = false;



	//UPROPERTY()
	//UGASSItemWidget* TempPlacedWidget = nullptr;

	//UPROPERTY()
	//bool IsTempPlacedWidgetMain = false;

	//UPROPERTY()
	//int32 TempPlacedWidgetIndex = -1;
};
