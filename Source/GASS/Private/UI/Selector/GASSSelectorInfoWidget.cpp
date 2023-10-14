// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Selector/GASSSelectorInfoWidget.h"

#include "Components/Image.h"
#include "Components/CanvasPanelSlot.h"

void UGASSSelectorInfoWidget::NativeConstruct()
{
	Super::NativeConstruct();
}

void UGASSSelectorInfoWidget::InitInfoWidget(UMaterialInterface* InMI, const FText& InDescription, float IconSize)
{
	InfoIcon = InMI;
	CurrentStackCount = 0;
	DescriptionToShow = InDescription;
	InfoIconSize = IconSize;
	SetIcon();
}

void UGASSSelectorInfoWidget::SetIcon()
{
	UCanvasPanelSlot* IconImageToSlot = Cast<UCanvasPanelSlot>(IconImage->Slot);
	IconImageToSlot->SetSize(FVector2D(InfoIconSize, InfoIconSize));
	
	IconImage->SetBrushFromMaterial(InfoIcon);
}


