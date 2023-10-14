// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Inventory/GASSInventoryGrid.h"
#include "UI/Equipment/GASSEquipmentSlot.h"
#include "UI/Equipment/GASSEquipmentManagerWidget.h"
#include "UI/Item/GASSItemWidget.h"
#include "UI/Item/GASSItemDragDrop.h"

#include "Player/GASSPlayerController.h"
#include "Inventory/GASSInventoryManagerComponent.h"
#include "Inventory/GASSInventoryItemInstance.h"
#include "Inventory/IStorage.h"

#include "Components/Border.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Blueprint/WidgetTree.h"
#include "Blueprint/WidgetBlueprintLibrary.h"

#include "Components/Image.h"


// UWidget Override Fucntions
void UGASSInventoryGrid::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	UE_LOG(LogTemp, Error, TEXT("NativeOnInitialized Called"));

	if (bImStorage)
	{
		UE_LOG(LogTemp, Error, TEXT("Storage :: NativeOnInitialized Called"));
		PlayerIMC = GetPlayerIMC();
		check(PlayerIMC);
	}
	else
	{
		if (!TrySetIMC_Character())
		{
			UE_LOG(LogTemp, Error, TEXT("Cannot Set Character IMC"));
			return;
		}

		if (!RegisterIMCDelegate())
		{
			UE_LOG(LogTemp, Error, TEXT("Cannot Register IMC Delegate"));
			return;
		}

		InitInventoryGrid();
		Refresh();
	}
}

void UGASSInventoryGrid::NativeConstruct()
{
	Super::NativeConstruct();

	if (bImStorage)
	{
		UE_LOG(LogTemp, Error, TEXT("InventoryGrid::NativeConstruct (Storage) Called"));
	}
	else
	{
		if (MyIMC == nullptr)
		{
			UE_LOG(LogTemp, Error, TEXT("Character NativeConstruct MyIMC nullptr"));
		}
	}
}

void UGASSInventoryGrid::NativeOnMouseLeave(const FPointerEvent& InMouseEvent)
{
	ClearCache();
	Super::NativeOnMouseLeave(InMouseEvent);
}

void UGASSInventoryGrid::NativeOnDragLeave(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	AbleToPlace = false;
	CurrentPlaceState = false;
	ClearCache();
	Super::NativeOnDragLeave(InDragDropEvent, InOperation);
}

