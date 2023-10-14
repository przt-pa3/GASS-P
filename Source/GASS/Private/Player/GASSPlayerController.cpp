// Copyright Epic Games, Inc. All Rights Reserved.

#include "Player/GASSPlayerController.h"
#include "CommonInputTypeEnum.h"
#include "Components/PrimitiveComponent.h"
#include "GASS/GASSLogChannels.h"
//#include "GASSCheatManager.h"
#include "Player/GASSPlayerState.h"
//#include "Camera/GASSPlayerCameraManager.h"
#include "UI/GASSHUD.h"
#include "AbilitySystem/GASSAbilitySystemComponent.h"
#include "EngineUtils.h"
#include "GASS/GASSGameplayTags.h"
#include "GameFramework/Pawn.h"
#include "AbilitySystemGlobals.h"
#include "CommonInputSubsystem.h"
#include "Player/GASSLocalPlayer.h"
#include "Settings/GASSSettingsShared.h"
//#include "Development/GASSDeveloperSettings.h"

// Inventory
#include "Inventory/GASSInventoryManagerComponent.h"
#include "Inventory/GASSInventoryItemInstance.h"
#include "Item/GASSItemDefinition.h"
#include "/Program Files/Epic Games/UE_5.2/Engine/Source/Runtime/Engine/Private/Net/NetSubObjectRegistryGetter.h"
//#include "/UnrealEngine5.2.0/Engine/Source/Runtime/Engine/Private/Net/NetSubObjectRegistryGetter.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GASSPlayerController)

namespace GASS
{
	namespace Input
	{
		static int32 ShouldAlwaysPlayForceFeedback = 0;
		static FAutoConsoleVariableRef CVarShouldAlwaysPlayForceFeedback(TEXT("GASSPC.ShouldAlwaysPlayForceFeedback"),
			ShouldAlwaysPlayForceFeedback,
			TEXT("Should force feedback effects be played, even if the last input device was not a gamepad?"));
	}
}

AGASSPlayerController::AGASSPlayerController(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	//PlayerCameraManagerClass = AGASSPlayerCameraManager::StaticClass();

	InventoryManagerComponent = ObjectInitializer.CreateDefaultSubobject<UGASSInventoryManagerComponent>(this, TEXT("InventorySystemComponent"));
	InventoryManagerComponent->SetIsReplicated(true);

//#if USING_CHEAT_MANAGER
//	CheatClass = UGASSCheatManager::StaticClass();
//#endif // #if USING_CHEAT_MANAGER
}

void AGASSPlayerController::PreInitializeComponents()
{
	Super::PreInitializeComponents();
}

void AGASSPlayerController::BeginPlay()
{
	Super::BeginPlay();
	SetActorHiddenInGame(false);
}

void AGASSPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
}

void AGASSPlayerController::ReceivedPlayer()
{
	Super::ReceivedPlayer();
}

void AGASSPlayerController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);

	// If we are auto running then add some player input
	if (GetIsAutoRunning())
	{
		if (APawn* CurrentPawn = GetPawn())
		{
			const FRotator MovementRotation(0.0f, GetControlRotation().Yaw, 0.0f);
			const FVector MovementDirection = MovementRotation.RotateVector(FVector::ForwardVector);
			CurrentPawn->AddMovementInput(MovementDirection, 1.0f);
		}
	}
}

AGASSPlayerState* AGASSPlayerController::GetGASSPlayerState() const
{
	return CastChecked<AGASSPlayerState>(PlayerState, ECastCheckedType::NullAllowed);
}

UGASSAbilitySystemComponent* AGASSPlayerController::GetGASSAbilitySystemComponent() const
{
	const AGASSPlayerState* GASSPS = GetGASSPlayerState();
	return (GASSPS ? GASSPS->GetGASSAbilitySystemComponent() : nullptr);
}

UGASSInventoryManagerComponent* AGASSPlayerController::GetGASSInventoryManagerComponent() const
{
	return InventoryManagerComponent;
}

AGASSHUD* AGASSPlayerController::GetGASSHUD() const
{
	return CastChecked<AGASSHUD>(GetHUD(), ECastCheckedType::NullAllowed);
}

//void AGASSPlayerController::OnPlayerStateChangedTeam(UObject* TeamAgent, int32 OldTeam, int32 NewTeam)
//{
//	ConditionalBroadcastTeamChanged(this, IntegerToGenericTeamId(OldTeam), IntegerToGenericTeamId(NewTeam));
//}
//

