// Fill out your copyright notice in the Description page of Project Settings.


#include "ThirdPersonLevelScriptActor.h"

AThirdPersonLevelScriptActor::AThirdPersonLevelScriptActor()
{
	// set pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPersonCPP/Blueprints/ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		pawnClass = PlayerPawnBPClass.Class;
	}

	static ConstructorHelpers::FClassFinder<UMyUserWidget> MyUserWidgetClass(TEXT("/Game/NewWidgetBlueprint"));	
	if (MyUserWidgetClass.Class != NULL)
	{
		myUserWidgetClass = MyUserWidgetClass.Class;
	}
	widget = nullptr;

	PrimaryActorTick.bCanEverTick = true;
}

void AThirdPersonLevelScriptActor::BeginPlay() {
	Super::BeginPlay();
	if (myUserWidgetClass != nullptr && widget == nullptr) {
		widget = CreateWidget<UMyUserWidget>(GetWorld()->GetGameInstance(), myUserWidgetClass);
		if (widget != nullptr) {
			widget->AddToViewport();
			widget->ShowImportOnly();
		}
	}

}

void AThirdPersonLevelScriptActor::Tick(float deltaSeconds) {
	if (GetWorld() == nullptr || GetWorld()->GetFirstPlayerController() == nullptr || widget == nullptr)return;
	bool bShowMouseCursor = GetWorld()->GetFirstPlayerController()->bShowMouseCursor;
	widget->SetVisibility(bShowMouseCursor ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
}