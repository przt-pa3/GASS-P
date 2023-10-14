// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Camera/GASSCameraAssistInterface.h"
#include "CommonPlayerController.h"
//#include "Teams/GASSTeamAgentInterface.h"
#include "GASSPlayerController.generated.h"

//struct FGenericTeamId;

class AGASSHUD;
class AGASSPlayerState;
class APawn;
class APlayerState;
class FPrimitiveComponentId;
class IInputInterface;
class UGASSAbilitySystemComponent;
class UGASSEquipmentManagerComponent;
class UGASSInventoryManagerComponent;
class UGASSInventoryItemInstance;
class UGASSItemDefinition;
class UGASSSettingsShared;
class UObject;
class UPlayer;
struct FFrame;

/**
 * AGASSPlayerController
 *
 *	The base player controller class used by this project.
 */
UCLASS(Config = Game, Meta = (ShortTooltip = "The base player controller class used by this project."))
class GASS_API AGASSPlayerController : public ACommonPlayerController, public IGASSCameraAssistInterface//, public IGASSTeamAgentInterface
{
	GENERATED_BODY()

public:

	AGASSPlayerController(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	UFUNCTION(BlueprintCallable, Category = "GASS|PlayerController")
	AGASSPlayerState* GetGASSPlayerState() const;

	UFUNCTION(BlueprintCallable, Category = "GASS|PlayerController")
	UGASSAbilitySystemComponent* GetGASSAbilitySystemComponent() const;

	UFUNCTION(BlueprintCallable, Category = "GASS|PlayerController")
	UGASSInventoryManagerComponent* GetGASSInventoryManagerComponent() const;

	UFUNCTION(BlueprintCallable, Category = "GASS|PlayerController")
	AGASSHUD* GetGASSHUD() const;

	//// Run a cheat command on the server.
	//UFUNCTION(Reliable, Server, WithValidation)
	//void ServerCheat(const FString& Msg);

	//// Run a cheat command on the server for all players.
	//UFUNCTION(Reliable, Server, WithValidation)
	//void ServerCheatAll(const FString& Msg);

	//~AActor interface
	virtual void PreInitializeComponents() override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	//~End of AActor interface

	//~AController interface
	virtual void OnUnPossess() override;
	//~End of AController interface

	//~APlayerController interface
	virtual void ReceivedPlayer() override;
	virtual void PlayerTick(float DeltaTime) override;
	//~End of APlayerController interface

	//~IGASSCameraAssistInterface interface
	virtual void OnCameraPenetratingTarget() override;
	//~End of IGASSCameraAssistInterface interface

	//~ACommonPlayerController interface
	virtual void OnPossess(APawn* InPawn) override;
	//~End of ACommonPlayerController interface

	////~IGASSTeamAgentInterface interface
	//virtual void SetGenericTeamId(const FGenericTeamId& NewTeamID) override;
	//virtual FGenericTeamId GetGenericTeamId() const override;
	//virtual FOnGASSTeamIndexChangedDelegate* GetOnTeamIndexChangedDelegate() override;
	////~End of IGASSTeamAgentInterface interface

	UFUNCTION(BlueprintCallable, Category = "GASS|Character")
	void SetIsAutoRunning(const bool bEnabled);

	UFUNCTION(BlueprintCallable, Category = "GASS|Character")
	bool GetIsAutoRunning() const;

	UFUNCTION()
	const UGASSInventoryItemInstance* GetInventoryInstance(TSubclassOf<UGASSItemDefinition> InItemDef);
private:
	//UPROPERTY()
	//FOnGASSTeamIndexChangedDelegate OnTeamChangedDelegate;

	UPROPERTY()
	TObjectPtr<APlayerState> LastSeenPlayerState;

	UPROPERTY(VisibleAnywhere, Category = "GASS|PlayerController")
	TObjectPtr<UGASSInventoryManagerComponent> InventoryManagerComponent;

private:
	//UFUNCTION()
	//void OnPlayerStateChangedTeam(UObject* TeamAgent, int32 OldTeam, int32 NewTeam);

protected:
	// Called when the player state is set or cleared
	virtual void OnPlayerStateChanged();

private:
	void BroadcastOnPlayerStateChanged();

protected:
	//~AController interface
	virtual void InitPlayerState() override;
	virtual void CleanupPlayerState() override;
	virtual void OnRep_PlayerState() override;
	//~End of AController interface

	//~APlayerController interface
	virtual void SetPlayer(UPlayer* InPlayer) override;
	//virtual void AddCheats(bool bForce) override;

	virtual void UpdateForceFeedback(IInputInterface* InputInterface, const int32 ControllerId) override;
	virtual void UpdateHiddenComponents(const FVector& ViewLocation, TSet<FPrimitiveComponentId>& OutHiddenComponents) override;

	virtual void PreProcessInput(const float DeltaTime, const bool bGamePaused) override;
	virtual void PostProcessInput(const float DeltaTime, const bool bGamePaused) override;
	//~End of APlayerController interface

	void OnSettingsChanged(UGASSSettingsShared* Settings);

	void OnStartAutoRun();
	void OnEndAutoRun();

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "OnStartAutoRun"))
	void K2_OnStartAutoRun();

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "OnEndAutoRun"))
	void K2_OnEndAutoRun();

	bool bHideViewTargetPawnNextFrame = false;

	

};


// A player controller used for replay capture and playback
UCLASS()
class AGASSReplayPlayerController : public AGASSPlayerController
{
	GENERATED_BODY()

	virtual void SetPlayer(UPlayer* InPlayer) override;
};
