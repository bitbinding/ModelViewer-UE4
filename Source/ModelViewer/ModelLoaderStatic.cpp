// Fill out your copyright notice in the Description page of Project Settings.


#include "ModelLoaderStatic.h"
#include "RawMesh/Public/RawMesh.h"
#include "ue_import/commonutils.h"
#include "MeshUtil.h"
#include "Model3d.h"
#include "EncodeTransfer.h"
#include <Runtime/Engine/Classes/Engine/StaticMeshActor.h>
#include <Runtime/ImageWrapper/Public/IImageWrapper.h>
#include <Runtime/ImageWrapper/Public/IImageWrapperModule.h>

#ifdef _WIN32
#include "io.h"
#include "direct.h"
#else
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#define _access(a,b) (access((a),(b)))
#define _mkdir(a) (mkdir((a),0755))
#endif




// 模型数据与unity的区别：
// 
// 法向量与贴图坐标在unity里与每个顶点一一对应，在虚幻里与每个三角面的拐角一一对应
// 
// 顶点坐标：
// 虚幻的xyz对应unity原先的zxy
// unity的xyz对应虚幻的yzx
//
// 顶点索引：
// 虚幻正面对应顶点环绕方向与unity的相反
// 
// 贴图坐标：
// 虚幻纵向贴图坐标从图片的左上角开始，unity纵向贴图坐标从图片的左下角开始


ModelLoaderStatic::ModelLoaderStatic()
{
	
}

ModelLoaderStatic::~ModelLoaderStatic()
{

}

std::string ModelLoaderStatic::ToStdStringPath(const FString& fstr) {
#ifdef _WIN32
	//return TCHAR_TO_ANSI(*fstr);
	return EncodeTransfer::UTF8ToGBK(TCHAR_TO_UTF8(*fstr));
#else
	return TCHAR_TO_UTF8(*fstr);
#endif
}
FString ModelLoaderStatic::FromStdStringPath(const std::string& str) {
#ifdef _WIN32
	//return ANSI_TO_TCHAR(str.c_str());
	return UTF8_TO_TCHAR(EncodeTransfer::GBKToUTF8(str).c_str());
#else
	return UTF8_TO_TCHAR(str.c_str());
#endif
}
FString ModelLoaderStatic::FromStdStringPath(const std::string& str, bool isGBK) {
	if (isGBK) {
		return UTF8_TO_TCHAR(EncodeTransfer::GBKToUTF8(str).c_str());
	}
	else {
		return UTF8_TO_TCHAR(str.c_str());
	}

}

UStaticMesh* ModelLoaderStatic::LoadSingleMesh(const FString& modelFileStr, float angleLimit, bool forceAutoUV) {
	return LoadSingleMesh(ToStdStringPath(modelFileStr), angleLimit, forceAutoUV);
}

UStaticMesh* ModelLoaderStatic::LoadSingleMesh(const std::string& modelFileStr, float angleLimit, bool forceAutoUV) {
    int i = 0;
    std::string fext = substring(modelFileStr, (int)(modelFileStr.rfind(".")) + 1);
    std::vector<unsigned char> data;
    std::string text;
    if (stringLowerCase(fext) == "3ds")
    {
        if (!ReadBinary(data, modelFileStr))return nullptr;
        return Read3dsData(data, angleLimit, forceAutoUV);
    }
    else if (stringLowerCase(fext) == "obj")
    {
        if (!ReadUTF8Text(text, modelFileStr))return nullptr;
		//UE_LOG(LogTemp, Log, TEXT("%s"),text.c_str());
        return ReadObjText(text, angleLimit, forceAutoUV);
    }
    else if (stringLowerCase(fext) == "wr" || stringLowerCase(fext) == "wrz")
    {
		if (!ReadBinary(data, modelFileStr))return nullptr;
		UStaticMesh* mesh=nullptr;
		LoadVrmlFile(modelFileStr, data, angleLimit, &mesh);
        return mesh;
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("该模型文件不支持，仅支持3ds,wrl,obj格式文件"));
        return nullptr;
    }
}

bool ModelLoaderStatic::ReadBinary(std::vector<unsigned char>& outData, const std::string& modelFileStr) {
    FILE* fp = fopen(modelFileStr.c_str(), "rb");
    if (fp == nullptr)return false;
    fseek(fp, 0, SEEK_END);
    long length=ftell(fp);
    outData.resize(length);
    fseek(fp, 0, SEEK_SET);
    fread(&outData[0], 1, length, fp);
    fclose(fp);
    return true;
}


bool ModelLoaderStatic::ReadUTF8Text(std::string& outText, const std::string& modelFileStr) {
    FILE* fp = fopen(modelFileStr.c_str(), "r");
    if (fp == nullptr)return false;
    fseek(fp, 0, SEEK_END);
    long length = ftell(fp);
    fseek(fp, 0, SEEK_SET);


    outText.reserve(length+1);
    outText.resize(0);
    unsigned char c = 0;
	unsigned short tempVal = 0;
    while (!feof(fp)) {
		c=fgetc(fp);
		outText += (char)c;
    }
	outText.shrink_to_fit();
    return true;
}


UTexture2D* ModelLoaderStatic::LoadTexture2D(const FString& FilePath)
{

	IImageWrapperModule& ImageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>(FName("ImageWrapper"));
	int dotIndex = 0;
	if (!FilePath.FindLastChar(TCHAR('.'), dotIndex)) {
		dotIndex = -1;
	}
	FString fext = FilePath.Mid(dotIndex + 1).ToLower();
	IImageWrapperPtr ImageWrapper = nullptr;
	if (fext == TEXT("png")) {
		ImageWrapper = ImageWrapperModule.CreateImageWrapper(EImageFormat::PNG);
	}
	else if (fext == TEXT("jpg")) {
		ImageWrapper = ImageWrapperModule.CreateImageWrapper(EImageFormat::JPEG);
	}
	else if (fext == TEXT("bmp")) {
		ImageWrapper = ImageWrapperModule.CreateImageWrapper(EImageFormat::BMP);
	}
	if (ImageWrapper == nullptr) {
		return nullptr;
	}
	if (!FPlatformFileManager::Get().GetPlatformFile().FileExists(*FilePath))
	{
		return nullptr;
	}
	TArray<uint8> RawFileData;
	UTexture2D* MyTexture = nullptr;
	if (FFileHelper::LoadFileToArray(RawFileData, *FilePath))
	{
		
		if (ImageWrapper.IsValid() && ImageWrapper->SetCompressed(RawFileData.GetData(), RawFileData.Num()))
		{
			TArray<uint8> UncompressedBGRA;
			if (ImageWrapper->GetRaw(ERGBFormat::BGRA, 8, UncompressedBGRA))
			{
				// Create the UTexture for rendering
				MyTexture = UTexture2D::CreateTransient(ImageWrapper->GetWidth(), ImageWrapper->GetHeight(), PF_B8G8R8A8);
				if (MyTexture != nullptr)
				{
					//IsValid = true;
					//OutWidth = ImageWrapper->GetWidth();
					//OutHeight = ImageWrapper->GetHeight();
					// Fill in the source data from the file
					void* TextureData = MyTexture->PlatformData->Mips[0].BulkData.Lock(LOCK_READ_WRITE);
					FMemory::Memcpy(TextureData, UncompressedBGRA.GetData(), UncompressedBGRA.Num());
					MyTexture->PlatformData->Mips[0].BulkData.Unlock();

					// Update the rendering resource from data.
					MyTexture->UpdateResource();
				}
			}
		}
	}
	return MyTexture;
}


