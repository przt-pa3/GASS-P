// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Components/GameStateComponent.h"
//#include "LoadingProcessInterface.h"

#include "GASSExperienceManagerComponent.generated.h"

namespace UE::GameFeatures { struct FResult; }

class UGASSExperienceDefinition;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnGASSExperienceLoaded, const UGASSExperienceDefinition* /*Experience*/);

enum class EGASSExperienceLoadState
{
	Unloaded,
	Loading,
	LoadingGameFeatures,
	LoadingChaosTestingDelay,
	ExecutingActions,
	Loaded,
	Deactivating
};

UCLASS()
class UGASSExperienceManagerComponent final : public UGameStateComponent//, public ILoadingProcessInterface
{
	GENERATED_BODY()

public:

	UGASSExperienceManagerComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	//~UActorComponent interface
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	//~End of UActorComponent interface

	////~ILoadingProcessInterface interface
	//virtual bool ShouldShowLoadingScreen(FString& OutReason) const override;
	////~End of ILoadingProcessInterface

	// Tries to set the current experience, either a UI or gameplay one
	void SetCurrentExperience(FPrimaryAssetId ExperienceId);

	// Ensures the delegate is called once the experience has been loaded,
	// before others are called.
	// However, if the experience has already loaded, calls the delegate immediately.
	void CallOrRegister_OnExperienceLoaded_HighPriority(FOnGASSExperienceLoaded::FDelegate&& Delegate);

	// Ensures the delegate is called once the experience has been loaded
	// If the experience has already loaded, calls the delegate immediately
	void CallOrRegister_OnExperienceLoaded(FOnGASSExperienceLoaded::FDelegate&& Delegate);

	// Ensures the delegate is called once the experience has been loaded
	// If the experience has already loaded, calls the delegate immediately
	void CallOrRegister_OnExperienceLoaded_LowPriority(FOnGASSExperienceLoaded::FDelegate&& Delegate);

	// This returns the current experience if it is fully loaded, asserting otherwise
	// (i.e., if you called it too soon)
	const UGASSExperienceDefinition* GetCurrentExperienceChecked() const;

	// Returns true if the experience is fully loaded
	bool IsExperienceLoaded() const;

private:
	UFUNCTION()
	void OnRep_CurrentExperience();

	void StartExperienceLoad();
	void OnExperienceLoadComplete();
	void OnGameFeaturePluginLoadComplete(const UE::GameFeatures::FResult& Result);
	void OnExperienceFullLoadCompleted();

	void OnActionDeactivationCompleted();
	void OnAllActionsDeactivated();

private:
	UPROPERTY(ReplicatedUsing=OnRep_CurrentExperience)
	TObjectPtr<const UGASSExperienceDefinition> CurrentExperience;

	EGASSExperienceLoadState LoadState = EGASSExperienceLoadState::Unloaded;

	int32 NumGameFeaturePluginsLoading = 0;
	TArray<FString> GameFeaturePluginURLs;

	int32 NumObservedPausers = 0;
	int32 NumExpectedPausers = 0;

	/**
	 * Delegate called when the experience has finished loading just before others
	 * (e.g., subsystems that set up for regular gameplay)
	 */
	FOnGASSExperienceLoaded OnExperienceLoaded_HighPriority;

	/** Delegate called when the experience has finished loading */
	FOnGASSExperienceLoaded OnExperienceLoaded;

	/** Delegate called when the experience has finished loading */
	FOnGASSExperienceLoaded OnExperienceLoaded_LowPriority;
};