void AGASSPlayerController::OnPlayerStateChanged()
{
	// Empty, place for derived classes to implement without having to hook all the other events
}

void AGASSPlayerController::BroadcastOnPlayerStateChanged()
{
	OnPlayerStateChanged();

	//// Unbind from the old player state, if any
	//FGenericTeamId OldTeamID = FGenericTeamId::NoTeam;
	//if (LastSeenPlayerState != nullptr)
	//{
	//	if (IGASSTeamAgentInterface* PlayerStateTeamInterface = Cast<IGASSTeamAgentInterface>(LastSeenPlayerState))
	//	{
	//		OldTeamID = PlayerStateTeamInterface->GetGenericTeamId();
	//		PlayerStateTeamInterface->GetTeamChangedDelegateChecked().RemoveAll(this);
	//	}
	//}

	//// Bind to the new player state, if any
	//FGenericTeamId NewTeamID = FGenericTeamId::NoTeam;
	//if (PlayerState != nullptr)
	//{
	//	if (IGASSTeamAgentInterface* PlayerStateTeamInterface = Cast<IGASSTeamAgentInterface>(PlayerState))
	//	{
	//		NewTeamID = PlayerStateTeamInterface->GetGenericTeamId();
	//		PlayerStateTeamInterface->GetTeamChangedDelegateChecked().AddDynamic(this, &ThisClass::OnPlayerStateChangedTeam);
	//	}
	//}

	//// Broadcast the team change (if it really has)
	//ConditionalBroadcastTeamChanged(this, OldTeamID, NewTeamID);

	LastSeenPlayerState = PlayerState;
}

void AGASSPlayerController::InitPlayerState()
{
	Super::InitPlayerState();
	BroadcastOnPlayerStateChanged();
}

void AGASSPlayerController::CleanupPlayerState()
{
	Super::CleanupPlayerState();
	BroadcastOnPlayerStateChanged();
}

void AGASSPlayerController::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();
	BroadcastOnPlayerStateChanged();
}

void AGASSPlayerController::SetPlayer(UPlayer* InPlayer)
{
	Super::SetPlayer(InPlayer);

	if (const UGASSLocalPlayer* GASSLocalPlayer = Cast<UGASSLocalPlayer>(InPlayer))
	{
		UGASSSettingsShared* UserSettings = GASSLocalPlayer->GetSharedSettings();
		//UserSettings->OnSettingChanged.AddUObject(this, &ThisClass::OnSettingsChanged);

		OnSettingsChanged(UserSettings);
	}
}

void AGASSPlayerController::OnSettingsChanged(UGASSSettingsShared* InSettings)
{
	//bForceFeedbackEnabled = InSettings->GetForceFeedbackEnabled();
}

//void AGASSPlayerController::AddCheats(bool bForce)
//{
//#if USING_CHEAT_MANAGER
//	Super::AddCheats(true);
//#else //#if USING_CHEAT_MANAGER
//	Super::AddCheats(bForce);
//#endif // #else //#if USING_CHEAT_MANAGER
//}

//void AGASSPlayerController::ServerCheat_Implementation(const FString& Msg)
//{
//#if USING_CHEAT_MANAGER
//	if (CheatManager)
//	{
//		UE_LOG(LogGASS, Warning, TEXT("ServerCheat: %s"), *Msg);
//		ClientMessage(ConsoleCommand(Msg));
//	}
//#endif // #if USING_CHEAT_MANAGER
//}
//
//bool AGASSPlayerController::ServerCheat_Validate(const FString& Msg)
//{
//	return true;
//}
//
//void AGASSPlayerController::ServerCheatAll_Implementation(const FString& Msg)
//{
//#if USING_CHEAT_MANAGER
//	if (CheatManager)
//	{
//		UE_LOG(LogGASS, Warning, TEXT("ServerCheatAll: %s"), *Msg);
//		for (TActorIterator<AGASSPlayerController> It(GetWorld()); It; ++It)
//		{
//			AGASSPlayerController* GASSPC = (*It);
//			if (GASSPC)
//			{
//				GASSPC->ClientMessage(GASSPC->ConsoleCommand(Msg));
//			}
//		}
//	}
//#endif // #if USING_CHEAT_MANAGER
//}
//
//bool AGASSPlayerController::ServerCheatAll_Validate(const FString& Msg)
//{
//	return true;
//}

