// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Item/GASSItemDragDrop.h"
#include "UI/Item/GASSItemWidget.h"

void UGASSItemDragDrop::SetPositionInfo(int32 Col, int32 Row, float X, float Y)
{
	GridCol = Col;
	GridRow = Row;

	if (ItemSizeRow > ItemSizeCol)
	{
		// +90 rot
		GridCol = Col;
		GridRow = Row;

		ColNotRotated = Col;
		RowNotRotated = Row;
		
		ColRotated = Row;
		RowRotated = ItemSizeCol - 1 - Col;

		OffsetRotated = FVector2D((Col + X - Row - Y) / ItemSizeRow, -(-Col - X - Row - Y + ItemSizeCol) / ItemSizeCol);

		GridOffset = FVector2f(X, Y);

		GridOffsetNotRotated = FVector2f(X, Y);
		GridOffsetRotated = FVector2f(Y, 1 - X);
	}
	else if(ItemSizeRow == ItemSizeCol)
	{
		// Do not rot
		GridCol = Col;
		GridRow = Row;
		GridOffset = FVector2f(X, Y);
	}
	else
	{
		// -90 rot
		GridCol = Col;
		GridRow = Row;

		ColNotRotated = Col;
		RowNotRotated = Row;

		ColRotated = ItemSizeRow - 1 - Row;
		RowRotated = Col;

		OffsetRotated = FVector2D((Col + X + Row + Y - ItemSizeRow) / ItemSizeRow, -(Col + X - Row - Y) / ItemSizeCol);

		GridOffset = FVector2f(X, Y);

		GridOffsetNotRotated = FVector2f(X, Y);
		GridOffsetRotated = FVector2f(1 - Y, X);
	}
}

void UGASSItemDragDrop::SetItemInfo(int32 SizeRow, int32 SizeCol)
{
	ItemSizeRow = SizeRow;
	ItemSizeCol = SizeCol;
}

void UGASSItemDragDrop::RotateDragDrop()
{
	int32 temp;
	if (ItemSizeRow == ItemSizeCol) return;

	// Hold

	temp = ItemSizeRow;
	ItemSizeRow = ItemSizeCol;
	ItemSizeCol = temp;

	IsRotated = !IsRotated;

	GridCol = IsRotated ? ColRotated : ColNotRotated;
	GridRow = IsRotated ? RowRotated : RowNotRotated;

	Offset = IsRotated ? OffsetRotated : FVector2D(0, 0);
	GridOffset = IsRotated ? GridOffsetRotated : GridOffsetNotRotated;

	Cast<UGASSItemWidget>(Payload)->RotateItemWidget();

	// Resume
}
