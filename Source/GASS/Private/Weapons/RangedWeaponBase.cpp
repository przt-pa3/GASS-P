// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/RangedWeaponBase.h"


// Sets default values
ARangedWeaponBase::ARangedWeaponBase()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SkeletalMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalMesh"));
	RootComponent = SkeletalMeshComponent;
}

// Called when the game starts or when spawned
void ARangedWeaponBase::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ARangedWeaponBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ARangedWeaponBase::Fire(TArray<FVector>& ImpactPositions, TArray<FVector>& ImpactNormals, TArray<uint8>& ImpactSurfaceTypes)
{
	FVector MuzzleLocation = SkeletalMeshComponent->GetSocketLocation(FName(TEXT("Muzzle")));

	if (bNeedsFakeProjectile)
	{
		AddFakeProjectileData();
	}

	if (!WeaponFire)
	{
		SpawnWeaponFire();
	}

	WeaponFire->Fire(ImpactPositions, ImpactNormals, ImpactSurfaceTypes);

	// TODO : Temp
	K2_OnFire();
	return;

	if (!WeaponImpacts)
	{
		SpawnWeaponImpacts();
	}

	WeaponImpacts->MuzzlePosition = MuzzleLocation;
	WeaponImpacts->Fire(ImpactPositions, ImpactNormals, ImpactSurfaceTypes);

	K2_OnFire();
}

void ARangedWeaponBase::AddFakeProjectileData()
{

}

void ARangedWeaponBase::SpawnWeaponFire()
{
	AActor* NewWeaponFire = GetWorld()->SpawnActor(ARangedWeaponFire::StaticClass());
	WeaponFire = Cast<ARangedWeaponFire>(NewWeaponFire);
	FAttachmentTransformRules AttachRule = FAttachmentTransformRules(EAttachmentRule::KeepRelative, true);
	WeaponFire->AttachToComponent(SkeletalMeshComponent, AttachRule);

	WeaponFire->SkeletalMeshComponent = SkeletalMeshComponent;

	WeaponFire->EnableShellEject = true;
	WeaponFire->ShellEjectSystem = ShellEjectSystem;
	WeaponFire->ShellEjectMesh = ShellEjectMesh;
	
	WeaponFire->EnableMuzzleFlash = true;
	WeaponFire->MuzzleFlashSystem = MuzzleFlashSystem;

	WeaponFire->EnableTracer = true;
	WeaponFire->TracerSystem = TracerSystem;
}

void ARangedWeaponBase::SpawnWeaponImpacts()
{
	AActor* NewWeaponImpacts = GetWorld()->SpawnActor(ARangedWeaponImpacts::StaticClass());
	WeaponImpacts = Cast<ARangedWeaponImpacts>(NewWeaponImpacts);
	FAttachmentTransformRules AttachRule = FAttachmentTransformRules(EAttachmentRule::KeepRelative, true);
	WeaponImpacts->AttachToComponent(SkeletalMeshComponent, AttachRule);

	WeaponImpacts->DistanceThreshold = 500.0f;
}