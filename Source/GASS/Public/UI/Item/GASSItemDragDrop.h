// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/DragDropOperation.h"
#include "GASSItemDragDrop.generated.h"

class UGASSItemDefinition;

/**
 * 
 */
UCLASS()
class GASS_API UGASSItemDragDrop : public UDragDropOperation
{
	GENERATED_BODY()
	
	UGASSItemDragDrop() 
	{
		UE_LOG(LogTemp, Warning, TEXT("DragDrop Created"));
	}

public:
	UFUNCTION()
	void SetPositionInfo(int32 Col, int32 Row, float X, float Y);

	UFUNCTION()
	void SetItemInfo(int32 SizeRow, int32 SizeCol);
	
	UFUNCTION()
	void RotateDragDrop();

	//TObjectPtr<UObject> Payload;
	//TObjectPtr<class UWidget> DefaultDragVisual;
	//EDragPivot Pivot;
	//FVector2D Offset;

	int32 GridRow;
	int32 GridCol;
	FVector2f GridOffset;

	int32 ItemSizeRow;
	int32 ItemSizeCol;

	bool IsRotated = false;

private:

	// Rotate Variables
	int32 RowNotRotated;
	int32 ColNotRotated;
	int32 RowRotated;
	int32 ColRotated;
	FVector2D OffsetRotated;
	FVector2f GridOffsetNotRotated;
	FVector2f GridOffsetRotated;
};
