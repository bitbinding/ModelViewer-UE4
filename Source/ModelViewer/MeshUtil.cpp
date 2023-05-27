// Fill out your copyright notice in the Description page of Project Settings.


#include "MeshUtil.h"
#include <Runtime/AssetRegistry/Public/AssetRegistry/AssetRegistryModule.h>
#include <Runtime/StaticMeshDescription/Public/StaticMeshDescription.h>
#include <Runtime/StaticMeshDescription/Public/StaticMeshAttributes.h>
#include <Runtime/StaticMeshDescription/Public/StaticMeshOperations.h>


#include "Engine/Engine.h"

//#include "MeshDescription.h"
//#include "MeshDescriptionBuilder.h"
//#include "StaticMeshAttributes.h"

static UTextBlock* textDebug = nullptr;

void MeshUtil::SetTextDebug(UTextBlock* textBlock)
{
	textDebug = textBlock;
}

MeshUtil::MeshUtil()
{
}

MeshUtil::~MeshUtil()
{
}

void MeshUtil::CreateNormal(FRawMesh& _rawMesh, float smoothAngleMax)
{
	TArray<FVector>& point = _rawMesh.VertexPositions;
	TArray<uint32>& indices = _rawMesh.WedgeIndices;

	float vx; float vy; float vz; float vx2; float vy2; float vz2;
	uint32 leng = point.Num();
	uint32 leng2 = indices.Num() / 3;
	
	TArray<FVector> indexn; indexn.SetNumZeroed(leng2);
	uint32 i; uint32 j; uint32 pos;

	for (i = 0; i < (int)leng2; i++)
	{
		pos = i * 3;
		if (indices[pos] >= leng || indices[pos + 1] >= leng || indices[pos + 2] >= leng) {
			indexn[i].X = 0;
			indexn[i].Y = 0;
			indexn[i].Z = 0;
			continue;
		}
		vx = point[indices[pos + 1]].X - point[indices[pos]].X;
		vy = point[indices[pos + 1]].Y - point[indices[pos]].Y;
		vz = point[indices[pos + 1]].Z - point[indices[pos]].Z;
		vx2 = point[indices[pos + 2]].X - point[indices[pos]].X;
		vy2 = point[indices[pos + 2]].Y - point[indices[pos]].Y;
		vz2 = point[indices[pos + 2]].Z - point[indices[pos]].Z;
		indexn[i].X = vz * vy2 - vy * vz2;//生成法向量
		indexn[i].Y = vx * vz2 - vz * vx2;
		indexn[i].Z = vy * vx2 - vx * vy2;
		indexn[i]=indexn[i].GetSafeNormal();
	}
	if (smoothAngleMax >= 180) {
		TArray<FVector> pointn; pointn.SetNumZeroed(leng);
		for (i = 0; i < leng2; i++)
		{
			pos = i * 3;
			j = indices[pos];//生成点法向量
			pointn[j].X += indexn[i].X;
			pointn[j].Y += indexn[i].Y;
			pointn[j].Z += indexn[i].Z;
			j = indices[pos + 1];
			pointn[j].X += indexn[i].X;
			pointn[j].Y += indexn[i].Y;
			pointn[j].Z += indexn[i].Z;
			j = indices[pos + 2];
			pointn[j].X += indexn[i].X;
			pointn[j].Y += indexn[i].Y;
			pointn[j].Z += indexn[i].Z;
		}

		for (i = 0; i < leng; i++)
		{
			pointn[i]=pointn[i].GetSafeNormal();
		}
		TArray<FVector>& normals = _rawMesh.WedgeTangentZ;
		normals.SetNumZeroed(leng2*3);
		for (i = 0; i < leng2; i++)
		{
			pos = i * 3;
			if (indices[pos] >= leng || indices[pos + 1] >= leng || indices[pos + 2] >= leng) {
				continue;
			}
			normals[pos] = pointn[indices[pos]];
			normals[pos+1] = pointn[indices[pos+1]];
			normals[pos+2] = pointn[indices[pos+2]];
		}
	}
	else if (smoothAngleMax <= 0) {
		TArray<FVector>& normals = _rawMesh.WedgeTangentZ;
		normals.SetNumZeroed(leng2 * 3);
		for (i = 0; i < leng2; i++)
		{
			pos = i * 3;
			normals[pos] = indexn[i];
			normals[pos+1] = indexn[i];
			normals[pos+2] = indexn[i];
		}
	}
	else {
		TArray<TArray<int>> pointci;//点所在平面的点序号数组
		for (i = 0; i < leng; i++) {
			pointci.Emplace(TArray<int>());
		}
		for (i = 0; i < leng2; i++) {
			pos = i * 3;
			if (indices[pos] >= leng || indices[pos+1] >= leng || indices[pos+2] >= leng) {
				continue;
			}
			pointci[indices[pos]].Push(pos);
			pointci[indices[pos+1]].Push(pos+1);
			pointci[indices[pos+2]].Push(pos+2);
		}
		uint32 k, l, m, n;
		TArray<uint32> jksign;//某个点的连续的平面组的编组序号
		float cos0 = FMath::Cos(smoothAngleMax*PI/180);
		float cost = 0;
		uint32 temp = 0;
		bool ordered=false;
		FVector normal = FVector();

		TArray<FVector>& normals = _rawMesh.WedgeTangentZ;
		normals.SetNumZeroed(leng2 * 3);

		for (i = 0; i < leng; i++) {
			l = (uint32)pointci[i].Num();
			if (l == 0) {
				continue;
			}
			if (l > (uint32)jksign.Num()) {
				jksign.SetNum(l);
			}
			for (j = 0; j < l; j++) {
				jksign[j] = j;
			}
			for (j = 0; j < l; j++) {
				for (k = j + 1; k < l; k++) {
					if (jksign[k] == jksign[j]) {
						continue;
					}
					m = pointci[i][j]/3;
					n = pointci[i][k]/3;
					cost = FVector::DotProduct(indexn[m],indexn[n]);
					if (cost >= cos0) {
						jksign[k] = jksign[j];
					}
				}
			}
			for (j = 0; j < l; j++) {
				ordered = true;
				for (k = 1; k < l - j; k++) {
					if (jksign[j] > jksign[k]) {
						temp = jksign[k];
						jksign[k] = jksign[j];
						jksign[j] = temp;
						temp = pointci[i][k];
						pointci[i][k] = pointci[i][j];
						pointci[i][j] = temp;
						ordered = false;
					}
				}
				if (ordered) {
					break;
				}
			}
			j = 0;
			normal = indexn[pointci[i][0]/3];
			for (k = 0; k < l; k++) {
				if (jksign[j] == jksign[k]) {
					normal += indexn[pointci[i][k]/3];
				}
				else {
					normal = normal.GetSafeNormal();
					for (m = j; m < k; m++) {
						normals[pointci[i][m]] = normal;
					}
					normal = indexn[pointci[i][k]/3];
					j = k;
				}
			}
			normal = normal.GetSafeNormal();
			for (m = j; m < k; m++) {
				normals[pointci[i][m]] = normal;
			}
		}
	}
	if (textDebug != nullptr) {
		textDebug->SetText(FText::FromString(textDebug->Text.ToString() + FString::Printf(TEXT("\nCreateNormal()"))));
	}
}

