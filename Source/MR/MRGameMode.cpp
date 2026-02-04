// Copyright Epic Games, Inc. All Rights Reserved.

#include "MRGameMode.h"
#include "MRCharacter.h"
#include "UObject/ConstructorHelpers.h"

AMRGameMode::AMRGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