bool UGASSInventoryGrid::NativeOnDragOver(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	UGASSItemDragDrop* DragDropOper = Cast<UGASSItemDragDrop>(InOperation);
	check(DragDropOper);
	UGASSItemWidget* DraggingItemWidget = Cast<UGASSItemWidget>(DragDropOper->Payload);
	check(DraggingItemWidget);

	FVector2f Position = InGeometry.AbsoluteToLocal(InDragDropEvent.GetScreenSpacePosition());
	FVector2f GridOffset = DragDropOper->GridOffset;
	Position += GridSize * (FVector2f(0.5f, 0.5f) - GridOffset);

	int32 CurrGridRow(-1), CurrGridCol(-1);

	if (Position.X < 0 || Position.X > InventoryMaxCol * GridSize || Position.Y < 0 || Position.Y > InventoryMaxRow * GridSize)
	{
		// Invalid
	}
	else
	{
		CurrGridCol = (int32)floor(Position.X / GridSize);
		CurrGridRow = (int32)floor(Position.Y / GridSize);
	}
	
	int32 ColStart = CurrGridCol - DragDropOper->GridCol;
	int32 RowStart = CurrGridRow - DragDropOper->GridRow;
	int32 ColSize = DragDropOper->ItemSizeCol;
	int32 RowSize = DragDropOper->ItemSizeRow;

	// 바꼈는지 체크
	if (ColStart != CachedWidgetCol || RowStart != CachedWidgetRow)
	{
		CachedWidgetCol = ColStart;
		CachedWidgetRow = RowStart;
		
		if (ColStart >= 0 && RowStart >= 0 && ColStart + ColSize - 1 < InventoryMaxCol && RowStart + RowSize - 1 < InventoryMaxRow)
		{
			UE_LOG(LogTemp, Warning, TEXT("Col : %d + %d,Row : %d + %d"), ColStart, ColSize, RowStart, RowSize);

			DrawBox = true;

			if (DraggingItemWidget->ParentWidget == this)
			{
				if (MyIMC->CanMoveMyItemAt(DraggingItemWidget->CurrIndex, RowStart * InventoryMaxCol + ColStart, DraggingItemWidget->Rotation, bImBlackbox))
				{
					AbleToPlace = true;
					CurrentPlaceState = true;
				}
				else
				{
					AbleToPlace = false;
					CurrentPlaceState = false;
				}
			}
			else
			{
				bool NeedToWeightCheck = true;
				UGASSInventoryGrid* DroppedParentGrid = Cast<UGASSInventoryGrid>(DraggingItemWidget->ParentWidget);
				if (DroppedParentGrid)
				{
					if (DroppedParentGrid->MyIMC == MyIMC)
					{
						NeedToWeightCheck = false;
					}
				}
				else
				{
					// From Equipment
					UGASSEquipmentSlot* EquipSlot = Cast<UGASSEquipmentSlot>(DraggingItemWidget->ParentWidget);
					if (EquipSlot)
					{
						APlayerController* OwningController = EquipSlot->GetOwningPlayer();
						if (GetOwningPlayer() == OwningController)
						{
							NeedToWeightCheck = false;
						}
					}
				}
				
				if (MyIMC->CanPlaceItemAt(NeedToWeightCheck, ColStart, RowStart, ColSize, RowSize, DraggingItemWidget->MyItemDef, DraggingItemWidget->CurrentStackCount, bImBlackbox))
				{
					AbleToPlace = true;
					CurrentPlaceState = true;
				}
				else
				{
					AbleToPlace = false;
					CurrentPlaceState = false;
				}
			}

			BoxInfo = FVector4f(ColStart, RowStart, ColSize, RowSize) * GridSize;
		}
		else
		{
			AbleToPlace = false;
			CurrentPlaceState = false;
			ClearCache();
		}
	}
	return Super::NativeOnDragOver(InGeometry, InDragDropEvent, InOperation);
}

bool UGASSInventoryGrid::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	UE_LOG(LogTemp, Warning, TEXT("Drop Detected On Grid"));

	UGASSItemDragDrop* DragDropOper = Cast<UGASSItemDragDrop>(InOperation);
	check(DragDropOper);

	UGASSItemWidget* DroppedWidget = Cast<UGASSItemWidget>(DragDropOper->Payload);
	check(DroppedWidget);

	if (CurrentPlaceState)
	{
		int32 TargetIndex = CachedWidgetRow * InventoryMaxCol + CachedWidgetCol;
		UE_LOG(LogTemp, Warning, TEXT("OnDrop :: TargetIndex - %d"), TargetIndex);

		if (DroppedWidget->ParentWidget == this) // Moved From Myself
		{
			PlayerIMC->ClientRequestMoveItem(DroppedWidget->MyItemDef, MyIMC, MyIMC, DroppedWidget->CurrIndex, TargetIndex, DroppedWidget->CurrentStackCount, DroppedWidget->Rotation, bImBlackbox, bImBlackbox);
			TemporaryPlaceItemWidget(DroppedWidget, TargetIndex, DroppedWidget->Rotation);
			ClearCache();
			MovedFromMySelf = true;
			return true;
		}
		else if (DroppedWidget->ParentWidget != this) // Moved From other
		{
			UGASSInventoryGrid* ParentGrid = Cast<UGASSInventoryGrid>(DroppedWidget->ParentWidget);

			if (ParentGrid != nullptr) // From Other Inventory Grid
			{
				PlayerIMC->ClientRequestMoveItem(DroppedWidget->MyItemDef, ParentGrid->MyIMC, MyIMC, DroppedWidget->CurrIndex, TargetIndex, DroppedWidget->CurrentStackCount, DroppedWidget->Rotation, ParentGrid->bImBlackbox, bImBlackbox);
				TemporaryPlaceItemWidget(DroppedWidget, TargetIndex, DroppedWidget->Rotation);
				ClearCache();
				return true;
			}
			else // From Equipment
			{
				UGASSEquipmentSlot* EquipSlot = Cast<UGASSEquipmentSlot>(DroppedWidget->ParentWidget);
				if (EquipSlot)
				{
					UE_LOG(LogTemp, Error, TEXT("From EquipSlot !!"));
					UGASSEquipmentManagerWidget* ManagerWidget = Cast<UGASSEquipmentManagerWidget>(EquipSlot->EquipmentManager);
					check(ManagerWidget);
					
					PlayerIMC->ClientRequestMoveItem(DroppedWidget->MyItemDef, Cast<UActorComponent>(ManagerWidget->MyEMC), MyIMC, EquipSlot->EquipmentIndex, TargetIndex, 1, DroppedWidget->Rotation, EquipSlot->bImMainEquipmentSlot, bImBlackbox);
					TemporaryPlaceItemWidget(DroppedWidget, TargetIndex, DroppedWidget->Rotation);
					ClearCache();
					return true;
				}
			}
		}
	}
	else
	{
		ClearCache();
		Super::NativeOnDrop(InGeometry, InDragDropEvent, InOperation);
		return false;
	}

	Super::NativeOnDrop(InGeometry, InDragDropEvent, InOperation);
	return false;
}