void AGASSPlayerController::PreProcessInput(const float DeltaTime, const bool bGamePaused)
{
	Super::PreProcessInput(DeltaTime, bGamePaused);
}

void AGASSPlayerController::PostProcessInput(const float DeltaTime, const bool bGamePaused)
{
	if (UGASSAbilitySystemComponent* GASSASC = GetGASSAbilitySystemComponent())
	{
		GASSASC->ProcessAbilityInput(DeltaTime, bGamePaused);
	}

	Super::PostProcessInput(DeltaTime, bGamePaused);
}

void AGASSPlayerController::OnCameraPenetratingTarget()
{
	bHideViewTargetPawnNextFrame = true;
}

void AGASSPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

#if WITH_SERVER_CODE && WITH_EDITOR
	if (GIsEditor && (InPawn != nullptr) && (GetPawn() == InPawn))
	{
		//for (const FGASSCheatToRun& CheatRow : GetDefault<UGASSDeveloperSettings>()->CheatsToRun)
		//{
		//	if (CheatRow.Phase == ECheatExecutionTime::OnPlayerPawnPossession)
		//	{
		//		ConsoleCommand(CheatRow.Cheat, /*bWriteToLog=*/ true);
		//	}
		//}
	}
#endif

	SetIsAutoRunning(false);
}

void AGASSPlayerController::SetIsAutoRunning(const bool bEnabled)
{
	const bool bIsAutoRunning = GetIsAutoRunning();
	if (bEnabled != bIsAutoRunning)
	{
		if (!bEnabled)
		{
			OnEndAutoRun();
		}
		else
		{
			OnStartAutoRun();
		}
	}
}

bool AGASSPlayerController::GetIsAutoRunning() const
{
	bool bIsAutoRunning = false;
	if (const UGASSAbilitySystemComponent* GASSASC = GetGASSAbilitySystemComponent())
	{
		bIsAutoRunning = GASSASC->GetTagCount(GASSGameplayTags::Status_AutoRunning) > 0;
	}
	return bIsAutoRunning;
}

void AGASSPlayerController::OnStartAutoRun()
{
	if (UGASSAbilitySystemComponent* GASSASC = GetGASSAbilitySystemComponent())
	{
		GASSASC->SetLooseGameplayTagCount(GASSGameplayTags::Status_AutoRunning, 1);
		K2_OnStartAutoRun();
	}
}

void AGASSPlayerController::OnEndAutoRun()
{
	if (UGASSAbilitySystemComponent* GASSASC = GetGASSAbilitySystemComponent())
	{
		GASSASC->SetLooseGameplayTagCount(GASSGameplayTags::Status_AutoRunning, 0);
		K2_OnEndAutoRun();
	}
}

void AGASSPlayerController::UpdateForceFeedback(IInputInterface* InputInterface, const int32 ControllerId)
{
	if (bForceFeedbackEnabled)
	{
		if (const UCommonInputSubsystem* CommonInputSubsystem = UCommonInputSubsystem::Get(GetLocalPlayer()))
		{
			const ECommonInputType CurrentInputType = CommonInputSubsystem->GetCurrentInputType();
			if (GASS::Input::ShouldAlwaysPlayForceFeedback || CurrentInputType == ECommonInputType::Gamepad || CurrentInputType == ECommonInputType::Touch)
			{
				InputInterface->SetForceFeedbackChannelValues(ControllerId, ForceFeedbackValues);
				return;
			}
		}
	}

	InputInterface->SetForceFeedbackChannelValues(ControllerId, FForceFeedbackValues());
}

