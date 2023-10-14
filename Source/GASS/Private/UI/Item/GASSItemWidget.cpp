// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Item/GASSItemWidget.h"
#include "UI/Inventory/GASSInventoryGrid.h"
#include "UI/Equipment/GASSEquipmentSlot.h"
#include "UI/Item/GASSItemDragDrop.h"

#include "Components/Image.h"
#include "Components/Border.h"
#include "Components/SizeBox.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"

#include "Item/GASSItemDefinition.h"
#include "Item/Fragments/GASSItemFragment_Essential.h"
#include "Inventory/GASSInventoryManagerComponent.h"


void UGASSItemWidget::NativeConstruct()
{
	Super::NativeConstruct();
	UE_LOG(LogTemp, Warning, TEXT("ItemWidget Const"));
}
void UGASSItemWidget::NativeDestruct()
{
	UE_LOG(LogTemp, Warning, TEXT("Destruct"));
	Super::NativeDestruct();
}


FReply UGASSItemWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	UE_LOG(LogTemp, Warning, TEXT("MouseButtonDown !"));

	// Set Detect Drag in BP
	return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}

void UGASSItemWidget::NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation)
{
	Super::NativeOnDragDetected(InGeometry, InMouseEvent, OutOperation);

	UGASSItemDragDrop* DragDropOperation = NewObject<UGASSItemDragDrop>();
	DragDropOperation->DefaultDragVisual = this;
	DragDropOperation->Pivot = EDragPivot::MouseDown;
	DragDropOperation->Offset = FVector2D(0,0);
	DragDropOperation->Payload = this;
	InitialRotation = Rotation;

	FVector2f StartPosition = InGeometry.AbsoluteToLocal(InMouseEvent.GetScreenSpacePosition());
	int32 GridCol = (int)floor(StartPosition.X / GridSize);
	int32 GridRow = (int)floor(StartPosition.Y / GridSize);

	float OffsetX = StartPosition.X - GridCol * GridSize;
	float OffsetY = StartPosition.Y - GridRow * GridSize;
	
	// Clamp GridCol, Row, Offset XY
	if (GridCol < 0)
	{
		GridCol = 0;
		OffsetX = 0;
	}
	if (GridRow < 0)
	{
		GridRow = 0;
		OffsetY = 0;
	}
	if (Rotation)
	{
		DragDropOperation->SetItemInfo(ItemSize.Y / GridSize, ItemSize.X / GridSize);
		if (GridCol >= ItemSize.X / GridSize)
		{
			GridCol--;
			OffsetX = GridSize - 0.1;
		}
		if (GridRow >= ItemSize.Y / GridSize)
		{
			GridRow--;
			OffsetY = GridSize - 0.1;
		}
	}
	else
	{
		DragDropOperation->SetItemInfo(ItemSize.X / GridSize, ItemSize.Y / GridSize);
		if (GridCol >= ItemSize.Y / GridSize)
		{
			GridCol--;
			OffsetX = GridSize - 0.1;
		}
		if (GridRow >= ItemSize.X / GridSize)
		{
			GridRow--;
			OffsetY = GridSize - 0.1;
		}
	}

	OffsetX /= GridSize;
	OffsetY /= GridSize;

	DragDropOperation->SetPositionInfo(GridCol, GridRow, OffsetX, OffsetY);

	// ItemWidget이 GridWidget 위에 있는게 아니라, Equipment Slot 위에 있을 수도 있다 !
	UGASSInventoryGrid* GridWidget = Cast<UGASSInventoryGrid>(ParentWidget);

	if (GridWidget)
	{
		int32 PositionInGridRow = (CurrIndex) / GridWidget->InventoryMaxCol;
		int32 PositionInGridCol = (CurrIndex) % GridWidget->InventoryMaxCol;

		GridWidget->GridCanvasPanel->RemoveChild(this);
	}
	else
	{
		// Check ParentWidget == EquipmentSlot
		UGASSEquipmentSlot* EquipSlot = Cast<UGASSEquipmentSlot>(ParentWidget);
		if (EquipSlot)
		{
			EquipSlot->HideItemWidget();
		}
	}

	OutOperation = DragDropOperation;
}

void UGASSItemWidget::NativeOnDragCancelled(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	UE_LOG(LogTemp, Warning, TEXT("ItemWidget :: Drag Canceled"));
	Super::NativeOnDragCancelled(InDragDropEvent, InOperation);
	
	// 다시 원래 있던 자리에 만들어야됨
	UGASSInventoryGrid* GridWidget = Cast<UGASSInventoryGrid>(ParentWidget);
	if (GridWidget != nullptr)
	{
		GridWidget->PlaceItemWidget(this, CurrIndex, InitialRotation);
	}
	else
	{
		// Check ParentWidget == EquipmentSlot
		UGASSEquipmentSlot* EquipSlot = Cast<UGASSEquipmentSlot>(ParentWidget);
		if (EquipSlot)
		{
			EquipSlot->PlaceItemWidget(this);
		}
	}
}

