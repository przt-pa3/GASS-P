// Copyright Epic Games, Inc. All Rights Reserved.

#include "Character/GASSCharacter.h"

#include "AbilitySystem/GASSAbilitySystemComponent.h"
#include "Inventory/GASSInventoryManagerComponent.h"
#include "Equipment/GASSEquipmentManagerComponent.h"
#include "Camera/GASSCameraComponent.h"
#include "Character/GASSHealthComponent.h"
#include "Character/GASSPawnExtensionComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Character/GASSCharacterMovementComponent.h"
#include "GASS/GASSGameplayTags.h"
#include "GASS/GASSLogChannels.h"
#include "Net/UnrealNetwork.h"
#include "Player/GASSPlayerController.h"
#include "Player/GASSPlayerState.h"
//#include "System/GASSSignificanceManager.h"
#include "TimerManager.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GASSCharacter)

class AActor;
class FLifetimeProperty;
class IRepChangedPropertyTracker;
class UInputComponent;

static FName NAME_GASSCharacterCollisionProfile_Capsule(TEXT("GASSPawnCapsule"));
static FName NAME_GASSCharacterCollisionProfile_Mesh(TEXT("GASSPawnMesh"));

AGASSCharacter::AGASSCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UGASSCharacterMovementComponent>(ACharacter::CharacterMovementComponentName))
{
	UE_LOG(LogTemp, Warning, TEXT("Character Const. called"));
	// Avoid ticking characters if possible.
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;

	NetCullDistanceSquared = 900000000.0f;

	UCapsuleComponent* CapsuleComp = GetCapsuleComponent();
	check(CapsuleComp);
	CapsuleComp->InitCapsuleSize(40.0f, 90.0f);
	CapsuleComp->SetCollisionProfileName(NAME_GASSCharacterCollisionProfile_Capsule);

	USkeletalMeshComponent* MeshComp = GetMesh();
	check(MeshComp);
	MeshComp->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));  // Rotate mesh to be X forward since it is exported as Y forward.
	MeshComp->SetCollisionProfileName(NAME_GASSCharacterCollisionProfile_Mesh);

	UGASSCharacterMovementComponent* GASSMoveComp = CastChecked<UGASSCharacterMovementComponent>(GetCharacterMovement());
	GASSMoveComp->GravityScale = 1.0f;
	GASSMoveComp->MaxAcceleration = 2400.0f;
	GASSMoveComp->BrakingFrictionFactor = 1.0f;
	GASSMoveComp->BrakingFriction = 6.0f;
	GASSMoveComp->GroundFriction = 8.0f;
	GASSMoveComp->BrakingDecelerationWalking = 1400.0f;
	GASSMoveComp->bUseControllerDesiredRotation = false;
	GASSMoveComp->bOrientRotationToMovement = false;
	GASSMoveComp->RotationRate = FRotator(0.0f, 720.0f, 0.0f);
	GASSMoveComp->bAllowPhysicsRotationDuringAnimRootMotion = false;
	GASSMoveComp->GetNavAgentPropertiesRef().bCanCrouch = true;
	GASSMoveComp->bCanWalkOffLedgesWhenCrouching = true;
	GASSMoveComp->SetCrouchedHalfHeight(65.0f);

	PawnExtComponent = CreateDefaultSubobject<UGASSPawnExtensionComponent>(TEXT("PawnExtensionComponent"));
	PawnExtComponent->OnAbilitySystemInitialized_RegisterAndCall(FSimpleMulticastDelegate::FDelegate::CreateUObject(this, &ThisClass::OnAbilitySystemInitialized));
	PawnExtComponent->OnAbilitySystemUninitialized_Register(FSimpleMulticastDelegate::FDelegate::CreateUObject(this, &ThisClass::OnAbilitySystemUninitialized));

	EquipmentManagerComponent = ObjectInitializer.CreateDefaultSubobject<UGASSEquipmentManagerComponent>(this, TEXT("EquipmentSystemComponent"));
	EquipmentManagerComponent->SetIsReplicated(true);

	HealthComponent = CreateDefaultSubobject<UGASSHealthComponent>(TEXT("HealthComponent"));
	//HealthComponent->OnDeathStarted.AddDynamic(this, &ThisClass::OnDeathStarted);
	//HealthComponent->OnDeathFinished.AddDynamic(this, &ThisClass::OnDeathFinished);

	CameraComponent = CreateDefaultSubobject<UGASSCameraComponent>(TEXT("CameraComponent"));
	CameraComponent->SetRelativeLocation(FVector(-300.0f, 0.0f, 75.0f));

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = true;
	bUseControllerRotationRoll = false;

	BaseEyeHeight = 80.0f;
	CrouchedEyeHeight = 50.0f;
}

