// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "AbilitySystemInterface.h"
#include "GameplayCueInterface.h"
#include "GameplayTagAssetInterface.h"
#include "ModularCharacter.h"
//#include "Teams/GASSTeamAgentInterface.h"

#include "GASSCharacter.generated.h"

class AActor;
class AController;
class AGASSPlayerController;
class AGASSPlayerState;
class FLifetimeProperty;
class IRepChangedPropertyTracker;
class UAbilitySystemComponent;
class UInputComponent;
class UGASSAbilitySystemComponent;
class UGASSInventoryManagerComponent;
class UGASSEquipmentManagerComponent;
class UGASSCameraComponent;
class UGASSHealthComponent;
class UGASSPawnExtensionComponent;
class UObject;
struct FFrame;
struct FGameplayTag;
struct FGameplayTagContainer;


/**
 * FGASSReplicatedAcceleration: Compressed representation of acceleration
 */
USTRUCT()
struct FGASSReplicatedAcceleration
{
	GENERATED_BODY()

	UPROPERTY()
	uint8 AccelXYRadians = 0;	// Direction of XY accel component, quantized to represent [0, 2*pi]

	UPROPERTY()
	uint8 AccelXYMagnitude = 0;	//Accel rate of XY component, quantized to represent [0, MaxAcceleration]

	UPROPERTY()
	int8 AccelZ = 0;	// Raw Z accel rate component, quantized to represent [-MaxAcceleration, MaxAcceleration]
};

/** The type we use to send FastShared movement updates. */
USTRUCT()
struct FSharedRepMovement
{
	GENERATED_BODY()

	FSharedRepMovement();

	bool FillForCharacter(ACharacter* Character);
	bool Equals(const FSharedRepMovement& Other, ACharacter* Character) const;

	bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess);

	UPROPERTY(Transient)
	FRepMovement RepMovement;

	UPROPERTY(Transient)
	float RepTimeStamp = 0.0f;

	UPROPERTY(Transient)
	uint8 RepMovementMode = 0;

	UPROPERTY(Transient)
	bool bProxyIsJumpForceApplied = false;

	UPROPERTY(Transient)
	bool bIsCrouched = false;
};

template<>
struct TStructOpsTypeTraits<FSharedRepMovement> : public TStructOpsTypeTraitsBase2<FSharedRepMovement>
{
	enum
	{
		WithNetSerializer = true,
		WithNetSharedSerialization = true,
	};
};

/**
 * AGASSCharacter
 *
 *	The base character pawn class used by this project.
 *	Responsible for sending events to pawn components.
 *	New behavior should be added via pawn components when possible.
 */
UCLASS(Config = Game, Meta = (ShortTooltip = "The base character pawn class used by this project."))
class GASS_API AGASSCharacter : public AModularCharacter, public IAbilitySystemInterface, public IGameplayCueInterface, public IGameplayTagAssetInterface//, public IGASSTeamAgentInterface
{
	GENERATED_BODY()

public:

