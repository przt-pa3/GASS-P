// Copyright Epic Games, Inc. All Rights Reserved.

#include "Character/GASSPawnExtensionComponent.h"

#include "AbilitySystem/GASSAbilitySystemComponent.h"
#include "Inventory/GASSInventoryManagerComponent.h"
#include "Equipment/GASSEquipmentManagerComponent.h"
#include "Components/GameFrameworkComponentDelegates.h"
#include "Components/GameFrameworkComponentManager.h"
#include "GameFramework/Controller.h"
#include "GameFramework/Pawn.h"
#include "GASS/GASSGameplayTags.h"
#include "GASS/GASSLogChannels.h"
#include "Character/GASSPawnData.h"
#include "Net/UnrealNetwork.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GASSPawnExtensionComponent)

class FLifetimeProperty;
class UActorComponent;

const FName UGASSPawnExtensionComponent::NAME_ActorFeatureName("PawnExtension");

UGASSPawnExtensionComponent::UGASSPawnExtensionComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryComponentTick.bStartWithTickEnabled = false;
	PrimaryComponentTick.bCanEverTick = false;

	SetIsReplicatedByDefault(true);

	PawnData = nullptr;
	AbilitySystemComponent = nullptr;
	InventoryManagerComponent = nullptr;
	UE_LOG(LogTemp, Warning, TEXT("Pawn Ext Data Const."));
}

void UGASSPawnExtensionComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UGASSPawnExtensionComponent, PawnData);
}

void UGASSPawnExtensionComponent::OnRegister()
{
	Super::OnRegister();

	const APawn* Pawn = GetPawn<APawn>();
	ensureAlwaysMsgf((Pawn != nullptr), TEXT("GASSPawnExtensionComponent on [%s] can only be added to Pawn actors."), *GetNameSafe(GetOwner()));

	TArray<UActorComponent*> PawnExtensionComponents;
	Pawn->GetComponents(UGASSPawnExtensionComponent::StaticClass(), PawnExtensionComponents);
	ensureAlwaysMsgf((PawnExtensionComponents.Num() == 1), TEXT("Only one GASSPawnExtensionComponent should exist on [%s]."), *GetNameSafe(GetOwner()));

	// Register with the init state system early, this will only work if this is a game world
	RegisterInitStateFeature();
}

void UGASSPawnExtensionComponent::BeginPlay()
{
	Super::BeginPlay();

	// Listen for changes to all features
	BindOnActorInitStateChanged(NAME_None, FGameplayTag(), false);

	// Notifies state manager that we have spawned, then try rest of default initialization
	ensure(TryToChangeInitState(GASSGameplayTags::InitState_Spawned));
	CheckDefaultInitialization();
}

void UGASSPawnExtensionComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UninitializeAbilitySystem();
	UnregisterInitStateFeature();

	Super::EndPlay(EndPlayReason);
}

void UGASSPawnExtensionComponent::SetPawnData(const UGASSPawnData* InPawnData)
{
	check(InPawnData);

	APawn* Pawn = GetPawnChecked<APawn>();

	if (Pawn->GetLocalRole() != ROLE_Authority)
	{
		return;
	}

	if (PawnData)
	{
		UE_LOG(LogGASS, Error, TEXT("Trying to set PawnData [%s] on pawn [%s] that already has valid PawnData [%s]."), *GetNameSafe(InPawnData), *GetNameSafe(Pawn), *GetNameSafe(PawnData));
		return;
	}

	PawnData = InPawnData;

	Pawn->ForceNetUpdate();

	CheckDefaultInitialization();
}

void UGASSPawnExtensionComponent::OnRep_PawnData()
{
	CheckDefaultInitialization();
}

void UGASSPawnExtensionComponent::InitializeAbilitySystem(UGASSAbilitySystemComponent* InASC, AActor* InOwnerActor)
{
	check(InASC);
	check(InOwnerActor);

	if (AbilitySystemComponent == InASC)
	{
		// The ability system component hasn't changed.
		return;
	}

	if (AbilitySystemComponent)
	{
		// Clean up the old ability system component.
		UninitializeAbilitySystem();
	}

	APawn* Pawn = GetPawnChecked<APawn>();
	AActor* ExistingAvatar = InASC->GetAvatarActor();

	UE_LOG(LogGASS, Verbose, TEXT("Setting up ASC [%s] on pawn [%s] owner [%s], existing [%s] "), *GetNameSafe(InASC), *GetNameSafe(Pawn), *GetNameSafe(InOwnerActor), *GetNameSafe(ExistingAvatar));

	if ((ExistingAvatar != nullptr) && (ExistingAvatar != Pawn))
	{
		UE_LOG(LogGASS, Log, TEXT("Existing avatar (authority=%d)"), ExistingAvatar->HasAuthority() ? 1 : 0);

		// There is already a pawn acting as the ASC's avatar, so we need to kick it out
		// This can happen on clients if they're lagged: their new pawn is spawned + possessed before the dead one is removed
		ensure(!ExistingAvatar->HasAuthority());

		if (UGASSPawnExtensionComponent* OtherExtensionComponent = FindPawnExtensionComponent(ExistingAvatar))
		{
			OtherExtensionComponent->UninitializeAbilitySystem();
		}
	}

	AbilitySystemComponent = InASC;
	AbilitySystemComponent->InitAbilityActorInfo(InOwnerActor, Pawn);

	if (ensure(PawnData))
	{
		InASC->SetTagMapping(PawnData->TagMapping);
	}

	OnAbilitySystemInitialized.Broadcast();
}