UStaticMesh* ModelLoaderStatic::Read3dsData(const std::vector<unsigned char>& ulbin, float angleLimit, bool forceAutoUV)
{
    std::vector<unsigned char> floatbin; //浮点数的暂存空间
	floatbin.resize(4);
    int leng = ulbin.size();
    int leng2 = 0; //顶点数
    int leng3 = 0; //面数
    if (leng < 6 || ulbin[0] != 0x4d || ulbin[1] != 0x4d || (ulbin[5] << 24 | ulbin[4] << 16 | ulbin[3] << 8 | ulbin[2]) != leng)
    {
        UE_LOG(LogTemp, Warning, TEXT("该文件不是3ds文件"));
        return nullptr;
    }
    int readFlag = -3;
    FRawMesh rawMesh;
    TArray<FVector>& point = rawMesh.VertexPositions;
    TArray<uint32>& indices = rawMesh.WedgeIndices;
    TArray<FVector2D> uv;

    int i = 0;
    int j = 0;
    int k = 0;
    int l = 0;
    float pointMax = 0.1f;
    float avax = 0;
    float avay = 0;

	for (i = 0; i + 8 < leng; i++) {
		if (readFlag == -3 && ulbin[i] == 0x3d && ulbin[i + 1] == 0x3d) {
			readFlag = -2;//主编辑块
		}
		if (readFlag == -2 && ulbin[i] == 0x00 && ulbin[i + 1] == 0x40) {
			readFlag = -1;//物体块
		}
		if (readFlag == -1 && ulbin[i] == 0x00 && ulbin[i + 1] == 0x41) {
			readFlag = 0;//网格块
		}
		if (readFlag == 0 && ulbin[i] == 0x10 && ulbin[i + 1] == 0x41) {
			//顶点列表块
			i += 6;
			readFlag = 1;
			leng2 = (ulbin[i + 1] << 8 | ulbin[i]);
			point.SetNumZeroed(leng2);
			//pointt=new Array(leng2);
			i += 2;
			j = 0;
			k = 0;

		}
		if (readFlag == 1) {
			//读取顶点列表
			floatbin[0] = ulbin[i];
			floatbin[1] = ulbin[i + 1];
			floatbin[2] = ulbin[i + 2];
			floatbin[3] = ulbin[i + 3];
			if (j < point.Num()) {
				if (k == 0) {
					point[j].Y = *(float*)(&floatbin[0]);
				}
				else if (k == 1) {
					point[j].X = *(float*)(&floatbin[0]);
				}
				else if (k == 2) {
					point[j].Z = *(float*)(&floatbin[0]);
				}
			}
			k++;
			if (k >= 3) {
				k = 0;
				j++;
			}
			if (j >= leng2) {
				readFlag = 2;
			}
			i += 3;
		}
		if (readFlag == 2) {
			//面信息列表块
			if (ulbin[i] == 0x11 && ulbin[i + 1] == 0x41) {

			}
			else if (ulbin[i] == 0x40 && ulbin[i + 1] == 0x41) {
				i += 6;
				readFlag = 4;
				uv.SetNumZeroed(leng2);
				i += 2;
				j = 0;
				k = 0;
			}
			else if (ulbin[i] == 0x70 && ulbin[i + 1] == 0x41) {

			}
			else if (ulbin[i] == 0x20 && ulbin[i + 1] == 0x41) {
				i += 6;
				readFlag = 3;
				leng3 = (ulbin[i + 1] << 8 | ulbin[i]);
				indices.SetNumZeroed(3 * leng3);
				i += 2;
				j = 0;
				k = 0;
			}
		}
		if (readFlag == 3) {
			//读取面信息列表
			if (j * 3 + 2 < indices.Num()) {
				indices[j * 3] = ulbin[i + 1] << 8 | ulbin[i];
				indices[j * 3 + 1] = ulbin[i + 3] << 8 | ulbin[i + 2];
				indices[j * 3 + 2] = ulbin[i + 5] << 8 | ulbin[i + 4];
			}
			j++;
			if (j >= leng3) {
				break;
			}
			i += 7;
		}
		if (readFlag == 4) {
			if (j < uv.Num()) {
				floatbin[0] = ulbin[i];
				floatbin[1] = ulbin[i + 1];
				floatbin[2] = ulbin[i + 2];
				floatbin[3] = ulbin[i + 3];
				uv[j].X = *(float*)(&floatbin[0]);
				i += 4;
				floatbin[0] = ulbin[i];
				floatbin[1] = ulbin[i + 1];
				floatbin[2] = ulbin[i + 2];
				floatbin[3] = ulbin[i + 3];
				//uv[j].Y = 1 - *(float*)(&floatbin[0]);
				uv[j].Y = *(float*)(&floatbin[0]);
			}
			
			j++;
			if (j >= leng2) {
				readFlag = 2;
			}
			i += 3;
		}
	}
	MeshUtil::CreateNormal(rawMesh, angleLimit);
	if (uv.Num() > 0 && !forceAutoUV && !MeshUtil::SameUV(uv)) {
		MeshUtil::SetUV(rawMesh,uv,TArray<uint32>());
	}
	else {
		MeshUtil::AutoUV(rawMesh);
	}
	

    return MeshUtil::GenerateUStaticMesh(rawMesh);
}