FReply UGASSInventoryGrid::NativeOnPreviewKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	if (InKeyEvent.GetKey() == FKey(FName("R")))
	{
		UGASSItemDragDrop* DragDropOper = Cast<UGASSItemDragDrop>(UWidgetBlueprintLibrary::GetDragDroppingContent());

		if (DragDropOper == nullptr)
			return FReply::Unhandled();

		DragDropOper->RotateDragDrop();
		return FReply::Handled();
	}

	return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

// Init Functions
void UGASSInventoryGrid::OnWidgetOwnerPawnChanged(APawn* OldPawn, APawn* NewPawn)
{
	if (NewPawn == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("Widget Owner Controller Loses Pawn"));
	}
}

bool UGASSInventoryGrid::RegisterIMCDelegate()
{
	if (MyIMC == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("RegisterIMC Delegate :: MyIMC not Set"));
		return false;
	}

	MyIMC->OnEntryStackChanged.AddUniqueDynamic(this, &UGASSInventoryGrid::OnEntryStackChanged);
	MyIMC->OnEntryNewLink.AddUniqueDynamic(this, &UGASSInventoryGrid::OnEntryNewLink);
	MyIMC->OnEntryLinkDeleted.AddUniqueDynamic(this, &UGASSInventoryGrid::OnEntryLinkDeleted);
	MyIMC->OnInventoryMoveCanceled_Dep.AddUniqueDynamic(this, &UGASSInventoryGrid::OnInventoryMoveCanceled_Dep);
	MyIMC->OnInventoryMoveCanceled_Arr.AddUniqueDynamic(this, &UGASSInventoryGrid::OnInventoryMoveCanceled_Arr);
	return true;
}

bool UGASSInventoryGrid::RemoveIMCDelegate()
{
	if (MyIMC == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("Remove IMC Delegate :: MyIMC not Set"));
		return false;
	}

	//MyIMC->OnEntryStackChanged.Remove()
	MyIMC->OnEntryStackChanged.RemoveAll(this);
	MyIMC->OnEntryNewLink.RemoveAll(this);
	MyIMC->OnEntryLinkDeleted.RemoveAll(this);
	MyIMC->OnInventoryMoveCanceled_Dep.RemoveAll(this);
	MyIMC->OnInventoryMoveCanceled_Arr.RemoveAll(this);

	return true;
}

bool UGASSInventoryGrid::TrySetIMC_Character()
{
	APlayerController* OwningController = GetOwningPlayer();

	if (OwningController != nullptr)
	{
		OwningController->OnPossessedPawnChanged.AddDynamic(this, &UGASSInventoryGrid::OnWidgetOwnerPawnChanged);

		AGASSPlayerController* GASSController = Cast<AGASSPlayerController>(OwningController);

		if (GASSController == nullptr)
		{
			UE_LOG(LogTemp, Warning, TEXT("Widget On Init, GASS Controller not found"));
			return false;
		}

		MyIMC = GASSController->GetGASSInventoryManagerComponent();
		PlayerIMC = MyIMC;
		return true;
	}
	else
	{
		return false;
	}
}

