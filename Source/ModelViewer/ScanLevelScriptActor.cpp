// Fill out your copyright notice in the Description page of Project Settings.


#include "ScanLevelScriptActor.h"
#include <Runtime/Engine/Classes/GameFramework/FloatingPawnMovement.h>
#include <Runtime/Engine/Classes/GameFramework/PlayerController.h>
#include <Runtime/Engine/Classes/GameFramework/PlayerInput.h>
#include <Runtime/Engine/Classes/GameFramework/Actor.h>


AScanLevelScriptActor::AScanLevelScriptActor()
{
	static ConstructorHelpers::FClassFinder<UMyUserWidget> MyUserWidgetClass(TEXT("/Game/NewWidgetBlueprint"));
	if (MyUserWidgetClass.Class != NULL)
	{
		myUserWidgetClass = MyUserWidgetClass.Class;
	}
	widget = nullptr;
	PrimaryActorTick.bCanEverTick = true;

	customLightOffset = FVector(0, 0, 0);
}

void AScanLevelScriptActor::BeginPlay() {
	Super::BeginPlay();
	if (myUserWidgetClass != nullptr && widget == nullptr) {
		widget = CreateWidget<UMyUserWidget>(GetWorld()->GetGameInstance(), myUserWidgetClass);
		widget->GetClass();
		if (widget != nullptr) {
			widget->AddToViewport();
			widget->ShowAllAbove();
			widget->BindMesh(mesh);
			widget->BindMaterials(opaqueMaterial, transparentMaterial);
			widget->scaleOnSetCamera = scaleOnSetCamera;
			widget->ButtonLight->OnClicked.AddDynamic(this, &AScanLevelScriptActor::OnSwitchLight);
			widget->OnPlot();
		}
	}
	APlayerController* player = GetWorld()->GetFirstPlayerController();
	player->bShowMouseCursor = true;
	player->bEnableClickEvents = true;
	player->InputComponent->BindKey(FKey(TEXT("L")), EInputEvent::IE_Pressed, this, &AScanLevelScriptActor::OnSwitchLight);
	player->InputComponent->BindAxis("Turn", this, &AScanLevelScriptActor::AddControllerYawInput);
	player->InputComponent->BindAxis("LookUp", this, &AScanLevelScriptActor::AddControllerPitchInput);


	player->InputComponent->BindAxisKey(FKey(TEXT("Gamepad_RightY")), this, &AScanLevelScriptActor::OnGamepadRightY);
	//player->InputComponent->BindAxis("TurnRate", this, &AScanLevelScriptActor::TurnAtRate);
	//player->InputComponent->BindAxis("LookUpRate", this, &AScanLevelScriptActor::LookUpAtRate);

	//player->InputComponent->BindTouch(EInputEvent::IE_Pressed, this, &AScanLevelScriptActor::AddControllerTouchPressed);
	//player->InputComponent->BindTouch(EInputEvent::IE_Repeat, this, &AScanLevelScriptActor::AddControllerTouchRepeat);
	//player->InputComponent->BindTouch(EInputEvent::IE_Released, this, &AScanLevelScriptActor::AddControllerTouchReleased);
	if (player->GetPawn()->FindComponentByClass<UFloatingPawnMovement>() != nullptr) {
		player->GetPawn()->FindComponentByClass<UFloatingPawnMovement>()->MaxSpeed = walkSpeed;
	}
	if (customLight != nullptr) {
		customLightOffset = customLight->GetRootComponent()->GetRelativeLocation();
		customLightPrevPos = customLight->GetActorLocation();
		customLight->SetEnabled(false);
		customLight->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

	}

}