void MeshUtil::AttachExternalNormalData(FRawMesh& _rawMesh, const TArray<FVector>& pointnSrc, const TArray<uint32>& nindex)
{
	TArray<FVector>& point = _rawMesh.VertexPositions;
	TArray<uint32>& indices = _rawMesh.WedgeIndices;
	TArray<FVector>& normals = _rawMesh.WedgeTangentZ;

	uint32 leng = point.Num();
	uint32 leng2 = indices.Num() / 3;

	uint32 lengSrc = pointnSrc.Num();
	uint32 i, pos;
	if (leng == lengSrc && nindex.Num() == 0) {
		normals.SetNumZeroed(leng2 * 3);
		for (i = 0; i < leng2; i++)
		{
			pos = i * 3;
			if (indices[pos] >= leng || indices[pos + 1] >= leng || indices[pos + 2] >= leng) {
				continue;
			}

			normals[pos] = pointnSrc[indices[pos]];
			normals[pos + 1] = pointnSrc[indices[pos + 1]];
			normals[pos + 2] = pointnSrc[indices[pos + 2]];
		}
	}
	else if (nindex.Num() == indices.Num()) {
		normals.SetNumZeroed(leng2 * 3);
		for (i = 0; i < leng2; i++)
		{
			pos = i * 3;
			if (nindex[pos] >= lengSrc || nindex[pos + 1] >= lengSrc || nindex[pos + 2] >= lengSrc) {
				continue;
			}
			normals[pos] = pointnSrc[nindex[pos]];
			normals[pos + 1] = pointnSrc[nindex[pos+1]];
			normals[pos + 2] = pointnSrc[nindex[pos+2]];
		}
	}
}