void AGASSCharacter::PreInitializeComponents()
{
	Super::PreInitializeComponents();
}

void AGASSCharacter::BeginPlay()
{
	Super::BeginPlay();

	UWorld* World = GetWorld();

	UE_LOG(LogTemp, Warning, TEXT("Character Beginplay called"));
	const bool bRegisterWithSignificanceManager = !IsNetMode(NM_DedicatedServer);
	if (bRegisterWithSignificanceManager)
	{
		//if (UGASSSignificanceManager* SignificanceManager = USignificanceManager::Get<UGASSSignificanceManager>(World))
		//{
		//	//@TODO: SignificanceManager->RegisterObject(this, (EFortSignificanceType)SignificanceType);
		//}
	}
}

void AGASSCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	UWorld* World = GetWorld();

	const bool bRegisterWithSignificanceManager = !IsNetMode(NM_DedicatedServer);
	if (bRegisterWithSignificanceManager)
	{
		//if (UGASSSignificanceManager* SignificanceManager = USignificanceManager::Get<UGASSSignificanceManager>(World))
		//{
		//	SignificanceManager->UnregisterObject(this);
		//}
	}
}

void AGASSCharacter::Reset()
{
	DisableMovementAndCollision();

	K2_OnReset();

	UninitAndDestroy();
}

void AGASSCharacter::GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ThisClass, ReplicatedAcceleration, COND_SimulatedOnly);
	//DOREPLIFETIME(ThisClass, MyTeamID)
	DOREPLIFETIME(ThisClass, ShouldLineTrace);
	DOREPLIFETIME(ThisClass, EquipmentManagerComponent);
}

void AGASSCharacter::PreReplication(IRepChangedPropertyTracker& ChangedPropertyTracker)
{
	Super::PreReplication(ChangedPropertyTracker);

	if (UCharacterMovementComponent* MovementComponent = GetCharacterMovement())
	{
		// Compress Acceleration: XY components as direction + magnitude, Z component as direct value
		const double MaxAccel = MovementComponent->MaxAcceleration;
		const FVector CurrentAccel = MovementComponent->GetCurrentAcceleration();
		double AccelXYRadians, AccelXYMagnitude;
		FMath::CartesianToPolar(CurrentAccel.X, CurrentAccel.Y, AccelXYMagnitude, AccelXYRadians);

		ReplicatedAcceleration.AccelXYRadians = FMath::FloorToInt((AccelXYRadians / TWO_PI) * 255.0);     // [0, 2PI] -> [0, 255]
		ReplicatedAcceleration.AccelXYMagnitude = FMath::FloorToInt((AccelXYMagnitude / MaxAccel) * 255.0);	// [0, MaxAccel] -> [0, 255]
		ReplicatedAcceleration.AccelZ = FMath::FloorToInt((CurrentAccel.Z / MaxAccel) * 127.0);   // [-MaxAccel, MaxAccel] -> [-127, 127]
	}
}

void AGASSCharacter::NotifyControllerChanged()
{
	//const FGenericTeamId OldTeamId = GetGenericTeamId();

	Super::NotifyControllerChanged();

	//// Update our team ID based on the controller
	//if (HasAuthority() && (Controller != nullptr))
	//{
	//	if (IGASSTeamAgentInterface* ControllerWithTeam = Cast<IGASSTeamAgentInterface>(Controller))
	//	{
	//		MyTeamID = ControllerWithTeam->GetGenericTeamId();
	//		ConditionalBroadcastTeamChanged(this, OldTeamId, MyTeamID);
	//	}
	//}
}

AGASSPlayerController* AGASSCharacter::GetGASSPlayerController() const
{
	return CastChecked<AGASSPlayerController>(Controller, ECastCheckedType::NullAllowed);
}

AGASSPlayerState* AGASSCharacter::GetGASSPlayerState() const
{
	return CastChecked<AGASSPlayerState>(GetPlayerState(), ECastCheckedType::NullAllowed);
}

UGASSAbilitySystemComponent* AGASSCharacter::GetGASSAbilitySystemComponent() const
{
	return Cast<UGASSAbilitySystemComponent>(GetAbilitySystemComponent());
}

UAbilitySystemComponent* AGASSCharacter::GetAbilitySystemComponent() const
{
	return PawnExtComponent->GetGASSAbilitySystemComponent();
}

