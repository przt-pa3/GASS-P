// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "NiagaraComponent.h"
#include "Weapons/Effects/RangedWeaponFire.h"
#include "Weapons/Effects/RangedWeaponImpacts.h"
#include "RangedWeaponBase.generated.h"

class USkeletalMeshComponent;

UCLASS()
class GASS_API ARangedWeaponBase : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ARangedWeaponBase();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintImplementableEvent, Category = Equipment, meta = (DisplayName = "OnFire"))
	void K2_OnFire();

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void Fire(UPARAM(ref) TArray<FVector>& ImpactPositions, UPARAM(ref) TArray<FVector>& ImpactNormals, UPARAM(ref) TArray<uint8>& ImpactSurfaceTypes);

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void AddFakeProjectileData();

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void SpawnWeaponFire();

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void SpawnWeaponImpacts();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<USkeletalMeshComponent> SkeletalMeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bNeedsFakeProjectile = true;

	// Weapon Fire
	UPROPERTY()
	TObjectPtr<ARangedWeaponFire> WeaponFire;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "WeaponFire")
	TObjectPtr<UNiagaraSystem> ShellEjectSystem;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "WeaponFire")
	TObjectPtr<UNiagaraSystem> MuzzleFlashSystem;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "WeaponFire")
	TObjectPtr<UNiagaraSystem> TracerSystem;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "WeaponFire")
	TObjectPtr<UStaticMesh> ShellEjectMesh;
	// ~Weapon Fire

	// Weapon Impacts
	UPROPERTY()
	TObjectPtr<ARangedWeaponImpacts> WeaponImpacts;

};