UStaticMesh* ModelLoaderStatic::ReadObjText(const std::string& str0, float angleLimit, bool forceAutoUV)
{
	std::vector<std::string> arr;
	splitString(arr, str0, "\n");
	if (arr.size() == 0) {
		splitString(arr, str0, "\r");
	}
	FRawMesh rawMesh;
	TArray<FVector>& point = rawMesh.VertexPositions;
	TArray<uint32>& indices = rawMesh.WedgeIndices;
	TArray<FVector2D> uv;
	TArray<uint32> indicesTex;
	int leng = arr.size();
	int i;
	int j;
	int k;

	std::vector<std::string> arrt;
	std::vector<std::string> arrt2;
	TArray<uint32> indexi;
	TArray<uint32> index2i;

	TArray<FVector> pointn;
	TArray<uint32> nindex;
	TArray<uint32> indexni;

	for (i = 0; i < leng; i++) {
		if (arr[i].size() < 3) {
			continue;
		}
		if (arr[i][0] == 'v' && arr[i][1] == ' ') {
			splitString(arrt, arr[i], " ");
			if (arrt.size() >= 4) {
				point.Push(FVector(-(float)atof(arrt[3].c_str()), (float)atof(arrt[1].c_str()), (float)atof(arrt[2].c_str())));
			}
		}
		if (arr[i][0] == 'v' && arr[i][1] == 't' && arr[i][2] == ' ') {
			splitString(arrt, arr[i], " ");
			if (arrt.size() >= 3) {
				//uv.Push(FVector2D((float)atof(arrt[1].c_str()), 1 - (float)atof(arrt[2].c_str())));
				uv.Push(FVector2D((float)atof(arrt[1].c_str()), (float)atof(arrt[2].c_str())));
			}
		}
		if (arr[i][0] == 'v' && arr[i][1] == 'n' && arr[i][2] == ' ') {
			splitString(arrt, arr[i], " ");
			if (arrt.size() >= 4) {
				pointn.Push(FVector(-(float)atof(arrt[3].c_str()), (float)atof(arrt[1].c_str()), (float)atof(arrt[2].c_str())));
			}
		}
		if (arr[i][0] == 'f' && arr[i][1] == ' ') {
			splitString(arrt, arr[i], " ");
			if (arrt.size() < 3) {
				continue;
			}
			indexi.SetNum(0);
			index2i.SetNum(0);
			indexni.SetNum(0);

			for (j = 1; j < arrt.size(); j++) {
				splitString(arrt2, arrt[j], "/");
				if (arrt2.size() >= 1) {
					indexi.Push(atoi(arrt2[0].c_str()) - 1);
				}
				if (arrt2.size() >= 2) {
					index2i.Push(atoi(arrt2[1].c_str()) - 1);
				}
				if (arrt2.size() >= 3) {
					indexni.Push(atoi(arrt2[2].c_str()) - 1);
				}
			}

			for (k = indexi.Num() - 1; k >= 2; k--) {
				indices.Push(indexi[0]);
				indices.Push(indexi[k-1]);
				indices.Push(indexi[k]);
			}
			for (k = index2i.Num() - 1; k >= 2; k--) {
				indicesTex.Push(index2i[0]);
				indicesTex.Push(index2i[k-1]);
				indicesTex.Push(index2i[k]);
			}
			for (k = indexni.Num() - 1; k >= 2; k--) {
				nindex.Push(indexni[0]);
				nindex.Push(indexni[k-1]);
				nindex.Push(indexni[k]);
			}
		}
	}
	if (nindex.Num() > 0 && pointn.Num() > 0) {
		MeshUtil::AttachExternalNormalData(rawMesh, pointn, nindex);
	}
	if (rawMesh.WedgeTangentZ.Num() != rawMesh.WedgeIndices.Num()) {
		MeshUtil::CreateNormal(rawMesh, angleLimit);
	}
	if (uv.Num() > 0 && !forceAutoUV) {
		MeshUtil::SetUV(rawMesh, uv, indicesTex);
	}
	else {
		MeshUtil::AutoUV(rawMesh);
	}
	return MeshUtil::GenerateUStaticMesh(rawMesh);
}


void ModelLoaderStatic::ClearAttached(AActor* v0) {
	if (v0 == nullptr || v0->GetRootComponent()==nullptr)return;
	TArray<USceneComponent*> attached = v0->GetRootComponent()->GetAttachChildren();
	for (int i = attached.Num() - 1; i >= 0; i--) {
		ClearAttached(attached[i]->GetOwner());
		attached[i]->GetOwner()->Destroy();
	}
}



void ModelLoaderStatic::LoadVrmlFileToActor(AActor* v0, const std::string& fileStr0, float defaultAngleLimit, UMaterialInterface* baseMaterial, UMaterialInterface* transparentMaterial, ModelLoaderStaticSetCameraHandler onSetCamera, bool forceAutoUV)
{
	if (v0 == nullptr)return;
	std::vector<unsigned char> data;
	if (!ReadBinary(data, fileStr0)) {
		return;
	}
	UStaticMeshComponent* comp = v0->FindComponentByClass<UStaticMeshComponent>();
	if (comp != nullptr) {
		comp->SetStaticMesh(nullptr);
	}
	ClearAttached(v0);
	LoadVrmlFile(fileStr0, data, defaultAngleLimit, nullptr, v0, baseMaterial, transparentMaterial, forceAutoUV, onSetCamera, false);
}


