// Copyright Epic Games, Inc. All Rights Reserved.

#include "Character/GASSHeroComponent.h"
#include "Components/GameFrameworkComponentDelegates.h"
#include "Logging/MessageLog.h"
#include "Input/GASSMappableConfigPair.h"
#include "GASS/GASSLogChannels.h"
#include "EnhancedInputSubsystems.h"
#include "Player/GASSPlayerController.h"
#include "Player/GASSPlayerState.h"
#include "Player/GASSLocalPlayer.h"
#include "Character/GASSPawnExtensionComponent.h"
#include "Character/GASSPawnData.h"
#include "Character/GASSCharacter.h"
#include "AbilitySystem/GASSAbilitySystemComponent.h"
#include "Input/GASSInputConfig.h"
#include "Input/GASSInputComponent.h"
#include "Camera/GASSCameraComponent.h"
#include "GASS/GASSGameplayTags.h"
#include "Components/GameFrameworkComponentManager.h"
#include "PlayerMappableInputConfig.h"
#include "Camera/GASSCameraMode.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GASSHeroComponent)

#if WITH_EDITOR
#include "Misc/UObjectToken.h"
#endif	// WITH_EDITOR

namespace GASSHero
{
	static const float LookYawRate = 300.0f;
	static const float LookPitchRate = 165.0f;
};

const FName UGASSHeroComponent::NAME_BindInputsNow("BindInputsNow");
const FName UGASSHeroComponent::NAME_ActorFeatureName("Hero");

UGASSHeroComponent::UGASSHeroComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	AbilityCameraMode = nullptr;
	bReadyToBindInputs = false;
}

void UGASSHeroComponent::OnRegister()
{
	Super::OnRegister();

	if (!GetPawn<APawn>())
	{
		UE_LOG(LogGASS, Error, TEXT("[UGASSHeroComponent::OnRegister] This component has been added to a blueprint whose base class is not a Pawn. To use this component, it MUST be placed on a Pawn Blueprint."));

#if WITH_EDITOR
		if (GIsEditor)
		{
			static const FText Message = NSLOCTEXT("GASSHeroComponent", "NotOnPawnError", "has been added to a blueprint whose base class is not a Pawn. To use this component, it MUST be placed on a Pawn Blueprint. This will cause a crash if you PIE!");
			static const FName HeroMessageLogName = TEXT("GASSHeroComponent");

			FMessageLog(HeroMessageLogName).Error()
				->AddToken(FUObjectToken::Create(this, FText::FromString(GetNameSafe(this))))
				->AddToken(FTextToken::Create(Message));

			FMessageLog(HeroMessageLogName).Open();
		}
#endif
	}
	else
	{
		// Register with the init state system early, this will only work if this is a game world
		RegisterInitStateFeature();
	}
}

bool UGASSHeroComponent::CanChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState) const
{
	check(Manager);

	APawn* Pawn = GetPawn<APawn>();

	if (!CurrentState.IsValid() && DesiredState == GASSGameplayTags::InitState_Spawned)
	{
		// As long as we have a real pawn, let us transition
		if (Pawn)
		{
			return true;
		}
	}
	else if (CurrentState == GASSGameplayTags::InitState_Spawned && DesiredState == GASSGameplayTags::InitState_DataAvailable)
	{
		// The player state is required.
		if (!GetPlayerState<AGASSPlayerState>())
		{
			return false;
		}

		// If we're authority or autonomous, we need to wait for a controller with registered ownership of the player state.
		if (Pawn->GetLocalRole() != ROLE_SimulatedProxy)
		{			
			AController* Controller = GetController<AController>();

			const bool bHasControllerPairedWithPS = (Controller != nullptr) && \
				(Controller->PlayerState != nullptr) && \
				(Controller->PlayerState->GetOwner() == Controller);

			if (!bHasControllerPairedWithPS)
			{
				return false;
			}
		}

		const bool bIsLocallyControlled = Pawn->IsLocallyControlled();
		const bool bIsBot = Pawn->IsBotControlled();

		if (bIsLocallyControlled && !bIsBot)
		{
			AGASSPlayerController* GASSPC = GetController<AGASSPlayerController>();

			// The input component and local player is required when locally controlled.
			if (!Pawn->InputComponent || !GASSPC || !GASSPC->GetLocalPlayer())
			{
				return false;
			}
		}

		return true;
	}
	else if (CurrentState == GASSGameplayTags::InitState_DataAvailable && DesiredState == GASSGameplayTags::InitState_DataInitialized)
	{
		// Wait for player state and extension component
		AGASSPlayerState* GASSPS = GetPlayerState<AGASSPlayerState>();

		return GASSPS && Manager->HasFeatureReachedInitState(Pawn, UGASSPawnExtensionComponent::NAME_ActorFeatureName, GASSGameplayTags::InitState_DataInitialized);

	}
	else if (CurrentState == GASSGameplayTags::InitState_DataInitialized && DesiredState == GASSGameplayTags::InitState_GameplayReady)
	{
		// TODO add ability initialization checks?
		return true;
	}

	return false;
}

