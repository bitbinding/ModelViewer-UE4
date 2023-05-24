// Copyright Epic Games, Inc. All Rights Reserved.

#include "ModelViewerGameMode.h"
#include "ModelViewerCharacter.h"
#include "UObject/ConstructorHelpers.h"



AModelViewerGameMode::AModelViewerGameMode()
{
	// set default pawn class to our Blueprinted character
	/*static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPersonCPP/Blueprints/ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}*/
}

void AModelViewerGameMode::BeginPlay(){
	//GetWorld()->GetTimerManager().SetTimer(TestHandel, this, &AModelViewerGameMode::OnTimer, 0.1f, true);
}

void AModelViewerGameMode::OnTimer(){
}