UGASSInventoryManagerComponent* AGASSCharacter::GetGASSInventoryManagerComponent() const
{
	return PawnExtComponent->GetGASSInventoryManagerComponent();
}

UGASSEquipmentManagerComponent* AGASSCharacter::GetGASSEquipmentManagerComponent() const
{
	return PawnExtComponent->GetGASSEquipmentManagerComponent();
}

void AGASSCharacter::OnAbilitySystemInitialized()
{
	UGASSAbilitySystemComponent* GASSASC = GetGASSAbilitySystemComponent();
	check(GASSASC);

	//HealthComponent->InitializeWithAbilitySystem(GASSASC);

	InitializeGameplayTags();
}

void AGASSCharacter::OnAbilitySystemUninitialized()
{
	//HealthComponent->UninitializeFromAbilitySystem();
}

void AGASSCharacter::PossessedBy(AController* NewController)
{
	//const FGenericTeamId OldTeamID = MyTeamID;

	Super::PossessedBy(NewController);

	PawnExtComponent->HandleControllerChanged();

	//// Grab the current team ID and listen for future changes
	//if (IGASSTeamAgentInterface* ControllerAsTeamProvider = Cast<IGASSTeamAgentInterface>(NewController))
	//{
	//	MyTeamID = ControllerAsTeamProvider->GetGenericTeamId();
	//	ControllerAsTeamProvider->GetTeamChangedDelegateChecked().AddDynamic(this, &ThisClass::OnControllerChangedTeam);
	//}
	//ConditionalBroadcastTeamChanged(this, OldTeamID, MyTeamID);
}

void AGASSCharacter::UnPossessed()
{
	AController* const OldController = Controller;

	//// Stop listening for changes from the old controller
	//const FGenericTeamId OldTeamID = MyTeamID;
	//if (IGASSTeamAgentInterface* ControllerAsTeamProvider = Cast<IGASSTeamAgentInterface>(OldController))
	//{
	//	ControllerAsTeamProvider->GetTeamChangedDelegateChecked().RemoveAll(this);
	//}

	Super::UnPossessed();

	PawnExtComponent->HandleControllerChanged();

	//// Determine what the new team ID should be afterwards
	//MyTeamID = DetermineNewTeamAfterPossessionEnds(OldTeamID);
	//ConditionalBroadcastTeamChanged(this, OldTeamID, MyTeamID);
}

void AGASSCharacter::OnRep_Controller()
{
	Super::OnRep_Controller();

	PawnExtComponent->HandleControllerChanged();
}

void AGASSCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	PawnExtComponent->HandlePlayerStateReplicated();
}

void AGASSCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PawnExtComponent->SetupPlayerInputComponent();
}

void AGASSCharacter::InitializeGameplayTags()
{
	// Clear tags that may be lingering on the ability system from the previous pawn.
	if (UGASSAbilitySystemComponent* GASSASC = GetGASSAbilitySystemComponent())
	{
		for (const TPair<uint8, FGameplayTag>& TagMapping : GASSGameplayTags::MovementModeTagMap)
		{
			if (TagMapping.Value.IsValid())
			{
				GASSASC->SetLooseGameplayTagCount(TagMapping.Value, 0);
			}
		}

		for (const TPair<uint8, FGameplayTag>& TagMapping : GASSGameplayTags::CustomMovementModeTagMap)
		{
			if (TagMapping.Value.IsValid())
			{
				GASSASC->SetLooseGameplayTagCount(TagMapping.Value, 0);
			}
		}

		UGASSCharacterMovementComponent* GASSMoveComp = CastChecked<UGASSCharacterMovementComponent>(GetCharacterMovement());
		SetMovementModeTag(GASSMoveComp->MovementMode, GASSMoveComp->CustomMovementMode, true);
	}
}

void AGASSCharacter::GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const
{
	if (const UGASSAbilitySystemComponent* GASSASC = GetGASSAbilitySystemComponent())
	{
		GASSASC->GetOwnedGameplayTags(TagContainer);
	}
}

bool AGASSCharacter::HasMatchingGameplayTag(FGameplayTag TagToCheck) const
{
	if (const UGASSAbilitySystemComponent* GASSASC = GetGASSAbilitySystemComponent())
	{
		return GASSASC->HasMatchingGameplayTag(TagToCheck);
	}

	return false;
}