bool UGASSInventoryGrid::TrySetIMC_Storage(AActor* StorageActor)
{
	UE_LOG(LogTemp, Error, TEXT("InventoryGrid::TrySetIMC_Storage Called"));

	// For Storage IMC : NativeOnInitialized() -> NativeConstruct() -> "TrySetIMC_Storage" -> Push Content
	// TODO : Set IMC -> RegisterIMCDelegate(), InitInventoryGrid(), Refresh(), 

	UGASSInventoryManagerComponent* NewIMC = UStorageStatics::GetIMCFromStorageActor(StorageActor);

	if (!NewIMC)
	{
		return false;
	}

	if (NewIMC != MyIMC)
	{
		if (MyIMC != nullptr)
		{
			// Remove Delegates
			RemoveIMCDelegate();
		}
		
		MyIMC = NewIMC;

		if (!RegisterIMCDelegate())
		{
			return false;
		}
		InitInventoryGrid();
		NewIMC->SetLastObservedAtInitialization();
		Refresh();
	}
	else
	{
		Refresh();
	}

	return true;
}

void UGASSInventoryGrid::InitInventoryGrid()
{
	if (MyIMC == nullptr) return;

	if (bImBlackbox)
	{
		InventoryMaxCol = MyIMC->BlackboxMaxCol;
		InventoryMaxRow = MyIMC->BlackboxMaxRow;
	}
	else
	{
		InventoryMaxCol = MyIMC->InventoryMaxCol;
		InventoryMaxRow = MyIMC->InventoryMaxRow;
	}

	GridBorderSize.X = GridSize * InventoryMaxCol;
	GridBorderSize.Y = GridSize * InventoryMaxRow;

	UE_LOG(LogTemp, Warning, TEXT("InventorySize : %d, %d"), InventoryMaxCol, InventoryMaxRow);

	SetGridBorderSize();
	CreateLineSegments();
}

void UGASSInventoryGrid::SetGridBorderSize()
{
	UCanvasPanelSlot* GridBorderToSlot = Cast<UCanvasPanelSlot>(GridBorder->Slot);
	GridBorderToSlot->SetSize(GridBorderSize);
	UE_LOG(LogTemp, Warning, TEXT("GridBoarderSize : %f, %f"), GridBorderToSlot->GetSize().X, GridBorderToSlot->GetSize().Y);
}

void UGASSInventoryGrid::CreateLineSegments()
{	
	
	float X = 0;
	float Y = 0;

	// Vertical
	Y = InventoryMaxRow * GridSize;
	for (auto i = 0; i <= InventoryMaxCol; ++i)
	{
		X = i * GridSize;
		FLineSegment NewLineSeg(X, 0, X, Y);
		LineSegments.Add(NewLineSeg);
	}

	// Horizontal
	X = InventoryMaxCol * GridSize;
	for (auto i = 0; i <= InventoryMaxRow; ++i)
	{
		Y = i * GridSize;
		FLineSegment NewLineSeg(0, Y, X, Y);
		LineSegments.Add(NewLineSeg);
	}
}

void UGASSInventoryGrid::Refresh()
{
	ClearCache();
	ItemWidgetMap.Reset();
	GridCanvasPanel->ClearChildren();

	CheckIMCtoSpawnItemWidgets();
}

void UGASSInventoryGrid::CheckIMCtoSpawnItemWidgets()
{
	if (!MyIMC)
	{
		return;
	}
	TArray<uint32> ItemsInfo;
	TArray<UGASSInventoryItemInstance*> Instances;
	MyIMC->GetEveryBaseEntryInfo(bImBlackbox, OUT ItemsInfo, OUT Instances);

	int32 MaxIndex = ItemsInfo.Num();

	for (int32 i = 0; i < MaxIndex; ++i)
	{
		uint16 TargetIndex = (uint16)(ItemsInfo[i] >> 16);
		uint8 TargetAmount = (uint8)(ItemsInfo[i] >> 8);
		bool TargetRotation = (uint8)(ItemsInfo[i]) > 0 ? true : false;

		CreateItemWidget(TargetIndex, TargetAmount, TargetRotation, Instances[i]);
	}
}