void UGASSHeroComponent::HandleChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState)
{
	if (CurrentState == GASSGameplayTags::InitState_DataAvailable && DesiredState == GASSGameplayTags::InitState_DataInitialized)
	{
		// Controller is Already Set at this Moment

		APawn* Pawn = GetPawn<APawn>();
		AGASSPlayerState* GASSPS = GetPlayerState<AGASSPlayerState>();
		AGASSPlayerController* GASSPC = GetController<AGASSPlayerController>();

		if (!ensure(Pawn && GASSPS))
		{
			return;
		}

		const bool bIsLocallyControlled = Pawn->IsLocallyControlled();
		const UGASSPawnData* PawnData = nullptr;

		if (UGASSPawnExtensionComponent* PawnExtComp = UGASSPawnExtensionComponent::FindPawnExtensionComponent(Pawn))
		{
			PawnData = PawnExtComp->GetPawnData<UGASSPawnData>();

			// The player state holds the persistent data for this player (state that persists across deaths and multiple pawns).
			// The ability system component and attribute sets live on the player state.
			UE_LOG(LogTemp, Warning, TEXT("HeroComp. HandleChange"));
			PawnExtComp->InitializeAbilitySystem(GASSPS->GetGASSAbilitySystemComponent(), GASSPS);

			if (GASSPC)
			{
				PawnExtComp->InitializeInventoryManager(GASSPC->GetGASSInventoryManagerComponent(), GASSPC);
			}

			if (Pawn)
			{
				PawnExtComp->InitializeEquipmentManager(Pawn->GetComponentByClass<UGASSEquipmentManagerComponent>(), Pawn);
			}
		}

		if (GASSPC)
		{
			if (Pawn->InputComponent != nullptr)
			{
				InitializePlayerInput(Pawn->InputComponent);
			}
		}

		if (bIsLocallyControlled && PawnData)
		{
			if (UGASSCameraComponent* CameraComponent = UGASSCameraComponent::FindCameraComponent(Pawn))
			{
				CameraComponent->DetermineCameraModeDelegate.BindUObject(this, &ThisClass::DetermineCameraMode);
			}
		}
	}
}

void UGASSHeroComponent::OnActorInitStateChanged(const FActorInitStateChangedParams& Params)
{
	if (Params.FeatureName == UGASSPawnExtensionComponent::NAME_ActorFeatureName)
	{
		if (Params.FeatureState == GASSGameplayTags::InitState_DataInitialized)
		{
			// If the extension component says all all other components are initialized, try to progress to next state
			CheckDefaultInitialization();
		}
	}
}

void UGASSHeroComponent::CheckDefaultInitialization()
{
	static const TArray<FGameplayTag> StateChain = { GASSGameplayTags::InitState_Spawned, GASSGameplayTags::InitState_DataAvailable, GASSGameplayTags::InitState_DataInitialized, GASSGameplayTags::InitState_GameplayReady };


	// TODO : Cannot Continue Chain??


	// This will try to progress from spawned (which is only set in BeginPlay) through the data initialization stages until it gets to gameplay ready
	ContinueInitStateChain(StateChain);
}