bool AGASSCharacter::HasAllMatchingGameplayTags(const FGameplayTagContainer& TagContainer) const
{
	if (const UGASSAbilitySystemComponent* GASSASC = GetGASSAbilitySystemComponent())
	{
		return GASSASC->HasAllMatchingGameplayTags(TagContainer);
	}

	return false;
}

bool AGASSCharacter::HasAnyMatchingGameplayTags(const FGameplayTagContainer& TagContainer) const
{
	if (const UGASSAbilitySystemComponent* GASSASC = GetGASSAbilitySystemComponent())
	{
		return GASSASC->HasAnyMatchingGameplayTags(TagContainer);
	}

	return false;
}

void AGASSCharacter::FellOutOfWorld(const class UDamageType& dmgType)
{
	//HealthComponent->DamageSelfDestruct(/*bFellOutOfWorld=*/ true);
}

void AGASSCharacter::OnDeathStarted(AActor*)
{
	DisableMovementAndCollision();
}

void AGASSCharacter::OnDeathFinished(AActor*)
{
	GetWorld()->GetTimerManager().SetTimerForNextTick(this, &ThisClass::DestroyDueToDeath);
}


void AGASSCharacter::DisableMovementAndCollision()
{
	if (Controller)
	{
		Controller->SetIgnoreMoveInput(true);
	}

	UCapsuleComponent* CapsuleComp = GetCapsuleComponent();
	check(CapsuleComp);
	CapsuleComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	CapsuleComp->SetCollisionResponseToAllChannels(ECR_Ignore);

	UGASSCharacterMovementComponent* GASSMoveComp = CastChecked<UGASSCharacterMovementComponent>(GetCharacterMovement());
	GASSMoveComp->StopMovementImmediately();
	GASSMoveComp->DisableMovement();
}

void AGASSCharacter::DestroyDueToDeath()
{
	K2_OnDeathFinished();

	UninitAndDestroy();
}


void AGASSCharacter::UninitAndDestroy()
{
	if (GetLocalRole() == ROLE_Authority)
	{
		DetachFromControllerPendingDestroy();
		SetLifeSpan(0.1f);
	}

	// Uninitialize the ASC if we're still the avatar actor (otherwise another pawn already did it when they became the avatar actor)
	if (UGASSAbilitySystemComponent* GASSASC = GetGASSAbilitySystemComponent())
	{
		if (GASSASC->GetAvatarActor() == this)
		{
			PawnExtComponent->UninitializeAbilitySystem();
		}
	}

	SetActorHiddenInGame(true);
}

void AGASSCharacter::OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode)
{
	Super::OnMovementModeChanged(PrevMovementMode, PreviousCustomMode);

	UGASSCharacterMovementComponent* GASSMoveComp = CastChecked<UGASSCharacterMovementComponent>(GetCharacterMovement());

	SetMovementModeTag(PrevMovementMode, PreviousCustomMode, false);
	SetMovementModeTag(GASSMoveComp->MovementMode, GASSMoveComp->CustomMovementMode, true);
}

void AGASSCharacter::SetMovementModeTag(EMovementMode MovementMode, uint8 CustomMovementMode, bool bTagEnabled)
{
	if (UGASSAbilitySystemComponent* GASSASC = GetGASSAbilitySystemComponent())
	{
		const FGameplayTag* MovementModeTag = nullptr;
		if (MovementMode == MOVE_Custom)
		{
			MovementModeTag = GASSGameplayTags::CustomMovementModeTagMap.Find(CustomMovementMode);
		}
		else
		{
			MovementModeTag = GASSGameplayTags::MovementModeTagMap.Find(MovementMode);
		}

		if (MovementModeTag && MovementModeTag->IsValid())
		{
			GASSASC->SetLooseGameplayTagCount(*MovementModeTag, (bTagEnabled ? 1 : 0));
		}
	}
}

void AGASSCharacter::ToggleCrouch()
{
	const UGASSCharacterMovementComponent* GASSMoveComp = CastChecked<UGASSCharacterMovementComponent>(GetCharacterMovement());

	if (bIsCrouched || GASSMoveComp->bWantsToCrouch)
	{
		UnCrouch();
	}
	else if (GASSMoveComp->IsMovingOnGround())
	{
		Crouch();
	}
}

void AGASSCharacter::OnStartCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
{
	if (UGASSAbilitySystemComponent* GASSASC = GetGASSAbilitySystemComponent())
	{
		GASSASC->SetLooseGameplayTagCount(GASSGameplayTags::Status_Crouching, 1);
	}


	Super::OnStartCrouch(HalfHeightAdjust, ScaledHalfHeightAdjust);
}