void UGASSPawnExtensionComponent::UninitializeAbilitySystem()
{
	if (!AbilitySystemComponent)
	{
		return;
	}

	// Uninitialize the ASC if we're still the avatar actor (otherwise another pawn already did it when they became the avatar actor)
	if (AbilitySystemComponent->GetAvatarActor() == GetOwner())
	{
		FGameplayTagContainer AbilityTypesToIgnore;
		AbilityTypesToIgnore.AddTag(GASSGameplayTags::Ability_Behavior_SurvivesDeath);

		AbilitySystemComponent->CancelAbilities(nullptr, &AbilityTypesToIgnore);
		AbilitySystemComponent->ClearAbilityInput();
		AbilitySystemComponent->RemoveAllGameplayCues();

		if (AbilitySystemComponent->GetOwnerActor() != nullptr)
		{
			AbilitySystemComponent->SetAvatarActor(nullptr);
		}
		else
		{
			// If the ASC doesn't have a valid owner, we need to clear *all* actor info, not just the avatar pairing
			AbilitySystemComponent->ClearActorInfo();
		}

		OnAbilitySystemUninitialized.Broadcast();
	}

	AbilitySystemComponent = nullptr;
}

void UGASSPawnExtensionComponent::InitializeInventoryManager(UGASSInventoryManagerComponent* InIMC, AActor* InOwnerActor)
{
	check(InIMC);
	check(InOwnerActor);
	// InOwnerActor is GASS Player Controller

	if (InventoryManagerComponent == InIMC)
	{
		// The IMC hasn't changed.
		UE_LOG(LogTemp, Warning, TEXT("Already Have Valid IMC"));
		return;
	}

	if (InventoryManagerComponent)
	{
		// Clean up the old IMC.
		UninitializeInventoryManager();
	}

	APawn* Pawn = GetPawnChecked<APawn>();
	AActor* ExistingAvatar = InIMC->GetAvatarActor();

	if ((ExistingAvatar != nullptr) && (ExistingAvatar != Pawn))
	{
		// There is already another pawn with IMC
		UE_LOG(LogTemp, Warning, TEXT("There is Already Another Pawn with IMC"));
	}

	InventoryManagerComponent = InIMC;
	InventoryManagerComponent->InitInventoryActorInfo(InOwnerActor, Pawn);

	// TODO : Init Where?, How to get Init Data?
	if (GetOwnerRole() != ROLE_Authority)
	{
		return;
	}
	else
	{
		InventoryManagerComponent->InitializeInventory(10, 6, 6);
		InventoryManagerComponent->InitializeBlackbox(4, 4);
	}
	
}

void UGASSPawnExtensionComponent::UninitializeInventoryManager()
{
	if (!InventoryManagerComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("UnInitialize IMC - nullptr"));
		return;
	}
	//Uninitialize!
	UE_LOG(LogTemp, Warning, TEXT("UnInitialize IMC - Not Null"));
	InventoryManagerComponent = nullptr;
	return;
}

void UGASSPawnExtensionComponent::InitializeEquipmentManager(UGASSEquipmentManagerComponent* InEMC, AActor* InOwnerActor)
{
	check(InEMC);
	check(InOwnerActor);

	if (EquipmentManagerComponent == InEMC)
	{
		// The EMC hasn't changed.
		UE_LOG(LogTemp, Warning, TEXT("Already Have Valid EMC"));
		return;
	}

	if (EquipmentManagerComponent)
	{
		// Clean up the old IMC.
		UninitializeEquipmentManager();
	}

	APawn* Pawn = GetPawnChecked<APawn>();
	AActor* ExistingOwner = InEMC->GetOwnerActor();

	if ((ExistingOwner != nullptr) && (ExistingOwner != Pawn))
	{
		// There is already another pawn with IMC
		UE_LOG(LogTemp, Warning, TEXT("There is Already Another Pawn with EMC"));
	}

	EquipmentManagerComponent = InEMC;
	EquipmentManagerComponent->InitEquipmentActorInfo(InOwnerActor);

	// TODO : Init Where?, How to get Init Data?
	if (GetOwnerRole() != ROLE_Authority)
	{
		return;
	}
	else
	{
		EquipmentManagerComponent->InitializeEquipmentComponent();
	}
}

