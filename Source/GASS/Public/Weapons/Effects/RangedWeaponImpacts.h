// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "NiagaraComponent.h"

#include "RangedWeaponImpacts.generated.h"

USTRUCT(BlueprintType)
struct FSurfaceSystemTemplate
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName SurfaceName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UNiagaraSystem> SurfaceSystem;
};

UCLASS()
class GASS_API ARangedWeaponImpacts : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ARangedWeaponImpacts();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable, Category = "WeaponImpacts")
	void Fire(UPARAM(ref) TArray<FVector>& ImpactPositions, UPARAM(ref) TArray<FVector>& ImpactNormals, UPARAM(ref) TArray<uint8>& ImpactSurfaceTypes);

	UFUNCTION(BlueprintCallable, Category = "WeaponImpacts")
	void SortArrays(TArray<FVector>& ImpactPositions, TArray<FVector>& ImpactNormals, TArray<uint8>& ImpactSurfaceTypes);

	UFUNCTION(BlueprintImplementableEvent, Category = "WeaponImpacts")
	void CheckAndDestoryActorsInBP();

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FSurfaceSystemTemplate> SurfaceSystems;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float DistanceThreshold = 500.0;
	
	UPROPERTY()
	FVector MuzzlePosition;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TArray<UNiagaraComponent*> ImpactSystems;

	UPROPERTY()
	TArray<FVector> SortedImpactPositions;

	UPROPERTY()
	TArray<FVector> SortedImpactNormals;

	UPROPERTY()
	TArray<uint8> SortedImpactSurfaceTypes;

	UPROPERTY()
	TArray<int32> StartOffset;

	UPROPERTY()
	TArray<int32> NumberOfImpacts;

	UPROPERTY()
	TArray<FVector> SystemPositions;

	
};