void AGASSCharacter::OnEndCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
{
	if (UGASSAbilitySystemComponent* GASSASC = GetGASSAbilitySystemComponent())
	{
		GASSASC->SetLooseGameplayTagCount(GASSGameplayTags::Status_Crouching, 0);
	}

	Super::OnEndCrouch(HalfHeightAdjust, ScaledHalfHeightAdjust);
}

bool AGASSCharacter::CanJumpInternal_Implementation() const
{
	// same as ACharacter's implementation but without the crouch check
	return JumpIsAllowedInternal();
}

void AGASSCharacter::OnRep_ReplicatedAcceleration()
{
	if (UGASSCharacterMovementComponent* GASSMovementComponent = Cast<UGASSCharacterMovementComponent>(GetCharacterMovement()))
	{
		// Decompress Acceleration
		const double MaxAccel = GASSMovementComponent->MaxAcceleration;
		const double AccelXYMagnitude = double(ReplicatedAcceleration.AccelXYMagnitude) * MaxAccel / 255.0; // [0, 255] -> [0, MaxAccel]
		const double AccelXYRadians = double(ReplicatedAcceleration.AccelXYRadians) * TWO_PI / 255.0;     // [0, 255] -> [0, 2PI]

		FVector UnpackedAcceleration(FVector::ZeroVector);
		FMath::PolarToCartesian(AccelXYMagnitude, AccelXYRadians, UnpackedAcceleration.X, UnpackedAcceleration.Y);
		UnpackedAcceleration.Z = double(ReplicatedAcceleration.AccelZ) * MaxAccel / 127.0; // [-127, 127] -> [-MaxAccel, MaxAccel]

		GASSMovementComponent->SetReplicatedAcceleration(UnpackedAcceleration);
	}
}

void AGASSCharacter::SetShouldLineTrace(bool InShouldLineTrace)
{
	if (InShouldLineTrace != ShouldLineTrace)
	{
		ShouldLineTrace = InShouldLineTrace;
	}
	else
	{
		return;
	}
}

bool AGASSCharacter::GetShouldLineTrace()
{
	return ShouldLineTrace;
}

void AGASSCharacter::OnRep_InteractionLineTrace()
{
}

//void AGASSCharacter::SetGenericTeamId(const FGenericTeamId& NewTeamID)
//{
//	if (GetController() == nullptr)
//	{
//		if (HasAuthority())
//		{
//			const FGenericTeamId OldTeamID = MyTeamID;
//			MyTeamID = NewTeamID;
//			ConditionalBroadcastTeamChanged(this, OldTeamID, MyTeamID);
//		}
//		else
//		{
//			UE_LOG(LogGASSTeams, Error, TEXT("You can't set the team ID on a character (%s) except on the authority"), *GetPathNameSafe(this));
//		}
//	}
//	else
//	{
//		UE_LOG(LogGASSTeams, Error, TEXT("You can't set the team ID on a possessed character (%s); it's driven by the associated controller"), *GetPathNameSafe(this));
//	}
//}
//
//FGenericTeamId AGASSCharacter::GetGenericTeamId() const
//{
//	return MyTeamID;
//}
//
//FOnGASSTeamIndexChangedDelegate* AGASSCharacter::GetOnTeamIndexChangedDelegate()
//{
//	return &OnTeamChangedDelegate;
//}
//
//void AGASSCharacter::OnControllerChangedTeam(UObject* TeamAgent, int32 OldTeam, int32 NewTeam)
//{
//	const FGenericTeamId MyOldTeamID = MyTeamID;
//	MyTeamID = IntegerToGenericTeamId(NewTeam);
//	ConditionalBroadcastTeamChanged(this, MyOldTeamID, MyTeamID);
//}
//
//void AGASSCharacter::OnRep_MyTeamID(FGenericTeamId OldTeamID)
//{
//	ConditionalBroadcastTeamChanged(this, OldTeamID, MyTeamID);
//}

bool AGASSCharacter::UpdateSharedReplication()
{
	if (GetLocalRole() == ROLE_Authority)
	{
		FSharedRepMovement SharedMovement;
		if (SharedMovement.FillForCharacter(this))
		{
			// Only call FastSharedReplication if data has changed since the last frame.
			// Skipping this call will cause replication to reuse the same bunch that we previously
			// produced, but not send it to clients that already received. (But a new client who has not received
			// it, will get it this frame)
			if (!SharedMovement.Equals(LastSharedReplication, this))
			{
				LastSharedReplication = SharedMovement;
				ReplicatedMovementMode = SharedMovement.RepMovementMode;

				FastSharedReplication(SharedMovement);
			}
			return true;
		}
	}

	// We cannot fastrep right now. Don't send anything.
	return false;
}

