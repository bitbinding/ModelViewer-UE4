// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include <vector>
#include <string>
#include "CoreMinimal.h"
#include "Model3d.h"
#include "Blueprint/UserWidget.h"
#include <Runtime/UMG/Public/Components/Button.h>
#include <Runtime/UMG/Public/Components/EditableTextBox.h>
#include <Runtime/UMG/Public/Components/TextBlock.h>
#include <Runtime/UMG/Public/Components/ListView.h>
#include <ProceduralMeshComponent.h>
#include "MyUserWidget.generated.h"


/**
 * 
 */
UCLASS(Blueprintable)
class MODELVIEWER_API UMyUserWidget : public UUserWidget
{
	GENERATED_BODY()

	virtual bool Initialize() override;
	UProceduralMeshComponent* m_meshComponent;
	FString prevFuncName;
	UModel3d* model3d;
	UMaterialInterface* opaqueMaterial;
	UMaterialInterface* transparentMaterial;

	FVector initialLocation;
	FRotator initialRotation;
	bool hasInitialPawnData = false;
	FVector initialRootScale;
public:
	float scaleOnSetCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (BindWidget))
		UEditableTextBox* TextFunction;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (BindWidget))
		UButton* ButtonPlot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (BindWidget))
		UButton* ButtonImport;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (BindWidget))
		UButton* ButtonLight;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (BindWidget))
		UTextBlock* TextBlock_Plot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (BindWidget))
		UTextBlock* TextBlock_Import;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (BindWidget))
		UTextBlock* TextDebug;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (BindWidget))
		UListView* ListView_FileBrowser;




	UFUNCTION(BlueprintCallable)
	void ShowAllAbove();

	UFUNCTION(BlueprintCallable)
	void ShowImportOnly();

	UFUNCTION(BlueprintCallable)
	void BindMesh(AActor* mesh);

	UFUNCTION(BlueprintCallable)
	void BindMaterials(UMaterialInterface* opaqueMaterial0, UMaterialInterface* transparentMaterial0);

	UFUNCTION(BlueprintCallable)
	void OnPlot();

	UFUNCTION(BlueprintCallable)
	void OnImport();

	UFUNCTION(BlueprintCallable)
	void SelectFileToListView(const FString& extNames0);
	UFUNCTION(BlueprintCallable)
	void CloseListView();

	void UpdateFileList();

	//called by blueprint
	UFUNCTION(BlueprintCallable)
	void OnSelectListItem(const FString& itemText);

	

	void ImportUrl(const FString& path);

	UFUNCTION(BlueprintCallable)
	bool IsFocused();
	
	static void OpenFileDialog(const FString& DialogTitle, const FString& FileTypes, TArray<FString>& OutFileNames);
	void ResetCamera();
	void SetCamera(FVector position, FVector eulerAngles, float fieldOfView);
};