void MeshUtil::SetUV(FRawMesh& _rawMesh, const TArray<FVector2D>& uvSrc, const TArray<uint32>& uvTriangles)
{
	TArray<FVector>& point = _rawMesh.VertexPositions;
	TArray<uint32>& indices = _rawMesh.WedgeIndices;
	TArray<FVector2D>& uv = _rawMesh.WedgeTexCoords[0];

	uint32 leng = point.Num();
	uint32 leng2 = indices.Num() / 3;

	uint32 lengSrc = uvSrc.Num();
	uint32 i, pos;
	if (leng == lengSrc && uvTriangles.Num() == 0) {
		uv.SetNumZeroed(leng2 * 3);
		for (i = 0; i < leng2; i++)
		{
			pos = i * 3;
			if (indices[pos] >= leng || indices[pos + 1] >= leng || indices[pos + 2] >= leng) {
				continue;
			}

			uv[pos] = uvSrc[indices[pos]];
			uv[pos + 1] = uvSrc[indices[pos + 1]];
			uv[pos + 2] = uvSrc[indices[pos + 2]];
		}
	}
	else if (uvTriangles.Num() == indices.Num()) {
		uv.SetNumZeroed(leng2 * 3);
		for (i = 0; i < leng2; i++)
		{
			pos = i * 3;
			if (uvTriangles[pos] >= lengSrc || uvTriangles[pos + 1] >= lengSrc || uvTriangles[pos + 2] >= lengSrc) {
				continue;
			}
			uv[pos] = uvSrc[uvTriangles[pos]];
			uv[pos + 1] = uvSrc[uvTriangles[pos + 1]];
			uv[pos + 2] = uvSrc[uvTriangles[pos + 2]];
		}
	}
	if (textDebug != nullptr) {
		textDebug->SetText(FText::FromString(textDebug->Text.ToString() + FString::Printf(TEXT("\nSetUV()"))));
	}
}
bool MeshUtil::SameUV(const TArray<FVector2D>& uv)
{
	if (uv.Num() <= 0) {
		return true;
	}
	int i = 0;
	int leng = uv.Num();
	FVector2D uvi0 = FVector2D(uv[0].X, uv[0].Y);
	for (i = 0; i < leng; i++) {
		if (uv[i].X != uvi0.X || uv[i].Y != uvi0.Y) {
			return false;
		}
	}
	return true;
}

void MeshUtil::AutoUV(FRawMesh& _rawMesh)
{
	if ( _rawMesh.VertexPositions.Num() <= 0) {
		return;
	}

	int leng = _rawMesh.VertexPositions.Num();
	TArray<FVector>& point = _rawMesh.VertexPositions;
	TArray<FVector2D> uv; uv.SetNumZeroed(leng);
	FVector boundsMin;
	FVector boundsMax;
	for (int i = 0; i < leng; i++) {
		FVector& pt = point[i];
		if (i == 0) {
			boundsMin = pt;
			boundsMax = pt;
		}
		else {
			if (boundsMin.X > pt.X) boundsMin.X = pt.X;
			if (boundsMin.Y > pt.Y) boundsMin.Y = pt.Y;
			if (boundsMin.Z > pt.Z) boundsMin.Z = pt.Z;
			if (boundsMax.X < pt.X) boundsMax.X = pt.X;
			if (boundsMax.Y < pt.Y) boundsMax.Y = pt.Y;
			if (boundsMax.Z < pt.Z) boundsMax.Z = pt.Z;
		}
	}

	float centx = (boundsMin.X + boundsMax.X) * 0.5f;
	float centy = (boundsMin.Y + boundsMax.Y) * 0.5f;
	float centz = (boundsMin.Z + boundsMax.Z) * 0.5f;
	float dx = boundsMax.X - boundsMin.X;
	float dy = boundsMax.Y - boundsMin.Y;
	float dz = boundsMax.Z - boundsMin.Z;
	float r = FMath::Max(0.001f, FMath::Max(dx, FMath::Max(dy, dz)) * 0.5f);
	float x0;
	float y0;
	float z0;

	for (int i = 0; i < leng; i++) {
		x0 = point[i].X - centx;
		y0 = point[i].Y - centy;
		z0 = point[i].Z - centz;

		if (x0 != 0 || z0 != 0) {
			uv[i] = FVector2D(FMath::Max(0.01f, FMath::Min(0.99f, FMath::Abs(FMath::Atan2(x0, y0) / PI))), 0.5f - FMath::Asin(z0 / r) / PI);
		}
		else {
			uv[i] = FVector2D(0.5f, 0.5f - FMath::Asin(z0 / r) / PI);
		}
	}
	MeshUtil::SetUV(_rawMesh, uv, TArray<uint32>());
}

