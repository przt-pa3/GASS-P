// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/Effects/RangedWeaponDecals.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraDataInterfaceArrayFunctionLibrary.h"

// Sets default values
ARangedWeaponDecals::ARangedWeaponDecals()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ARangedWeaponDecals::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ARangedWeaponDecals::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ARangedWeaponDecals::Fire(TArray<FVector>& ImpactPositions, TArray<FVector>& ImpactNormals, TArray<uint8>& ImpactSurfaceTypes)
{
	
}