bool UGASSItemWidget::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	UE_LOG(LogTemp, Warning, TEXT("Drop On ItemWidget"));

	UGASSItemDragDrop* DragDropOper = Cast<UGASSItemDragDrop>(InOperation);
	check(DragDropOper);

	UGASSItemWidget* DroppedItemWidget = Cast<UGASSItemWidget>(DragDropOper->Payload);
	check(DroppedItemWidget);

	if (MyItemDef == DroppedItemWidget->MyItemDef)
	{
		const UGASSItemDefinition* ItemCDO = GetDefault<UGASSItemDefinition>(MyItemDef);
		int32 Stackable = ItemCDO->MaxStackCount - CurrentStackCount;

		// Check Stackable
		if (ItemCDO->bCanStack && (Stackable > 0))
		{
			UGASSInventoryGrid* MyParentGrid = Cast<UGASSInventoryGrid>(ParentWidget);
			UGASSInventoryGrid* OtherParentGrid = Cast<UGASSInventoryGrid>(DroppedItemWidget->ParentWidget);
			if (MyParentGrid == nullptr || OtherParentGrid == nullptr)
			{
				return false;
			}

			if (Stackable >= DroppedItemWidget->CurrentStackCount)
			{
				// 전부 쌓기
				MyParentGrid->PlayerIMC->ClientRequestStackItem_IMCtoIMC(MyItemDef, OtherParentGrid->MyIMC, MyParentGrid->MyIMC, DroppedItemWidget->CurrIndex, CurrIndex, DroppedItemWidget->CurrentStackCount, OtherParentGrid->bImBlackbox, MyParentGrid->bImBlackbox);
				MyParentGrid->ClearCache();
				return true;
			}
			else
			{
				// Stackable 만큼 쌓고, DroppedItemWidget StackCount 빼주고, ItemWidget 다시 만들어야됨
				MyParentGrid->PlayerIMC->ClientRequestStackItem_IMCtoIMC(MyItemDef, OtherParentGrid->MyIMC, MyParentGrid->MyIMC, DroppedItemWidget->CurrIndex, CurrIndex, Stackable, OtherParentGrid->bImBlackbox, MyParentGrid->bImBlackbox);
				OtherParentGrid->PlaceItemWidget(DroppedItemWidget, DroppedItemWidget->CurrIndex, DroppedItemWidget->InitialRotation);
				MyParentGrid->ClearCache();
				return true;
			}
		}
		else
		{
			// Swap !
		}
	}
	else
	{
		if (ItemSize == DroppedItemWidget->ItemSize)
		{
			// Swap !
		}
		else
		{
		}
	}
	UGASSInventoryGrid* MyParentGrid = Cast<UGASSInventoryGrid>(ParentWidget);
	if (MyParentGrid != nullptr)
	{
		MyParentGrid->ClearCache();
	}
	return false;
}

FReply UGASSItemWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{	
	return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

void UGASSItemWidget::NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	BackgroundBorder->SetBrushColor(MouseEnterBackgroundColor);
	Super::NativeOnMouseEnter(InGeometry, InMouseEvent);
}

void UGASSItemWidget::NativeOnMouseLeave(const FPointerEvent& InMouseEvent)
{
	BackgroundBorder->SetBrushColor(NormalBackgroundColor);
	Super::NativeOnMouseLeave(InMouseEvent);
}

void UGASSItemWidget::NativeOnDragEnter(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	Super::NativeOnDragEnter(InGeometry, InDragDropEvent, InOperation);

	UGASSInventoryGrid* ParentGrid = Cast<UGASSInventoryGrid>(ParentWidget);
	if (ParentGrid == nullptr)
	{
		return;
	}

	UGASSItemDragDrop* DragDropOper = Cast<UGASSItemDragDrop>(InOperation);
	check(DragDropOper);

	UGASSItemWidget* DroppedItemWidget = Cast<UGASSItemWidget>(DragDropOper->Payload);
	check(DroppedItemWidget);

	if (MyItemDef == DroppedItemWidget->MyItemDef)
	{
		const UGASSItemDefinition* ItemCDO = GetDefault<UGASSItemDefinition>(MyItemDef);
		int32 Stackable = ItemCDO->MaxStackCount - CurrentStackCount;

		// Check Stackable
		if (ItemCDO->bCanStack && (Stackable > 0))
		{
			//BoxInfo = FVector4f(ColStart, RowStart, ColSize, RowSize) * GridSize;

			int32 ColStart = CurrIndex % ParentGrid->InventoryMaxCol;
			int32 RowStart = CurrIndex / ParentGrid->InventoryMaxCol;

			float ColSize = Rotation ? ItemSize.X : ItemSize.Y;
			float RowSize = Rotation ? ItemSize.Y : ItemSize.X;

			ParentGrid->BoxInfoFromItemWidget = FVector4f(ColStart * (ParentGrid->GridSize), RowStart * (ParentGrid->GridSize), ColSize, RowSize);
			ParentGrid->DrawBoxFromItemWidget = true;
			UE_LOG(LogTemp, Warning, TEXT(" Over Item True "));
			return;
		}
	}
	ParentGrid->DrawBoxFromItemWidget = false;
	return;

}