void ModelLoaderStatic::LoadVrmlFile(const std::string& fileStr0, std::vector<unsigned char>& ulstr0, float defaultAngleLimit, UStaticMesh** destSingleMesh, AActor* v0, UMaterialInterface* baseMaterial, UMaterialInterface* transparentMaterial, bool forceAutoUV, ModelLoaderStaticSetCameraHandler onSetCamera, bool isInline) {
	std::vector<std::string> stackType;//按大括号的关系构建的类型堆栈
	AActor* vt = v0;

	//float temp;//用于交换的临时变量
	int i = 0; int j = 0; int k = 0; int l = 0; int m = 0;//循环变量
	bool isComment = false;//是否为注释
	bool isLabel = false;//是否为标识符
	std::string str;
	std::vector<float> infoArr{ 0,0,0,0 };
	bool ccw = true;
	std::string definedGeometryKey = "";
	std::map<std::string, TArray<FVector>> definedPoint;
	std::map<std::string, TArray<FVector2D>> definedUv;
	std::map<std::string, UStaticMesh*> definedMesh;
	std::string materialKey;
	bool isTransparent = false;
	float alpha = 1;
	std::map<std::string, UMaterialInstance*> urlToMaterial;
	std::map<std::string, UMaterialInstance*> transparentUrlToMaterial;
	bool useSharedMesh = false;
	bool isPreset = false;
	int shapeCount = 0;
	int shapeCountInTransform = 0;
	bool hasCamera = false;
	bool hasInlineRef = false;

	FRawMesh rawMesh;
	TArray<FVector2D> uv;
	TArray<uint32> indicesTex;
	UStaticMesh* mesh = nullptr;
	UStaticMeshComponent* meshComp = nullptr;

	float rgeo = std::numeric_limits<float>::quiet_NaN();
	float hgeo = std::numeric_limits<float>::quiet_NaN();

	std::vector<float> uvinfo{ 0,0,1,1,0,0,0 };
	std::vector<float> uvMatrix = std::vector<float>{ 1,0,0,1,0,0 };

	std::vector<AActor*> inlineModelArray;
	std::vector<std::string> inlineUrlArray;

	unsigned char ulstri = 0;
	std::string lastTransfromName = "";
	float angleLimit = defaultAngleLimit;
	unsigned char ulstr0i = 0;

	j = 0;
	int leng0 = ulstr0.size();//vrml总文件的字符数
	int leng=0;//数组的长度

	FVector cameraPosition = FVector(0, 0, 0);
	FVector cameraEulerAngles = FVector(0, 0, 0);
	float cameraFieldOfView = 60.0f;

	std::vector<unsigned char> ulstr = std::vector<unsigned char>(ulstr0.size());
	for (i = 0; i < leng0; i++) {
		ulstr0i = ulstr0[i];
		if (ulstr0i == '\r' || ulstr0i == '\n') {
			if (isComment) {
				j = i;
			}
			isComment = false;
		}
		if (ulstr0i == '$') {
			isComment = true;
		}
		if (isComment) {
			ulstr[i] = (unsigned char)' ';
		}
		else {
			ulstr[i] = ulstr0[i];
		}
	}
	ulstr0 = ulstr;
	for (i = 0; i < leng0; i++) {
		ulstri = ulstr[i];
		if (ulstri == '{') {
			stackType.push_back(getPrevWord(ulstr, i - 1));
			str = stackType[stackType.size() - 1];
			if (str == "Shape") {
				shapeCount++;
				
			}
			if (str == "Inline") {
				hasInlineRef = true;
			}
			if (str == "Viewpoint") {
				hasCamera = true;
			}
		}
		if (ulstri == '}' && stackType.size() > 0) {
			stackType.erase(stackType.begin() + (stackType.size() - 1));
		}
	}
	stackType.resize(0);
	j = 0;
	isComment = false;

	if (v0 == nullptr && shapeCount > 1) {
		UE_LOG(LogTemp, Log, TEXT("shapeCount: %d"), shapeCount);
		return;
	}
	
	for (i = 0; i < leng0; i++) {
		ulstri = ulstr[i];
		if (ulstri == '{') {
			stackType.push_back(getPrevWord(ulstr, i - 1));
			str = stackType[stackType.size() - 1];
			if (str == "Transform" || str == "Shape" || str == "Inline") {
				if (str == "Transform" && getPrevWord(ulstr, i - 1, 3) == "DEF") {
					lastTransfromName = getPrevWord(ulstr, i - 1, 2);
					shapeCountInTransform = 0;
				}
				else if (str == "Transform") {
					lastTransfromName = "";
					shapeCountInTransform = 0;
				}
				if (str == "Shape") {
					shapeCountInTransform++;
				}
				if (v0 != nullptr) {
					if (str != "Shape" || shapeCountInTransform > 1 || vt==v0) {
						if (shapeCount > 0 || hasCamera || hasInlineRef || isInline) {
							AActor* vtp = vt;
							vt = v0->GetWorld()->SpawnActor<AStaticMeshActor>(FVector::ZeroVector, FRotator::ZeroRotator);
							static_cast<AStaticMeshActor*>(vt)->SetMobility(EComponentMobility::Movable);

							/*vt = v0->GetWorld()->SpawnActor<AActor>(FVector::ZeroVector, FRotator::ZeroRotator);
							USceneComponent* comp = NewObject<USceneComponent>(vt);
							vt->SetRootComponent(comp);
							//comp->RegisterComponent();
							UStaticMeshComponent *comp1= NewObject<UStaticMeshComponent>(vt);
							vt->AddInstanceComponent(comp1);
							comp1->RegisterComponent();*/

							vt->AttachToActor(vtp,FAttachmentTransformRules::KeepWorldTransform);
						}

						vt->SetActorRelativeLocation(FVector(0, 0, 0));
						vt->SetActorRelativeRotation(FRotator::MakeFromEuler(FVector(0, 0, 0)));
						vt->SetActorRelativeScale3D(FVector(1, 1, 1));
					}
					meshComp = vt->FindComponentByClass<UStaticMeshComponent>();
					if (meshComp != nullptr && baseMaterial !=nullptr) {
						meshComp->SetMaterial(0, baseMaterial);
					}
					/*if ((vt->FindComponentByClass<MeshRenderer>() == nullptr)) {

						//_comp220044=vt.AddComponent(typeof(MeshRenderer))
						UStaticMeshComponent* _comp220044 = vt->CreateDefaultSubobject<MeshRenderer>("MeshRenderer");
						_comp220044->AttachTo(vt);
						//_comp220044
						//_comp220044;
					}
					vt->FindComponentByClass<MeshRenderer>().material = UMaterialInstance(FindObject<AActor>(FShader.GetOwner(), shaderName));*/


					if (lastTransfromName != "" && str != "Inline" && vt!=v0) {
						#if UE_EDITOR
						vt->SetActorLabel(lastTransfromName.c_str());
						#endif
					}
					if (str == "Inline") {
						inlineModelArray.push_back(vt);
						inlineUrlArray.push_back("");
					}
				}
				
				ccw = true;
				rawMesh.Empty();
				indicesTex.SetNum(0);
				uv.SetNum(0);
				mesh = nullptr;
				rgeo = std::numeric_limits<float>::quiet_NaN();
				hgeo = std::numeric_limits<float>::quiet_NaN();
				uvinfo = std::vector<float>{ 0,0,1,1,0,0,0 };
				uvMatrix = std::vector<float>{ 1,0,0,1,0,0 };
				materialKey = "";
				isTransparent = false;
				isPreset = false;
				definedGeometryKey = "";
				angleLimit = defaultAngleLimit;
			}
		}

		if (ulstri >= 'A' && ulstri <= 'Z' || ulstri >= 'a' && ulstri <= 'z' || ulstri == '_') {
			if (!isLabel) {
				j = i;
			}
			isLabel = true;
		}
		else if (isLabel && stackType.size() > 0) {
			isLabel = false;
			str = std::string((char*)(&ulstr[j]), i - j);
			if (str == "position" && stackType[stackType.size() - 1] == "Viewpoint") {
				getNumber(ulstr, infoArr, 3, i);
				cameraPosition = FVector((-infoArr[2]), infoArr[0], infoArr[1]);
			}
			if (str == "orientation" && stackType[stackType.size() - 1] == "Viewpoint") {
				infoArr[2] = 1;
				getNumber(ulstr, infoArr, 4, i);
				cameraEulerAngles = vrmlAxisRotationToEularAngles(infoArr[0], infoArr[1], infoArr[2], infoArr[3]);
			}
			if (str == "fieldOfView" && stackType[stackType.size() - 1] == "Viewpoint") {
				getNumber(ulstr, infoArr, 1, i);
				cameraFieldOfView = infoArr[0] * 90 / PI;
			}
			if (str == "translation" && stackType[stackType.size() - 1] == "Transform" && vt!=nullptr) {
				getNumber(ulstr, infoArr, 3, i);
				vt->SetActorRelativeLocation(FVector((-infoArr[2]), infoArr[0], infoArr[1]));
			}
			if (str == "rotation" && stackType[stackType.size() - 1] == "Transform" && vt != nullptr) {
				infoArr[2] = 1;
				getNumber(ulstr, infoArr, 4, i);
				vt->SetActorRelativeRotation(FRotator::MakeFromEuler(vrmlAxisRotationToEularAngles(infoArr[0], infoArr[1], infoArr[2], infoArr[3])));
			}
			if (str == "scale" && stackType[stackType.size() - 1] == "Transform" && vt != nullptr) {
				infoArr[0] = 1;
				infoArr[1] = 1;
				infoArr[2] = 1;
				getNumber(ulstr, infoArr, 3, i);
				vt->SetActorRelativeScale3D(FVector(infoArr[2], infoArr[0], infoArr[1]));
			}
			if (str == "center" && stackType[stackType.size() - 1] == "TextureTransform") {
				getNumber(ulstr, infoArr, 2, i);
				uvinfo[0] = infoArr[0];
				uvinfo[1] = infoArr[1];
				uvMatrix = std::vector<float>{ 1,0,0,1,0,0 };
			}
			if (str == "scale" && stackType[stackType.size() - 1] == "TextureTransform") {
				getNumber(ulstr, infoArr, 3, i);
				uvinfo[2] = infoArr[0];
				uvinfo[3] = infoArr[1];
				uvMatrix = std::vector<float>{ 1,0,0,1,0,0 };
			}
			if (str == "rotation" && stackType[stackType.size() - 1] == "TextureTransform") {
				getNumber(ulstr, infoArr, 1, i);
				uvinfo[4] = infoArr[0];
				uvMatrix = std::vector<float>{ 1,0,0,1,0,0 };
			}
			if (str == "translation" && stackType[stackType.size() - 1] == "TextureTransform") {
				getNumber(ulstr, infoArr, 2, i);
				uvinfo[5] = infoArr[0];
				uvinfo[6] = infoArr[1];
				uvMatrix = std::vector<float>{ 1,0,0,1,0,0 };
			}
			if (str == "diffuseColor" && stackType[stackType.size() - 1] == "Material" && vt!=nullptr && meshComp!=nullptr && baseMaterial != nullptr) {
				getNumber(ulstr, infoArr, 3, i);
				int colorIntKey = (int)(infoArr[0]*255) << 16 | (int)(infoArr[1] * 255) << 8 | (int)(infoArr[2] * 255);
				materialKey="color:"+std::to_string(colorIntKey);
				if (isTransparent) {
					materialKey += "|alpha:" + std::to_string((int)(alpha * 255));
				}
				std::map<std::string, UMaterialInstance*>& matMap = isTransparent ? transparentUrlToMaterial : urlToMaterial;
				if (matMap.find(materialKey)!= matMap.end()) {
					meshComp->SetMaterial(0, matMap[materialKey]);
				}
				else {
					UMaterialInstanceDynamic* mat = UMaterialInstanceDynamic::Create(!isTransparent ? baseMaterial : transparentMaterial, meshComp);
					mat->SetVectorParameterValue(FName(TEXT("Color")), FLinearColor(infoArr[0], infoArr[1], infoArr[2],1));
					//mat->SetVectorParameterValue(FName(TEXT("TextureBlend")), FLinearColor(0,0,0,0));
					mat->SetScalarParameterValue(FName(TEXT("TextureBlend")), 0);
					if (isTransparent) {
						mat->SetScalarParameterValue(FName(TEXT("Opacity")), alpha);
					}
					matMap[materialKey]=mat;
					meshComp->SetMaterial(0, mat);
				}
			}
			if (str == "shininess" && stackType[stackType.size() - 1] == "Material" && vt != nullptr && meshComp != nullptr && baseMaterial != nullptr) {
				getNumber(ulstr, infoArr, 1, i);
			}
			if (str == "transparency" && stackType[stackType.size() - 1] == "Material" && vt != nullptr && meshComp != nullptr && baseMaterial != nullptr) {
				getNumber(ulstr, infoArr, 1, i);
				alpha = 1.0f - infoArr[0];
				if (alpha < 0.01f) {
					vt->SetHidden(true);
				}
				else if (alpha <= 0.99f) {
					bool prevIsTransparent = isTransparent;
					std::string prevMaterialKey = materialKey;
					isTransparent = true;
					

					
					std::map<std::string, UMaterialInstance*>& matMap = transparentUrlToMaterial;
					if (!prevIsTransparent) {
						materialKey = prevMaterialKey + "|alpha:" + std::to_string((int)(alpha*255));
					}
					if (prevIsTransparent==isTransparent && matMap.find(materialKey) != matMap.end()) {
						meshComp->SetMaterial(0, matMap[materialKey]);
					}
					else {
						std::map<std::string, UMaterialInstance*>& prevMatMap = prevIsTransparent ? transparentUrlToMaterial : urlToMaterial;
						UMaterialInstance* prevMat = nullptr;
						if (prevMatMap.find(prevMaterialKey) != prevMatMap.end()) {
							prevMat = prevMatMap[prevMaterialKey];
						}
						UMaterialInstanceDynamic* mat = UMaterialInstanceDynamic::Create(transparentMaterial, meshComp);
						if (prevMat != nullptr) {
							FLinearColor color=FLinearColor(0,0,0,1);
							UTexture* tex=nullptr;
							float textureBlend=0;
							prevMat->GetVectorParameterValue(FName(TEXT("Color")), color);
							prevMat->GetTextureParameterValue(FName(TEXT("Texture")), tex);
							prevMat->GetScalarParameterValue(FName(TEXT("TextureBlend")), textureBlend);
							mat->SetVectorParameterValue(FName(TEXT("Color")), color);
							mat->SetTextureParameterValue(FName(TEXT("Texture")), tex);
							mat->SetScalarParameterValue(FName(TEXT("TextureBlend")), textureBlend);
						}
						mat->SetScalarParameterValue(FName(TEXT("Opacity")), alpha);
						matMap[materialKey] = mat;
						meshComp->SetMaterial(0, mat);
					}
				}
			}

			if (str == "url" && stackType[stackType.size() - 1] == "ImageTexture" && vt != nullptr && meshComp != nullptr && baseMaterial != nullptr) {
				for (k = i; k < leng0; k++) {
					if (ulstr[k] == '"') {
						break;
					}
				}
				for (l = k + 1; l < leng0; l++) {
					if (ulstr[l] == '"') {
						break;
					}
				}
				if (l > k) {
					std::string str0 = std::string((char*)(&ulstr[k + 1]), l - k - 1);
					#ifdef _WIN32
					materialKey = EncodeTransfer::UTF8ToGBK(str0);
					if (_access(GetAbsolutePath(materialKey, fileStr0).c_str(), 0)==-1)
					{
						materialKey = str0;
					}
					#else
					materialKey = str0;
					if (_access(GetAbsolutePath(materialKey, fileStr0).c_str(),0)==-1)
					{
						materialKey = EncodeTransfer::GBKToUTF8(str0);
					}
					#endif
					std::map<std::string, UMaterialInstance*>& matMap = isTransparent ? transparentUrlToMaterial : urlToMaterial;
					if (isTransparent) {
						materialKey += "|alpha:" + std::to_string((int)(alpha * 255));
					}
					if (matMap.find(materialKey) != matMap.end()) {
						meshComp->SetMaterial(0, matMap[materialKey]);
					}
					else {
						std::string absPath = GetAbsolutePath(materialKey, fileStr0);
						UTexture2D* tex = LoadTexture2D(FromStdStringPath(absPath.c_str()));
						if (tex != nullptr) {
							UMaterialInstanceDynamic* mat = UMaterialInstanceDynamic::Create(!isTransparent ? baseMaterial : transparentMaterial, meshComp);
							mat->SetTextureParameterValue(FName(TEXT("Texture")), tex);
							//mat->SetVectorParameterValue(FName(TEXT("TextureBlend")), FLinearColor(1, 1, 1, 1));
							mat->SetScalarParameterValue(FName(TEXT("TextureBlend")), 1);
							if (isTransparent) {
								mat->SetScalarParameterValue(FName(TEXT("Opacity")), alpha);
							}
							matMap[materialKey] = mat;
							meshComp->SetMaterial(0, mat);
						}
					}
				}
				
			}
			if (str == "url" && stackType[stackType.size() - 1] == "Inline" && vt!=nullptr) {
				//k=ulstr.indexOf("\"",i);
				//l=ulstr.indexOf("\"",k+1);
				for (k = i; k < leng0; k++) {
					if (ulstr[k] == '"') {
						break;
					}
				}
				for (l = k + 1; l < leng0; l++) {
					if (ulstr[l] == 34) {
						break;
					}
				}
				if (l > k) {
					std::string str0 = std::string((char*)(&ulstr[k + 1]), l - k - 1);
					#ifdef _WIN32
					std::string path = GetAbsolutePath(EncodeTransfer::UTF8ToGBK(str0), fileStr0);
					if (_access(path.c_str(), 0)==-1)
					{
						path = GetAbsolutePath(str0, fileStr0);
					}
					#else
					std::string path = GetAbsolutePath(str0, fileStr0);
					if (_access(path.c_str(),0)==-1)
					{
						path = GetAbsolutePath(EncodeTransfer::GBKToUTF8(str0), fileStr0);
					}
					#endif
					if (inlineUrlArray.size() > 0) {
						inlineUrlArray[inlineUrlArray.size() - 1] = path;
					}
					std::vector<std::string> nameSeg;
					splitString(nameSeg, path, ".");
					if (nameSeg.size() >= 2) {
					#if UE_EDITOR
						vt->SetActorLabel(UTF8_TO_TCHAR(nameSeg[nameSeg.size() - 2].c_str()));
					#endif
					}
				}
			}
			if (str == "geometry" && stackType[stackType.size() - 1] == "Shape" && getNextWord(ulstr, i, 1) == "DEF" && vt != nullptr) {
				std::string key = getNextWord(ulstr, i, 2);
				definedGeometryKey = key;
				#if UE_EDITOR
				vt->SetActorLabel(stringReplace(key, "-FACES", "").c_str());
				#endif			
				useSharedMesh = false;
			}
			if (str == "geometry" && stackType[stackType.size() - 1] == "Shape" && getNextWord(ulstr, i, 1) == "USE" && vt != nullptr) {
				useSharedMesh = false;
				std::string key = getNextWord(ulstr, i, 2);

				if (definedMesh.find(key) != definedMesh.end()) {
					
					UStaticMeshComponent* mf=vt->FindComponentByClass<UStaticMeshComponent>();
					mf->SetStaticMesh(definedMesh[key]);
					useSharedMesh = true;
				}
			}
			if (str == "ccw" && stackType[stackType.size() - 1] == "IndexedFaceSet") {
				//ccw=ulstr.substring(i+1,i+6)!="FALSE";
				ccw = ulstr[i + 1] != 'F';
			}
			/*if(str=="solid" && stackType[stackType.Count-1]=="IndexedFaceSet"){

				}*/
			if (str == "creaseAngle" && stackType[stackType.size() - 1] == "IndexedFaceSet") {
				getNumber(ulstr, infoArr, 1, i);
				angleLimit = infoArr[0] * 180 / PI;
			}
			if (str == "point" && stackType[stackType.size() - 1] == "Coordinate") {
				useSharedMesh = false;
				std::string strw = "";
				if (getPrevWord(ulstr, i, 4) == "DEF") {
					strw = getPrevWord(ulstr, i, 3);
				}

				std::vector<std::vector<float>> point0;
				i = getNumberTable(ulstr, point0, 3, false, i);
				leng = point0.size();
				TArray<FVector>& point = rawMesh.VertexPositions;
				point.SetNumZeroed(leng);
				for (l = 0; l < leng; l++) {
					if (point0[l].size() >= 3) {

						point[l] = FVector((-point0[l][2]), point0[l][0], point0[l][1]);
					}
				}
				if (strw.size()>0) {
					definedPoint[strw] = TArray<FVector>(point);
				}
			}
			if (str == "coord" && stackType[stackType.size() - 1] == "IndexedFaceSet" && getNextWord(ulstr, i, 1) == "USE") {
				useSharedMesh = false;
				std::string strw = getNextWord(ulstr, i, 2);
				if ((definedPoint.find(strw) != definedPoint.end())) {
					rawMesh.VertexPositions = TArray<FVector>(definedPoint[strw]);
				}
			}
			if (str == "coordIndex" && stackType[stackType.size() - 1] == "IndexedFaceSet") {
				useSharedMesh = false;
				std::vector<std::vector<float>> index0;
				i = getNumberTable(ulstr, index0, 100, true, i);
				leng = index0.size();
				TArray<uint32>& indices = rawMesh.WedgeIndices;
				indices.SetNum(0);
				for (l = 0; l < leng; l++) {
					for (m = index0[l].size() - 1; m >= 2; m--) {
						indices.Add((int)index0[l][0]);
						indices.Add((ccw ? (int)index0[l][m - 1] : (int)index0[l][m]));
						indices.Add((ccw ? (int)index0[l][m] : (int)index0[l][m - 1]));
					}
				}
			}
			if (str == "point" && stackType[stackType.size() - 1] == "TextureCoordinate") {
				useSharedMesh = false;
				std::string strw = "";
				if (getPrevWord(ulstr, i, 4) == "DEF") {
					strw = getPrevWord(ulstr, i, 3);
				}

				if (uvMatrix.size() >= 6 && uvinfo.size() >= 7) {
					float tx0 = uvinfo[5] - uvinfo[0];
					float ty0 = uvinfo[6] - uvinfo[1];
					float cost = std::cos(uvinfo[4]);
					float sint = std::sin(uvinfo[4]);
					uvMatrix[0] = uvinfo[2] * cost;
					uvMatrix[1] = uvinfo[3] * sint;
					uvMatrix[2] = -uvinfo[2] * sint;
					uvMatrix[3] = uvinfo[3] * cost;
					uvMatrix[4] = tx0 * uvMatrix[0] + ty0 * uvMatrix[2] + uvinfo[0];
					uvMatrix[5] = tx0 * uvMatrix[1] + ty0 * uvMatrix[3] + uvinfo[1];
				}
				std::vector<std::vector<float>> uv0;
				i = getNumberTable(ulstr, uv0, 2, false, i);

				leng = uv0.size();
				uv.SetNumZeroed(leng);
				for (l = 0; l < leng; l++) {
					if (uv0[l].size() >= 2) {
						if (uvMatrix.size() >= 6) {
							uv[l] = FVector2D(uv0[l][0] * uvMatrix[0] + uv0[l][1] * uvMatrix[2] + uvMatrix[4], (uv0[l][0] * uvMatrix[1] + uv0[l][1] * uvMatrix[3] + uvMatrix[5]));
						}
						else {
							uv[l] = FVector2D(uv0[l][0], uv0[l][1]);
						}

						uv[l].Y = 1 - uv[l].Y;
					}
				}
				if (strw != "") {
					definedUv[strw] = TArray<FVector2D>(uv);
				}
			}
			if (str == "texCoord" && stackType[stackType.size() - 1] == "IndexedFaceSet" && getNextWord(ulstr, i, 1) == "USE") {
				useSharedMesh = false;
				std::string strw = getNextWord(ulstr, i, 2);
				if (definedUv.find(strw) != definedUv.end()) {
					uv = TArray<FVector2D>(definedUv[strw]);
				}
			}
			if (str == "texCoordIndex" && stackType[stackType.size() - 1] == "IndexedFaceSet") {
				useSharedMesh = false;
				std::vector<std::vector<float>> index0;
				i = getNumberTable(ulstr, index0, 100, true, i);
				leng = index0.size();
				indicesTex.SetNum(0);
				for (l = 0; l < leng; l++) {
					for (m = index0[l].size() - 1; m >= 2; m--) {
						indicesTex.Add((int)index0[l][0]);
						indicesTex.Add((ccw ? (int)index0[l][m - 1] : (int)index0[l][m]));
						indicesTex.Add((ccw ? (int)index0[l][m] : (int)index0[l][m - 1]));
						
					}
				}
			}
			if (str == "size" && stackType[stackType.size() - 1] == "Box") {
				getNumber(ulstr, infoArr, 3, i);
				FRawMesh rawMesh1;
				UModel3d::Cuboid(rawMesh1, infoArr[0], infoArr[1], infoArr[2]);
				mesh= MeshUtil::GenerateUStaticMesh(rawMesh1);
				isPreset = true;
			}
			if (str == "radius" && stackType[stackType.size() - 1] == "Sphere") {
				getNumber(ulstr, infoArr, 1, i);
				mesh = MeshUtil::GenerateUStaticMesh(UModel3d::Sphere(infoArr[0])->m_rawMesh);
				isPreset = true;
			}
			if (str == "bottomRadius" && stackType[stackType.size() - 1] == "Cone") {
				getNumber(ulstr, infoArr, 1, i);
				rgeo = infoArr[0];
				if (!std::isnan(rgeo) && !std::isnan(hgeo)) {
					mesh = MeshUtil::GenerateUStaticMesh(UModel3d::Cone(rgeo,hgeo)->m_rawMesh);
					isPreset = true;
				}
			}
			if (str == "height" && stackType[stackType.size() - 1] == "Cone") {
				getNumber(ulstr, infoArr, 1, i);
				hgeo = infoArr[0];
				if (!std::isnan(rgeo) && !std::isnan(hgeo)) {
					mesh = MeshUtil::GenerateUStaticMesh(UModel3d::Cone(rgeo, hgeo)->m_rawMesh);
					isPreset = true;
				}
			}
			if (str == "height" && stackType[stackType.size() - 1] == "Cylinder") {
				getNumber(ulstr, infoArr, 1, i);
				rgeo = infoArr[0];
				if (!std::isnan(rgeo) && !std::isnan(hgeo)) {
					mesh = MeshUtil::GenerateUStaticMesh(UModel3d::Cone(rgeo, hgeo)->m_rawMesh);
					isPreset = true;
				}
			}
		}
		else if (isLabel) {
			isLabel = false;
		}
		if (ulstri == '}' && stackType.size() > 0) {
			if (stackType[stackType.size() - 1] == "Shape") {
				if (useSharedMesh)
				{
					/*MeshRenderer* mr = vt->FindComponentByClass<MeshRenderer>();
					if ((mr != nullptr)) {
						mr->shadowCastingMode = UnityEngine.Rendering.ShadowCastingMode.TwoSided;
					}*/
				}
				else if (mesh==nullptr && rawMesh.VertexPositions.Num() > 0 && rawMesh.WedgeIndices.Num() > 0) 
				{
					
					MeshUtil::CreateNormal(rawMesh, angleLimit);
					if (uv.Num() <= 0 || forceAutoUV) {
						MeshUtil::AutoUV(rawMesh);
					}
					else {
						MeshUtil::SetUV(rawMesh, uv, indicesTex);
					}
					mesh = MeshUtil::GenerateUStaticMesh(rawMesh);

					if (definedGeometryKey != "")
					{
						definedMesh[definedGeometryKey] = mesh;
					}
					if (meshComp != nullptr) {
						meshComp->SetStaticMesh(mesh);
					}
				}
				
				if (vt!=nullptr && shapeCountInTransform > 1 && vt != v0 && (vt->GetAttachParentActor() != nullptr)) {
					vt = vt->GetAttachParentActor();
				}
			}
			else if ((stackType[stackType.size() - 1] == "Transform" || stackType[stackType.size() - 1] == "Inline") && vt != nullptr) {
				

				/*if (!((mf != nullptr) && mf->GetStaticMesh()->vertexCount > 0 && _rawMesh.WedgeIndices.Length > 0)) {

					//_rawMesh.WedgeIndices=mf.GetStaticMesh().triangles
					FRawMesh _rawMesh;//TODO: Only declared at first
					mf->GetStaticMesh()->GetSourceModel(0).LoadRawMesh(_rawMesh);//TODO: Only called at first
					//_rawMesh.WedgeIndices


					if ((vt->FindComponentByClass<UStaticMeshComponent>() != nullptr)) {
						vt->FindComponentByClass<UStaticMeshComponent>().Destroy();
					}
					if ((vt->FindComponentByClass<MeshRenderer>() != nullptr)) {
						vt->FindComponentByClass<MeshRenderer>().Destroy();
					}
				}*/
				if (vt != v0 && (vt->GetAttachParentActor() != nullptr)) {
					vt = vt->GetAttachParentActor();
				}
			}
			stackType.erase(stackType.begin() + (stackType.size() - 1));
		}
	}
	if (!isInline) {
		if (hasCamera) {
			//v0->SetActorRelativeLocation(FVector(0, 0, 0));
			//v0->SetActorRelativeRotation(FRotator::MakeFromEuler(FVector(0, 0, 0)));
			//v0->SetActorRelativeScale3D(FVector(1, 1, 1));
			if (onSetCamera != nullptr) {
				onSetCamera(cameraPosition, cameraEulerAngles, cameraFieldOfView);
			}			
		}
		else if (shapeCount == 1 && !hasInlineRef && destSingleMesh!=nullptr) {
			*destSingleMesh = mesh;
		}
		else {
			
		}
		UE_LOG(LogTemp,Log,TEXT("shapeCount: %d"), shapeCount);
	}
	if (inlineModelArray.size() >= 0) {
		for (i = 0; i < inlineModelArray.size() && i < inlineUrlArray.size(); i++) {
			if (inlineUrlArray[i] != "") {
				std::vector<unsigned char> data;
				ReadBinary(data, inlineUrlArray[i]);
				if (data.size() > 0) {
					LoadVrmlFile(inlineUrlArray[i], data, defaultAngleLimit, nullptr, inlineModelArray[i], baseMaterial,transparentMaterial,forceAutoUV, nullptr, true);
				}
				
			}
		}
	}

}