void MeshUtil::CreateTangents(FRawMesh& _rawMesh) {
	int indexNum = _rawMesh.WedgeIndices.Num();
	if (_rawMesh.WedgeTangentZ.Num() != indexNum) return;
	if (_rawMesh.WedgeTexCoords[0].Num() != indexNum) return;
	if (_rawMesh.WedgeTangentX.Num() != indexNum) _rawMesh.WedgeTangentX.SetNumZeroed(indexNum);
	if (_rawMesh.WedgeTangentY.Num() != indexNum) _rawMesh.WedgeTangentY.SetNumZeroed(indexNum);


	float du1;//某点uv与其相邻点uv的差，即uv向量
	float dv1;
	float du2;
	float dv2;
	float du3;
	float dv3;
	float vx1; float vy1; float vz1; //某点坐标与其相邻点坐标的差，即坐标向量
	float vx2; float vy2; float vz2;
	float vx3; float vy3; float vz3; 
	float coordtx = 0;//平面上某点的沿u或v方向归一化切向量，为坐标向量矩阵左除以uv向量矩阵的值的归一化，然后有多个的取平均
	float coordty = 0;
	float coordtz = 0;
	int i, j, k;

	TArray<uint32>& arrc = _rawMesh.WedgeIndices;
	TArray<FVector>& vertices = _rawMesh.VertexPositions;
	TArray<FVector2D>& uv = _rawMesh.WedgeTexCoords[0];

	TArray<FVector>& vtangentx = _rawMesh.WedgeTangentX;
	TArray<FVector>& vtangenty = _rawMesh.WedgeTangentY;


	for (i = 0; i+2 < indexNum; i+=3) {
		//算出坐标向量和uv向量
		j = arrc[i];
		k = arrc[i + 1];
		vx1 = vertices[k].X - vertices[j].X;
		vy1 = vertices[k].Y - vertices[j].Y;
		vz1 = vertices[k].Z - vertices[j].Z;
		du1 = uv[i+1].X - uv[i].X;
		dv1 = uv[i+1].Y - uv[i].Y;

		j = arrc[i];
		k = arrc[i + 2];
		vx2 = vertices[k].X - vertices[j].X;
		vy2 = vertices[k].Y - vertices[j].Y;
		vz2 = vertices[k].Z - vertices[j].Z;
		du2 = uv[i+2].X - uv[i].X;
		dv2 = uv[i+2].Y - uv[i].Y;

		j = arrc[i + 1];
		k = arrc[i + 2];
		vx3 = vertices[k].X - vertices[j].X;
		vy3 = vertices[k].Y - vertices[j].Y;
		vz3 = vertices[k].Z - vertices[j].Z;
		du3 = uv[i+2].X - uv[i+1].X;
		dv3 = uv[i+2].Y - uv[i+1].Y;

		//算出每个点的切向量的平均值，为保持前后向量的旋转方向一致（顺时针），有些位置向量和uv向量取反，即uv切向量递减
		j = arrc[i];
		coordtx = dv2 * vx1 - dv1 * vx2;
		coordty = dv2 * vy1 - dv1 * vy2;
		coordtz = dv2 * vz1 - dv1 * vz2;
		vtangentx[i] = -FVector(coordtx, coordty, coordtz).GetSafeNormal();

		coordtx = -du2 * vx1 + du1 * vx2;
		coordty = -du2 * vy1 + du1 * vy2;
		coordtz = -du2 * vz1 + du1 * vz2;
		vtangenty[i] = -FVector(coordtx, coordty, coordtz).GetSafeNormal();


		coordtx = dv3 * vx1 - dv1 * vx3;
		coordty = dv3 * vy1 - dv1 * vy3;
		coordtz = dv3 * vz1 - dv1 * vz3;
		vtangentx[i+1] = -FVector(coordtx, coordty, coordtz).GetSafeNormal();

		coordtx = -du3 * vx1 + du1 * vx3;
		coordty = -du3 * vy1 + du1 * vy3;
		coordtz = -du3 * vz1 + du1 * vz3;
		vtangenty[i+1] = -FVector(coordtx, coordty, coordtz).GetSafeNormal();

		j = arrc[i + 2];
		coordtx = dv3 * vx2 - dv2 * vx3;
		coordty = dv3 * vy2 - dv2 * vy3;
		coordtz = dv3 * vz2 - dv2 * vz3;
		vtangentx[i+2] = -FVector(coordtx, coordty, coordtz).GetSafeNormal();

		coordtx = -du3 * vx2 + du2 * vx3;
		coordty = -du3 * vy2 + du2 * vy3;
		coordtz = -du3 * vz2 + du2 * vz3;
		vtangenty[i+2] = -FVector(coordtx, coordty, coordtz).GetSafeNormal();
	}
}