void UGASSItemWidget::NativeOnDragLeave(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	Super::NativeOnDragLeave(InDragDropEvent, InOperation);

	UGASSInventoryGrid* ParentGrid = Cast<UGASSInventoryGrid>(ParentWidget);
	if (ParentGrid == nullptr)
	{
		return;
	}

	ParentGrid->DrawBoxFromItemWidget = false;
}

void UGASSItemWidget::InitItemWidget(float InGridSize, uint16 Index, int32 NewAmount, bool InRotation, TSubclassOf<UGASSItemDefinition> ItemDef)
{
	GridSize = InGridSize;
	CurrIndex = Index;
	CurrentStackCount = NewAmount;
	Rotation = InRotation;
	MyItemDef = ItemDef;

	SetIconInfo();
	SetIcon();
}

void UGASSItemWidget::SetIcon()
{
	UE_LOG(LogTemp, Warning, TEXT("SetIcon"));

	UCanvasPanelSlot* ItemImageToSlot = Cast<UCanvasPanelSlot>(ItemImage->Slot);
	BackgroundBorder->SetBrushColor(NormalBackgroundColor);

	if (Rotation)
	{
		BackgroundSizeBox->SetWidthOverride(ItemSize.X);
		BackgroundSizeBox->SetHeightOverride(ItemSize.Y);
		ItemImage->SetBrushFromMaterial(IconRotateTrue);

		ItemImageToSlot->SetSize(ItemSize);
	}
	else
	{
		BackgroundSizeBox->SetWidthOverride(ItemSize.Y);
		BackgroundSizeBox->SetHeightOverride(ItemSize.X);
		ItemImage->SetBrushFromMaterial(IconRotateFalse);

		FVector2D Temp(ItemSize.Y, ItemSize.X);
		ItemImageToSlot->SetSize(Temp);
	}
}

void UGASSItemWidget::SetIconInfo()
{
	UE_LOG(LogTemp, Warning, TEXT("SetIconInfo"));

	if (MyItemDef == nullptr)
	{
		return;
	}
	const UGASSItemDefinition* ItemCDO = GetDefault<UGASSItemDefinition>(MyItemDef);
	UGASSItemFragment_Essential* EssentialFrag = Cast<UGASSItemFragment_Essential>(ItemCDO->EssentialFragment);
	
	if (EssentialFrag == nullptr)
	{
		return;
	}
	
	ItemSize.X = (ItemCDO->ItemSizeShorter) * GridSize;
	ItemSize.Y = (ItemCDO->ItemSizeLonger) * GridSize;

	IconRotateTrue = EssentialFrag->InventoryIconRotTrue;
	IconRotateFalse = EssentialFrag->InventoryIconRotFalse;

	if (IconRotateTrue == nullptr || IconRotateTrue == nullptr)
	{
		return;
	}
}

void UGASSItemWidget::SetParentWidget(UUserWidget* InParentWidget)
{
	ParentWidget = InParentWidget;
}

void UGASSItemWidget::RotateItemWidget()
{
	UE_LOG(LogTemp, Warning, TEXT("Rotate Item Widget Called"));

	UCanvasPanelSlot* ItemImageToSlot = Cast<UCanvasPanelSlot>(ItemImage->Slot);
	Rotation = !Rotation;

	if (Rotation)
	{
		BackgroundSizeBox->SetWidthOverride(ItemSize.X);
		BackgroundSizeBox->SetHeightOverride(ItemSize.Y);
		ItemImage->SetBrushFromMaterial(IconRotateTrue);
		UE_LOG(LogTemp, Error, TEXT("SizeBox : %f, %f"), ItemSize.X, ItemSize.Y);
		UE_LOG(LogTemp, Error, TEXT("Slot : %f, %f"), ItemSize.X, ItemSize.Y);
		ItemImageToSlot->SetSize(ItemSize);
	}
	else
	{
		BackgroundSizeBox->SetWidthOverride(ItemSize.Y);
		BackgroundSizeBox->SetHeightOverride(ItemSize.X - 40);
		ItemImage->SetBrushFromMaterial(IconRotateFalse);

		FVector2D Temp(ItemSize.Y, ItemSize.X);

		UE_LOG(LogTemp, Error, TEXT("SizeBox : %f, %f"), ItemSize.Y, ItemSize.X);
		UE_LOG(LogTemp, Error, TEXT("Slot : %f, %f"), Temp.X, Temp.Y);
		ItemImageToSlot->SetSize(Temp);
	}
}

