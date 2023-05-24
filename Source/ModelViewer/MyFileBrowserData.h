// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "MyFileBrowserData.generated.h"

/**
 * 
 */
UCLASS(Blueprintable)
class MODELVIEWER_API UMyFileBrowserData : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite)
	FString text;
};