void UGASSInventoryGrid::ClearCache()
{
	DrawBox = false;
	DrawBoxFromItemWidget = false;
	BoxInfo = FVector4f(0, 0, 0, 0);
	CachedWidgetCol = -1;
	CachedWidgetRow = -1;
	CurrentPlaceState = false;
}

// Creating Functions
void UGASSInventoryGrid::CreateItemWidget(uint16 Index, int32 NewAmount, bool Rotation, UGASSInventoryItemInstance* Instance)
{	
	UGASSItemWidget* NewItemWidget = CreateItemWidgetInBP();
	check(NewItemWidget);

	NewItemWidget->InitItemWidget(GridSize, Index, NewAmount, Rotation, Instance->GetItemDef());

	UCanvasPanelSlot* NewItemWidgetSlot = GridCanvasPanel->AddChildToCanvas(NewItemWidget);
	NewItemWidget->SetParentWidget(this);
	NewItemWidgetSlot->SetAutoSize(true);

	FVector2D NewPosition((Index % InventoryMaxCol) * GridSize, (Index / InventoryMaxCol) * GridSize);

	NewItemWidgetSlot->SetPosition(NewPosition);

	ItemWidgetMap.Add(Index, NewItemWidget);
}

void UGASSInventoryGrid::PlaceItemWidget(UGASSItemWidget* WidgetToPlace, uint16 Index, bool Rotation)
{
	WidgetToPlace->CurrIndex = Index;

	if (WidgetToPlace->Rotation != Rotation)
	{
		WidgetToPlace->RotateItemWidget();
	}

	UCanvasPanelSlot* NewItemWidgetSlot = GridCanvasPanel->AddChildToCanvas(WidgetToPlace);
	WidgetToPlace->SetParentWidget(this);
	NewItemWidgetSlot->SetAutoSize(true);

	FVector2D NewPosition((Index % InventoryMaxCol) * GridSize, (Index / InventoryMaxCol) * GridSize);
	NewItemWidgetSlot->SetPosition(NewPosition);
	
	WidgetToPlace->BackgroundBorder->SetBrushColor(WidgetToPlace->NormalBackgroundColor);

	const int32* MapKey = ItemWidgetMap.FindKey(WidgetToPlace);

	if (MapKey == nullptr)
	{
		ItemWidgetMap.Add(Index, WidgetToPlace);
	}
}

void UGASSInventoryGrid::TemporaryPlaceItemWidget(UGASSItemWidget* WidgetToPlace, uint16 Index, bool Rotation)
{
	UCanvasPanelSlot* NewItemWidgetSlot = GridCanvasPanel->AddChildToCanvas(WidgetToPlace);
	NewItemWidgetSlot->SetAutoSize(true);
	FVector2D NewPosition((Index % InventoryMaxCol) * GridSize, (Index / InventoryMaxCol) * GridSize);
	NewItemWidgetSlot->SetPosition(NewPosition);

	WidgetToPlace->BackgroundBorder->SetBrushColor(WidgetToPlace->NormalBackgroundColor);

	ItemWidgetCache = WidgetToPlace;
	ItemWidgetCacheIndex = Index;
	ItemWidgetCacheRotation = Rotation;
}

void UGASSInventoryGrid::ConfirmMovedItemWidget()
{
	check(ItemWidgetCache);

	UE_LOG(LogTemp, Warning, TEXT(" Confirm Move Called "));

	if (!(GridCanvasPanel->HasChild(ItemWidgetCache)))
	{
		TemporaryPlaceItemWidget(ItemWidgetCache, ItemWidgetCacheIndex, ItemWidgetCacheRotation);
	}

	ItemWidgetCache->SetParentWidget(this);
	ItemWidgetCache->CurrIndex = ItemWidgetCacheIndex;
	ItemWidgetMap.Add(ItemWidgetCacheIndex, ItemWidgetCache);

	ItemWidgetCache = nullptr;
	ItemWidgetCacheIndex = -1;
}