void UGASSPawnExtensionComponent::UninitializeEquipmentManager()
{
	if (!EquipmentManagerComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("UnInitialize EMC - nullptr"));
		return;
	}
	UE_LOG(LogTemp, Warning, TEXT("UnInitialize EMC - Not Null"));
	EquipmentManagerComponent->UninitializeComponent();
	return;
}

void UGASSPawnExtensionComponent::HandleControllerChanged()
{
	if (AbilitySystemComponent && (AbilitySystemComponent->GetAvatarActor() == GetPawnChecked<APawn>()))
	{
		ensure(AbilitySystemComponent->AbilityActorInfo->OwnerActor == AbilitySystemComponent->GetOwnerActor());
		if (AbilitySystemComponent->GetOwnerActor() == nullptr)
		{
			UninitializeAbilitySystem();
		}
		else
		{
			AbilitySystemComponent->RefreshAbilityActorInfo();
		}
	}

	CheckDefaultInitialization();
}

void UGASSPawnExtensionComponent::HandlePlayerStateReplicated()
{
	CheckDefaultInitialization();
}

void UGASSPawnExtensionComponent::SetupPlayerInputComponent()
{
	CheckDefaultInitialization();
}

void UGASSPawnExtensionComponent::CheckDefaultInitialization()
{
	// Before checking our progress, try progressing any other features we might depend on
	CheckDefaultInitializationForImplementers();

	static const TArray<FGameplayTag> StateChain = { GASSGameplayTags::InitState_Spawned, GASSGameplayTags::InitState_DataAvailable, GASSGameplayTags::InitState_DataInitialized, GASSGameplayTags::InitState_GameplayReady };

	// This will try to progress from spawned (which is only set in BeginPlay) through the data initialization stages until it gets to gameplay ready
	ContinueInitStateChain(StateChain);
}

bool UGASSPawnExtensionComponent::CanChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState) const
{
	check(Manager);
	APawn* Pawn = GetPawn<APawn>();
	if (!CurrentState.IsValid() && DesiredState == GASSGameplayTags::InitState_Spawned)
	{
		// As long as we are on a valid pawn, we count as spawned
		if (Pawn)
		{
			return true;
		}
	}
	if (CurrentState == GASSGameplayTags::InitState_Spawned && DesiredState == GASSGameplayTags::InitState_DataAvailable)
	{
		// Pawn data is required.
		if (!PawnData)
		{
			return false;
		}

		const bool bHasAuthority = Pawn->HasAuthority();
		const bool bIsLocallyControlled = Pawn->IsLocallyControlled();

		if (bHasAuthority || bIsLocallyControlled)
		{
			// Check for being possessed by a controller.
			if (!GetController<AController>())
			{
				return false;
			}
		}
		UE_LOG(LogTemp, Warning, TEXT("PawnExtComp State :: Spawned -> DataAvailable"));
		return true;
	}
	else if (CurrentState == GASSGameplayTags::InitState_DataAvailable && DesiredState == GASSGameplayTags::InitState_DataInitialized)
	{
		// Transition to initialize if all features have their data available
		//return Manager->HaveAllFeaturesReachedInitState(Pawn, GASSGameplayTags::InitState_DataAvailable);
		if (Manager->HaveAllFeaturesReachedInitState(Pawn, GASSGameplayTags::InitState_DataAvailable)) {
			return true;
		}
		else {
			return false;
		}
	}
	else if (CurrentState == GASSGameplayTags::InitState_DataInitialized && DesiredState == GASSGameplayTags::InitState_GameplayReady)
	{
		return true;
	}

	return false;
}

void UGASSPawnExtensionComponent::HandleChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState)
{
	if (DesiredState == GASSGameplayTags::InitState_DataInitialized)
	{
		// This is currently all handled by other components listening to this state change
	}
}

void UGASSPawnExtensionComponent::OnActorInitStateChanged(const FActorInitStateChangedParams& Params)
{
	// If another feature is now in DataAvailable, see if we should transition to DataInitialized
	if (Params.FeatureName != NAME_ActorFeatureName)
	{
		if (Params.FeatureState == GASSGameplayTags::InitState_DataAvailable)
		{
			CheckDefaultInitialization();
		}
	}
}

void UGASSPawnExtensionComponent::OnAbilitySystemInitialized_RegisterAndCall(FSimpleMulticastDelegate::FDelegate Delegate)
{
	if (!OnAbilitySystemInitialized.IsBoundToObject(Delegate.GetUObject()))
	{
		OnAbilitySystemInitialized.Add(Delegate);
	}

	if (AbilitySystemComponent)
	{
		Delegate.Execute();
	}
}

void UGASSPawnExtensionComponent::OnAbilitySystemUninitialized_Register(FSimpleMulticastDelegate::FDelegate Delegate)
{
	if (!OnAbilitySystemUninitialized.IsBoundToObject(Delegate.GetUObject()))
	{
		OnAbilitySystemUninitialized.Add(Delegate);
	}
}

