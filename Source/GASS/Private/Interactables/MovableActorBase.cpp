// Fill out your copyright notice in the Description page of Project Settings.

#include "Interactables/MovableActorBase.h"
#include "Misc/CString.h"
#include "Kismet/KismetStringLibrary.h"
#include "Interactables/CurveFloatsManager.h"

bool FMovingInfo::CheckMovingInfo()
{
	if (ElapsedTime < 0 || CurrentState == TargetState || (TransVector.IsZero() && RotVector.IsZero()) || !TimeCurveFloat)
	{
		return false;
	}
	return true;
}

bool FMovingInfo::StartMove(const FMovingStateInfo& CurrentStateInfo, const FMovingStateInfo& TargetStateInfo, int32 TargetStateIndex, float TotalTime)
{
	FVector NewTransVector = TargetStateInfo.TransOffsetFromOrigin - CurrentStateInfo.TransOffsetFromOrigin;
	FRotator NewRotVector = TargetStateInfo.RotOffsetFromOrigin - CurrentStateInfo.RotOffsetFromOrigin;

	if (NewTransVector.IsZero() && NewRotVector.IsZero())
	{
		UE_LOG(LogTemp, Error, TEXT("Transition, Rotation Both Zero"));
		return false;
	}

	TransVector = NewTransVector;
	RotVector = NewRotVector;
	TargetState = TargetStateIndex;
	ElapsedTime = 0;

	TimeCurveFloat = GetCurveFloat(TotalTime, 0.1);
	return true;
}

const UCurveFloat* FMovingInfo::GetCurveFloat(float TimeLength, float TimeOffset)
{
	UCurveFloatsManager& CurveManager = UCurveFloatsManager::Get();
	return CurveManager.GetCurveFloat(TimeLength, TimeOffset);
}

void  FMovingInfo::InitMovingInfoOnArrive(const FMovingStateInfo& ArrivedStateInfo)
{
	ElapsedTime = -1.0;
	CurrentState = TargetState;

	DepLocation = ArrivedStateInfo.TransOffsetFromOrigin;
	DepRotation = ArrivedStateInfo.RotOffsetFromOrigin;

	TransVector = FVector::ZeroVector;
	RotVector = FRotator::ZeroRotator;
}

int32 FMovingTemplate::GetCurrentState()
{
	return MovingInformation.CurrentState;
}

void FMovingTemplate::InitMovingTemplate(UStaticMeshComponent* InMeshComponent, int32 InIndex)
{
	MovingMeshComponent = InMeshComponent;
	MyIndex = InIndex;

	const FMovingStateInfo& TargetStateInfo = MovingStates[0];
	MovingInformation.InitMovingInfoOnArrive(TargetStateInfo);
}

void FMovingTemplate::MoveMeshComponent(float DeltaTime)
{

	if (!MovingMeshComponent)
	{
		UE_LOG(LogTemp, Error, TEXT("MovingMesh Null"));
		return;
	}

	MovingInformation.ElapsedTime += DeltaTime;
	
	float MinTime, MaxTime, Multiplier;
	bool HasArrived = false;

	MovingInformation.TimeCurveFloat->GetTimeRange(OUT MinTime, OUT MaxTime);

	if (MovingInformation.ElapsedTime >= MaxTime)
	{
		HasArrived = true;
		Multiplier = 1.0;
	}
	else
	{
		Multiplier = MovingInformation.TimeCurveFloat->GetFloatValue(MovingInformation.ElapsedTime);
	}

	

	if (!(MovingInformation.TransVector.IsZero()))
	{
		MovingMeshComponent->SetRelativeLocation(MovingInformation.DepLocation + Multiplier * MovingInformation.TransVector);
	}
	if (!(MovingInformation.RotVector.IsZero()))
	{
		FRotator Temp = MovingInformation.DepRotation + Multiplier * MovingInformation.RotVector;
		MovingMeshComponent->SetRelativeRotation(MovingInformation.DepRotation + Multiplier * MovingInformation.RotVector);
	}

	if (HasArrived)
	{
		const FMovingStateInfo& TargetStateInfo = MovingStates[MovingInformation.TargetState];
		MovingInformation.InitMovingInfoOnArrive(TargetStateInfo);

		AMovableActorBase* OwnerActor = Cast<AMovableActorBase>(MovingMeshComponent->GetOwner());
		check(OwnerActor);
		OwnerActor->OnMovingComponentArrive(MyIndex);
	}
}

bool FMovingTemplate::StartMove(int32 TargetState)
{
	if (MovingInformation.ElapsedTime != -1.0)
	{
		UE_LOG(LogTemp, Error, TEXT("Already Moving"));
		return false;
	}
	
	if (TargetState >= MovingStates.Num())
	{
		UE_LOG(LogTemp, Error, TEXT("Invalid TargetState Index"));
		return false;
	}

	FMovingStateInfo& TargetStateInfo = MovingStates[TargetState];
	FMovingStateInfo& CurrentStateInfo = MovingStates[MovingInformation.CurrentState];
	float TotalTime = UnitTime * (abs(MovingInformation.CurrentState - TargetState));
	if (TotalTime == 0)
	{
		UE_LOG(LogTemp, Error, TEXT("Time is zero"));
		return false;
	}

	return MovingInformation.StartMove(CurrentStateInfo, TargetStateInfo, TargetState, TotalTime);
}