void MeshUtil::ValidateMesh(FRawMesh& _rawMesh, bool defaultSmooth) {
	int indexNum = _rawMesh.WedgeIndices.Num();
	int faceNum = indexNum / 3;
	
	if(_rawMesh.FaceMaterialIndices.Num()!=faceNum) _rawMesh.FaceMaterialIndices.SetNumZeroed(faceNum);
	if(_rawMesh.FaceSmoothingMasks.Num()!=faceNum) _rawMesh.FaceSmoothingMasks.Init(defaultSmooth?1:0, faceNum);
	if(_rawMesh.WedgeTangentX.Num()!=indexNum) _rawMesh.WedgeTangentX.SetNumZeroed(indexNum);
	if(_rawMesh.WedgeTangentY.Num()!=indexNum) _rawMesh.WedgeTangentY.SetNumZeroed(indexNum);
	if(_rawMesh.WedgeTangentZ.Num()!=indexNum) _rawMesh.WedgeTangentZ.SetNumZeroed(indexNum);
	if(_rawMesh.WedgeColors.Num()!=indexNum) _rawMesh.WedgeColors.SetNumZeroed(indexNum);
	if(_rawMesh.WedgeTexCoords[0].Num()!=indexNum) _rawMesh.WedgeTexCoords[0].SetNumZeroed(indexNum);
}

/*UStaticMesh* MeshUtil::GenerateUStaticMeshOld(FRawMesh& _rawMesh, wchar_t** outMessage, bool defaultSmooth, bool recomputeTangents) {
	UStaticMesh* mesh = NewObject<UStaticMesh>();
	mesh->PreEditChange(nullptr);
	int indexNum = _rawMesh.WedgeIndices.Num();
	FStaticMeshSourceModel& srcModel = mesh->AddSourceModel();
	srcModel.BuildSettings.bRecomputeNormals = _rawMesh.WedgeTangentZ.Num()!=indexNum;
	srcModel.BuildSettings.bRecomputeTangents = recomputeTangents || _rawMesh.WedgeTangentX.Num() != indexNum || _rawMesh.WedgeTangentY.Num() != indexNum;
	srcModel.BuildSettings.bRemoveDegenerates = false;
	//srcModel.BuildSettings.bUseHighPrecisionTangentBasis = bUseHighPrecisionTangents;
	//srcModel.BuildSettings.bUseFullPrecisionUVs = bUseFullPrecisionUVs;
	//srcModel.BuildSettings.bGenerateLightmapUVs = true;
	srcModel.BuildSettings.SrcLightmapIndex = 0;
	srcModel.BuildSettings.DstLightmapIndex = 1;


	MeshUtil::ValidateMesh(_rawMesh, defaultSmooth);
	srcModel.SaveRawMesh(_rawMesh);

	TArray<FText> BuildErrors;
	mesh->Build(true, &BuildErrors);

	if (outMessage != nullptr) {
		FString BuildErrorsString = FText::Join(FText::FromString(L"\n"), BuildErrors).ToString();
		*outMessage = (wchar_t *)(*BuildErrorsString);
	}
	//FAssetRegistryModule::AssetCreated(mesh);
	return mesh;
}*/