void AGASSCharacter::FastSharedReplication_Implementation(const FSharedRepMovement& SharedRepMovement)
{
	if (GetWorld()->IsPlayingReplay())
	{
		return;
	}

	// Timestamp is checked to reject old moves.
	if (GetLocalRole() == ROLE_SimulatedProxy)
	{
		// Timestamp
		ReplicatedServerLastTransformUpdateTimeStamp = SharedRepMovement.RepTimeStamp;

		// Movement mode
		if (ReplicatedMovementMode != SharedRepMovement.RepMovementMode)
		{
			ReplicatedMovementMode = SharedRepMovement.RepMovementMode;
			GetCharacterMovement()->bNetworkMovementModeChanged = true;
			GetCharacterMovement()->bNetworkUpdateReceived = true;
		}

		// Location, Rotation, Velocity, etc.
		FRepMovement& MutableRepMovement = GetReplicatedMovement_Mutable();
		MutableRepMovement = SharedRepMovement.RepMovement;

		// This also sets LastRepMovement
		OnRep_ReplicatedMovement();

		// Jump force
		bProxyIsJumpForceApplied = SharedRepMovement.bProxyIsJumpForceApplied;

		// Crouch
		if (bIsCrouched != SharedRepMovement.bIsCrouched)
		{
			bIsCrouched = SharedRepMovement.bIsCrouched;
			OnRep_IsCrouched();
		}
	}
}

FSharedRepMovement::FSharedRepMovement()
{
	RepMovement.LocationQuantizationLevel = EVectorQuantization::RoundTwoDecimals;
}

bool FSharedRepMovement::FillForCharacter(ACharacter* Character)
{
	if (USceneComponent* PawnRootComponent = Character->GetRootComponent())
	{
		UCharacterMovementComponent* CharacterMovement = Character->GetCharacterMovement();

		RepMovement.Location = FRepMovement::RebaseOntoZeroOrigin(PawnRootComponent->GetComponentLocation(), Character);
		RepMovement.Rotation = PawnRootComponent->GetComponentRotation();
		RepMovement.LinearVelocity = CharacterMovement->Velocity;
		RepMovementMode = CharacterMovement->PackNetworkMovementMode();
		bProxyIsJumpForceApplied = Character->bProxyIsJumpForceApplied || (Character->JumpForceTimeRemaining > 0.0f);
		bIsCrouched = Character->bIsCrouched;

		// Timestamp is sent as zero if unused
		if ((CharacterMovement->NetworkSmoothingMode == ENetworkSmoothingMode::Linear) || CharacterMovement->bNetworkAlwaysReplicateTransformUpdateTimestamp)
		{
			RepTimeStamp = CharacterMovement->GetServerLastTransformUpdateTimeStamp();
		}
		else
		{
			RepTimeStamp = 0.f;
		}

		return true;
	}
	return false;
}

bool FSharedRepMovement::Equals(const FSharedRepMovement& Other, ACharacter* Character) const
{
	if (RepMovement.Location != Other.RepMovement.Location)
	{
		return false;
	}

	if (RepMovement.Rotation != Other.RepMovement.Rotation)
	{
		return false;
	}

	if (RepMovement.LinearVelocity != Other.RepMovement.LinearVelocity)
	{
		return false;
	}

	if (RepMovementMode != Other.RepMovementMode)
	{
		return false;
	}

	if (bProxyIsJumpForceApplied != Other.bProxyIsJumpForceApplied)
	{
		return false;
	}

	if (bIsCrouched != Other.bIsCrouched)
	{
		return false;
	}

	return true;
}

bool FSharedRepMovement::NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess)
{
	bOutSuccess = true;
	RepMovement.NetSerialize(Ar, Map, bOutSuccess);
	Ar << RepMovementMode;
	Ar << bProxyIsJumpForceApplied;
	Ar << bIsCrouched;

	// Timestamp, if non-zero.
	uint8 bHasTimeStamp = (RepTimeStamp != 0.f);
	Ar.SerializeBits(&bHasTimeStamp, 1);
	if (bHasTimeStamp)
	{
		Ar << RepTimeStamp;
	}
	else
	{
		RepTimeStamp = 0.f;
	}

	return true;
}
