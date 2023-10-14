// Copyright Epic Games, Inc. All Rights Reserved.

#include "Character/GASSPawn.h"

#include "GameFramework/Controller.h"
#include "GASS/GASSLogChannels.h"
#include "Net/UnrealNetwork.h"
#include "UObject/ScriptInterface.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GASSPawn)

class FLifetimeProperty;
class UObject;

AGASSPawn::AGASSPawn(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	
}

void AGASSPawn::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	//DOREPLIFETIME(ThisClass, MyTeamID);
}

void AGASSPawn::PreInitializeComponents()
{
	Super::PreInitializeComponents();
}

void AGASSPawn::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
}

void AGASSPawn::PossessedBy(AController* NewController)
{
	//const FGenericTeamId OldTeamID = MyTeamID;

	Super::PossessedBy(NewController);

	//// Grab the current team ID and listen for future changes
	//if (IGASSTeamAgentInterface* ControllerAsTeamProvider = Cast<IGASSTeamAgentInterface>(NewController))
	//{
	//	MyTeamID = ControllerAsTeamProvider->GetGenericTeamId();
	//	ControllerAsTeamProvider->GetTeamChangedDelegateChecked().AddDynamic(this, &ThisClass::OnControllerChangedTeam);
	//}
	//ConditionalBroadcastTeamChanged(this, OldTeamID, MyTeamID);
}

void AGASSPawn::UnPossessed()
{
	AController* const OldController = Controller;

	//// Stop listening for changes from the old controller
	//const FGenericTeamId OldTeamID = MyTeamID;
	//if (IGASSTeamAgentInterface* ControllerAsTeamProvider = Cast<IGASSTeamAgentInterface>(OldController))
	//{
	//	ControllerAsTeamProvider->GetTeamChangedDelegateChecked().RemoveAll(this);
	//}

	Super::UnPossessed();

	//// Determine what the new team ID should be afterwards
	//MyTeamID = DetermineNewTeamAfterPossessionEnds(OldTeamID);
	//ConditionalBroadcastTeamChanged(this, OldTeamID, MyTeamID);
}

//void AGASSPawn::SetGenericTeamId(const FGenericTeamId& NewTeamID)
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
//			UE_LOG(LogGASSTeams, Error, TEXT("You can't set the team ID on a pawn (%s) except on the authority"), *GetPathNameSafe(this));
//		}
//	}
//	else
//	{
//		UE_LOG(LogGASSTeams, Error, TEXT("You can't set the team ID on a possessed pawn (%s); it's driven by the associated controller"), *GetPathNameSafe(this));
//	}
//}
//
//FGenericTeamId AGASSPawn::GetGenericTeamId() const
//{
//	return MyTeamID;
//}
//
//FOnGASSTeamIndexChangedDelegate* AGASSPawn::GetOnTeamIndexChangedDelegate()
//{
//	return &OnTeamChangedDelegate;
//}
//
//void AGASSPawn::OnControllerChangedTeam(UObject* TeamAgent, int32 OldTeam, int32 NewTeam)
//{
//	const FGenericTeamId MyOldTeamID = MyTeamID;
//	MyTeamID = IntegerToGenericTeamId(NewTeam);
//	ConditionalBroadcastTeamChanged(this, MyOldTeamID, MyTeamID);
//}
//
//void AGASSPawn::OnRep_MyTeamID(FGenericTeamId OldTeamID)
//{
//	ConditionalBroadcastTeamChanged(this, OldTeamID, MyTeamID);
//}

