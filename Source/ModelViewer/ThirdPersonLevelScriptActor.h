// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include <ModelViewer/MyUserWidget.h>

#include "CoreMinimal.h"
#include "Engine/LevelScriptActor.h"
#include "ThirdPersonLevelScriptActor.generated.h"

/**
 * 
 */
UCLASS()
class MODELVIEWER_API AThirdPersonLevelScriptActor : public ALevelScriptActor
{
	GENERATED_BODY()
	
	TSubclassOf<UMyUserWidget> myUserWidgetClass;
	TSubclassOf<APawn> pawnClass;
	UMyUserWidget* widget;

public:
	UPROPERTY(EditAnyWhere, BluePrintReadWrite)
		AActor* mesh;

	UPROPERTY(EditAnyWhere, BluePrintReadWrite)
		UMaterialInterface* opaqueMaterial;

	UPROPERTY(EditAnyWhere, BluePrintReadWrite)
		UMaterialInterface* transparentMaterial;

	AThirdPersonLevelScriptActor();
	virtual void BeginPlay() override;
	virtual void Tick(float deltaSeconds) override;
};