std::string ModelLoaderStatic::getPrevWord(std::vector<unsigned char>& ulstr, int i0, int wordNumber)
{
	int i = 0;
	int j = 0;
	bool readBegin = false;
	bool readBeginPrev = false;
	int count = 0;
	int ulstri = 0;
	j = i0;
	for (i = i0; i >= 0; i--) {
		ulstri = ulstr[i];
		readBeginPrev = readBegin;
		if ((ulstri >= 'A' && ulstri <= 'Z') || (ulstri >= 'a' && ulstri <= 'z') || (ulstri >= '0' && ulstri <= '9') || ulstri == '_' || ulstri == '-' || ulstri == '+' || ulstri == '.' || ulstri == ':' || ulstri >= 128) {
			if (!readBegin) {
				j = i;
			}
			readBegin = true;
		}
		else if (readBegin) {
			readBegin = false;
			if (readBeginPrev) {
				count++;
				if (count >= wordNumber) {
					break;
				}
			}
		}
	}
	i++;
	j++;
	if (j >= ulstr.size()) {
		j = ulstr.size();
	}
	if (j <= i) {
		return std::string();
	}
	else {
		return std::string((char*)(&ulstr[i]), j-i);
	}
}

std::string ModelLoaderStatic::getNextWord(std::vector<unsigned char>& ulstr, int i0, int wordNumber)
{
	int i = 0;
	int j = 0;
	bool readBegin = false;
	bool readBeginPrev = false;
	int count = 0;
	int ulstri = 0;
	j = i0;
	int leng0 = ulstr.size();
	for (i = i0; i < leng0; i++) {
		ulstri = ulstr[i];
		readBeginPrev = readBegin;
		if ((ulstri >= 'A' && ulstri <= 'Z') || (ulstri >= 'a' && ulstri <= 'z') || (ulstri >= '0' && ulstri <= '9') || ulstri == '_' || ulstri == '-' || ulstri == '+' || ulstri == '.' || ulstri == ':' || ulstri >= 128) {
			if (!readBegin) {
				j = i;
			}
			readBegin = true;
		}
		else if (readBegin) {
			readBegin = false;
			if (readBeginPrev) {
				count++;
				if (count >= wordNumber) {
					break;
				}
			}
		}
	}
	if (i <= j) {
		return std::string();
	}
	else {
		return std::string((char*)(&ulstr[j]), i - j);
	}
}