void AScanLevelScriptActor::Tick(float deltaSeconds) {
	//if (widget->IsFocused())return;
	APlayerController* player = GetWorld()->GetFirstPlayerController();
	if (player == nullptr)return;

	bool isSlowKeyDown = player->IsInputKeyDown(FKey(TEXT("LeftShift")));
	bool isFastKeyDown = player->IsInputKeyDown(FKey(TEXT("Tab"))) || player->IsInputKeyDown(FKey(TEXT("CapsLock"))) || player->IsInputKeyDown(FKey(TEXT("LeftAlt")));
	if (isSlowKeyDown != isSlowKeyDownPrev || isFastKeyDown != isFastKeyDownPrev) {
		isSlowKeyDownPrev = isSlowKeyDown;
		isFastKeyDownPrev = isFastKeyDown;

		
		UFloatingPawnMovement* movement = player->GetPawn()->FindComponentByClass<UFloatingPawnMovement>();
		if (movement == nullptr)return;
		if (isSlowKeyDown) {
			movement->MaxSpeed = 0.2 * walkSpeed;
		}
		else if (isFastKeyDown) {
			movement->MaxSpeed = 5 * walkSpeed;
		}
		else {
			movement->MaxSpeed = walkSpeed;
		}
	}
	
	//UE_LOG(LogTemp, Warning, TEXT("%s"), widget->IsFocused() ? TEXT("true") : TEXT("false"));
	/*
	UE_LOG(LogTemp, Warning, TEXT("%s"), *(player->RotationInput.ToString()));
	if (!isDown) {
	}*/
}

void AScanLevelScriptActor::AddControllerYawInput(float Val)
{
	APlayerController* player = GetWorld()->GetFirstPlayerController();
	if (Val != 0.f && player!=nullptr)
	{
		bool isDown = player->IsInputKeyDown(FKey(TEXT("LeftMouseButton")));
		if (isDown)player->AddYawInput(Val);
	}
}

void AScanLevelScriptActor::AddControllerPitchInput(float Val)
{
	APlayerController* player = GetWorld()->GetFirstPlayerController();
	
	if (Val != 0.f && player != nullptr)
	{
		bool isDown = player->IsInputKeyDown(FKey(TEXT("LeftMouseButton")));
		if(isDown)player->AddPitchInput(Val);
	}
}



void AScanLevelScriptActor::OnSwitchLight() {
	APlayerController* player = GetWorld()->GetFirstPlayerController();
	if (player == nullptr || player->GetPawn()==nullptr)return;

	FVector newPos = player->GetPawn()->GetTransform().TransformPosition(customLightOffset);
	customLight->SetActorLocation(newPos);
	customLight->SetActorRotation(player->GetControlRotation().Quaternion());

	if (newPos == customLightPrevPos) {
		customLight->ToggleEnabled();
	}
	else {
		customLight->SetEnabled(true);
	}
	customLightPrevPos = newPos;
}

FVector prevLocation = FVector::ZeroVector;
int touchIndex = -1;
float touchSensitivity = 0.2f;
float maxTouchMovement = 16.0f;


void AScanLevelScriptActor::AddControllerTouchPressed(const ETouchIndex::Type FingerIndex, const FVector Location) {
	APlayerController* player = GetWorld()->GetFirstPlayerController();
	if (player == nullptr) return;
	if (touchIndex == -1) {
		touchIndex = (int)FingerIndex;
		prevLocation = Location;
	}
}

void AScanLevelScriptActor::AddControllerTouchRepeat(const ETouchIndex::Type FingerIndex, const FVector Location) {
	APlayerController* player = GetWorld()->GetFirstPlayerController();
	if (player == nullptr) return;
	if (touchIndex >= 0 && touchIndex == (int)FingerIndex) {
		FVector dist = Location - prevLocation;
		if (dist.X != 0 && FMath::Abs(dist.X) <= maxTouchMovement) {
			player->AddYawInput(dist.X * touchSensitivity);
		}
		if (dist.Y != 0 && FMath::Abs(dist.Y) <= maxTouchMovement) {
			player->AddPitchInput(dist.Y * touchSensitivity);
		}
	}
	prevLocation = Location;
}

void AScanLevelScriptActor::AddControllerTouchReleased(const ETouchIndex::Type FingerIndex, const FVector Location) {
	if (touchIndex == (int)FingerIndex) {
		touchIndex = -1;
	}
}

void AScanLevelScriptActor::OnGamepadRightY(float Val) {
	if (Val != 0) {
		APlayerController* player = GetWorld()->GetFirstPlayerController();
		if (player == nullptr) return;
		player->GetPawn()->AddActorWorldOffset(FVector(0, 0, -Val));
	}
	
}