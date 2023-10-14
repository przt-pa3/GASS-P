// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Equipment/GASSEquipmentManagerWidget.h"
#include "UI/Equipment/GASSEquipmentSlot.h"
#include "UI/Inventory/GASSInventoryGrid.h"
#include "UI/Item/GASSItemWidget.h"
#include "UI/Item/GASSItemDragDrop.h"

#include "Player/GASSPlayerController.h"
#include "Equipment/GASSEquipmentManagerComponent.h"
#include "Item/GASSItemDefinition.h"
#include "Inventory/GASSInventoryManagerComponent.h"
#include "Inventory/GASSInventoryItemInstance.h"

#include "Components/TileView.h"
#include "Components/Border.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Blueprint/WidgetTree.h"
#include "Blueprint/WidgetBlueprintLibrary.h"

#include "Components/Image.h"


void UGASSEquipmentManagerWidget::NativeOnInitialized()
{
	if (!TrySetEMC())
	{
		UE_LOG(LogTemp, Error, TEXT("Cannot Set EMC"));
		return;
	}

	if (!RegisterEMCDelegate())
	{
		UE_LOG(LogTemp, Error, TEXT("Cannot Register EMC Delegate"));
		return;
	}


	InitMainEquipmentSlots();
	InitAdditionalSlots();
	Refresh();
}

void UGASSEquipmentManagerWidget::NativeConstruct()
{
	
}

bool UGASSEquipmentManagerWidget::NativeOnDragOver(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	UGASSItemDragDrop* DragDropOper = Cast<UGASSItemDragDrop>(InOperation);
	check(DragDropOper);

	UGASSItemWidget* DroppedWidget = Cast<UGASSItemWidget>(DragDropOper->Payload);
	check(DroppedWidget);

	UGASSInventoryGrid* ParentGrid = Cast<UGASSInventoryGrid>(DroppedWidget->ParentWidget);

	if (!ParentGrid || (DroppedWidget == ItemWidgetCache))
	{
		return Super::NativeOnDragOver(InGeometry, InDragDropEvent, InOperation);
	}

	ItemWidgetCache = DroppedWidget;

	const UGASSItemDefinition* ItemCDO = GetDefault<UGASSItemDefinition>(DroppedWidget->MyItemDef);
	TSubclassOf<UGASSEquipmentDefinition> EquipDef = ItemCDO->GetEquipmentDefinition();

	if (EquipDef)
	{
		uint8 TargetIndex = MyEMC->GetEquipmentIndexByItemTag(ItemCDO->ItemTag);
		
		if (TargetIndex < MainEquipmentSlots.Num())
		{
			// Main Equipment
			IsCachedWidgetMain = true;
		}
		else
		{
			// Additional
			IsCachedWidgetMain = false;
			TargetIndex = MyEMC->GetAvailableAdditionalEquipmentSlot();
			if (TargetIndex < 0)
			{
				CanEquipItem = false;
				return Super::NativeOnDragOver(InGeometry, InDragDropEvent, InOperation);
			}
		}

		CachedWidgetIndex = TargetIndex;
		CanEquipItem = MyEMC->CanEquipItemWeightCheck(ItemCDO->ItemWeight, ParentGrid->MyIMC == GetInventoryManagerComponent(), IsCachedWidgetMain, TargetIndex);

		SetBoxInfoWithCache();
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT(" Is NOT Equippable "));
	}

	return Super::NativeOnDragOver(InGeometry, InDragDropEvent, InOperation);
}

void UGASSEquipmentManagerWidget::NativeOnDragLeave(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	Super::NativeOnDragLeave(InDragDropEvent, InOperation);
	ClearCache();
}

