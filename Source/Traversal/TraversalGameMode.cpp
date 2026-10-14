// Copyright Epic Games, Inc. All Rights Reserved.

#include "TraversalGameMode.h"
#include "TraversalCharacter.h"
#include "UObject/ConstructorHelpers.h"

ATraversalGameMode::ATraversalGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
