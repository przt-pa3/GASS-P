// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Logging/LogMacros.h"

class UObject;

GASS_API DECLARE_LOG_CATEGORY_EXTERN(LogGASS, Log, All);
GASS_API DECLARE_LOG_CATEGORY_EXTERN(LogGASSExperience, Log, All);
GASS_API DECLARE_LOG_CATEGORY_EXTERN(LogGASSAbilitySystem, Log, All);
GASS_API DECLARE_LOG_CATEGORY_EXTERN(LogGASSTeams, Log, All);

GASS_API FString GetClientServerContextString(UObject* ContextObject = nullptr);
