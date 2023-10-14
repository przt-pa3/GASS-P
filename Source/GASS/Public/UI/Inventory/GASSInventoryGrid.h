// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GASSInventoryGrid.generated.h"

class UGASSInventoryManagerComponent;
class UGASSItemWidget;

USTRUCT(BlueprintType)
struct FLineSegment
{
	GENERATED_BODY()

	FLineSegment()
		:Start(FVector2D(0,0)), End(FVector2D(0,0))
	{}

	FLineSegment(float InSX, float InSY, float InEX, float InEY)
		:Start(FVector2D(InSX, InSY)), End(FVector2D(InEX, InEY))
	{}

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector2D Start;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector2D End;
};


/**
 * 
 */
UCLASS(Abstract)
class GASS_API UGASSInventoryGrid : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;
	void NativeOnMouseLeave(const FPointerEvent& InMouseEvent) override;
	void NativeOnDragLeave(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
	bool NativeOnDragOver(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
	bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
	FReply NativeOnPreviewKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;
public:
	// Initializing Functions
	UFUNCTION(BlueprintCallable)
	void OnWidgetOwnerPawnChanged(APawn* OldPawn, APawn* NewPawn);

	UFUNCTION(BlueprintCallable)
	bool RegisterIMCDelegate();

	UFUNCTION()
	bool RemoveIMCDelegate();

	UFUNCTION(BlueprintCallable)
	bool TrySetIMC_Character();

	UFUNCTION(BlueprintCallable)
	bool TrySetIMC_Storage(AActor* StorageActor);

	UFUNCTION(BlueprintCallable)
	void InitInventoryGrid();

	UFUNCTION(BlueprintCallable)
	void SetGridBorderSize();

	UFUNCTION(BlueprintCallable)
	void CreateLineSegments();

	// Refresh
	UFUNCTION(BlueprintCallable)
	void Refresh();

	UFUNCTION(BlueprintCallable)
	void CheckIMCtoSpawnItemWidgets();

	UFUNCTION()
	void ClearCache();

	// Create, StackChange, Delete
	UFUNCTION(BlueprintImplementableEvent, Category = "GASSItemWidget")
	UGASSItemWidget* CreateItemWidgetInBP();

	UFUNCTION()
	void CreateItemWidget(uint16 Index, int32 NewAmount, bool Rotation, UGASSInventoryItemInstance* Instance);

	UFUNCTION()
	void PlaceItemWidget(UGASSItemWidget* WidgetToPlace, uint16 Index, bool Rotation);

	UFUNCTION()
	void TemporaryPlaceItemWidget(UGASSItemWidget* WidgetToPlace, uint16 Index, bool Rotation);

	UFUNCTION()
	void ConfirmMovedItemWidget();

	// IMC Delegate
	UFUNCTION()
	void OnEntryStackChanged(bool IsBlackbox, uint16 Index, int32 Amount);

	UFUNCTION()
	void OnEntryNewLink(bool IsBlackbox, uint16 Index, int32 NewAmount, bool Rotation, UGASSInventoryItemInstance* Instance);

	UFUNCTION()
	void OnEntryLinkDeleted(bool IsBlackbox, uint16 Index);

	UFUNCTION()
	void OnInventoryMoveCanceled_Dep(bool IsBlackbox, uint16 Index);

	UFUNCTION()
	void OnInventoryMoveCanceled_Arr(bool IsBlackbox);

	// For Storage
	UFUNCTION()
	UGASSInventoryManagerComponent* GetPlayerIMC();
public:
	// Child Widgets
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GASSInventoryWidget", meta = (BindWidget))
	class UCanvasPanel* OuterCanvasPanel;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GASSInventoryWidget", meta = (BindWidget))
	class UBorder* GridBorder;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GASSInventoryWidget", meta = (BindWidget))
	class UCanvasPanel* GridCanvasPanel;

	// Set in BP Properties
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GASSInventory")
	float GridSize = 64.0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GASSInventory")
	FLinearColor AbleToPlaceColor;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GASSInventory")
	FLinearColor UnableToPlaceColor;

	// Needs to be Initialized
	UPROPERTY(BlueprintReadOnly, Category = "GASSInventoryWidget")
	TArray<FLineSegment> LineSegments;

	UPROPERTY(BlueprintReadOnly, Category = "GASSInventory")
	FVector2D GridBorderSize;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GASSInventory")
	bool bImBlackbox = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GASSInventory")
	bool bImStorage = false;

	UPROPERTY(BlueprintReadOnly, Category = "GASSInventory")
	int32 InventoryMaxCol;

	UPROPERTY(BlueprintReadOnly, Category = "GASSInventory")
	int32 InventoryMaxRow;

	UPROPERTY(BlueprintReadOnly, Category = "GASSInventory")
	TObjectPtr<UGASSInventoryManagerComponent> PlayerIMC = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "GASSInventory")
	TObjectPtr<UGASSInventoryManagerComponent> MyIMC = nullptr;

	UPROPERTY()
	TMap<int32, UGASSItemWidget*> ItemWidgetMap;

	// Drag over
	UPROPERTY(BlueprintReadOnly, Category = "GASSInventoryWidget")
	FVector4f BoxInfo = FVector4f(0, 0, 0, 0);

	UPROPERTY(BlueprintReadOnly, Category = "GASSInventoryWidget")
	bool DrawBox = false;

	UPROPERTY(BlueprintReadOnly, Category = "GASSInventoryWidget")
	FVector4f BoxInfoFromItemWidget = FVector4f(0, 0, 0, 0);

	UPROPERTY(BlueprintReadOnly, Category = "GASSInventoryWidget")
	bool DrawBoxFromItemWidget = false;

	UPROPERTY(BlueprintReadOnly, Category = "GASSInventoryWidget")
	bool AbleToPlace = false;

private:
	// Caches About Drag Drop Operation
	UPROPERTY()
	int32 CachedWidgetCol = -1;

	UPROPERTY()
	int32 CachedWidgetRow = -1;

	UPROPERTY()
	bool CurrentPlaceState = false;

public:
	UPROPERTY()
	UGASSItemWidget* ItemWidgetCache = nullptr;

	UPROPERTY()
	int32 ItemWidgetCacheIndex = -1;

	UPROPERTY()
	bool ItemWidgetCacheRotation = true;

	UPROPERTY()
	bool MovedFromMySelf = false;
};