void UGASSHeroComponent::BeginPlay()
{
	Super::BeginPlay();

	// Listen for when the pawn extension component changes init state
	BindOnActorInitStateChanged(UGASSPawnExtensionComponent::NAME_ActorFeatureName, FGameplayTag(), false);

	// Notifies that we are done spawning, then try the rest of initialization
	ensure(TryToChangeInitState(GASSGameplayTags::InitState_Spawned));
	CheckDefaultInitialization();
}

void UGASSHeroComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UnregisterInitStateFeature();

	Super::EndPlay(EndPlayReason);
}

void UGASSHeroComponent::InitializePlayerInput(UInputComponent* PlayerInputComponent)
{
	check(PlayerInputComponent);

	const APawn* Pawn = GetPawn<APawn>();
	if (!Pawn)
	{
		return;
	}

	const APlayerController* PC = GetController<APlayerController>();
	check(PC);

	const UGASSLocalPlayer* LP = Cast<UGASSLocalPlayer>(PC->GetLocalPlayer());
	check(LP);

	UEnhancedInputLocalPlayerSubsystem* Subsystem = LP->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
	check(Subsystem);

	Subsystem->ClearAllMappings();

	if (const UGASSPawnExtensionComponent* PawnExtComp = UGASSPawnExtensionComponent::FindPawnExtensionComponent(Pawn))
	{
		if (const UGASSPawnData* PawnData = PawnExtComp->GetPawnData<UGASSPawnData>())
		{
			if (const UGASSInputConfig* InputConfig = PawnData->InputConfig)
			{
				// Register any default input configs with the settings so that they will be applied to the player during AddInputMappings
				for (const FMappableConfigPair& Pair : DefaultInputConfigs)
				{
					if (Pair.bShouldActivateAutomatically && Pair.CanBeActivated())
					{
						FModifyContextOptions Options = {};
						Options.bIgnoreAllPressedKeysUntilRelease = false;
						// Actually add the config to the local player							
						Subsystem->AddPlayerMappableConfig(Pair.Config.LoadSynchronous(), Options);
					}
				}

				// The GASS Input Component has some additional functions to map Gameplay Tags to an Input Action.
				// If you want this functionality but still want to change your input component class, make it a subclass
				// of the UGASSInputComponent or modify this component accordingly.
				UGASSInputComponent* GASSIC = Cast<UGASSInputComponent>(PlayerInputComponent);
				if (ensureMsgf(GASSIC, TEXT("Unexpected Input Component class! The Gameplay Abilities will not be bound to their inputs. Change the input component to UGASSInputComponent or a subclass of it.")))
				{
					// Add the key mappings that may have been set by the player
					GASSIC->AddInputMappings(InputConfig, Subsystem);

					// This is where we actually bind and input action to a gameplay tag, which means that Gameplay Ability Blueprints will
					// be triggered directly by these input actions Triggered events. 
					TArray<uint32> BindHandles;
					GASSIC->BindAbilityActions(InputConfig, this, &ThisClass::Input_AbilityInputTagPressed, &ThisClass::Input_AbilityInputTagReleased, /*out*/ BindHandles);

					UE_LOG(LogTemp, Warning, TEXT("Binding Input and Tag"));
					GASSIC->BindNativeAction(InputConfig, GASSGameplayTags::InputTag_Move, ETriggerEvent::Triggered, this, &ThisClass::Input_Move, /*bLogIfNotFound=*/ true);
					GASSIC->BindNativeAction(InputConfig, GASSGameplayTags::InputTag_Look_Mouse, ETriggerEvent::Triggered, this, &ThisClass::Input_LookMouse, /*bLogIfNotFound=*/ true);
					GASSIC->BindNativeAction(InputConfig, GASSGameplayTags::InputTag_Look_Stick, ETriggerEvent::Triggered, this, &ThisClass::Input_LookStick, /*bLogIfNotFound=*/ true);
					GASSIC->BindNativeAction(InputConfig, GASSGameplayTags::InputTag_Crouch, ETriggerEvent::Triggered, this, &ThisClass::Input_Crouch, /*bLogIfNotFound=*/ false);
					GASSIC->BindNativeAction(InputConfig, GASSGameplayTags::InputTag_AutoRun, ETriggerEvent::Triggered, this, &ThisClass::Input_AutoRun, /*bLogIfNotFound=*/ false);
				}
			}
		}
	}

	if (ensure(!bReadyToBindInputs))
	{
		bReadyToBindInputs = true;
	}

	UGameFrameworkComponentManager::SendGameFrameworkComponentExtensionEvent(const_cast<APlayerController*>(PC), NAME_BindInputsNow);
	UGameFrameworkComponentManager::SendGameFrameworkComponentExtensionEvent(const_cast<APawn*>(Pawn), NAME_BindInputsNow);
}