int ModelLoaderStatic::getNumber(std::vector<unsigned char>& ulstr, std::vector<float>& nums, int numCount, int i0)
{
	int i = 0;
	int j = 0;
	int k = 0;
	bool readBegin = false;
	bool readBeginPrev = false;
	unsigned char ulstri = 0;
	j = i0;
	int leng0 = ulstr.size();
	for (i = i0; i < leng0; i++) {
		ulstri = ulstr[i];
		readBeginPrev = readBegin;
		if ((ulstri >= '0' && ulstri <= '9') || ulstri == 'E' || ulstri == 'e' || ulstri == '-' || ulstri == '+' || ulstri == '.') {
			if (!readBegin) {
				j = i;
			}
			readBegin = true;
		}
		else if (readBegin) {
			readBegin = false;
			if (readBeginPrev) {
				float num = 0.0f;
				if (k < nums.size()){
					std::string str((char*)(&ulstr[j]), i - j);
					num = (float)atof(str.c_str());
					nums[k] = num;
					k++;
				}
				if (k >= numCount || k >= nums.size()) {
					break;
				}
			}
		}
	}
	return i;
}

int ModelLoaderStatic::getNumberTable(std::vector<unsigned char>& ulstr, std::vector<std::vector<float>>& nums, int columnCount, bool useSeparatorM1, int i0)
{
	int i = 0;
	int j = 0;
	bool readBegin = false;
	bool readBeginPrev = false;
	unsigned char ulstri = 0;
	j = i0;
	int leng0 = ulstr.size();
	nums.clear();
	for (i = i0; i < leng0; i++) {
		ulstri = ulstr[i];
		readBeginPrev = readBegin;

		if ((ulstri >= '0' && ulstri <= '9') || ulstri == 'E' || ulstri == 'e' || ulstri == '-' || ulstri == '+' || ulstri == '.') {
			if (!readBegin) {
				j = i;
			}
			readBegin = true;
		}
		else if (readBegin) {
			readBegin = false;
			if (readBeginPrev) {
				std::string str((char*)(&ulstr[j]), i - j);
				if (useSeparatorM1 && str == "-1") {
					nums.push_back((std::vector<float>()));
				}
				else {
					float num = 0.0f;
					num = (float)atof(str.c_str());
					if (nums.size() == 0) {
						nums.push_back((std::vector<float>()));
					}
					nums[nums.size() - 1].push_back(num);
					if (columnCount > 0 && nums[nums.size() - 1].size() >= columnCount) {
						nums.push_back((std::vector<float>()));
					}
				}
			}

		}
		if (ulstri == ']' || ulstri == '}') {
			break;
		}
	}
	if (nums.size() > 0 && nums[nums.size() - 1].size() == 0) {
		nums.erase(nums.begin() + (nums.size() - 1));
	}
	return i;
}