bool UGASSEquipmentManagerWidget::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	// Do Something
	if (!ItemWidgetCache || !CanEquipItem)
	{
		ClearCache();
		return Super::NativeOnDrop(InGeometry, InDragDropEvent, InOperation);
	}

	UGASSItemDragDrop* DragDropOper = Cast<UGASSItemDragDrop>(InOperation);
	check(DragDropOper);

	if (ItemWidgetCache != DragDropOper->Payload)
	{
		ClearCache();
		return Super::NativeOnDrop(InGeometry, InDragDropEvent, InOperation);
	}

	UGASSInventoryGrid* ParentGrid = Cast<UGASSInventoryGrid>(ItemWidgetCache->ParentWidget);
	check(ParentGrid);

	const UGASSItemDefinition* ItemCDO = GetDefault<UGASSItemDefinition>(ItemWidgetCache->MyItemDef);

	MyEMC->ClientRequestEquipItem(ItemCDO->GetEquipmentDefinition(), ParentGrid->MyIMC, ItemWidgetCache->CurrIndex, ParentGrid->bImBlackbox);
	TemporaryPlaceItemWidget(ItemWidgetCache, IsCachedWidgetMain, CachedWidgetIndex);

	Super::NativeOnDrop(InGeometry, InDragDropEvent, InOperation);
	return true;
}

bool UGASSEquipmentManagerWidget::TrySetEMC()
{
	APlayerController* OwningController = GetOwningPlayer();

	if (OwningController != nullptr)
	{
		APawn* OwnerPawn = OwningController->GetPawn();
		MyEMC = OwnerPawn->GetComponentByClass<UGASSEquipmentManagerComponent>();


		//AGASSPlayerController* GASSController = Cast<AGASSPlayerController>(OwningController);

		//if (GASSController == nullptr)
		//{
		//	UE_LOG(LogTemp, Error, TEXT("Widget On Init, GASS Controller not found"));
		//	return false;
		//}

		//MyEMC = GASSController->GetGASSEquipmentManagerComponent();
		return true;
	}
	else
	{
		return false;
	}
}

bool UGASSEquipmentManagerWidget::RegisterEMCDelegate()
{
	if (MyEMC == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("RegisterEMC Delegate :: MyEMC not Set"));
		return false;
	}
	
	MyEMC->OnEntryEquipped.AddUniqueDynamic(this, &UGASSEquipmentManagerWidget::OnEntryEquipped);
	MyEMC->OnEntryUnequipped.AddUniqueDynamic(this, &UGASSEquipmentManagerWidget::OnEntryUnequipped);
	MyEMC->OnEquipCanceled.AddUniqueDynamic(this, &UGASSEquipmentManagerWidget::OnEntryEquipCanceled);
	MyEMC->OnUnequipCanceled.AddUniqueDynamic(this, &UGASSEquipmentManagerWidget::OnEntryUnequipCanceled);
	return true;
}

void UGASSEquipmentManagerWidget::InitMainEquipmentSlots()
{
	check(MyEMC);
	int32 MainSlotsNum = MyEMC->GetEquipmentListSize(true);
	MainEquipmentSlots.Empty();
	MainEquipmentSlots.SetNum(MainSlotsNum);

	TArray<UWidget*> SlotWidgets = SlotCanvasPanel->GetAllChildren();
	for (UWidget* SlotWidget : SlotWidgets)
	{
		UGASSEquipmentSlot* EquipSlot = Cast<UGASSEquipmentSlot>(SlotWidget);
		if (EquipSlot)
		{
			
			UCanvasPanelSlot* SlotWidgetToSlot = Cast<UCanvasPanelSlot>(SlotWidget->Slot);
			SlotWidgetToSlot->SetAutoSize(true);

			UEquipmentSlotSizeConfig* EquipSlotSizeConfig = Cast<UEquipmentSlotSizeConfig>(SlotSizeConfig);
			check(EquipSlotSizeConfig);

			FVector2D SlotSize = EquipSlotSizeConfig->GetSlotSizeByEquipmentIndex(EquipSlot->EquipmentIndex);
			
			EquipSlot->InitEquipmentSlot(this, SlotGridSize, MainSlotEdgeThickness, (int32)floor(SlotSize.X), (int32)floor(SlotSize.Y));
			if (!(EquipSlot->bImMainEquipmentSlot))
			{
				continue;
			}

			MainEquipmentSlots[EquipSlot->EquipmentIndex] = EquipSlot;
		}
	}
}

