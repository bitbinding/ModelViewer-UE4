// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include <string>
#include <vector>
#include <map>



#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "RawMesh/Public/RawMesh.h"
#include <Runtime/UMG/Public/Components/TextBlock.h>
#include "Model3d.generated.h"

/**
 * 
 */
UCLASS()
class MODELVIEWER_API UModel3d : public UObject
{
	GENERATED_BODY()
	
private:
	const int functionMaxLeng = 1001;


	

	/// <summary>
	/// f(x,y)的数值和返回地址表
	/// </summary>
	std::vector<int> num1;

	/// <summary>
	/// f(x,y)的运算符表（为正时表示用的是数值，为负时表示用的是链接地址，为零时表示返回或结束）
	/// </summary>
	std::vector<int> oper1;


	/// <summary>
	/// g(x,y)的数值和返回地址表
	/// </summary>
	std::vector<int> num2;

	/// <summary>
	/// g(x,y)的运算符表（为正时表示用的是数值，为负时表示用的是链接地址，为零时表示返回或结束）
	/// </summary>
	std::vector<int> oper2;

	/// <summary>
	/// h(x,y)的数值和返回地址表
	/// </summary>
	std::vector<int> num3;

	/// <summary>
	/// h(x,y)的运算符表（为正时表示用的是数值，为负时表示用的是链接地址，为零时表示返回或结束）
	/// </summary>
	std::vector<int> oper3;

	/// <summary>
	/// f(x,y)的常量表
	/// </summary>
	std::vector<float> con1;

	/// <summary>
	/// g(x,y)的常量表
	/// </summary>
	std::vector<float> con2;

	/// <summary>
	/// h(x,y)的常量表
	/// </summary>
	std::vector<float> con3;

	const float stackMaxLeng = 50;

	/// <summary>
	/// 公式编译用的字符串索引“栈”（表左括号后第一个字符的索引）
	/// </summary>
	std::vector<int> stack0;

	/// <summary>
	/// 公式编译用的返回地址“栈”（表括号内运算后返回的（数值和返回地址表）的地址）
	/// </summary>
	std::vector<int> stack1;

	/// <summary>
	/// 公式执行时用的“栈”
	/// </summary>
	std::vector<float> stackf;

	/// <summary>
	/// 编译用的“栈”顶端索引
	/// </summary>
	int stackTop = -1;

	std::vector<int> numLengArray{ 0,0,0 };
	std::vector<int> conLengArray{ 0,0,0 };
	//var strkh:String;//加了括号的字符串
	std::string strFunc1 = "";//f(x)的公式字符串
	std::string strFunc2 = "";//g(x)的公式字符串
	std::string strFunc3 = "";//h(x)的公式字符串
	int xseg; int yseg;//x方向分段，y方向分段
	std::string funcName = "";//函数全名
	std::vector<float> cnt;//常量
	int drawType = 0;//函数呈现方式
	std::string strResult1 = "empty";//函数编译结果情况
	std::string strResult2 = "empty";
	std::string strResult3 = "empty";
	//float scaleShape = 1.0f;//缩放比例
	float scaleU = 1.0f;//u贴图坐标缩放比例
	float scaleV = 1.0f;//v贴图坐标缩放比例
	bool uclosed = false;//模型为参数方程时，沿贴图u方向闭合
	bool vclosed = false;//模型为参数方程时，沿贴图v方向闭合
	bool usame0 = false;//模型为参数方程时，沿贴图u方向闭合
	bool usamet = false;//模型为参数方程时，沿贴图v方向闭合
	bool vsame0 = false;//模型为参数方程时，沿贴图u方向闭合
	bool vsamet = false;//模型为参数方程时，沿贴图v方向闭合
	int NaNMode = 2;//非数值的处理方式
	float modelHeight = 0;
	float minz = 0;
	float maxz = -1;

	float ordinaryFunctionRange = 10.0f;
	float parameticFunctionRange = 6.2831853f;
	float ordinaryFunctionScale = 1;
	float parameticFunctionScale = 1;

public:
	FRawMesh m_rawMesh;
	static UPackage* MeshPackage;

	UModel3d();
	~UModel3d();

	/// <summary>
	/// 初始化方法
	/// </summary>
	/// <param name="funcName0">静态网格名称</param>
	/// <param name="funcName0">函数名称</param>
	/// <param name="xseg0">u方向分段数</param>
	/// <param name="yseg0">v方向分段数</param>
	/// <param name="ordinaryFunctionRange0">非参数方程的定义域范围（关于原点对称）</param>
	/// <param name="ordinaryFunctionScale0">非参数方程的缩放系数</param>
	/// <param name="parameticFunctionScale0">参数方程的缩放系数</param>
	/// <param name="scaleU0">uv贴图中，u方向的缩放系数</param>
	/// <param name="scaleV0">uv贴图中，v方向的缩放系数</param>
	void UpdateFunction(const std::string& funcName0 = "", int xseg0 = 100, int yseg0 = 100, float ordinaryFunctionRange0 = 10.0f, float ordinaryFunctionScale0 = 1.0f, float parameticFunctionScale0 = 1.0f, float scaleU0 = 1, float scaleV0 = 1);


	std::string getFunctionName();

private:

	std::string readstring(const std::string& strFunc0, int numLengid = 1);

	std::string compileWithStack(const std::string& strFunc, std::vector<int>& num, std::vector<int>& oper, std::vector<float>& con, int numConLengId);
public:
	float CalculateArray(float xt, float yt, int pos = 0);

private:
	float CalculateArray(float xt, float yt, std::vector<int>& num, std::vector<int>& oper, std::vector<float>& con, int numConLengId);

public:
	void PlotGrid(bool updateCoord0 = true, bool willCreateN = true);




	/// <summary>
	/// 生成球
	/// </summary>
	static UModel3d* Sphere(float r, int jseg = 50, int wseg = 50, bool invertU = false);

	/// <summary>
	/// 生成圆锥侧面
	/// </summary>
	static UModel3d* Cone(float r, float h, int jseg = 50, int wseg = 50);

	/// <summary>
	/// 生成圆台侧面
	/// </summary>
	static UModel3d* ConeEx(float rtop, float rbottom, float h, int jseg = 50, int wseg = 50);
	/// <summary>
	/// 生成圆柱侧面
	/// </summary>
	static UModel3d* Cylinder(float r, float h, int jseg = 50, int wseg = 50);

	/// <summary>
	/// 生成圆环
	/// </summary>
	static UModel3d* Ring(float r1, float r2, int jseg = 50, int wseg = 50);

	/// <summary>
	/// 生成四边形ABCD构成的平面
	/// </summary>
	static void Plane(FRawMesh &rawMesh, FVector a, FVector b, FVector c, FVector d, int xseg0 = 1, int yseg0 = 1, float scaleU0 = 1, float scaleV0 = 1);

	/// <summary>
	/// 生成圆盘，圆盘的分段必须为偶数段
	/// </summary>
	static void RoundPlane(FRawMesh &rawMesh, float r, int seg = 50);

	static void Cuboid(FRawMesh &rawMesh, float a0, float b0, float c0, bool verticalUV = false, bool invertU = false);

	static void SetTextDebug(UTextBlock* textBlock);
};
