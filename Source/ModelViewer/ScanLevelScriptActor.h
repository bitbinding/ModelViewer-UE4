// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include <ModelViewer/MyUserWidget.h>
#include <Runtime/Engine/Classes/Engine/PointLight.h>
#include "CoreMinimal.h"
#include "Engine/LevelScriptActor.h"
#include "ScanLevelScriptActor.generated.h"




/**
 * 
 */
UCLASS()
class MODELVIEWER_API AScanLevelScriptActor : public ALevelScriptActor
{
	GENERATED_BODY()
	
	TSubclassOf<UMyUserWidget> myUserWidgetClass;
	UMyUserWidget* widget;
	bool isSlowKeyDownPrev = false;
	bool isFastKeyDownPrev = false;
	
public:
	UPROPERTY(EditAnyWhere, BluePrintReadWrite)
	AActor* mesh;

	UPROPERTY(EditAnyWhere, BluePrintReadWrite)
	UMaterialInterface* opaqueMaterial;

	UPROPERTY(EditAnyWhere, BluePrintReadWrite)
	UMaterialInterface* transparentMaterial;

	UPROPERTY(EditAnyWhere, BluePrintReadWrite)
	ALight* customLight;

	UPROPERTY(EditAnyWhere, BluePrintReadWrite)
	float scaleOnSetCamera=1;

	UPROPERTY(EditAnyWhere, BluePrintReadWrite)
	float walkSpeed = 600.0f;

	FVector customLightOffset;
	FVector customLightPrevPos;

	AScanLevelScriptActor();
	virtual void BeginPlay() override;


	virtual void Tick(float deltaSeconds) override;



	void AddControllerYawInput(float Val);
	void AddControllerPitchInput(float Val);

	UFUNCTION()
	void OnSwitchLight();

	void AddControllerTouchPressed(const ETouchIndex::Type FingerIndex, const FVector Location);
	void AddControllerTouchRepeat(const ETouchIndex::Type FingerIndex, const FVector Location);
	void AddControllerTouchReleased(const ETouchIndex::Type FingerIndex, const FVector Location);

	void OnGamepadRightY(float Val);
};