// IMC Delegate Functions
void UGASSInventoryGrid::OnEntryStackChanged(bool IsBlackbox, uint16 Index, int32 Amount)
{
	if (IsBlackbox != this->bImBlackbox)
		return;

	auto Target = ItemWidgetMap.Find(Index);
	if (Target == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("Lost"));
		return;
	}

	UGASSItemWidget* TargetItemWidget = (*Target);
	if (TargetItemWidget != nullptr)
	{
		TargetItemWidget->CurrentStackCount += Amount;
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Target Item Widget Is nullptr"));
	}
}

void UGASSInventoryGrid::OnEntryNewLink(bool IsBlackbox, uint16 Index, int32 NewAmount, bool Rotation, UGASSInventoryItemInstance* Instance)
{
	if (IsBlackbox != this->bImBlackbox)
		return;

	UE_LOG(LogTemp, Warning, TEXT(" Grid :: On Entry New Link "));

	// Determine to Create / Just Place Item Widget
	if (ItemWidgetCache == nullptr)
	{
		CreateItemWidget(Index, NewAmount, Rotation, Instance);
	}
	else
	{
		// Should Called After Deleted
		ConfirmMovedItemWidget();
	}
}

void UGASSInventoryGrid::OnEntryLinkDeleted(bool IsBlackbox, uint16 Index)
{
	if (IsBlackbox != this->bImBlackbox)
		return;

	UE_LOG(LogTemp, Warning, TEXT(" Grid :: On Entry Del Link "));

	if (ItemWidgetMap.Find(Index) != nullptr)
	{
		if (!MovedFromMySelf)
		{
			GridCanvasPanel->RemoveChild(*ItemWidgetMap.Find(Index));
		}
		ItemWidgetMap.Remove(Index);
	}
	MovedFromMySelf = false;
}

void UGASSInventoryGrid::OnInventoryMoveCanceled_Dep(bool IsBlackbox, uint16 Index)
{
	if (IsBlackbox != bImBlackbox)
	{
		return;
	}

	if (ItemWidgetMap.Find(Index) != nullptr)
	{
		UGASSItemWidget* WidgetToPlace = *ItemWidgetMap.Find(Index);
		if (WidgetToPlace->Rotation != WidgetToPlace->InitialRotation)
		{
			WidgetToPlace->RotateItemWidget();
		}

		WidgetToPlace->BackgroundBorder->SetBrushColor(WidgetToPlace->NormalBackgroundColor);

		UCanvasPanelSlot* NewItemWidgetSlot = GridCanvasPanel->AddChildToCanvas(WidgetToPlace);
		NewItemWidgetSlot->SetAutoSize(true);
		FVector2D NewPosition((Index % InventoryMaxCol) * GridSize, (Index / InventoryMaxCol) * GridSize);
		NewItemWidgetSlot->SetPosition(NewPosition);

		//WidgetToPlace->Refresh();
		// 수량이 변했거나 없어졌을 수 있음
	}

	
}

void UGASSInventoryGrid::OnInventoryMoveCanceled_Arr(bool IsBlackbox)
{
	if (IsBlackbox != bImBlackbox)
	{
		return;
	}
	GridCanvasPanel->RemoveChild(ItemWidgetCache);
	ItemWidgetCache = nullptr;
	ItemWidgetCacheIndex = -1;
	MovedFromMySelf = false;
}

UGASSInventoryManagerComponent* UGASSInventoryGrid::GetPlayerIMC()
{
	APlayerController* OwningController = GetOwningPlayer();
	if (OwningController == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT(" OwningController Not Found "));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT(" OwningController Found "));
		AGASSPlayerController* GASSController = Cast<AGASSPlayerController>(OwningController);
		return GASSController->GetGASSInventoryManagerComponent();
	}

	return nullptr;
}