void AGASSPlayerController::UpdateHiddenComponents(const FVector& ViewLocation, TSet<FPrimitiveComponentId>& OutHiddenComponents)
{
	Super::UpdateHiddenComponents(ViewLocation, OutHiddenComponents);

	if (bHideViewTargetPawnNextFrame)
	{
		AActor* const ViewTargetPawn = PlayerCameraManager ? Cast<AActor>(PlayerCameraManager->GetViewTarget()) : nullptr;
		if (ViewTargetPawn)
		{
			// internal helper func to hide all the components
			auto AddToHiddenComponents = [&OutHiddenComponents](const TInlineComponentArray<UPrimitiveComponent*>& InComponents)
			{
				// add every component and all attached children
				for (UPrimitiveComponent* Comp : InComponents)
				{
					if (Comp->IsRegistered())
					{
						OutHiddenComponents.Add(Comp->ComponentId);

						for (USceneComponent* AttachedChild : Comp->GetAttachChildren())
						{
							static FName NAME_NoParentAutoHide(TEXT("NoParentAutoHide"));
							UPrimitiveComponent* AttachChildPC = Cast<UPrimitiveComponent>(AttachedChild);
							if (AttachChildPC && AttachChildPC->IsRegistered() && !AttachChildPC->ComponentTags.Contains(NAME_NoParentAutoHide))
							{
								OutHiddenComponents.Add(AttachChildPC->ComponentId);
							}
						}
					}
				}
			};

			//TODO Solve with an interface.  Gather hidden components or something.
			//TODO Hiding isn't awesome, sometimes you want the effect of a fade out over a proximity, needs to bubble up to designers.

			// hide pawn's components
			TInlineComponentArray<UPrimitiveComponent*> PawnComponents;
			ViewTargetPawn->GetComponents(PawnComponents);
			AddToHiddenComponents(PawnComponents);

			//// hide weapon too
			//if (ViewTargetPawn->CurrentWeapon)
			//{
			//	TInlineComponentArray<UPrimitiveComponent*> WeaponComponents;
			//	ViewTargetPawn->CurrentWeapon->GetComponents(WeaponComponents);
			//	AddToHiddenComponents(WeaponComponents);
			//}
		}
		else {
			UE_LOG(LogTemp, Warning, TEXT("No Camera Manager"));
		}
		// we consumed it, reset for next frame
		bHideViewTargetPawnNextFrame = false;
	}
}

//void AGASSPlayerController::SetGenericTeamId(const FGenericTeamId& NewTeamID)
//{
//	UE_LOG(LogGASSTeams, Error, TEXT("You can't set the team ID on a player controller (%s); it's driven by the associated player state"), *GetPathNameSafe(this));
//}
//
//FGenericTeamId AGASSPlayerController::GetGenericTeamId() const
//{
//	if (const IGASSTeamAgentInterface* PSWithTeamInterface = Cast<IGASSTeamAgentInterface>(PlayerState))
//	{
//		return PSWithTeamInterface->GetGenericTeamId();
//	}
//	return FGenericTeamId::NoTeam;
//}
//
//FOnGASSTeamIndexChangedDelegate* AGASSPlayerController::GetOnTeamIndexChangedDelegate()
//{
//	return &OnTeamChangedDelegate;
//}
//

void AGASSPlayerController::OnUnPossess()
{
	// Make sure the pawn that is being unpossessed doesn't remain our ASC's avatar actor
	if (APawn* PawnBeingUnpossessed = GetPawn())
	{
		if (UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(PlayerState))
		{
			if (ASC->GetAvatarActor() == PawnBeingUnpossessed)
			{
				ASC->SetAvatarActor(nullptr);
			}
		}
	}

	Super::OnUnPossess();
}

//////////////////////////////////////////////////////////////////////
// InventoryItemInstance

const UGASSInventoryItemInstance* AGASSPlayerController::GetInventoryInstance(TSubclassOf<UGASSItemDefinition> InItemDef)
{
	const UE::Net::FReplicatedComponentInfo* ComponentInfo = UE::Net::FSubObjectRegistryGetter::GetReplicatedComponentInfoForComponent(this, InventoryManagerComponent);
	const TArray<UE::Net::FSubObjectRegistry::FEntry>& Entries = (ComponentInfo->SubObjects.GetRegistryList());
	for (const UE::Net::FSubObjectRegistry::FEntry& Entry : Entries)
	{
		const UObject* SubObject = Entry.GetSubObject();
		const UGASSInventoryItemInstance* ItemInstance = Cast<UGASSInventoryItemInstance>(SubObject);
		if (ItemInstance != nullptr)
		{
			if (ItemInstance->GetItemDef() == InItemDef)
			{
				return ItemInstance;
			}
		}
	}
	return nullptr;
}

//////////////////////////////////////////////////////////////////////
// AGASSReplayPlayerController

void AGASSReplayPlayerController::SetPlayer(UPlayer* InPlayer)
{
	Super::SetPlayer(InPlayer);
}
