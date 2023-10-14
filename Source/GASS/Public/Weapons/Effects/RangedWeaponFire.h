// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "NiagaraComponent.h"

#include "RangedWeaponFire.generated.h"

UCLASS()
class GASS_API ARangedWeaponFire : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ARangedWeaponFire();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable, Category = "WeaponFire")
	void Fire(UPARAM(ref) TArray<FVector>& ImpactPositions, UPARAM(ref) TArray<FVector>& ImpactNormals, UPARAM(ref) TArray<uint8>& ImpactSurfaceTypes);

	UFUNCTION(BlueprintCallable, Category = "WeaponFire")
	void SpawnShellEject();

	UFUNCTION(BlueprintCallable, Category = "WeaponFire")
	void SpawnMuzzleFlash();

	UFUNCTION(BlueprintCallable, Category = "WeaponFire")
	void SpawnTracer();

	UFUNCTION(BlueprintImplementableEvent, Category = "WeaponFire")
	void CheckAndDestoryActorsInBP();

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ShellEject")
	bool EnableShellEject = true;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ShellEject")
	TObjectPtr<UNiagaraSystem> ShellEjectSystem;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ShellEject")
	TObjectPtr<UStaticMesh> ShellEjectMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MuzzleFlash")
	bool EnableMuzzleFlash = true;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MuzzleFlash")
	TObjectPtr<UNiagaraSystem> MuzzleFlashSystem;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tracer")
	bool EnableTracer = true;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tracer")
	TObjectPtr<UNiagaraSystem> TracerSystem;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<USkeletalMeshComponent> SkeletalMeshComponent;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UNiagaraComponent> NCShellEject;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UNiagaraComponent> NCMuzzleFlash;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UNiagaraComponent> NCTracer;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bShellEjectTrigger = true;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bMuzzleFlashTrigger = true;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bTracerTrigger = true;
};