void UGASSHeroComponent::AddAdditionalInputConfig(const UGASSInputConfig* InputConfig)
{
	TArray<uint32> BindHandles;

	const APawn* Pawn = GetPawn<APawn>();
	if (!Pawn)
	{
		return;
	}

	const APlayerController* PC = GetController<APlayerController>();
	check(PC);

	const ULocalPlayer* LP = PC->GetLocalPlayer();
	check(LP);

	UEnhancedInputLocalPlayerSubsystem* Subsystem = LP->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
	check(Subsystem);

	if (const UGASSPawnExtensionComponent* PawnExtComp = UGASSPawnExtensionComponent::FindPawnExtensionComponent(Pawn))
	{
		UGASSInputComponent* GASSIC = Pawn->FindComponentByClass<UGASSInputComponent>();
		if (ensureMsgf(GASSIC, TEXT("Unexpected Input Component class! The Gameplay Abilities will not be bound to their inputs. Change the input component to UGASSInputComponent or a subclass of it.")))
		{
			GASSIC->BindAbilityActions(InputConfig, this, &ThisClass::Input_AbilityInputTagPressed, &ThisClass::Input_AbilityInputTagReleased, /*out*/ BindHandles);
		}
	}
}

void UGASSHeroComponent::RemoveAdditionalInputConfig(const UGASSInputConfig* InputConfig)
{
	//@TODO: Implement me!
}

bool UGASSHeroComponent::IsReadyToBindInputs() const
{
	return bReadyToBindInputs;
}

void UGASSHeroComponent::Input_AbilityInputTagPressed(FGameplayTag InputTag)
{
	if (const APawn* Pawn = GetPawn<APawn>())
	{
		if (const UGASSPawnExtensionComponent* PawnExtComp = UGASSPawnExtensionComponent::FindPawnExtensionComponent(Pawn))
		{
			if (UGASSAbilitySystemComponent* GASSASC = PawnExtComp->GetGASSAbilitySystemComponent())
			{
				GASSASC->AbilityInputTagPressed(InputTag);
			}
		}
	}
}

void UGASSHeroComponent::Input_AbilityInputTagReleased(FGameplayTag InputTag)
{
	const APawn* Pawn = GetPawn<APawn>();
	if (!Pawn)
	{
		return;
	}

	if (const UGASSPawnExtensionComponent* PawnExtComp = UGASSPawnExtensionComponent::FindPawnExtensionComponent(Pawn))
	{
		if (UGASSAbilitySystemComponent* GASSASC = PawnExtComp->GetGASSAbilitySystemComponent())
		{
			GASSASC->AbilityInputTagReleased(InputTag);
		}
	}
}

