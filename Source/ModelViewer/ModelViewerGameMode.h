// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once



#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "ModelViewerGameMode.generated.h"


UCLASS(minimalapi)
class AModelViewerGameMode : public AGameModeBase
{
	GENERATED_BODY()

	FTimerHandle TestHandel;
public:
	
	AModelViewerGameMode();
	virtual void BeginPlay() override;
	void OnTimer();

};



