// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "Templates/SubclassOf.h"
#include "UObject/Interface.h"

#include "UObject/ObjectPtr.h"
#include "IStorage.generated.h"

template <typename InterfaceType> class TScriptInterface;

class AActor;
class UGASSItemDefinition;
class UGASSInventoryItemInstance;
class UGASSInventoryManagerComponent;
class UObject;
struct FFrame;

USTRUCT(BlueprintType)
struct FStorageItem
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	TSubclassOf<UGASSItemDefinition> ItemDef;

	UPROPERTY(EditAnywhere)
	int32 StackCount = 1;
};

USTRUCT(BlueprintType)
struct FStorageTemplate
{
	GENERATED_BODY()

public:
	UPROPERTY()
	UGASSInventoryManagerComponent* StorageIMC = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 StorageMaxCol = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 StorageMaxRow = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool AddRandomItem = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FStorageItem> StorageItems;
};

/**  */
UINTERFACE(MinimalAPI, BlueprintType, meta = (CannotImplementInterfaceInBlueprint))
class UStorage : public UInterface
{
	GENERATED_BODY()
};

/**  */
class IStorage
{
	GENERATED_BODY()

public:
	UFUNCTION()
	virtual void SetAndInitializeStorageIMC() = 0;

	UFUNCTION(BlueprintCallable)
	virtual UGASSInventoryManagerComponent* GetIMC() const = 0;
};

/**  */
UCLASS()
class UStorageStatics : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UStorageStatics();

public:
	UFUNCTION(BlueprintCallable)
	static UGASSInventoryManagerComponent* GetIMCFromStorageActor(AActor* Actor);

	UFUNCTION()
	static void InitStorageIMC(FStorageTemplate& StorageTemplate, AActor* OwnerActor);

	UFUNCTION()
	static const UGASSInventoryItemInstance* GetInventoryInstance(AActor* StorageActor, TSubclassOf<UGASSItemDefinition> InItemDef);
};
