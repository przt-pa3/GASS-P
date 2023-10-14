// Fill out your copyright notice in the Description page of Project Settings.

#include "Player/GASSLocalPlayer.h"

#include "AudioMixerBlueprintLibrary.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Settings/GASSSettingsLocal.h"
#include "Settings/GASSSettingsShared.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GASSLocalPlayer)

class UObject;

UGASSLocalPlayer::UGASSLocalPlayer()
{
}

void UGASSLocalPlayer::PostInitProperties()
{
	Super::PostInitProperties();

	if (UGASSSettingsLocal* LocalSettings = GetLocalSettings())
	{
		//LocalSettings->OnAudioOutputDeviceChanged.AddUObject(this, &UGASSLocalPlayer::OnAudioOutputDeviceChanged);
	}
}

void UGASSLocalPlayer::SwitchController(class APlayerController* PC)
{
	Super::SwitchController(PC);

	OnPlayerControllerChanged(PlayerController);
}

bool UGASSLocalPlayer::SpawnPlayActor(const FString& URL, FString& OutError, UWorld* InWorld)
{
	const bool bResult = Super::SpawnPlayActor(URL, OutError, InWorld);

	OnPlayerControllerChanged(PlayerController);

	return bResult;
}

void UGASSLocalPlayer::InitOnlineSession()
{
	OnPlayerControllerChanged(PlayerController);

	Super::InitOnlineSession();
}

void UGASSLocalPlayer::OnPlayerControllerChanged(APlayerController* NewController)
{
	//// Stop listening for changes from the old controller
	//FGenericTeamId OldTeamID = FGenericTeamId::NoTeam;
	//if (IGASSTeamAgentInterface* ControllerAsTeamProvider = Cast<IGASSTeamAgentInterface>(LastBoundPC.Get()))
	//{
	//	OldTeamID = ControllerAsTeamProvider->GetGenericTeamId();
	//	ControllerAsTeamProvider->GetTeamChangedDelegateChecked().RemoveAll(this);
	//}

	//// Grab the current team ID and listen for future changes
	//FGenericTeamId NewTeamID = FGenericTeamId::NoTeam;
	//if (IGASSTeamAgentInterface* ControllerAsTeamProvider = Cast<IGASSTeamAgentInterface>(NewController))
	//{
	//	NewTeamID = ControllerAsTeamProvider->GetGenericTeamId();
	//	ControllerAsTeamProvider->GetTeamChangedDelegateChecked().AddDynamic(this, &ThisClass::OnControllerChangedTeam);
	//	LastBoundPC = NewController;
	//}

	//ConditionalBroadcastTeamChanged(this, OldTeamID, NewTeamID);
}

//void UGASSLocalPlayer::SetGenericTeamId(const FGenericTeamId& NewTeamID)
//{
//	// Do nothing, we merely observe the team of our associated player controller
//}
//
//FGenericTeamId UGASSLocalPlayer::GetGenericTeamId() const
//{
//	if (IGASSTeamAgentInterface* ControllerAsTeamProvider = Cast<IGASSTeamAgentInterface>(PlayerController))
//	{
//		return ControllerAsTeamProvider->GetGenericTeamId();
//	}
//	else
//	{
//		return FGenericTeamId::NoTeam;
//	}
//}
//
//FOnGASSTeamIndexChangedDelegate* UGASSLocalPlayer::GetOnTeamIndexChangedDelegate()
//{
//	return &OnTeamChangedDelegate;
//}

UGASSSettingsLocal* UGASSLocalPlayer::GetLocalSettings() const
{
	return nullptr;
	//return UGASSSettingsLocal::Get();
}

UGASSSettingsShared* UGASSLocalPlayer::GetSharedSettings() const
{
	if (!SharedSettings)
	{
		//SharedSettings = UGASSSettingsShared::LoadOrCreateSettings(this);
	}

	return SharedSettings;
}

void UGASSLocalPlayer::OnAudioOutputDeviceChanged(const FString& InAudioOutputDeviceId)
{
	FOnCompletedDeviceSwap DevicesSwappedCallback;
	DevicesSwappedCallback.BindUFunction(this, FName("OnCompletedAudioDeviceSwap"));
	UAudioMixerBlueprintLibrary::SwapAudioOutputDevice(GetWorld(), InAudioOutputDeviceId, DevicesSwappedCallback);
}

void UGASSLocalPlayer::OnCompletedAudioDeviceSwap(const FSwapAudioOutputResult& SwapResult)
{
	if (SwapResult.Result == ESwapAudioOutputDeviceResultState::Failure)
	{
	}
}

void UGASSLocalPlayer::OnControllerChangedTeam(UObject* TeamAgent, int32 OldTeam, int32 NewTeam)
{
	//ConditionalBroadcastTeamChanged(this, IntegerToGenericTeamId(OldTeam), IntegerToGenericTeamId(NewTeam));
}