void UGASSHeroComponent::Input_Move(const FInputActionValue& InputActionValue)
{
	APawn* Pawn = GetPawn<APawn>();
	AController* Controller = Pawn ? Pawn->GetController() : nullptr;

	// If the player has attempted to move again then cancel auto running
	if (AGASSPlayerController* GASSController = Cast<AGASSPlayerController>(Controller))
	{
		GASSController->SetIsAutoRunning(false);
	}

	if (Controller)
	{
		const FVector2D Value = InputActionValue.Get<FVector2D>();
		const FRotator MovementRotation(0.0f, Controller->GetControlRotation().Yaw, 0.0f);

		if (Value.X != 0.0f)
		{
			const FVector MovementDirection = MovementRotation.RotateVector(FVector::RightVector);
			Pawn->AddMovementInput(MovementDirection, Value.X);
		}

		if (Value.Y != 0.0f)
		{
			const FVector MovementDirection = MovementRotation.RotateVector(FVector::ForwardVector);
			Pawn->AddMovementInput(MovementDirection, Value.Y);
		}
	}
}

void UGASSHeroComponent::Input_LookMouse(const FInputActionValue& InputActionValue)
{
	APawn* Pawn = GetPawn<APawn>();

	if (!Pawn)
	{
		return;
	}

	const FVector2D Value = InputActionValue.Get<FVector2D>();

	if (Value.X != 0.0f)
	{
		Pawn->AddControllerYawInput(Value.X);
	}

	if (Value.Y != 0.0f)
	{
		Pawn->AddControllerPitchInput(Value.Y);
	}
}

void UGASSHeroComponent::Input_LookStick(const FInputActionValue& InputActionValue)
{
	APawn* Pawn = GetPawn<APawn>();

	if (!Pawn)
	{
		return;
	}

	const FVector2D Value = InputActionValue.Get<FVector2D>();

	const UWorld* World = GetWorld();
	check(World);

	if (Value.X != 0.0f)
	{
		Pawn->AddControllerYawInput(Value.X * GASSHero::LookYawRate * World->GetDeltaSeconds());
	}

	if (Value.Y != 0.0f)
	{
		Pawn->AddControllerPitchInput(Value.Y * GASSHero::LookPitchRate * World->GetDeltaSeconds());
	}
}

void UGASSHeroComponent::Input_Crouch(const FInputActionValue& InputActionValue)
{
	if (AGASSCharacter* Character = GetPawn<AGASSCharacter>())
	{
		Character->ToggleCrouch();
	}
}

void UGASSHeroComponent::Input_AutoRun(const FInputActionValue& InputActionValue)
{
	if (APawn* Pawn = GetPawn<APawn>())
	{
		if (AGASSPlayerController* Controller = Cast<AGASSPlayerController>(Pawn->GetController()))
		{
			// Toggle auto running
			Controller->SetIsAutoRunning(!Controller->GetIsAutoRunning());
		}
	}
}

TSubclassOf<UGASSCameraMode> UGASSHeroComponent::DetermineCameraMode() const
{
	if (AbilityCameraMode)
	{
		return AbilityCameraMode;
	}

	const APawn* Pawn = GetPawn<APawn>();
	if (!Pawn)
	{
		return nullptr;
	}

	if (UGASSPawnExtensionComponent* PawnExtComp = UGASSPawnExtensionComponent::FindPawnExtensionComponent(Pawn))
	{
		if (const UGASSPawnData* PawnData = PawnExtComp->GetPawnData<UGASSPawnData>())
		{
			return PawnData->DefaultCameraMode;
		}
	}

	return nullptr;
}

void UGASSHeroComponent::SetAbilityCameraMode(TSubclassOf<UGASSCameraMode> CameraMode, const FGameplayAbilitySpecHandle& OwningSpecHandle)
{
	if (CameraMode)
	{
		AbilityCameraMode = CameraMode;
		AbilityCameraModeOwningSpecHandle = OwningSpecHandle;
	}
}

void UGASSHeroComponent::ClearAbilityCameraMode(const FGameplayAbilitySpecHandle& OwningSpecHandle)
{
	if (AbilityCameraModeOwningSpecHandle == OwningSpecHandle)
	{
		AbilityCameraMode = nullptr;
		AbilityCameraModeOwningSpecHandle = FGameplayAbilitySpecHandle();
	}
}

