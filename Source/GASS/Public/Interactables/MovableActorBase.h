// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "MovableActorBase.generated.h"

USTRUCT(BlueprintType)
struct FMovingStateInfo
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FVector TransOffsetFromOrigin;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FRotator RotOffsetFromOrigin;
};

USTRUCT(BlueprintType)
struct FMovingInfo
{
	GENERATED_BODY()

public:
	UPROPERTY(VisibleInstanceOnly)
	float ElapsedTime = -1.0;

	UPROPERTY(VisibleInstanceOnly)
	int32 CurrentState = 0;

	UPROPERTY(VisibleInstanceOnly)
	int32 TargetState = 0;

	UPROPERTY(VisibleInstanceOnly)
	FVector DepLocation;

	UPROPERTY(VisibleInstanceOnly)
	FRotator DepRotation;

	UPROPERTY(VisibleInstanceOnly)
	FVector TransVector;

	UPROPERTY(VisibleInstanceOnly)
	FRotator RotVector;

	UPROPERTY(VisibleInstanceOnly)
	const UCurveFloat* TimeCurveFloat;

	bool CheckMovingInfo();
	bool StartMove(const FMovingStateInfo& CurrentStateInfo, const FMovingStateInfo& TargetStateInfo, int32 TargetStateIndex, float TotalTime);
	const UCurveFloat* GetCurveFloat(float TimeLength, float TimeOffset);
	void InitMovingInfoOnArrive(const FMovingStateInfo& ArrivedStateInfo);
};

USTRUCT(BlueprintType)
struct FMovingTemplate
{
	GENERATED_BODY()

public:
	// Translation Movement Settings
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float UnitTime = 5.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TArray<FMovingStateInfo> MovingStates;

	// Target Mesh
	UPROPERTY(VisibleInstanceOnly)
	TObjectPtr<UStaticMeshComponent> MovingMeshComponent;

	UPROPERTY()
	FVector InitLocation;

	UPROPERTY()
	FRotator InitRotation;

	// Moving Information
	UPROPERTY(VisibleInstanceOnly, NotReplicated)
	FMovingInfo MovingInformation;

	UPROPERTY()
	int32 MyIndex;

	int32 GetCurrentState();
	void InitMovingTemplate(UStaticMeshComponent* InMeshComponent, int32 InIndex);
	void MoveMeshComponent(float DeltaTime);
	bool StartMove(int32 TargetState);
};

UCLASS()
class GASS_API AMovableActorBase : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AMovableActorBase(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION()
	void Move(float DeltaTime);

	UFUNCTION(BlueprintCallable)
	UStaticMeshComponent* GetFrameMeshComponent() const;

	UFUNCTION(BlueprintCallable)
	UStaticMeshComponent* GetMovingMeshComponent(int32 Index) const;

	UFUNCTION(BlueprintCallable)
	FMovingTemplate& GetMovingTemplate(int32 Index);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, meta = (WorldContext = "MovableActor"))
	void RegisterEveryMovingMeshComponents();

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, meta = (WorldContext = "MovableActor"))
	void RegisterMovingMeshComponent(UStaticMeshComponent* InMeshComponent);

	UFUNCTION(BlueprintCallable)
	const UCurveFloat* GetCurveFloat(float TimeLength, float TimeOffset);

	UFUNCTION(BlueprintCallable)
	void GetCurrentStates(TArray<int32>& CurrentStates);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly)
	virtual void StartMove(int32 TemplateIndex, int32 TemplateTargetState);

	UFUNCTION()
	virtual void OnMovingComponentArrive(int32 ArrivedTemplateIndex);

	UFUNCTION()
	void AddCurrentMovingComponents();

	UFUNCTION()
	void SubCurrentMovingComponents();

protected:
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	UStaticMeshComponent* FrameMeshComponent;

	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	TArray<FMovingTemplate> MovingTemplates;

	UPROPERTY()
	int32 CurrentMovingComponents = 0;
};