UStaticMesh* MeshUtil::GenerateUStaticMesh(FRawMesh& _rawMesh, bool defaultSmooth, bool recomputeTangents, bool generateCollider) {
	UStaticMesh* mesh = NewObject<UStaticMesh>();
	
	MeshUtil::ValidateMesh(_rawMesh, defaultSmooth);

	if (textDebug != nullptr) {
		textDebug->SetText(FText::FromString(textDebug->Text.ToString() + FString::Printf(TEXT("\n_rawMesh_Count:%d"), _rawMesh.VertexPositions.Num())));
	}


	UStaticMeshDescription* staticMeshDescription = UStaticMesh::CreateStaticMeshDescription();
	FStaticMeshAttributes StaticMeshAttributes(staticMeshDescription->GetMeshDescription());
	StaticMeshAttributes.Register();
	TMap<int32, FName> MaterialMap;
	/*for (int32 MaterialIndex = 0; MaterialIndex < mesh->StaticMaterials.Num(); ++MaterialIndex)
	{
		FName MaterialName = mesh->StaticMaterials[MaterialIndex].ImportedMaterialSlotName;
		if (MaterialName == NAME_None)
		{
			MaterialName = *(TEXT("MaterialSlot_") + FString::FromInt(MaterialIndex));
		}
		MaterialMap.Add(MaterialIndex, MaterialName);
	}*/
	MaterialMap.Add(0, FName(TEXT("MaterialSlot_0")));
	FStaticMeshOperations::ConvertFromRawMesh(_rawMesh, staticMeshDescription->GetMeshDescription(), MaterialMap, !recomputeTangents);

	if (textDebug != nullptr) {
		textDebug->SetText(FText::FromString(textDebug->Text.ToString()+FString::Printf(TEXT("\nmeshDescription_Count:%d"), staticMeshDescription->GetMeshDescription().Polygons().Num())));
	}

	


	FStaticMaterial mat(nullptr, TEXT("MaterialSlot_0"));
	mesh->StaticMaterials.Add(mat);


	TArray<UStaticMeshDescription*> staticMeshDescriptions;
	staticMeshDescriptions.Emplace(staticMeshDescription);

	if (textDebug != nullptr) {
		textDebug->SetText(FText::FromString(textDebug->Text.ToString() + FString::Printf(TEXT("\nBefore Build"))));
	}

	mesh->BuildFromStaticMeshDescriptions(staticMeshDescriptions,generateCollider);

	return mesh;
}

void MeshUtil::UpdateToUProcdualMesh(UProceduralMeshComponent* component, FRawMesh& _rawMesh, bool defaultSmooth, bool recomputeTangents, bool generateCollider) {
	if (textDebug != nullptr) {
		textDebug->SetText(FText::FromString(textDebug->Text.ToString() + FString::Printf(TEXT("\n_rawMesh_Count:%d"), _rawMesh.VertexPositions.Num())));
	}

	MeshUtil::ValidateMesh(_rawMesh, defaultSmooth);
	int num = _rawMesh.WedgeIndices.Num();

	uint32 vnum = _rawMesh.VertexPositions.Num();

	TArray<FVector> vertices;
	TArray<int32> triangles;
	TArray<FVector>& vertices0 = _rawMesh.VertexPositions;
	TArray<uint32>& triangles0 = _rawMesh.WedgeIndices;

	vertices.SetNumUninitialized(num);
	triangles.SetNumUninitialized(num);
	int lastValidIndex = 0;
	for (int i = 0; i < num; i++) {
		//triangles[i] = triangles0[i];
		if (triangles0[i] < vnum) {
			vertices[i] = vertices0[triangles0[i]];
			triangles[i] = i;
			//triangles[i] = i%3==0?i:(i/3*3)+(3-i%3);
			lastValidIndex = i;
		}
		else {
			vertices[i] = FVector::ZeroVector;
			triangles[i] = lastValidIndex;
		}		
	}
	TArray<FProcMeshTangent> tangent;

	if (recomputeTangents) {
		tangent.Init(FProcMeshTangent(1, 0, 0), num);
		CreateTangents(_rawMesh);
		for (int i = 0; i < num; i++) {
			tangent[i] = FProcMeshTangent(_rawMesh.WedgeTangentX[i], false);
		}
	}
	

	if (textDebug != nullptr) {
		textDebug->SetText(FText::FromString(textDebug->Text.ToString() + FString::Printf(TEXT("\nBefore Create"))));
	}


	component->ClearAllMeshSections();
	component->CreateMeshSection(0, vertices, triangles, _rawMesh.WedgeTangentZ, _rawMesh.WedgeTexCoords[0], _rawMesh.WedgeColors, tangent, generateCollider);
}