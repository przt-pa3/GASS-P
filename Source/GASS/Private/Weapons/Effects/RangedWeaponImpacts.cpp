// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/Effects/RangedWeaponImpacts.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraDataInterfaceArrayFunctionLibrary.h"

// Sets default values
ARangedWeaponImpacts::ARangedWeaponImpacts()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ARangedWeaponImpacts::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ARangedWeaponImpacts::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ARangedWeaponImpacts::Fire(TArray<FVector>& ImpactPositions, TArray<FVector>& ImpactNormals, TArray<uint8>& ImpactSurfaceTypes)
{
	SortArrays(ImpactPositions, ImpactNormals, ImpactSurfaceTypes);

	for (int32 idx = 0; idx < NumberOfImpacts.Num(); ++idx)
	{
		int32 TargetIndex = StartOffset[idx];
		uint8 CurrentSurfaceType = SortedImpactSurfaceTypes[TargetIndex]; // Current Surface type

		UE_LOG(LogTemp, Error, TEXT("Current Surface Type : %d"), CurrentSurfaceType);

		check(SurfaceSystems.IsValidIndex(CurrentSurfaceType));
		TObjectPtr<UNiagaraSystem> CurrentSystemTemplate = SurfaceSystems[CurrentSurfaceType].SurfaceSystem;

		if (!CurrentSystemTemplate)
		{
			UE_LOG(LogTemp, Error, TEXT("Niagara System is Not Valid"));
			return;
		}

		// TODO : Why [ 0 ] ??
		FVector CurrentImpactPosition = SortedImpactPositions[0];
		UNiagaraComponent* TargetImpactSystem = nullptr;

		if (ImpactSystems.IsValidIndex(CurrentSurfaceType))
		{
			bool ImpactOutsideThresholdDistance = false;
			if (SystemPositions.IsValidIndex(CurrentSurfaceType))
			{
				ImpactOutsideThresholdDistance = FVector::DistSquared(CurrentImpactPosition, SystemPositions[CurrentSurfaceType]) > (DistanceThreshold * DistanceThreshold);
			}

			SystemPositions.EmplaceAt(CurrentSurfaceType, CurrentImpactPosition);

			if (ImpactSystems[CurrentSurfaceType])
			{
				if (!ImpactOutsideThresholdDistance)
				{
					TargetImpactSystem = ImpactSystems[CurrentSurfaceType];
				}
			}
		}

		if (!TargetImpactSystem)
		{
			TargetImpactSystem = UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), CurrentSystemTemplate, CurrentImpactPosition);
			ImpactSystems.EmplaceAt(CurrentSurfaceType, TargetImpactSystem);
		}

		TargetImpactSystem->SetWorldLocation(CurrentImpactPosition);

		UNiagaraDataInterfaceArrayFunctionLibrary::SetNiagaraArrayPosition(TargetImpactSystem, FName(TEXT("User.ImpactPositions")), SortedImpactPositions);
		UNiagaraDataInterfaceArrayFunctionLibrary::SetNiagaraArrayVector(TargetImpactSystem, FName(TEXT("User.ImpactNormals")), SortedImpactNormals);
		TargetImpactSystem->SetNiagaraVariableInt(FString(TEXT("User.NumberOfHits")), NumberOfImpacts[idx]);
		TargetImpactSystem->SetNiagaraVariablePosition(FString(TEXT("User.MuzzlePosition")), MuzzlePosition);
		TargetImpactSystem->SetNiagaraVariableInt(FString(TEXT("User.StartOffset")), StartOffset[idx]);
		TargetImpactSystem->SetTickBehavior(ENiagaraTickBehavior::UseComponentTickGroup);
		TargetImpactSystem->Activate(false);
	}

	CheckAndDestoryActorsInBP();
}


void ARangedWeaponImpacts::SortArrays(TArray<FVector>& ImpactPositions, TArray<FVector>& ImpactNormals, TArray<uint8>& ImpactSurfaceTypes)
{
	SortedImpactPositions.Empty();
	SortedImpactNormals.Empty();
	SortedImpactSurfaceTypes.Empty();
	StartOffset.Empty();
	NumberOfImpacts.Empty();

	// ImpactSurfaceType 을 기준으로 정렬

	TArray<uint8> SortSurfaceTypes = ImpactSurfaceTypes;
	SortSurfaceTypes.HeapSort();

	int32 PreviousMin = -1;
	int32 SurfaceTypeCount = 0;
	for (int32 idx = 0; idx < ImpactSurfaceTypes.Num(); ++idx)
	{
		uint8 NextMin;
		SortSurfaceTypes.HeapPop(OUT NextMin, true);

		if (PreviousMin != NextMin)
		{
			SurfaceTypeCount++;
			StartOffset.Add(idx);
			NumberOfImpacts.Add(0);
			PreviousMin = NextMin;
		}

		uint8 NextMinIdx = -1;

		for (int32 MinSearch = 0; MinSearch < ImpactSurfaceTypes.Num(); ++MinSearch)
		{
			if (ImpactSurfaceTypes[MinSearch] == NextMin)
			{
				NextMinIdx = MinSearch;
				break;
			}
		}

		SortedImpactSurfaceTypes.Add(ImpactSurfaceTypes[NextMinIdx]);
		SortedImpactPositions.Add(ImpactPositions[NextMinIdx]);
		SortedImpactNormals.Add(ImpactNormals[NextMinIdx]);
		(NumberOfImpacts[SurfaceTypeCount - 1])++;

		ImpactSurfaceTypes[NextMinIdx] = 255;
	}
}