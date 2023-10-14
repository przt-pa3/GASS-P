// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "NiagaraComponent.h"

#include "RangedWeaponDecals.generated.h"

UCLASS()
class GASS_API ARangedWeaponDecals : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ARangedWeaponDecals();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable, Category = "WeaponFire")
	void Fire(UPARAM(ref) TArray<FVector>& ImpactPositions, UPARAM(ref) TArray<FVector>& ImpactNormals, UPARAM(ref) TArray<uint8>& ImpactSurfaceTypes);


};
