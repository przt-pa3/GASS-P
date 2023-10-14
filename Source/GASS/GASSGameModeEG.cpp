// Copyright Epic Games, Inc. All Rights Reserved.

#include "GASSGameModeEG.h"
#include "GASSCharacterEG.h"
#include "UObject/ConstructorHelpers.h"

AGASSGameModeEG::AGASSGameModeEG()
{
	// set default pawn class to our Blueprinted character		/Game/BP_GASSCharacterEG		/Game/BP_Test_GASSCharacter
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/BP_Test_GASSCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
	else {
		UE_LOG(LogTemp, Warning, TEXT("Cannot Find TPS Character"));
	}
}