void UGASSEquipmentManagerWidget::InitAdditionalSlots()
{
	UCanvasPanelSlot* AdditionalBorderToSlot = Cast<UCanvasPanelSlot>(AdditionalSlotBackGroundBorder->Slot);
	AdditionalBorderToSlot->SetSize(AdditionalSlotSize * (SlotGridSize + AdditionalSlotEdgeThickness) + FVector2D(10, 10));

	check(MyEMC);
	int32 AdditionalSlotsNum = MyEMC->GetEquipmentListSize(false);
	AdditionalEquipmentSlots.Empty();
	AdditionalEquipmentSlots.SetNum(AdditionalSlotsNum);

	AdditionalSlotsBasePosition.X = AdditionalBorderToSlot->GetPosition().X;
	AdditionalSlotsBasePosition.Y = AdditionalBorderToSlot->GetPosition().Y;

	for (int32 i = 0; i < AdditionalSlotsNum; ++i)
	{
		AdditionalEquipmentSlots[i] = CreateSlotWidget(i);
		AdditionalEquipmentSlots[i]->EquipmentIndex = i;
	}
}

void UGASSEquipmentManagerWidget::SetBoxInfoWithCache()
{
	UE_LOG(LogTemp, Error, TEXT("SetBoxInfo Called !!"));
	DrawBox = true;
	AbleToEquip = CanEquipItem;

	float Thickness = IsCachedWidgetMain ? MainSlotEdgeThickness : AdditionalSlotEdgeThickness;

	if (IsCachedWidgetMain)
	{
		UCanvasPanelSlot* SlotWidgetToSlot = Cast<UCanvasPanelSlot>(MainEquipmentSlots[CachedWidgetIndex]->Slot);
		check(SlotWidgetToSlot);

		BoxInfo.X = SlotWidgetToSlot->GetPosition().X + Thickness / 2;
		BoxInfo.Y = SlotWidgetToSlot->GetPosition().Y + Thickness / 2;
		BoxInfo.Z = SlotWidgetToSlot->GetSize().X - Thickness;
		BoxInfo.W = SlotWidgetToSlot->GetSize().Y - Thickness;
	}
	else
	{
		FVector2D NextPosition = GetAdditionalSlotPositionByIndex(CachedWidgetIndex);

		UE_LOG(LogTemp, Error, TEXT("%f, %f"), NextPosition.X, NextPosition.Y);

		BoxInfo.X = AdditionalSlotsBasePosition.X + NextPosition.X + Thickness / 2 + 5;
		BoxInfo.Y = AdditionalSlotsBasePosition.Y + NextPosition.Y + Thickness / 2 + 5;
		BoxInfo.Z = SlotGridSize;
		BoxInfo.W = SlotGridSize;
	}
}

void UGASSEquipmentManagerWidget::ClearCache()
{
	DrawBox = false;
	BoxInfo = FVector4f(0, 0, 0, 0);

	ItemWidgetCache = nullptr;
	CachedWidgetIndex = -1;
	IsCachedWidgetMain = false;
	CanEquipItem = false;
}

UGASSInventoryManagerComponent* UGASSEquipmentManagerWidget::GetInventoryManagerComponent()
{
	APlayerController* OwningController = GetOwningPlayer();

	if (OwningController != nullptr)
	{
		AGASSPlayerController* GASSController = Cast<AGASSPlayerController>(OwningController);

		if (GASSController == nullptr)
		{
			UE_LOG(LogTemp, Error, TEXT("Widget On Init, GASS Controller not found"));
			return nullptr;
		}

		return GASSController->GetGASSInventoryManagerComponent();
	}
	else
	{
		return nullptr;
	}
}

void UGASSEquipmentManagerWidget::Refresh()
{
	UE_LOG(LogTemp, Warning, TEXT("EquipManagerWidget::Refresh Called"));

	TArray<TSubclassOf<UGASSItemDefinition>> MainItemDefs;
	TArray<TSubclassOf<UGASSItemDefinition>> AdditionalItemDefs;

	check(MyEMC);

	MyEMC->GetItemDefFromList(true, OUT MainItemDefs);
	MyEMC->GetItemDefFromList(false, OUT AdditionalItemDefs);

	for (auto ItemDef : MainItemDefs)
	{
		if (!ItemDef)
		{
			continue;
		}
		const UGASSItemDefinition* ItemCDO = GetDefault<UGASSItemDefinition>(ItemDef);
		// Do Something With ItemCDO
	}
}

