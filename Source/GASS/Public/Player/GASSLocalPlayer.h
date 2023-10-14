// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CommonLocalPlayer.h"
//#include "Teams/GASSTeamAgentInterface.h"

#include "GASSLocalPlayer.generated.h"

//struct FGenericTeamId;

class APlayerController;
class UInputMappingContext;
class UGASSSettingsLocal;
class UGASSSettingsShared;
class UObject;
class UWorld;
struct FFrame;
struct FSwapAudioOutputResult;

/**
 * UGASSLocalPlayer
 */
UCLASS()
class GASS_API UGASSLocalPlayer : public UCommonLocalPlayer//, public IGASSTeamAgentInterface
{
	GENERATED_BODY()

public:

	UGASSLocalPlayer();

	//~UObject interface
	virtual void PostInitProperties() override;
	//~End of UObject interface

	//~UPlayer interface
	virtual void SwitchController(class APlayerController* PC) override;
	//~End of UPlayer interface

	//~ULocalPlayer interface
	virtual bool SpawnPlayActor(const FString& URL, FString& OutError, UWorld* InWorld) override;
	virtual void InitOnlineSession() override;
	//~End of ULocalPlayer interface

	////~IGASSTeamAgentInterface interface
	//virtual void SetGenericTeamId(const FGenericTeamId& NewTeamID) override;
	//virtual FGenericTeamId GetGenericTeamId() const override;
	//virtual FOnGASSTeamIndexChangedDelegate* GetOnTeamIndexChangedDelegate() override;
	////~End of IGASSTeamAgentInterface interface

public:
	UFUNCTION()
	UGASSSettingsLocal* GetLocalSettings() const;

	UFUNCTION()
	UGASSSettingsShared* GetSharedSettings() const;

protected:
	void OnAudioOutputDeviceChanged(const FString& InAudioOutputDeviceId);

	UFUNCTION()
	void OnCompletedAudioDeviceSwap(const FSwapAudioOutputResult& SwapResult);

private:
	void OnPlayerControllerChanged(APlayerController* NewController);

	UFUNCTION()
	void OnControllerChangedTeam(UObject* TeamAgent, int32 OldTeam, int32 NewTeam);

private:
	UPROPERTY(Transient)
	mutable TObjectPtr<UGASSSettingsShared> SharedSettings;

	UPROPERTY(Transient)
	mutable TObjectPtr<const UInputMappingContext> InputMappingContext;

	//UPROPERTY()
	//FOnGASSTeamIndexChangedDelegate OnTeamChangedDelegate;

	UPROPERTY()
	TWeakObjectPtr<APlayerController> LastBoundPC;
};