std::string ModelLoaderStatic::GetAbsolutePath(const std::string& relativePath0, const std::string& filePathNameBase0)
{
	std::string relativePath = stringReplace(relativePath0, "\\", "/");
	std::string filePathNameBase = stringReplace(filePathNameBase0, "\\", "/");
	if ((relativePath.find(":") == std::string::npos) && (int)relativePath.find("/") != 0) {
		int pos = filePathNameBase.size();
		while (substring(relativePath, 0, 0 + 3) == "../" && pos > 0) {
			relativePath = substring(relativePath, 3);
			pos = (int)filePathNameBase.rfind("/", pos) - 1;
		}
		pos = (int)filePathNameBase.rfind("/", pos);
		return substring(filePathNameBase, 0, 0 + (pos + 1)) + relativePath;
	}
	else {
		return relativePath;
	}
}

FVector ModelLoaderStatic::AxisRotationToEularAngles(float x0, float y0, float z0, float w0)
{
	float d = FMath::Sqrt(x0 * x0 + y0 * y0 + z0 * z0);
	x0 /= d;
	y0 /= d;
	z0 /= d;
	float cosw02 = FMath::Cos(w0 / 2);
	float sinw02 = FMath::Sin(-w0 / 2);
	float rx = x0 * sinw02;
	float ry = y0 * sinw02;
	float rz = z0 * sinw02;
	float rw = cosw02;
	FQuat q = FQuat(rx, ry, rz, rw);
	return q.Euler();
}

FVector ModelLoaderStatic::vrmlAxisRotationToEularAngles(float x0, float y0, float z0, float w0) {
	return AxisRotationToEularAngles(-z0,x0,y0,w0);
}