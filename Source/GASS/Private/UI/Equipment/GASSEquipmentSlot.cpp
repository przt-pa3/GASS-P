// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Equipment/GASSEquipmentSlot.h"
#include "UI/Inventory/GASSInventoryGrid.h"
#include "UI/Item/GASSItemWidget.h"
#include "UI/Item/GASSItemDragDrop.h"

#include "Equipment/GASSEquipmentManagerComponent.h"
//#include "Inventory/GASSInventoryManagerComponent.h"
#include "Inventory/GASSInventoryItemInstance.h"

#include "Components/Border.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Blueprint/WidgetTree.h"
#include "Blueprint/WidgetBlueprintLibrary.h"

#include "Components/Image.h"

void UGASSEquipmentSlot::InitEquipmentSlot(UWidget* InManager, float InGridSize, float InEdgeThickness, int32 RowSize, int32 ColSize)
{
	EquipmentManager = InManager;
	GridSize = InGridSize;
	EdgeThickness = InEdgeThickness;

	if (bImMainEquipmentSlot)
	{
		SlotMaxRow = RowSize;
		SlotMaxCol = ColSize;
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Slot is Additional"));
		SlotMaxCol = 1;
		SlotMaxRow = 1;
	}

	SlotBorderSize.X = GridSize * SlotMaxRow;
	SlotBorderSize.Y = GridSize * SlotMaxCol;
	UE_LOG(LogTemp, Error, TEXT("Slot Size : %f"), SlotBorderSize.X);
	SetEquipmentSlotSize();
}

void UGASSEquipmentSlot::SetEquipmentSlotSize()
{
	UCanvasPanelSlot* SlotBorderToSlot = Cast<UCanvasPanelSlot>(SlotBorder->Slot);
	SlotBorderToSlot->SetSize(SlotBorderSize);

	UCanvasPanelSlot* EdgeBorderToSlot = Cast<UCanvasPanelSlot>(EdgeBorder->Slot);
	EdgeBorderToSlot->SetSize(SlotBorderSize + FVector2D(EdgeThickness, EdgeThickness));

	UE_LOG(LogTemp, Warning, TEXT("Border : %f, %f // Edge : %f, %f"), SlotBorderToSlot->GetSize().X, SlotBorderToSlot->GetSize().Y, EdgeBorderToSlot->GetSize().X, EdgeBorderToSlot->GetSize().Y);
	//UE_LOG(LogTemp, Warning, TEXT("GridBoarderSize : %f, %f"), SlotBorderToSlot->GetSize().X, SlotBorderToSlot->GetSize().Y);
}

void UGASSEquipmentSlot::Refresh()
{
	// Do Something
	UE_LOG(LogTemp, Error, TEXT("Test :: Refresh"));
	if (EquippedItemWidget)
	{
		UE_LOG(LogTemp, Error, TEXT("Has Equipped"));
		UCanvasPanelSlot* NewItemWidgetSlot = SlotCanvasPanel->AddChildToCanvas(EquippedItemWidget);
		int32 Count = SlotCanvasPanel->GetChildrenCount();

		UE_LOG(LogTemp, Error, TEXT("Children : %d"), Count);
	}

}

void UGASSEquipmentSlot::HideItemWidget()
{
	SlotCanvasPanel->ClearChildren();
}

void UGASSEquipmentSlot::RemoveItemWidget()
{
	SlotCanvasPanel->ClearChildren();
	EquippedItemWidget = nullptr;
}

void UGASSEquipmentSlot::ShowHiddenItemWidget()
{
	check(EquippedItemWidget);
	PlaceItemWidget(EquippedItemWidget);
}

void UGASSEquipmentSlot::PlaceItemWidget(UGASSItemWidget* WidgetToPlace)
{
	TemporaryPlaceItemWidget(WidgetToPlace);
	ConfirmPlacedItemWidget();
}

void UGASSEquipmentSlot::TemporaryPlaceItemWidget(UGASSItemWidget* WidgetToPlace)
{

	UE_LOG(LogTemp, Error, TEXT("Slot :: Temp Place Item Widget !"));

	EquippedItemWidget = WidgetToPlace;

	UCanvasPanelSlot* NewItemWidgetSlot = SlotCanvasPanel->AddChildToCanvas(WidgetToPlace);
	NewItemWidgetSlot->SetAutoSize(true);

	// SetPosition to Middle Somehow
	FVector2D NewPosition(0, 0);
	if (WidgetToPlace->Rotation)
	{

	}
	WidgetToPlace->ItemSize;
	NewItemWidgetSlot->SetPosition(NewPosition);
	check(EquippedItemWidget);
	// ~ SetPosition to Middle Somehow
}

void UGASSEquipmentSlot::ConfirmPlacedItemWidget()
{
	check(EquippedItemWidget);
	EquippedItemWidget->SetParentWidget(this);
	EquippedItemWidget->CurrIndex = EquipmentIndex;
}