// Sets default values
AMovableActorBase::AMovableActorBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;

	bReplicates = true;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

	FrameMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FrameMeshComponent"));

	FrameMeshComponent->SetupAttachment(RootComponent);

	RootComponent->SetMobility(EComponentMobility::Static);
	FrameMeshComponent->SetMobility(EComponentMobility::Static);

	FrameMeshComponent->SetSimulatePhysics(true);

	FrameMeshComponent->SetIsReplicated(false);
	//FrameMeshComponent->SetCollisionProfileName(MeshCollisionProfileName);
	//FrameMeshComponent->SetRelativeLocation(FVector(0.0f, 0.0f, 0.0f));
	//FrameMeshComponent->SetRelativeRotation(FRotator(0.0f, 0.0f, 0.0f));
}

// Called when the game starts or when spawned
void AMovableActorBase::BeginPlay()
{
	Super::BeginPlay();
	RegisterEveryMovingMeshComponents();
}

// Called every frame
void AMovableActorBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	Move(DeltaTime);
}

void AMovableActorBase::Move(float DeltaTime)
{
	
	if (!HasAuthority())
	{
		return;
	}

	for (FMovingTemplate& TargetTemplate : MovingTemplates)
	{
		if (TargetTemplate.MovingInformation.CheckMovingInfo())
		{
			TargetTemplate.MoveMeshComponent(DeltaTime);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("CheckInfo false"));
		}
	}
}

UStaticMeshComponent* AMovableActorBase::GetFrameMeshComponent() const
{
	return nullptr;
}

UStaticMeshComponent* AMovableActorBase::GetMovingMeshComponent(int32 Index) const
{
	check(Index < MovingTemplates.Num());
	return MovingTemplates[Index].MovingMeshComponent;
}

FMovingTemplate& AMovableActorBase::GetMovingTemplate(int32 Index)
{
	check(Index < MovingTemplates.Num());
	return MovingTemplates[Index];
}

void AMovableActorBase::RegisterEveryMovingMeshComponents()
{
	TArray<UStaticMeshComponent*> OutComponents;
	GetComponents<UStaticMeshComponent>(OUT OutComponents, true);

	for (UStaticMeshComponent* OutComponent : OutComponents)
	{
		RegisterMovingMeshComponent(OutComponent);
	}
}

void AMovableActorBase::RegisterMovingMeshComponent(UStaticMeshComponent* InMeshComponent)
{
	FString TargetString = InMeshComponent->GetFName().ToString();
	int32 UnderBarIndex = TargetString.Find("_MM", ESearchCase::CaseSensitive, ESearchDir::FromEnd);

	if (UnderBarIndex == -1)
	{
		return;
	}

	int32 TargetIndex = UKismetStringLibrary::Conv_StringToInt(TargetString.RightChop(UnderBarIndex + 3)) - 1;

	FMovingTemplate& TargetTemplate = GetMovingTemplate(TargetIndex);
	TargetTemplate.InitMovingTemplate(InMeshComponent, TargetIndex);
	InMeshComponent->SetIsReplicated(true);
}

const UCurveFloat* AMovableActorBase::GetCurveFloat(float TimeLength, float TimeOffset)
{
	UCurveFloatsManager& CurveManager = UCurveFloatsManager::Get();

	return CurveManager.GetCurveFloat(TimeLength, TimeOffset);
}

void AMovableActorBase::GetCurrentStates(TArray<int32>& CurrentStates)
{
	CurrentStates.Reset();
	for (FMovingTemplate& TargetTemplate : MovingTemplates)
	{
		CurrentStates.Add(TargetTemplate.GetCurrentState());
	}
}

void AMovableActorBase::StartMove(int32 TemplateIndex, int32 TemplateTargetState)
{
	FMovingTemplate& TargetTemplate = GetMovingTemplate(TemplateIndex);
	if (TargetTemplate.StartMove(TemplateTargetState))
	{
		AddCurrentMovingComponents();
		return;
	}

	UE_LOG(LogTemp, Error, TEXT("Failed to Move %s(%d), State : %d to %d"), 
		*(TargetTemplate.MovingMeshComponent->GetName()), TemplateIndex, TargetTemplate.MovingInformation.CurrentState, TemplateTargetState);

	return;
}

void AMovableActorBase::OnMovingComponentArrive(int32 ArrivedTemplateIndex)
{
	SubCurrentMovingComponents();
}

void AMovableActorBase::AddCurrentMovingComponents()
{
	check(CurrentMovingComponents >= 0);

	if (CurrentMovingComponents == 0)
	{
		SetActorTickEnabled(true);
	}

	CurrentMovingComponents++;
}

void AMovableActorBase::SubCurrentMovingComponents()
{
	CurrentMovingComponents--;
	check(CurrentMovingComponents >= 0);

	if (CurrentMovingComponents == 0)
	{
		bReplicates = false;
		SetActorTickEnabled(false);
	}
}