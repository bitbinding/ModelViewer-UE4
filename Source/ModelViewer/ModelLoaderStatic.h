// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include <string>
#include <vector>
#include <map>
#include <functional>
/**
 * 
 */
using ModelLoaderStaticSetCameraHandler = std::function<void(FVector position, FVector eulerAngles, float fieldOfView)>;
class ModelLoaderStatic
{
public:


	ModelLoaderStatic();
	~ModelLoaderStatic();

	static std::string ToStdStringPath(const FString& fstr);
	static FString FromStdStringPath(const std::string& str);
	static FString FromStdStringPath(const std::string& str, bool isGBK);

	static UStaticMesh* LoadSingleMesh(const std::string& modelFileStr, float angleLimit = 30.0f, bool forceAutoUV = false);
	static UStaticMesh* LoadSingleMesh(const FString& modelFileStr, float angleLimit = 30.0f, bool forceAutoUV = false);
	static UTexture2D* LoadTexture2D(const FString& FilePath);
	


	static bool ReadBinary(std::vector<unsigned char>& outData,const std::string& modelFileStr);
	static bool ReadUTF8Text(std::string& outText, const std::string& modelFileStr);


	static UStaticMesh* Read3dsData(const std::vector<unsigned char>& ulbin, float angleLimit = 30.0f, bool forceAutoUV = false);

	static UStaticMesh* ReadObjText(const std::string& str0, float angleLimit = 30.0f, bool forceAutoUV = false);

	static void ClearAttached(AActor* v0);

	static void LoadVrmlFileToActor(AActor* v0, const std::string& fileStr0, float defaultAngleLimit = 30.0f, UMaterialInterface* baseMaterial = nullptr, UMaterialInterface* transparentMaterial = nullptr, ModelLoaderStaticSetCameraHandler onSetCamera=nullptr, bool forceAutoUV=false);

	static void LoadVrmlFile(const std::string& fileStr0, std::vector<unsigned char>& ulstr0, float defaultAngleLimit = 30.0f, UStaticMesh** destSingleMesh=nullptr,AActor* v0=nullptr, UMaterialInterface* baseMaterial=nullptr, UMaterialInterface* transparentMaterial = nullptr, bool forceAutoUV = false, ModelLoaderStaticSetCameraHandler onSetCamera = nullptr, bool isInline = false);

private:
	static std::string getPrevWord(std::vector<unsigned char>& ulstr, int i0, int wordNumber = 1);

	static std::string getNextWord(std::vector<unsigned char>& ulstr, int i0, int wordNumber = 1);

	static int getNumber(std::vector<unsigned char>& ulstr, std::vector<float>& nums, int numCount, int i0);

	static int getNumberTable(std::vector<unsigned char>& ulstr, std::vector<std::vector<float>>& nums, int columnCount, bool useSeparatorM1, int i0);

	static FVector vrmlAxisRotationToEularAngles(float x0, float y0, float z0, float w0);

public:
	static std::string GetAbsolutePath(const std::string& relativePath0, const std::string& filePathNameBase0);

	static FVector AxisRotationToEularAngles(float x0, float y0, float z0, float w0);
};
