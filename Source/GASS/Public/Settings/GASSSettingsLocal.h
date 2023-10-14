// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "GameFramework/GameUserSettings.h"
#include "Input/GASSMappableConfigPair.h"
#include "InputCoreTypes.h"

#include "GASSSettingsLocal.generated.h"

enum class ECommonInputType : uint8;
enum class EGASSDisplayablePerformanceStat : uint8;
enum class EGASSStatDisplayMode : uint8;

class UGASSLocalPlayer;
class UObject;
class UPlayerMappableInputConfig;
class USoundControlBus;
class USoundControlBusMix;
struct FFrame;


/**
 * UGASSSettingsLocal
 */
UCLASS()
class UGASSSettingsLocal : public UGameUserSettings
{
	GENERATED_BODY()

public:

	UGASSSettingsLocal();

	static UGASSSettingsLocal* Get();

	//~UObject interface
	virtual void BeginDestroy() override;
	//~End of UObject interface

	//~UGameUserSettings interface
	virtual void SetToDefaults() override;
	virtual void LoadSettings(bool bForceReload) override;
	virtual void ResetToCurrentSettings() override;
	//~End of UGameUserSettings interface

	void OnExperienceLoaded();
	void OnHotfixDeviceProfileApplied();

	//////////////////////////////////////////////////////////////////
	// Keybindings
public:


	// Sets the controller representation to use, a single platform might support multiple kinds of controllers.  For
	// example, Win64 games could be played with both an XBox or Playstation controller.
	UFUNCTION()
	void SetControllerPlatform(const FName InControllerPlatform);
	UFUNCTION()
	FName GetControllerPlatform() const;

	DECLARE_EVENT_OneParam(UGASSSettingsLocal, FInputConfigDelegate, const FLoadedMappableConfigPair& /*Config*/);

	/** Delegate called when a new input config has been registered */
	FInputConfigDelegate OnInputConfigRegistered;

	/** Delegate called when a registered input config has been activated */
	FInputConfigDelegate OnInputConfigActivated;

	/** Delegate called when a registered input config has been deactivate */
	FInputConfigDelegate OnInputConfigDeactivated;

	/** Register the given input config with the settings to make it available to the player. */
	void RegisterInputConfig(ECommonInputType Type, const UPlayerMappableInputConfig* NewConfig, const bool bIsActive);

	/** Unregister the given input config. Returns the number of configs removed. */
	int32 UnregisterInputConfig(const UPlayerMappableInputConfig* ConfigToRemove);

	/** Get an input config with a certain name. If the config doesn't exist then nullptr will be returned. */
	UFUNCTION(BlueprintCallable)
	const UPlayerMappableInputConfig* GetInputConfigByName(FName ConfigName) const;

	/** Get all currently registered input configs */
	const TArray<FLoadedMappableConfigPair>& GetAllRegisteredInputConfigs() const { return RegisteredInputConfigs; }

	/**
	 * Get all registered input configs that match the input type.
	 *
	 * @param Type		The type of config to get, ECommonInputType::Count will include all configs.
	 * @param OutArray	Array to be populated with the current registered input configs that match the type
	 */
	void GetRegisteredInputConfigsOfType(ECommonInputType Type, OUT TArray<FLoadedMappableConfigPair>& OutArray) const;

	/**
	 * Returns the display name of any actions with that key bound to it
	 *
	 * @param InKey The key to check for current mappings of
	 * @param OutActionNames Array to store display names of actions of bound keys
	 */
	void GetAllMappingNamesFromKey(const FKey InKey, TArray<FName>& OutActionNames);

	/**
	 * Maps the given keyboard setting to the new key
	 *
	 * @param MappingName	The name of the FPlayerMappableKeyOptions that you would like to change
	 * @param NewKey		The new key to bind this option to
	 * @param LocalPlayer   local player to reset the keybinding on
	 */
	void AddOrUpdateCustomKeyboardBindings(const FName MappingName, const FKey NewKey, UGASSLocalPlayer* LocalPlayer);

	/**
	 * Resets keybinding to its default value in its input mapping context
	 *
	 * @param MappingName	The name of the FPlayerMappableKeyOptions that you would like to change
	 * @param LocalPlayer   local player to reset the keybinding on
	 */
	void ResetKeybindingToDefault(const FName MappingName, UGASSLocalPlayer* LocalPlayer);

	/** Resets all keybindings to their default value in their input mapping context
	 * @param LocalPlayer   local player to reset the keybinding on
	 */
	void ResetKeybindingsToDefault(UGASSLocalPlayer* LocalPlayer);

	const TMap<FName, FKey>& GetCustomPlayerInputConfig() const { return CustomKeyboardConfig; }

private:

	/**
	 * The name of the controller the player is using.  This is maps to the name of a UCommonInputBaseControllerData
	 * that is available on this current platform.  The gamepad data are registered per platform, you'll find them
	 * in <Platform>Game.ini files listed under +ControllerData=...
	 */
	UPROPERTY(Config)
	FName ControllerPlatform;

	UPROPERTY(Config)
	FName ControllerPreset = TEXT("Default");

	/** The name of the current input config that the user has selected. */
	UPROPERTY(Config)
	FName InputConfigName = TEXT("Default");

	/**
	 * Array of currently registered input configs. This is populated by game feature plugins
	 *
	 * @see UGameFeatureAction_AddInputConfig
	 */
	UPROPERTY(VisibleAnywhere)
	TArray<FLoadedMappableConfigPair> RegisteredInputConfigs;

	/** Array of custom key mappings that have been set by the player. Empty by default. */
	UPROPERTY(Config)
	TMap<FName, FKey> CustomKeyboardConfig;

private:
	void OnAppActivationStateChanged(bool bIsActive);
	void ReapplyThingsDueToPossibleDeviceProfileChange();

private:
	FDelegateHandle OnApplicationActivationStateChangedHandle;
};
