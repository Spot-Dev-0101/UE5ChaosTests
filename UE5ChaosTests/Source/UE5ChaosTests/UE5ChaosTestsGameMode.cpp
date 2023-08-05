// Copyright Epic Games, Inc. All Rights Reserved.

#include "UE5ChaosTestsGameMode.h"
#include "UE5ChaosTestsCharacter.h"
#include "UObject/ConstructorHelpers.h"

AUE5ChaosTestsGameMode::AUE5ChaosTestsGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