void UGASSEquipmentManagerWidget::CreateItemWidget()
{
	UGASSItemWidget* NewItemWidget = CreateItemWidgetInBP();
	check(NewItemWidget);

	//NewItemWidget->InitItemWidget(GridSize, Index, NewAmount, Rotation, Instance->GetItemDef());

	//UCanvasPanelSlot* NewItemWidgetSlot = GridCanvasPanel->AddChildToCanvas(NewItemWidget);
	//NewItemWidget->SetParentWidget(this);
	//NewItemWidgetSlot->SetAutoSize(true);

	//FVector2D NewPosition((Index % InventoryMaxCol) * GridSize, (Index / InventoryMaxCol) * GridSize);

	//NewItemWidgetSlot->SetPosition(NewPosition);
	// Set NewItemWidget
}

UGASSEquipmentSlot* UGASSEquipmentManagerWidget::CreateSlotWidget(int32 TargetIndex)
{
	UGASSEquipmentSlot* NewSlotWidget = CreateSlotWidgetInBP();
	NewSlotWidget->InitEquipmentSlot(this, SlotGridSize, AdditionalSlotEdgeThickness, 1, 1);
	NewSlotWidget->EquipmentIndex = AdditionalEquipmentSlots.Num();

	FVector2D NewPosition = GetAdditionalSlotPositionByIndex(TargetIndex);
	AdditionalEquipmentSlots.Add(NewSlotWidget);

	UCanvasPanelSlot* NewSlotWidgetSlot = AdditionalSlotCanvasPanel->AddChildToCanvas(NewSlotWidget);
	NewSlotWidgetSlot->SetPosition(NewPosition + FVector2D(5, 5));
	NewSlotWidgetSlot->SetAutoSize(true);

	return NewSlotWidget;
}

void UGASSEquipmentManagerWidget::DestroySlotWidget(int32 Index)
{
	check(Index < AdditionalEquipmentSlots.Num() && Index >= 0);
	AdditionalSlotCanvasPanel->RemoveChild(AdditionalEquipmentSlots[Index]);

	for (int32 MoveIndex = Index + 1; MoveIndex < AdditionalEquipmentSlots.Num(); ++MoveIndex)
	{
		UE_LOG(LogTemp, Error, TEXT("MoveIndex = %d"), MoveIndex);

		UCanvasPanelSlot* SlotWidgetToSlot = Cast<UCanvasPanelSlot>(AdditionalEquipmentSlots[MoveIndex]->Slot);
		check(SlotWidgetToSlot);
		FVector2D NewPosition = GetAdditionalSlotPositionByIndex(MoveIndex - 1);

		SlotWidgetToSlot->SetPosition(NewPosition + FVector2D(5, 5));
		(AdditionalEquipmentSlots[MoveIndex]->EquipmentIndex)--;

		UE_LOG(LogTemp, Error, TEXT("Index : %d"), AdditionalEquipmentSlots[MoveIndex]->EquipmentIndex);
	}
	
	auto Iter = AdditionalEquipmentSlots.CreateIterator();
	Iter += Index;
	Iter.RemoveCurrent();
}

FVector2D UGASSEquipmentManagerWidget::GetNextAdditionalSlotPosition()
{
	int32 NumItems = AdditionalEquipmentSlots.Num();

	return GetAdditionalSlotPositionByIndex(NumItems);
}

FVector2D UGASSEquipmentManagerWidget::GetAdditionalSlotPositionByIndex(int32 Index)
{
	FVector2D NextPosition;

	UE_LOG(LogTemp, Error, TEXT("%d"), Index);

	int AdditionalSlotCol = Index % (int32)AdditionalSlotSize.X;
	int AdditionalSlotRow = Index / (int32)AdditionalSlotSize.X;

	NextPosition.X = AdditionalSlotCol * (SlotGridSize + AdditionalSlotEdgeThickness);
	NextPosition.Y = AdditionalSlotRow * (SlotGridSize + AdditionalSlotEdgeThickness);

	return NextPosition;
}

void UGASSEquipmentManagerWidget::PlaceItemWidget(UGASSItemWidget* WidgetToPlace, bool IsMainEquipment, int32 Index)
{
	if (IsMainEquipment)
	{
		MainEquipmentSlots[Index]->PlaceItemWidget(WidgetToPlace);
	}
	else
	{
		AdditionalEquipmentSlots[Index]->PlaceItemWidget(WidgetToPlace);
	}
}

