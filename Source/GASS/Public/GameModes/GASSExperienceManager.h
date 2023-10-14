// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Subsystems/EngineSubsystem.h"
#include "GASSExperienceManager.generated.h"

/**
 * Manager for experiences - primarily for arbitration between multiple PIE sessions
 */
UCLASS(MinimalAPI)
class UGASSExperienceManager : public UEngineSubsystem
{
	GENERATED_BODY()

public:
#if WITH_EDITOR
	GASS_API void OnPlayInEditorBegun();

	static void NotifyOfPluginActivation(const FString PluginURL);
	static bool RequestToDeactivatePlugin(const FString PluginURL);
#else
	static void NotifyOfPluginActivation(const FString PluginURL) {}
	static bool RequestToDeactivatePlugin(const FString PluginURL) { return true; }
#endif

private:
	// The map of requests to active count for a given game feature plugin
	// (to allow first in, last out activation management during PIE)
	TMap<FString, int32> GameFeaturePluginRequestCountMap;
};
