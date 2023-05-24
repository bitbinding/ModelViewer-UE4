// Fill out your copyright notice in the Description page of Project Settings.

#pragma once


#include "CoreMinimal.h"
#include "RawMesh/Public/RawMesh.h"

#include <Runtime/UMG/Public/Components/TextBlock.h>
#include <ProceduralMeshComponent.h>
/**
 * 
 */
class MeshUtil
{
public:
	MeshUtil();
	~MeshUtil();

	/// <summary>
	/// 每个模型的最大顶点数
	/// </summary>
	static const int verticeLimit = 65535;

	/// <summary>
	/// 创建顶点法向量
	/// </summary>
	/// <param name="_rawMesh">模型网格</param>
	/// <param name="externalTriangles">外部的顶点三角形索引数据，顶点索引值可超过65535</param>
	/// <param name="smoothAngleMax">面夹角不超过这个角度时进行平滑</param>
	static void CreateNormal(FRawMesh& _rawMesh, float smoothAngleMax=180);

	/// <summary>
	/// 附加外部顶点法向量
	/// </summary>
	/// <param name="_rawMesh">模型（包含顶点数据）</param>
	/// <param name="pointnSrc">源顶点法向量值</param>
	/// <param name="nindex">源顶点法向量的独立三角形索引</param>
	/// <param name="externalTriangles">外部的顶点三角形索引数据，顶点索引值可超过65535</param>
	static void AttachExternalNormalData(FRawMesh& _rawMesh, const TArray<FVector>& pointnSrc, const TArray<uint32>& nindex);

	/// <summary>
	/// 当uv使用不同的三角形索引时，将共用顶点分成多个，来让uv和顶点用到相同的三角形索引
	/// </summary>
	/// <param name="_rawMesh">模型网格</param>
	/// <param name="uvSrc">源uv值</param>
	/// <param name="uvTriangles">源uv的独立三角形索引(不需要时设为空)</param>
	static void SetUV(FRawMesh& _rawMesh, const TArray<FVector2D>& uvSrc, const TArray<uint32>& uvTriangles);

	/// <summary>
	/// 判断所有的uv值是否相同
	/// </summary>
	/// <param name="uv">uv数组</param>
	static bool SameUV(const TArray<FVector2D>& uv);

	/// <summary>
	/// 按球形自动生成形体的uv
	/// </summary>
	/// <param name="_rawMesh">模型网格</param>
	static void AutoUV(FRawMesh& _rawMesh);


	/// <summary>
	/// 生成切向量
	/// </summary>
	static void CreateTangents(FRawMesh& _rawMesh);

	/// <summary>
	/// 使网格最终可用，原先空白的用一些常量填充
	/// </summary>
	/// <param name="_rawMesh">模型网格</param>
	/// <param name="defaultSmooth">没有指定法向量时，是否应用平滑</param>
	static void ValidateMesh(FRawMesh& _rawMesh, bool defaultSmooth=true);

	/// <summary>
	/// 生成UStaticMesh
	/// </summary>
	/// <param name="_rawMesh">模型网格</param>
	/// <param name="defaultSmooth">没有指定法向量时，是否应用平滑</param>
	/// <param name="recomputeTangents">是否需要重新生成切向量</param>
	/// <returns></returns>
	static UStaticMesh* GenerateUStaticMesh(FRawMesh& _rawMesh, bool defaultSmooth=true, bool recomputeTangents=true, bool generateCollider=false);
	static void UpdateToUProcdualMesh(UProceduralMeshComponent *component, FRawMesh& _rawMesh, bool defaultSmooth = true, bool recomputeTangents = true, bool generateCollider=false);

	static void SetTextDebug(UTextBlock* textBlock);
};
