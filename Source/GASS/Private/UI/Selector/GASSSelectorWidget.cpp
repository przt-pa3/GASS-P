// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Selector/GASSSelectorWidget.h"
#include "UI/Selector/GASSSelectorInfoWidget.h"

#include "UnrealClient.h"

#include "Blueprint/WidgetTree.h"

#include "Slate/SGameLayerManager.h"
#include "Components/Image.h"
#include "Components/Border.h"
#include "Components/SizeBox.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"

#include "Kismet/KismetMathLibrary.h"

void UGASSSelectorWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	SetWidgetsSize();
	SetMaterialInstance();
}

void UGASSSelectorWidget::NativeConstruct()
{
	Super::NativeOnInitialized();
	ShouldUpdatePositionInfo = true;
}

FReply UGASSSelectorWidget::NativeOnKeyUp(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	if (InKeyEvent.GetKey() == WidgetKey)
	{
		OnWidgetKeyUp();
	}

	return Super::NativeOnKeyUp(InGeometry, InKeyEvent);
}

FReply UGASSSelectorWidget::NativeOnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	// Viewport Change에 대응을 못함 !
	if (ShouldUpdatePositionInfo)
	{
		SetWidgetScreenPosition();
		ShouldUpdatePositionInfo = false;
	}

	SetWidgetScreenPosition();

	FVector2f MousePosition = InGeometry.AbsoluteToLocal(InMouseEvent.GetScreenSpacePosition());
	MousePosition -= FVector2f(BorderSize.X, BorderSize.Y) / 2.0f;

	double Angle = UKismetMathLibrary::Atan2(MousePosition.Y, MousePosition.X);

	Angle = (Angle < (-1) * PI / 2) ? (Angle + PI * 5 / 2) : (Angle + PI / 2);

	int NewSection = GetSectionNumberByAngle(Angle);
	if (NewSection != CurrentSectionNumber)
	{
		CurrentSectionNumber = NewSection;
		CurrentSectionChanged();
	}

	if (MousePosition.Length() > MouseMaxLength)
	{
		MousePosition *= (MouseMaxLength / MousePosition.Length() * 0.7);
		MousePosition += FVector2f(BorderSize.X, BorderSize.Y) / 2.0f;
		SetMousePosition(MousePosition);
	}

	return Super::NativeOnMouseMove(InGeometry, InMouseEvent);
}

void UGASSSelectorWidget::NativeOnMouseLeave(const FPointerEvent& InMouseEvent)
{
	FVector2f MousePosition = InMouseEvent.GetScreenSpacePosition();

	UE_LOG(LogTemp, Warning, TEXT("%f, %f"), MousePosition.X, MousePosition.Y);
	FVector2f Center(BorderSize.X / 2.0f, BorderSize.Y / 2.0f);
	SetMousePosition(Center);

	Super::NativeOnMouseLeave(InMouseEvent);
}

void UGASSSelectorWidget::SetWidgetScreenPosition()
{
	APlayerController* OwningPlayer = GetOwningPlayer();
	FVector2D AbsolutePosition = GetCachedGeometry().GetAbsolutePosition();

	UGameViewportClient* VC = CastChecked<ULocalPlayer>(OwningPlayer->Player)->ViewportClient;
	TSharedPtr<IGameLayerManager> GameLayerManager = VC->GetGameLayerManager();
	const FGeometry& ViewportGeometry = GameLayerManager->GetViewportWidgetHostGeometry();

	FVector2D ViewportSize;
	FVector2D ViewportPosition;
	
	VC->GetViewportSize(OUT ViewportSize);
	ViewportPosition = ViewportGeometry.AbsoluteToLocal(AbsolutePosition);
	WidgetScreenPosition = (ViewportPosition / ViewportGeometry.GetLocalSize()) * ViewportSize;

	auto AbsoluteSize = GetCachedGeometry().GetAbsoluteSize();
	BorderAbsoluteSize = FVector2D(AbsoluteSize.X, AbsoluteSize.Y);
}

void UGASSSelectorWidget::SetMousePosition(const FVector2f& InLocalMousePosition)
{
	APlayerController* OwningPlayerController = GetOwningPlayer();
	check(OwningPlayerController);

	const int TargetX = UKismetMathLibrary::Round(WidgetScreenPosition.X + InLocalMousePosition.X * BorderAbsoluteSize.X / BorderSize.X);
	const int TargetY = UKismetMathLibrary::Round(WidgetScreenPosition.Y + InLocalMousePosition.Y * BorderAbsoluteSize.Y / BorderSize.Y);

	OwningPlayerController->SetMouseLocation(TargetX, TargetY);
}

void UGASSSelectorWidget::SetNumberOfSections(int32 InNumber)
{
	NumberOfSections = InNumber;
}

void UGASSSelectorWidget::SetWidgetsSize()
{
	UCanvasPanelSlot* BorderToSlot;

	BorderToSlot = Cast<UCanvasPanelSlot>(SectionImage->Slot);
	check(BorderToSlot);
	BorderToSlot->SetSize(BorderSize);

	BorderToSlot = Cast<UCanvasPanelSlot>(HighlightImage->Slot);
	check(BorderToSlot);
	BorderToSlot->SetSize(BorderSize);

	BorderToSlot = Cast<UCanvasPanelSlot>(SectionInfoBorder->Slot);
	check(BorderToSlot);
	BorderToSlot->SetSize(BorderSize);

	BorderToSlot = Cast<UCanvasPanelSlot>(BackgroundSizeBox->Slot);
	check(BorderToSlot);
	BorderToSlot->SetSize(BorderSize);

	//BackgroundSizeBox->SetWidthOverride(BorderSize.X);
	//BackgroundSizeBox->SetWidthOverride(BorderSize.Y);

	InfoIconSize = BorderSize.X * 0.15;
}