	AGASSCharacter(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	UFUNCTION(BlueprintCallable, Category = "GASS|Character")
	AGASSPlayerController* GetGASSPlayerController() const;

	UFUNCTION(BlueprintCallable, Category = "GASS|Character")
	AGASSPlayerState* GetGASSPlayerState() const;

	UFUNCTION(BlueprintCallable, Category = "GASS|Character")
	UGASSAbilitySystemComponent* GetGASSAbilitySystemComponent() const;
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	UFUNCTION(BlueprintCallable, Category = "GASS|Character")
	UGASSInventoryManagerComponent* GetGASSInventoryManagerComponent() const;

	UFUNCTION(BlueprintCallable, Category = "GASS|Character")
	UGASSEquipmentManagerComponent* GetGASSEquipmentManagerComponent() const;

	virtual void GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const override;
	virtual bool HasMatchingGameplayTag(FGameplayTag TagToCheck) const override;
	virtual bool HasAllMatchingGameplayTags(const FGameplayTagContainer& TagContainer) const override;
	virtual bool HasAnyMatchingGameplayTags(const FGameplayTagContainer& TagContainer) const override;

	void ToggleCrouch();

	//~AActor interface
	virtual void PreInitializeComponents() override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void Reset() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PreReplication(IRepChangedPropertyTracker& ChangedPropertyTracker) override;
	//~End of AActor interface

	//~APawn interface
	virtual void NotifyControllerChanged() override;
	//~End of APawn interface

	////~IGASSTeamAgentInterface interface
	//virtual void SetGenericTeamId(const FGenericTeamId& NewTeamID) override;
	//virtual FGenericTeamId GetGenericTeamId() const override;
	//virtual FOnGASSTeamIndexChangedDelegate* GetOnTeamIndexChangedDelegate() override;
	////~End of IGASSTeamAgentInterface interface

	UFUNCTION()
	void SetShouldLineTrace(bool InShouldLineTrace);

	UFUNCTION()
	bool GetShouldLineTrace();

	/** RPCs that is called on frames when default property replication is skipped. This replicates a single movement update to everyone. */
	UFUNCTION(NetMulticast, unreliable)
	void FastSharedReplication(const FSharedRepMovement& SharedRepMovement);

	// Last FSharedRepMovement we sent, to avoid sending repeatedly.
	FSharedRepMovement LastSharedReplication;

	virtual bool UpdateSharedReplication();

protected:

	virtual void OnAbilitySystemInitialized();
	virtual void OnAbilitySystemUninitialized();

	virtual void PossessedBy(AController* NewController) override;
	virtual void UnPossessed() override;

	virtual void OnRep_Controller() override;
	virtual void OnRep_PlayerState() override;

	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	void InitializeGameplayTags();

	virtual void FellOutOfWorld(const class UDamageType& dmgType) override;

	// Begins the death sequence for the character (disables collision, disables movement, etc...)
	UFUNCTION()
	virtual void OnDeathStarted(AActor* OwningActor);

	// Ends the death sequence for the character (detaches controller, destroys pawn, etc...)
	UFUNCTION()
	virtual void OnDeathFinished(AActor* OwningActor);

	void DisableMovementAndCollision();
	void DestroyDueToDeath();
	void UninitAndDestroy();

	// Called when the death sequence for the character has completed
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "OnDeathFinished"))
	void K2_OnDeathFinished();

	virtual void OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode) override;
	void SetMovementModeTag(EMovementMode MovementMode, uint8 CustomMovementMode, bool bTagEnabled);

	virtual void OnStartCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override;
	virtual void OnEndCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override;

	virtual bool CanJumpInternal_Implementation() const;

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GASS|Character", Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UGASSPawnExtensionComponent> PawnExtComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GASS|Character", Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UGASSHealthComponent> HealthComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GASS|Character", Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UGASSCameraComponent> CameraComponent;

	UPROPERTY(Replicated)
	TObjectPtr<UGASSEquipmentManagerComponent> EquipmentManagerComponent;

	UPROPERTY(Transient, ReplicatedUsing = OnRep_ReplicatedAcceleration)
	FGASSReplicatedAcceleration ReplicatedAcceleration;

	UPROPERTY(ReplicatedUsing = OnRep_InteractionLineTrace)
	bool ShouldLineTrace = false;
	//UPROPERTY(ReplicatedUsing = OnRep_MyTeamID)
	//FGenericTeamId MyTeamID;

	//UPROPERTY()
	//FOnGASSTeamIndexChangedDelegate OnTeamChangedDelegate;

protected:
	//// Called to determine what happens to the team ID when possession ends
	//virtual FGenericTeamId DetermineNewTeamAfterPossessionEnds(FGenericTeamId OldTeamID) const
	//{
	//	// This could be changed to return, e.g., OldTeamID if you want to keep it assigned afterwards, or return an ID for some neutral faction, or etc...
	//	return FGenericTeamId::NoTeam;
	//}

private:
	//UFUNCTION()
	//void OnControllerChangedTeam(UObject* TeamAgent, int32 OldTeam, int32 NewTeam);

	UFUNCTION()
	void OnRep_ReplicatedAcceleration();

	UFUNCTION()
	void OnRep_InteractionLineTrace();

	//UFUNCTION()
	//void OnRep_MyTeamID(FGenericTeamId OldTeamID);
};
