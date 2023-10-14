// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/Effects/RangedWeaponFire.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraDataInterfaceArrayFunctionLibrary.h"

// Sets default values
ARangedWeaponFire::ARangedWeaponFire()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultRootComponent"));
}

// Called when the game starts or when spawned
void ARangedWeaponFire::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ARangedWeaponFire::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ARangedWeaponFire::Fire(TArray<FVector>& ImpactPositions, TArray<FVector>& ImpactNormals, TArray<uint8>& ImpactSurfaceTypes)
{
	if (EnableShellEject)
	{
		if (!NCShellEject || (NCShellEject && !(NCShellEject->IsActive())))
		{
			SpawnShellEject();
		}

		bShellEjectTrigger = !bShellEjectTrigger;
		NCShellEject->SetNiagaraVariableBool(FString(TEXT("User.Trigger")), bShellEjectTrigger);
	}

	if (EnableMuzzleFlash)
	{
		if (!NCMuzzleFlash || (NCMuzzleFlash && !(NCMuzzleFlash->IsActive())))
		{
			SpawnMuzzleFlash();
		}

		bMuzzleFlashTrigger = !bMuzzleFlashTrigger;
		NCMuzzleFlash->SetNiagaraVariableBool(FString(TEXT("User.Trigger")), bMuzzleFlashTrigger);

		FVector MuzzleLocation = SkeletalMeshComponent->GetSocketLocation(FName(TEXT("Muzzle")));
		FVector Direction = ImpactPositions[0] - MuzzleLocation;
		Direction.Normalize(0.0001);

		NCMuzzleFlash->SetNiagaraVariableVec3(FString(TEXT("User.Direction")), Direction);
	}

	if (EnableTracer)
	{
		if (!NCTracer || (NCTracer && !(NCTracer->IsActive())))
		{
			SpawnTracer();
		}

		bTracerTrigger = !bTracerTrigger;
		NCTracer->SetNiagaraVariableBool(FString(TEXT("User.Trigger")), bTracerTrigger);

		UNiagaraDataInterfaceArrayFunctionLibrary::SetNiagaraArrayVector(NCTracer, FName(TEXT("User.ImpactPositions")), ImpactPositions);
	}

	CheckAndDestoryActorsInBP();
}

void ARangedWeaponFire::SpawnShellEject()
{
	FTransform SocketTransform = SkeletalMeshComponent->GetSocketTransform(FName(TEXT("ShellEject")), ERelativeTransformSpace::RTS_Actor);
	FVector SocketLocation = SocketTransform.GetLocation();

	FRotator NewRotation = FRotator(0, 90.0, 0);
	SkeletalMeshComponent->GetSocketTransform(FName(TEXT("ShellEject")), ERelativeTransformSpace::RTS_Actor);
	UE_LOG(LogTemp, Error, TEXT("%f, %f, %f"), SocketLocation.X, SocketLocation.Y, SocketLocation.Z);

	NCShellEject = UNiagaraFunctionLibrary::SpawnSystemAttached(ShellEjectSystem, RootComponent, NAME_None, SocketLocation, NewRotation, EAttachLocation::KeepRelativeOffset, true, true, ENCPoolMethod::None, true);
	
	NCShellEject->SetVariableStaticMesh(FName(TEXT("User.ShellEjectStaticMesh")), ShellEjectMesh);
	bShellEjectTrigger = false;
}

void ARangedWeaponFire::SpawnMuzzleFlash()
{
	FTransform SocketTransform = SkeletalMeshComponent->GetSocketTransform(FName(TEXT("Muzzle")), ERelativeTransformSpace::RTS_Actor);
	FVector SocketLocation = SocketTransform.GetLocation();
	//FVector SocketLocation = SkeletalMeshComponent->GetSocketLocation(FName(TEXT("Muzzle")));
	FRotator NewRotation = FRotator(0, 90.0, 0);
	
	NCMuzzleFlash = UNiagaraFunctionLibrary::SpawnSystemAttached(MuzzleFlashSystem, RootComponent, NAME_None, SocketLocation, NewRotation, EAttachLocation::KeepRelativeOffset, true, true, ENCPoolMethod::None, true);

	bShellEjectTrigger = false;
}

void ARangedWeaponFire::SpawnTracer()
{
	FTransform SocketTransform = SkeletalMeshComponent->GetSocketTransform(FName(TEXT("Muzzle")), ERelativeTransformSpace::RTS_Actor);
	FVector SocketLocation = SocketTransform.GetLocation();
	//FVector SocketLocation = SkeletalMeshComponent->GetSocketLocation(FName(TEXT("Muzzle")));
	FRotator NewRotation = FRotator(0, 90.0, 0);

	NCTracer = UNiagaraFunctionLibrary::SpawnSystemAttached(TracerSystem, RootComponent, NAME_None, SocketLocation, NewRotation, EAttachLocation::KeepRelativeOffset, true, true, ENCPoolMethod::None, true);

	bTracerTrigger = false;
}