void UGASSSelectorWidget::SetMaterialInstance()
{
	UMaterialInstanceDynamic* SectionMaterial = SectionImage->GetDynamicMaterial();
	UMaterialInstanceDynamic* HighlightMaterial = HighlightImage->GetDynamicMaterial();

	if (SectionMaterial == nullptr)
	{
		SectionMaterial = UMaterialInstanceDynamic::Create(SectionMI, this);
		SectionImage->SetBrushFromMaterial(SectionMaterial);
	}

	if (HighlightMaterial == nullptr)
	{
		HighlightMaterial = UMaterialInstanceDynamic::Create(HighlightMI, this);
		HighlightImage->SetBrushFromMaterial(HighlightMaterial);
	}

	SectionMaterial->SetScalarParameterValue("NumberOfSections", NumberOfSections);
	HighlightMaterial->SetScalarParameterValue("NumberOfSections", NumberOfSections);

	SectionMaterial->SetVectorParameterValue("BaseColor", SectionColor);
	HighlightMaterial->SetVectorParameterValue("BaseColor", HighlightColor);
	
	HighlightMaterial->SetScalarParameterValue("SectionNumber", CurrentSectionNumber);
}

void UGASSSelectorWidget::ClearOptionsInfo()
{
	OptionsInfo.Empty();
	NumberOfSections = 0;
}

void UGASSSelectorWidget::AddCancelOption()
{
	if (ShouldAddCancelOption)
	{
		FString Cancel = "Cancel";
		const FName CancelOption(*Cancel);
		AddOptionInfo(CancelOption);
	}
	else
	{
		return;
	}
}

void UGASSSelectorWidget::AddOptionInfo(const FName& InOptionName)
{
	for (const FGASSSelectorOptionInfo& OptionInfo : OptionsInfo)
	{
		if (OptionInfo.OptionName == InOptionName)
		{
			UE_LOG(LogTemp, Error, TEXT("Option Name [%s] Already Exists"), *InOptionName.ToString());
			return;
		}
	}

	SetNumberOfSections(NumberOfSections + 1);

	int32 Index = SelectorInfoConfig->GetOptionsIndexByOptionName(InOptionName);
	OptionsInfo.Add(SelectorInfoConfig->GetSelectorOptionInfoByIndex(Index));
}

int32 UGASSSelectorWidget::GetSectionNumberByAngle(float InAngle)
{
	float SectionAngle = PI * 2 / NumberOfSections;
	float SectionStart = 0;

	for (int i = 0; i < NumberOfSections; ++i)
	{
		if (InAngle >= SectionStart && InAngle < SectionStart + SectionAngle)
		{
			return i + 1;
		}
		SectionStart += SectionAngle;
	}
	return NumberOfSections;
}

void UGASSSelectorWidget::CurrentSectionChanged()
{
	UMaterialInstanceDynamic* HighlightMaterial = HighlightImage->GetDynamicMaterial();
	check(HighlightMaterial);

	HighlightMaterial->SetScalarParameterValue("SectionNumber", CurrentSectionNumber);
}

void UGASSSelectorWidget::ClearInfoWidgets()
{
	for (UGASSSelectorInfoWidget* SelectorInfoWidget : InfoWidgets)
	{
		SelectorInfoWidget->RemoveFromParent();
	}
	InfoWidgets.Empty();
}

void UGASSSelectorWidget::CreateInfoWidgets()
{
	for (int i = 0; i < NumberOfSections; ++i)
	{
		CreateInfoWidget(i);
	}
}

void UGASSSelectorWidget::CreateInfoWidget(int OptionIndex)
{
	UUserWidget* NewInfoWidget = CreateInfoWidgetInBP();
	UGASSSelectorInfoWidget* InfoWidget = Cast<UGASSSelectorInfoWidget>(NewInfoWidget);
	
	InfoWidget->InitInfoWidget(OptionsInfo[OptionIndex].IconImage, OptionsInfo[OptionIndex].Description, InfoIconSize);

	UCanvasPanelSlot* NewItemWidgetSlot = SectionInfoCanvasPanel->AddChildToCanvas(NewInfoWidget);
	NewItemWidgetSlot->SetAutoSize(true);

	FVector2D NewPosition = GetPositionForInfoWidget(OptionIndex);

	NewItemWidgetSlot->SetAlignment(FVector2D(0.5, 0.5));
	NewItemWidgetSlot->SetPosition(NewPosition);

	InfoWidgets.Add(InfoWidget);

	return;
}

FVector2D UGASSSelectorWidget::GetPositionForInfoWidget(int Index)
{
	float Radius = BorderSize.X * 0.5 * 0.7;
	FVector2D Result (0, -Radius);

	float SectionAngle = 2 * PI / NumberOfSections;
	float TargetAngleRad = SectionAngle * Index;
	
	TargetAngleRad += SectionAngle / 2;

	float TargetAngleDeg = TargetAngleRad * 180.0f / PI;

	Result = Result.GetRotated(TargetAngleDeg);

	UE_LOG(LogTemp, Warning, TEXT(" Index %d : Angle %f, Vector %f, %f"), Index, TargetAngleDeg, Result.X, Result.Y);

	Result += BorderSize / 2.0f;

	return Result;
}

void UGASSSelectorWidget::SetInfoWidgetCount(int Index, int NewCount)
{
	InfoWidgets[Index]->CurrentStackCount = NewCount;
}

int32 UGASSSelectorWidget::GetCurrentSectionNumber()
{
	return CurrentSectionNumber;
}

void UGASSSelectorWidget::SetWidgetKey(const FName& KeyName)
{
	WidgetKey = FKey(KeyName);
}

void UGASSSelectorWidget::OnWidgetKeyUp()
{

}




