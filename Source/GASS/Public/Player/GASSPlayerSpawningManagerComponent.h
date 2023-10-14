// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Components/GameStateComponent.h"

#include "GASSPlayerSpawningManagerComponent.generated.h"

class AController;
class APlayerController;
class APlayerState;
class APlayerStart;
class AGASSPlayerStart;
class AActor;

/**
 * @class UGASSPlayerSpawningManagerComponent
 */
UCLASS()
class GASS_API UGASSPlayerSpawningManagerComponent : public UGameStateComponent
{
	GENERATED_BODY()

public:
	UGASSPlayerSpawningManagerComponent(const FObjectInitializer& ObjectInitializer);

	/** UActorComponent */
	virtual void InitializeComponent() override;
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	/** ~UActorComponent */

protected:
	// Utility
	APlayerStart* GetFirstRandomUnoccupiedPlayerStart(AController* Controller, const TArray<AGASSPlayerStart*>& FoundStartPoints) const;
	
	virtual AActor* OnChoosePlayerStart(AController* Player, TArray<AGASSPlayerStart*>& PlayerStarts) { return nullptr; }
	virtual void OnFinishRestartPlayer(AController* Player, const FRotator& StartRotation) { }

	UFUNCTION(BlueprintImplementableEvent, meta=(DisplayName=OnFinishRestartPlayer))
	void K2_OnFinishRestartPlayer(AController* Player, const FRotator& StartRotation);

private:

	/** We proxy these calls from AGASSGameMode, to this component so that each experience can more easily customize the respawn system they want. */
	AActor* ChoosePlayerStart(AController* Player);
	bool ControllerCanRestart(AController* Player);
	void FinishRestartPlayer(AController* NewPlayer, const FRotator& StartRotation);
	friend class AGASSGameMode;
	/** ~AGASSGameMode */

	UPROPERTY(Transient)
	TArray<TWeakObjectPtr<AGASSPlayerStart>> CachedPlayerStarts;

private:
	void OnLevelAdded(ULevel* InLevel, UWorld* InWorld);
	void HandleOnActorSpawned(AActor* SpawnedActor);

#if WITH_EDITOR
	APlayerStart* FindPlayFromHereStart(AController* Player);
#endif
};