void UGASSEquipmentManagerWidget::TemporaryPlaceItemWidget(UGASSItemWidget* WidgetToPlace, bool IsMainEquipment, int32 Index)
{
	if (IsMainEquipment)
	{
		MainEquipmentSlots[Index]->TemporaryPlaceItemWidget(WidgetToPlace);
	}
	else
	{
		AdditionalEquipmentSlots[Index]->TemporaryPlaceItemWidget(WidgetToPlace);
	}
	(WidgetToPlace->BackgroundBorder)->SetBrushColor(WidgetToPlace->NormalBackgroundColor);
}

void UGASSEquipmentManagerWidget::ConfirmMovedItemWidget()
{
	check(ItemWidgetCache);
	UE_LOG(LogTemp, Warning, TEXT(" Confirm Move Called "));

	if (IsCachedWidgetMain)
	{
		MainEquipmentSlots[CachedWidgetIndex]->ConfirmPlacedItemWidget();
	}
	else
	{
		AdditionalEquipmentSlots[CachedWidgetIndex]->ConfirmPlacedItemWidget();
	}
}

void UGASSEquipmentManagerWidget::OnEntryEquipped(bool IsMainEquipment, uint16 Index, TSubclassOf<UGASSItemDefinition> ItemDef)
{
	UE_LOG(LogTemp, Warning, TEXT("Entry Equipped Delegate !"));
	bool flag = false;
	if (IsMainEquipment == IsCachedWidgetMain && Index == CachedWidgetIndex)
	{
		if (ItemWidgetCache->MyItemDef == ItemDef)
		{
			flag = true;
		}
	}
	
	if (flag)
	{
		// Moved (Dragged)
		ConfirmMovedItemWidget();
		ClearCache();
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("flag == false"));
		// Do Something - Create New Item
		// PlaceItemWidget(ItemWidgetCache, IsCachedWidgetMain, CachedWidgetIndex);
		//ClearCache();
	}
}

void UGASSEquipmentManagerWidget::OnEntryUnequipped(bool IsMainEquipment, uint16 Index)
{
	UE_LOG(LogTemp, Warning, TEXT("Entry Unequipped Delegate !"));

	if (IsMainEquipment)
	{
		MainEquipmentSlots[Index]->RemoveItemWidget();
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Unequipped : %d"), Index);
		AdditionalEquipmentSlots[Index]->RemoveItemWidget();
	}
}

void UGASSEquipmentManagerWidget::OnEntryEquipCanceled(bool IsMainEquipment, uint16 Index)
{
	UE_LOG(LogTemp, Warning, TEXT("Equip Canceled Delegate !"));
	UGASSEquipmentSlot* TargetEquipSlot = nullptr;

	if (ItemWidgetCache)
	{
		if (IsCachedWidgetMain)
		{
			TargetEquipSlot = MainEquipmentSlots[CachedWidgetIndex];
		}
		else
		{
			TargetEquipSlot = AdditionalEquipmentSlots[CachedWidgetIndex];
		}
	}
	else
	{
		if (Index < 0)
		{
			//Refresh()
		}
	}
	
	if (TargetEquipSlot)
	{
		TargetEquipSlot->RemoveItemWidget();
		if (!IsCachedWidgetMain)
		{
			//DestroySlotWidget(CachedWidgetIndex);
		}
	}
	else
	{
		// Refresh
	}

	ClearCache();
}

void UGASSEquipmentManagerWidget::OnEntryUnequipCanceled(bool IsMainEquipment, uint16 Index)
{
	UE_LOG(LogTemp, Warning, TEXT("Unequip Canceled Delegate !"));

	UGASSEquipmentSlot* TargetEquipSlot = nullptr;

	if (IsMainEquipment)
	{
		TargetEquipSlot = MainEquipmentSlots[Index];
	}
	else
	{
		TargetEquipSlot = AdditionalEquipmentSlots[Index];
	}

	if (TargetEquipSlot)
	{
		TargetEquipSlot->ShowHiddenItemWidget();
	}
	else
	{
		// Refresh()
	}
}