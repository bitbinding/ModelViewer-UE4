

#include "Model3d.h"
//using namespace std;

#include "ue_import/commonutils.h"
#include "MeshUtil.h"

const float NaN = std::numeric_limits<float>::quiet_NaN();

static UTextBlock* textDebug = nullptr;

void UModel3d::SetTextDebug(UTextBlock* textBlock)
{
	textDebug = textBlock;
}

UModel3d::UModel3d() {
}

UModel3d::~UModel3d() {
}

void UModel3d::UpdateFunction(const std::string& funcName0, int xseg0, int yseg0, float ordinaryFunctionRange0, float ordinaryFunctionScale0, float parameticFunctionScale0, float scaleU0, float scaleV0)
{
	if (textDebug != nullptr) {
		textDebug->SetText(FText::FromString(textDebug->Text.ToString() + FString::Printf(TEXT("\nUpdateFunction():%d"),funcName0.size())));
	}
	float a = 0; float b = 1; float c = 1; float d = 0.61803398874989484820f; float e = std::exp(1); float f = 0.5772156649015328f;
	funcName = funcName0;
	ordinaryFunctionScale = ordinaryFunctionScale0;
	parameticFunctionScale = parameticFunctionScale0;
	xseg = xseg0;
	yseg = yseg0;
	cnt = std::vector<float>{ a,b,c,d,e,f };

	numLengArray= std::vector<int>{ 0,0,0 };
	conLengArray= std::vector<int>{ 0,0,0 };


	int lf = functionMaxLeng;
	num1 = std::vector<int>(lf);//f(x,y)的数值和返回地址表
	oper1 = std::vector<int>(lf);//f(x,y)的运算符表（为正时表示用的是数值，为负时表示用的是链接地址，为零时表示返回或结束）

	num2 = std::vector<int>(lf);//g(x,y)的数值和返回地址表
	oper2 = std::vector<int>(lf);//g(x,y)的运算符表（为正时表示用的是数值，为负时表示用的是链接地址，为零时表示返回或结束）
	num3 = std::vector<int>(lf);//h(x,y)的数值和返回地址表
	oper3 = std::vector<int>(lf);//h(x,y)的运算符表（为正时表示用的是数值，为负时表示用的是链接地址，为零时表示返回或结束）
	con1 = std::vector<float>(lf);//f(x,y)的常量表
	con2 = std::vector<float>(lf);//g(x,y)的常量表
	con3 = std::vector<float>(lf);//h(x,y)的常量表

	int ls = functionMaxLeng;
	stack0 = std::vector<int>(ls);//公式编译用的字符串索引“栈”（表左括号后第一个字符的索引）
	stack1 = std::vector<int>(ls);//公式编译用的返回地址“栈”（表括号内运算后返回的（数值和返回地址表）的地址）
	stackf = std::vector<float>(ls);


	//this.scaleShape=scaleShape0;
	scaleU = scaleU0;
	scaleV = scaleV0;

	std::vector<std::string> func;
	splitString(func, funcName0, ",");
	drawType = func.size() <= 3 ? func.size() : 3;
	if (drawType >= 1) {
		strFunc1 = func[0];
		strFunc1 = readstring(strFunc1, 1);
		strResult1 = compileWithStack(strFunc1, num1, oper1, con1, 0);
	}
	else {
		strFunc1 = "";
	}
	//trace("______________________________");
	//trace(num1);
	//trace(oper1);
	//trace(con1);
	if (drawType >= 2) {
		strFunc2 = func[1];
		strFunc2 = readstring(strFunc2, 2);
		strResult2 = compileWithStack(strFunc2, num2, oper2, con2, 1);
	}
	else {
		strFunc2 = "";
	}
	if (drawType >= 3) {
		strFunc3 = func[2];
		strFunc3 = readstring(strFunc3, 3);
		strResult3 = compileWithStack(strFunc3, num3, oper3, con3, 2);
	}
	else {
		strFunc3 = "";
	}
	uclosed = false;
	vclosed = false;
	usame0 = false;
	usamet = false;
	vsame0 = false;
	vsamet = false;
	if (func.size() > 3) {
		uclosed = (int)func[3].find("%") == 0;
		vclosed = (int)func[3].rfind("%") > 0;
		usame0 = vclosed && (int)func[3].find("*") == 0;
		usamet = vclosed && (int)func[3].rfind("*") > 0;
		vsame0 = uclosed && (int)func[3].find("*") == 1;
		vsamet = uclosed && (int)func[3].rfind("*") > 1;
	}
	if (strFunc1 == "" || strFunc2 == "" || strFunc3 == "") {

	}
	if (textDebug != nullptr) {
		textDebug->SetText(FText::FromString(textDebug->Text.ToString() + FString::Printf(TEXT("\nBefore PlotGrid()"))));
	}
	PlotGrid(true);
	modelHeight = maxz - minz;
	//Debug.Log(CalculateArray(1f,0.0f));
	//plot();
}



std::string UModel3d::getFunctionName()
{
	return funcName;
}

std::string UModel3d::readstring(const std::string& strFunc0, int numLengid)
{
	//预编译字符串，包括加入必要的乘号
	std::string strFunc = strFunc0;
	strFunc = stringReplace(strFunc, "\r", "");
	strFunc = stringReplace(strFunc, "\n", "");
	strFunc = stringReplace(strFunc, " ", "");
	int strLeng = strFunc.size();
	int i = 0; int j = 0; std::string nstrFunc;
	//int grade=1;
	//bool numflag=false;
	nstrFunc = "";
	for (i = 0; i < strLeng; i++) {
		if (i >= 1 && (strFunc[i - 1] >= 48 && strFunc[i - 1] <= 57 || strFunc[i - 1] == '.' || strFunc[i - 1] == ')' || strFunc[i - 1] >= 65 && strFunc[i - 1] <= 70 || strFunc[i - 1] == 'x' || strFunc[i - 1] == 'y' || strFunc[i - 1] == 'P') && (strFunc[i] == 'x' || strFunc[i] == 'y' || strFunc[i] == 'P' || strFunc[i] == '(' || strFunc[i] >= 65 && strFunc[i] <= 70 || strFunc[i] >= 97 && strFunc[i] <= 119)) {
			nstrFunc += substring(strFunc, j, j + (i - j)) + '*';
			j = i;
		}
	}
	nstrFunc += substring(strFunc, j, j + (i - j));
	strFunc = nstrFunc;
	nstrFunc = "";
	strLeng = strFunc.size();
	j = 0;

	stackTop = -1;

	return strFunc;
}

std::string UModel3d::compileWithStack(const std::string& strFunc, std::vector<int>& num, std::vector<int>& oper, std::vector<float>& con, int numConLengId)
{
	//借助堆栈编译字符串，函数字符串可省略乘号和右括号
	int beginIndex = 0; int returni = -1;
	int strLeng = strFunc.size();
	numLengArray[numConLengId] = 0;
	conLengArray[numConLengId] = 0;
	int numLeng = 0;
	int stackTopPrev = 0;
	if (strFunc == "") {
		numLeng = 0; numLengArray[numConLengId] = numLeng; conLengArray[numConLengId] = 0; return "empty";
	}

	int i = beginIndex; int j = beginIndex;
	int khlevel = 0;
	bool numflag = false;
	wchar_t operstr;
	int operstrj = 0;
	int operi = 0;
	int numBegin = numLeng;
	int numLeng2 = 0;
	bool firstPlusFlag = false;
	int stackTop0 = stackTop + 1;
	int stackTopt = stackTop0;
	bool willAdjustPower = false;
	bool firstTimesFlag = false;
	int numLeng2m = 0;
	//string resultStr="";
	bool isNum = false;//判断某位是否可作为为数值、自变量或常量
	std::string strji = "";//截取后的数值字符串

	do {
		i = beginIndex;
		j = beginIndex;
		khlevel = 0;
		numflag = false;
		operstrj = 0;
		operi = 0;
		numBegin = numLeng;
		numLeng2 = 0;
		firstPlusFlag = false;
		stackTop0 = stackTop + 1;
		stackTopt = stackTop0;
		willAdjustPower = false;
		firstTimesFlag = false;
		numLeng2m = 0;
		//resultStr="";
		isNum = false;//判断某位是否可作为为数值、自变量或常量
		strji = "";//截取后的数值字符串

		if (numLeng >= functionMaxLeng - 1) {
			numLeng = 0; numLengArray[numConLengId] = numLeng; conLengArray[numConLengId] = 0; return "numLeng error";
		}
		for (i = beginIndex; i < strLeng; i++) {
			isNum = (strFunc[i] >= 48 && strFunc[i] <= 57 || strFunc[i] == '.' || strFunc[i] == 'x' || strFunc[i] == 'y' || strFunc[i] == 'P' || strFunc[i] >= 65 && strFunc[i] <= 70);
			if (khlevel == 0 && isNum) {
				if (numflag == false) {
					numflag = true;
					j = i;
				}
			}
			if (numflag && (!isNum || khlevel != 0 || i == strLeng - 1)) {
				numflag = false;
				strji = substring(strFunc, j, j + ((isNum ? i + 1 : i) - j));
				if (conLengArray[numConLengId] >= 100 - 1) {
					numLeng = 0; numLengArray[numConLengId] = numLeng; conLengArray[numConLengId] = 0; return "conLeng error";
				}

				float floatResult = 0.0f;
				if (strji == "x") {
					num[numLeng] = -1;
				}
				else if (strji == "P") {
					con[conLengArray[numConLengId]] = PI;
					num[numLeng] = conLengArray[numConLengId];
					conLengArray[numConLengId]++;
				}
				else if (!strji.empty() && strji[0]>='0' && strji[0]<='9'){
				floatResult = (float)atof(strji.c_str());
				con[conLengArray[numConLengId]] = ((float)atof(strji.c_str())/*std::stof(strji)*/);
				num[numLeng] = conLengArray[numConLengId];
				conLengArray[numConLengId]++;
				}
				else {
					num[numLeng] = -1;
				}

				if (j == beginIndex) {
					oper[numLeng] = 1;
				}
				else {
					operstr = strFunc[j - 1];
					switch (operstr) {
						case '+':oper[numLeng] = 1;
						break;
						case '-':oper[numLeng] = 2;
						break;
						case '*':oper[numLeng] = 3;
						break;
						case '/':oper[numLeng] = 4;
						break;
						case '^':oper[numLeng] = 5;
						break;
						case '%':oper[numLeng] = 6;
						break;
						/*case ')' :
								break;*/
					default:numLeng = 0; numLengArray[numConLengId] = numLeng; conLengArray[numConLengId] = 0; return "opeator error";
					}
				}
				if (strji == "y" && oper[numLeng] > 0) {
					oper[numLeng] += 8;
				}
				else if (strji == "A" && oper[numLeng] > 0) {
					oper[numLeng] += 16;
				}
				else if (strji == "B" && oper[numLeng] > 0) {
					oper[numLeng] += 24;
				}
				else if (strji == "C" && oper[numLeng] > 0) {
					oper[numLeng] += 32;
				}
				else if (strji == "D" && oper[numLeng] > 0) {
					oper[numLeng] += 40;
				}
				else if (strji == "E" && oper[numLeng] > 0) {
					oper[numLeng] += 48;
				}
				else if (strji == "F" && oper[numLeng] > 0) {
					oper[numLeng] += 56;
				}
				if (num[numLeng] < 0 && strji != "x" && oper[numLeng] < 8) {
					//未识别的数值
					numLeng = 0; numLengArray[numConLengId] = numLeng; conLengArray[numConLengId] = 0;
					return "number error";
				}
				j = i;
				numLeng++;
			}
			if (i == strLeng - 1 && !isNum && strFunc[i] != ')') {
				//函数不完整
				numLeng = 0; numLengArray[numConLengId] = numLeng; conLengArray[numConLengId] = 0;
				return "not complete error";
			}
			if (strFunc[i] == '(') {
				if (khlevel == 0) {
					num[numLeng] = -1;
					if (stackTop >= stackMaxLeng - 1) {
						numLeng = 0; numLengArray[numConLengId] = numLeng; conLengArray[numConLengId] = 0; return "stack error";
					}
					else {
						stackTop++;
						stack0[stackTop] = i + 1;
						stack1[stackTop] = numLeng;
					}
					if (i >= 6 && substring(strFunc, (i - 6), (i - 6) + 6) == "arcsin") {
						oper[numLeng] = -56;
						operstrj = i - 7;
					}
					else if (i >= 6 && substring(strFunc, (i - 6), (i - 6) + 6) == "arccos") {
						oper[numLeng] = -64;
						operstrj = i - 7;
					}
					else if (i >= 6 && substring(strFunc, (i - 6), (i - 6) + 6) == "arctan") {
						oper[numLeng] = -72;
						operstrj = i - 7;
					}
					else if (i >= 3 && substring(strFunc, (i - 3), (i - 3) + 3) == "sin") {
						oper[numLeng] = -8;
						operstrj = i - 4;
					}
					else if (i >= 3 && substring(strFunc, (i - 3), (i - 3) + 3) == "cos") {
						oper[numLeng] = -16;
						operstrj = i - 4;
					}
					else if (i >= 3 && substring(strFunc, (i - 3), (i - 3) + 3) == "tan") {
						oper[numLeng] = -24;
						operstrj = i - 4;
					}
					else if (i >= 2 && substring(strFunc, (i - 2), (i - 2) + 2) == "ln") {
						oper[numLeng] = -32;
						operstrj = i - 3;
					}
					else if (i >= 3 && substring(strFunc, (i - 3), (i - 3) + 3) == "lif") {
						oper[numLeng] = -40;
						operstrj = i - 4;
					}
					else if (i >= 4 && substring(strFunc, (i - 4), (i - 4) + 4) == "sqrt") {
						oper[numLeng] = -48;
						operstrj = i - 5;
					}
					else if (i >= 3 && substring(strFunc, (i - 3), (i - 3) + 3) == "rif") {
						oper[numLeng] = -80;
						operstrj = i - 4;
					}
					else if (i >= 3 && substring(strFunc, (i - 3), (i - 3) + 3) == "bif") {
						oper[numLeng] = -88;
						operstrj = i - 4;
					}
					else if (i >= 3 && substring(strFunc, (i - 3), (i - 3) + 3) == "abs") {
						oper[numLeng] = -96;
						operstrj = i - 4;
					}
					else if (i >= 3 && substring(strFunc, (i - 3), (i - 3) + 3) == "fif") {
						oper[numLeng] = -104;
						operstrj = i - 4;
					}
					else if (i >= 5 && substring(strFunc, (i - 5), (i - 5) + 5) == "floor") {
						oper[numLeng] = -112;
						operstrj = i - 6;
					}
					else {
						oper[numLeng] = 0;
						operstrj = i - 1;
					}
					if (operstrj < beginIndex) {
						oper[numLeng] += -1;
					}
					else {
						operstr = strFunc[operstrj];
						switch (operstr) {
							case '+':oper[numLeng] += -1;
							break;
							case '-':oper[numLeng] += -2;
							break;
							case '*':oper[numLeng] += -3;
							break;
							case '/':oper[numLeng] += -4;
							break;
							case '^':oper[numLeng] += -5;
							break;
							case '%':oper[numLeng] += -6;
							break;
							case '(':break;
						default:numLeng = 0; numLengArray[numConLengId] = numLeng; conLengArray[numConLengId] = 0; return "minus operator error";
						}
					}
					numLeng++;
					khlevel++;
				}
				else {
					khlevel++;
				}
			}
			else if (strFunc[i] == ')') {
				if (khlevel == 0) {
					num[numLeng] = returni;
					oper[numLeng] = 0;
					numLeng++;
					break;
				}
				else if (khlevel > 0) {
					khlevel--;
				}
				else {
					numLeng = 0; numLengArray[numConLengId] = numLeng; conLengArray[numConLengId] = 0;
					return "quote error";
				}
			}
			if (numLeng >= 100 - 1) {
				numLeng = 0; numLengArray[numConLengId] = numLeng; conLengArray[numConLengId] = 0; return "numLeng error";
			}
		}

		if (numLeng > 0 && oper[numLeng - 1] != 0 || numLeng == 0) {

			num[numLeng] = returni;

			oper[numLeng] = 0;
			numLeng++;

		}
		numLeng2 = numLeng;
		if (numLeng >= 100 - 1) {
			numLeng = 0; numLengArray[numConLengId] = numLeng; conLengArray[numConLengId] = 0; return "numLeng error";
		}
		numLeng2 = numLeng;
		firstPlusFlag = true;
		firstTimesFlag = true;
		//trace(numLeng);
		for (i = numBegin; i < numLeng; i++) {//抽取运算主干。抽取的是加减法，和第一项无取反运算时该项的乘除法，并将操作数改成抽取源的位置编号。
			operi = oper[i] >= 0 ? (oper[i] & 7) : (oper[i] | 120);
			if (operi == 1 && i > numBegin || operi == 2 || operi == -1 && i > numBegin || operi == -2) {
				num[numLeng2] = i;
				//oper[numLeng2]=oper[i]>=0?oper[i]+(j<<7):oper[i]-(j<<7);
				oper[numLeng2] = oper[i];
				numLeng2++;
				firstPlusFlag = false;
			}
			else if (operi == 1 || operi == -1 || (operi == 3 || operi == 4 || operi == 5 || operi == 6 || operi == -3 || operi == -4 || operi == -5 || operi == -6) && firstPlusFlag || operi == 0) {
				num[numLeng2] = i;
				oper[numLeng2] = oper[i];
				numLeng2++;
			}
			if (operi == 1 || operi == -1 || operi == 2 || operi == -2 || operi == 0) {
				firstTimesFlag = true;
			}
			else if (operi == 3 || operi == 4 || operi == 6 || operi == -3 || operi == -4 || operi == -6) {
				firstTimesFlag = false;
			}
			else if (!firstTimesFlag && (operi == 5 || operi == -5)) {
				willAdjustPower = true;
			}
			if (numLeng2 >= 100 - 1) {
				numLeng = 0; numLengArray[numConLengId] = numLeng; conLengArray[numConLengId] = 0; return "numLeng2 error";
			}
		}
		//cal1Count=((numLeng2-numLeng)>>1);
		numLeng2m = numLeng2;
		firstPlusFlag = true;
		for (i = numLeng; i < numLeng2m && i < 100 - 2; i++) {//i++
			//j=oper[i]>=0?oper[i]>>7:(-oper[i])>>7;
			//oper+=oper[i]>0?-(j<<7):j<<7;
			j = (int)num[i];
			operi = oper[i] >= 0 ? (oper[i] & 7) : (oper[i] | 120);
			if (firstPlusFlag && (operi == 1 && i > numLeng || operi == 2 || operi == -1 && i > numLeng || operi == -2)) {
				firstPlusFlag = false;
			}
			operi = oper[j + 1] >= 0 ? (oper[j + 1] & 7) : (oper[j + 1] | 120);
			if (!firstPlusFlag && (operi == 3 || operi == 4 || operi == 5 || operi == 6 || operi == -3 || operi == -4 || operi == -5 || operi == -6)) {
				if (oper[i] > 0) {
					num[i] = numLeng2 - numLeng + numBegin;
					num[numLeng2] = num[j];
					oper[numLeng2] = ((oper[i] & 120) | 1);
					oper[i] = -(oper[i] & 7);//
					numLeng2++;
				}
				else if (oper[i] < 0) {
					num[i] = numLeng2 - numLeng + numBegin;
					num[numLeng2] = num[j];
					oper[numLeng2] = (oper[i] | 7);
					oper[i] = (oper[i] | 120);
					while (stackTopt <= stackTop) {
						if (stack1[stackTopt] == j) {
							stack1[stackTopt] = (int)num[i];
							break;
						}
						stackTopt++;
					}
					numLeng2++;
				}
				else {
					num[i] = num[j];
					break;
				}
				if (numLeng2 >= functionMaxLeng - 1) {
					numLeng = 0; numLengArray[numConLengId] = numLeng; conLengArray[numConLengId] = 0; return "numLeng2 error";
				}
				operi = oper[j + 1] >= 0 ? (oper[j + 1] & 7) : (oper[j + 1] | 120);
				firstTimesFlag = true;
				for (j = j + 1; operi == 3 || operi == 4 || operi == 5 || operi == 6 || operi == -3 || operi == -4 || operi == -5 || operi == -6; j++) {
					num[numLeng2] = num[j];
					oper[numLeng2] = oper[j];
					while (operi < 0 && stackTopt <= stackTop) {
						if (stack1[stackTopt] == j) {
							stack1[stackTopt] = numLeng2 - numLeng + numBegin;
							break;
						}
						stackTopt++;
					}
					numLeng2++;
					if (numLeng2 >= functionMaxLeng - 1) {
						numLeng = 0; numLengArray[numConLengId] = numLeng; conLengArray[numConLengId] = 0; return "numLeng2 error";
					}
					operi = oper[j + 1] >= 0 ? (oper[j + 1] & 7) : (oper[j + 1] | 120);
				}
				num[numLeng2] = i - numLeng + numBegin;
				oper[numLeng2] = 0;
				numLeng2++;
				if (numLeng2 >= functionMaxLeng - 1) {
					numLeng = 0; numLengArray[numConLengId] = numLeng; conLengArray[numConLengId] = 0; return "numLeng2 error";
				}
			}
			else {
				num[i] = num[j];
			}
			/*if (oper[i+1]==0) {
						num[i+1]=num[num[i+1]];
					}*/
		}
		for (i = numLeng, j = numBegin; i < numLeng2; i++, j++) {
			//if (oper[i]==7) {
			//i++;
			//}
			num[j] = num[i];
			oper[j] = oper[i];
		}
		numLeng = j;
		if (willAdjustPower) {
			numLeng2 = numLeng;
			stackTopt = stackTop0;
			firstTimesFlag = true;
			//trace(numLeng);
			for (i = numBegin; i < numLeng; i++) {//抽取运算主干。
				operi = oper[i] >= 0 ? (oper[i] & 7) : (oper[i] | 120);
				if (operi == 1 || operi == -1 || operi == 2 || operi == -2) {
					num[numLeng2] = i;
					oper[numLeng2] = oper[i];
					numLeng2++;
					firstTimesFlag = true;
				}
				else if (operi == 3 || operi == 4 || operi == 6 || operi == -3 || operi == -4 || operi == -6) {
					num[numLeng2] = i;
					oper[numLeng2] = oper[i];
					numLeng2++;
					firstTimesFlag = false;
				}
				else if (firstTimesFlag && (operi == 5 || operi == -5) || operi == 0) {
					num[numLeng2] = i;
					oper[numLeng2] = oper[i];
					numLeng2++;
				}
				if (numLeng2 >= functionMaxLeng - 1) {
					numLeng = 0; numLengArray[numConLengId] = numLeng; conLengArray[numConLengId] = 0; return "numLeng2 error";
				}
			}

			numLeng2m = numLeng2;
			firstTimesFlag = true;
			for (i = numLeng; i < numLeng2m && i < functionMaxLeng - 2; i++) {//i++
				//j=oper[i]>=0?oper[i]>>7:(-oper[i])>>7;
				//oper+=oper[i]>0?-(j<<7):j<<7;
				j = (int)num[i];
				operi = oper[i] >= 0 ? (oper[i] & 7) : (oper[i] | 120);
				if (operi == 1 || operi == -1 || operi == 2 || operi == -2) {
					firstTimesFlag = true;
				}
				else if (operi == 3 || operi == 4 || operi == 6 || operi == -3 || operi == -4 || operi == -6) {
					firstTimesFlag = false;
				}
				operi = oper[j + 1] >= 0 ? (oper[j + 1] & 7) : (oper[j + 1] | 120);
				if (!firstTimesFlag && (operi == 5 || operi == -5)) {
					if (oper[i] > 0) {
						num[i] = numLeng2 - numLeng + numBegin;
						num[numLeng2] = num[j];
						oper[numLeng2] = ((oper[i] & 120) | 1);
						oper[i] = -(oper[i] & 7);//
						numLeng2++;
					}
					else if (oper[i] < 0) {
						num[i] = numLeng2 - numLeng + numBegin;
						num[numLeng2] = num[j];
						oper[numLeng2] = (oper[i] | 7);
						oper[i] = (oper[i] | 120);
						stackTopt = stackTop0;
						while (stackTopt <= stackTop) {
							if (stack1[stackTopt] == j) {
								stack1[stackTopt] = (int)num[i];
								break;
							}
							stackTopt++;
						}
						numLeng2++;
					}
					else {
						num[i] = num[j];
						break;
					}
					if (numLeng2 >= functionMaxLeng - 1) {
						numLeng = 0; numLengArray[numConLengId] = numLeng; conLengArray[numConLengId] = 0; return "numLeng2 error";
					}
					operi = oper[j + 1] >= 0 ? (oper[j + 1] & 7) : (oper[j + 1] | 120);
					firstTimesFlag = true;
					for (j = j + 1; operi == 5 || operi == -5; j++) {
						num[numLeng2] = num[j];
						oper[numLeng2] = oper[j];
						stackTopt = stackTop0;
						while (operi < 0 && stackTopt <= stackTop) {
							if (stack1[stackTopt] == j) {
								stack1[stackTopt] = numLeng2 - numLeng + numBegin;
								break;
							}
							stackTopt++;
						}
						numLeng2++;
						if (numLeng2 >= functionMaxLeng - 1) {
							numLeng = 0; numLengArray[numConLengId] = numLeng; conLengArray[numConLengId] = 0; return "numLeng2 error";
						}
						operi = oper[j + 1] >= 0 ? (oper[j + 1] & 7) : (oper[j + 1] | 120);
					}
					num[numLeng2] = i - numLeng + numBegin;
					oper[numLeng2] = 0;
					numLeng2++;
					if (numLeng2 >= functionMaxLeng - 1) {
						numLeng = 0; numLengArray[numConLengId] = numLeng; conLengArray[numConLengId] = 0; return "numLeng2 error";
					}
				}
				else {
					num[i] = num[j];
				}
				/*if (oper[i+1]==0) {
							num[i+1]=num[num[i+1]];
						}*/
			}
			for (i = numLeng, j = numBegin; i < numLeng2; i++, j++) {
				//if (oper[i]==7) {
				//i++;
				//}
				num[j] = num[i];
				oper[j] = oper[i];
			}
			numLeng = j;
		}
		stackTopPrev = stackTop;
		if (stackTop >= 0) {
			//trace(stackTop);
			num[stack1[stackTop]] = numLeng;
			stackTop--;
			beginIndex = stack0[stackTop + 1];
			returni = stack1[stackTop + 1];
		}
	} while (stackTopPrev >= 0);

	numLengArray[numConLengId] = numLeng;

	return "complete";
}

float UModel3d::CalculateArray(float xt, float yt, int pos)
{
	if (pos == 0)
	{
		return CalculateArray(xt, yt, num1, oper1, con1, pos);
	}
	else if (pos == 1)
	{
		return CalculateArray(xt, yt, num2, oper2, con2, pos);
	}
	else if (pos == 2)
	{
		return CalculateArray(xt, yt, num3, oper3, con3, pos);
	}
	return NaN;
}

float UModel3d::CalculateArray(float xt, float yt, std::vector<int>& num, std::vector<int>& oper, std::vector<float>& con, int numConLengId)
{//计算函数
	int i = 0;
	stackTop = 0;
	stackf[0] = 0;
	float numi = 0.0f;
	int operi = 0;
	int operfi = 0;
	int numLeng = numLengArray[numConLengId];
	int conLeng = conLengArray[numConLengId];
	if (numLeng <= 0 || numLeng > functionMaxLeng) {
		return NaN;
	}
	if (conLeng > functionMaxLeng) {
		return NaN;
	}
	//trace(xt);
	//var opergot:Number=0;
	for (i = 0; i < numLeng; i++) {
		if (oper[i] > 0) {
			numi = num[i] < 0 ? (oper[i] < 8 ? xt : (oper[i] < 16 ? yt : cnt[(oper[i] >> 3) - 2])) : (num[i] < conLeng ? con[(int)num[i]] : NaN);
		}
		else {
			numi = num[i];
		}
		operi = oper[i] >= 0 ? (oper[i] & 7) : (oper[i] | 120);
		//operfi=oper[i]>=0?0:(-oper[i])>>3;
		if (std::isnan(stackf[stackTop])) {
			return NaN;
		}
		else if (operi == 1) {
			stackf[stackTop] += numi;
		}
		else if (operi == 2) {
			stackf[stackTop] -= numi;
		}
		else if (operi == 3) {
			stackf[stackTop] *= numi;
		}
		else if (operi == 4) {
			stackf[stackTop] = numi != 0 ? stackf[stackTop] / (float)numi : NaN;
		}
		else if (operi == 5) {
			stackf[stackTop] = (stackf[stackTop] == 0 && numi <= 0) ? NaN : std::pow(stackf[stackTop], numi);
		}
		else if (operi == 6) {
			if (numi == 0) {
				stackf[stackTop] = NaN;
			}
			else {
				stackf[stackTop] = stackf[stackTop] - std::floor(stackf[stackTop] / (float)std::abs(numi)) * std::abs(numi);
			}
		}
		else if (operi == 0) {
			if (stackTop <= 0) {
				return stackf[0];
			}
			else {
				if (num[i] < 0) {
					return NaN;
				}
				i = num[i];
				//trace(stackf[stackTop]);
				numi = stackf[stackTop];
				operi = oper[i] >= 0 ? (oper[i] & 7) : (oper[i] | 120);
				operfi = oper[i] >= 0 ? 0 : (-oper[i]) >> 3;
				if (std::isnan(numi)) {
					return NaN;
				}
				else if (operfi == 0) {
				}
				else if (operfi == 1) {
					numi = std::sin(numi);
				}
				else if (operfi == 2) {
					numi = std::cos(numi);
				}
				else if (operfi == 3) {
					numi = std::tan(numi);
					if (numi > 9.9f || numi < -9.9f)numi = NaN;
				}
				else if (operfi == 4) {
					numi = numi != 0 ? 1.0f / std::log10(std::exp(1)) * std::log10(numi) : NaN;
				}
				else if (operfi == 5) {
					numi = xt >= numi ? 1 : NaN;
				}
				else if (operfi == 6) {
					numi = std::sqrt(numi);
				}
				else if (operfi == 7) {
					numi = std::asin(numi);
				}
				else if (operfi == 8) {
					numi = std::acos(numi);
				}
				else if (operfi == 9) {
					numi = std::atan(numi);
				}
				else if (operfi == 10) {
					numi = xt <= numi ? 1 : NaN;
				}
				else if (operfi == 11) {
					numi = yt >= numi ? 1 : NaN;
				}
				else if (operfi == 12) {
					numi = std::abs(numi);
				}
				else if (operfi == 13) {
					numi = yt <= numi ? 1 : NaN;
				}
				else if (operfi == 14) {
					numi = numi >= 0 ? (int)numi : (int)numi - 1;
				}
				if (operi == -1) {
					stackf[stackTop - 1] += numi;
				}
				else if (operi == -2) {
					stackf[stackTop - 1] -= numi;
				}
				else if (operi == -3) {
					stackf[stackTop - 1] *= numi;
				}
				else if (operi == -4) {
					stackf[stackTop - 1] = numi != 0 ? stackf[stackTop - 1] / numi : NaN;
				}
				else if (operi == -5) {
					stackf[stackTop - 1] = (stackf[stackTop - 1] == 0 && numi <= 0) ? NaN : std::pow(stackf[stackTop - 1], numi);
				}
				else if (operi == -6) {
					stackf[stackTop - 1] = std::fmod(stackf[stackTop - 1],numi);
				}
				else {
					return NaN;
				}
				stackTop--;
			}
		}
		else if (operi < 0) {
			i = num[i] - 1;
			if (stackTop >= 50 - 1 || i < 0) {
				return NaN;
			}
			else {
				stackTop++;
				stackf[stackTop] = 0;
			}
		}
		else {
			return NaN;
		}
	}
	return stackf[0];
}

void UModel3d::PlotGrid(bool updateCoord0, bool willCreateN)
{

	if (strResult1 != "complete" || drawType >= 2 && strResult2 != "complete" || drawType >= 3 && strResult3 != "complete") {
		return;
	}

	if (textDebug != nullptr) {
		textDebug->SetText(FText::FromString(textDebug->Text.ToString() + FString::Printf(TEXT("\nPlotGrid()"))));
	}
	
	//从函数生成平面信息
	int i = 0;//循环变量		
	int j = 0;
	int k = 0;
	int l = 0;
	int m = 0;
	int n = 0;



	//var zxy:Number;
	volatile int xseg1 = xseg + 1;//行向分段数加一
	volatile int yseg1 = yseg + 1;//列后向分段数加一
	volatile int xsegp = (drawType == 3 && uclosed) ? xseg : xseg1;//每行的顶点数
	volatile int ysegp = (drawType == 3 && vclosed) ? yseg : yseg1;//每列的顶点数
	volatile int pointTotal = xsegp * ysegp;//总顶点数
	volatile int indicesTotal = xseg * yseg;//最大面积数

	int uvTotal = xseg1 * yseg1;//最大面积数
	float xseg0 = xseg / 2.0f;//每行分段数（四边形个数）的一半
	float yseg0 = yseg / 2.0f;//每列分段数（四边形个数）的一半
	//trace(xseg0);
	int zseg = std::max(xseg1, yseg1);//横纵向分段数的较大值
	volatile int shapeCount = drawType == 2 ? 2 : 1;//模型数
	float dxseg = parameticFunctionRange / xseg;//计算参数方程中过程中，x和y的增加幅度
	float dyseg = parameticFunctionRange / yseg;
	bool updateCoord = true;
	//Vector3 arrp;//point[i0]对应的数组
	std::vector<int> arrc;//indices[i0]对应的数组
	arrc.resize(4);
	bool indicesAvailable = true;//四边形平面是否可用		
	bool hasNaNPoint = false;//是否含有值为NaN的顶点
	int myNaNMode = (!uclosed && !vclosed) ? NaNMode : 0;//参数方程中实际使用的非数值处理方式
	int indicesTexTotal = (drawType >= 3 && (uclosed || vclosed)) ? indicesTotal : 0;//indicesTex应有的长度
	int pointcur = 0;//处理闭合曲面时用的当前顶点号


	//m_rawMesh.Empty();
	TArray<FVector>& point = m_rawMesh.VertexPositions;
	TArray<uint32>& indices = m_rawMesh.WedgeIndices;
	TArray<FVector2D> uv;
	TArray<uint32> indicesTex;

	point.SetNumZeroed(pointTotal * shapeCount);
	uv.SetNumZeroed(uvTotal * shapeCount);
	int uvCount = uvTotal * shapeCount;
	for (k = 0; k < uvCount; k++) {
		uv[k].Set(scaleU >= 0 ? m / (float)xseg * scaleU : 1 - m / (float)xseg * scaleU, scaleV >= 0 ? 1 - n / (float)yseg * scaleV : n / (float)yseg * scaleV);
		n++;
		if (n >= yseg1) {
			n = 0;
			m++;
		}
		if ((k + 1) % uvTotal == 0) {
			n = 0;
			m = 0;
		}
	}
	//trace(drawType>=3 && (uclosed || vclosed));

	int indicesCount = indicesTotal * shapeCount * 6;
	int indicesTexCount = indicesTexTotal * shapeCount * 6;
	indices.SetNumZeroed(indicesCount);
	indicesTex.SetNumZeroed(indicesTexCount);

	//trace(indices.length);
	l = 0;
	i = 0;
	j = 0;
	k = 0;
	if (drawType >= 1 && drawType < 3) {
		hasNaNPoint = false;
		for (k = 0; k < pointTotal; k++) {
			point[k].Y = ordinaryFunctionScale * (i - xseg0) / zseg;
			point[k].X = ordinaryFunctionScale * (j - yseg0) / zseg;
			point[k].Z = ordinaryFunctionScale * CalculateArray((i - xseg0) / zseg * ordinaryFunctionRange, (j - yseg0) / zseg * ordinaryFunctionRange, num1, oper1, con1, 0) / ordinaryFunctionRange;
			if (std::isnan(point[k].Z)) {
				if (NaNMode == 0) {
					point[k].Z = 0;
				}
				hasNaNPoint = true;
				updateCoord = true;
			}
			j++;
			if (j >= yseg1) {
				j = 0;
				i++;
			}
		}

		i = 0;
		j = 0;
		for (k = 0; k < pointTotal; k++) {
			if (updateCoord) {
				if (i != 0 && j != 0 && l + 5 < indicesCount) {
					arrc[0] = k - 1;
					arrc[1] = k;
					arrc[2] = k - yseg1 - 1;
					arrc[3] = k - yseg1;

					indices[l] = arrc[0];
					indices[l + 1] = arrc[1];
					indices[l + 2] = arrc[2];
					indices[l + 3] = arrc[3];
					indices[l + 4] = arrc[2];
					indices[l + 5] = arrc[1];
					if (hasNaNPoint) {
						indicesAvailable = true;
						if (NaNMode > 0 && (std::isnan(point[arrc[0]].Z) || std::isnan(point[arrc[1]].Z) || std::isnan(point[arrc[2]].Z) || std::isnan(point[arrc[3]].Z))) {
							indicesAvailable = false;
						}

						if (indicesAvailable) {
							l += 6;
						}
					}
					else {
						l += 6;
					}
				}
			}
			j++;
			if (j >= yseg1) {
				j = 0;
				i++;
			}
			if (l >= indicesCount) {
				break;
			}
		}

		if (NaNMode != 0 && hasNaNPoint) {
			for (k = 0; k < pointTotal; k++) {
				if (std::isnan(point[k].Z)) {
					point[k].Z = 0;
				}
			}
		}
		//gridWid=ordinaryFunctionScale*xseg/(float)scaleU/(float)zseg;
		//gridHei=ordinaryFunctionScale*yseg/(float)scaleV/(float)zseg;

	}
	else if (drawType == 3) {
		for (k = 0; k < pointTotal; k++) {
			point[k].Y = parameticFunctionScale * CalculateArray(i * dxseg, j * dyseg, num1, oper1, con1, 0);
			point[k].X = parameticFunctionScale * CalculateArray(i * dxseg, j * dyseg, num2, oper2, con2, 1);
			point[k].Z = parameticFunctionScale * CalculateArray(i * dxseg, j * dyseg, num3, oper3, con3, 2);
			if (std::isnan(point[k].Y)) {
				if (myNaNMode == 0) {
					point[k].Y = 0;
				}
				hasNaNPoint = true;
				updateCoord = true;
			}
			if (std::isnan(point[k].Z)) {
				if (myNaNMode == 0) {
					point[k].Z = 0;
				}
				hasNaNPoint = true;
				updateCoord = true;
			}
			if (std::isnan(point[k].X)) {
				if (myNaNMode == 0) {
					point[k].X = 0;
				}
				hasNaNPoint = true;
				updateCoord = true;
			}
			j++;
			if (j >= ysegp) {
				j = 0;
				i++;
			}
		}

		i = 0;
		j = 0;
		k = 0;
		l = 0;
		pointcur = 0;
		for (k = 0; k < uvTotal; k++) {
			if (updateCoord) {
				if (i != 0 && j != 0 && l + 5 < indicesCount && (indicesTexCount <= 0 || l + 5 < indicesTexCount)) {
					arrc[0] = k - 1;
					arrc[1] = k;
					arrc[2] = k - yseg1 - 1;
					arrc[3] = k - yseg1;


					if (indicesTexCount > 0) {
						indicesTex[l] = arrc[0];
						indicesTex[l + 1] = arrc[1];
						indicesTex[l + 2] = arrc[2];
						indicesTex[l + 3] = arrc[3];
						indicesTex[l + 4] = arrc[2];
						indicesTex[l + 5] = arrc[1];

						pointcur = i * ysegp + j;
						arrc[0] = (uclosed && i == xseg) ? j - 1 : pointcur - 1;
						arrc[1] = (vclosed && j == yseg) ? ((uclosed && i == xseg) ? 0 : pointcur - j) : ((uclosed && i == xseg) ? j : pointcur);
						arrc[2] = pointcur - ysegp - 1;
						arrc[3] = (vclosed && j == yseg) ? pointcur - ysegp - j : pointcur - ysegp;

						if (usame0 && i == 1) {
							arrc[2] = 0;
							arrc[3] = 0;
						}
						if (usamet && i == xseg) {
							arrc[0] = uclosed ? 0 : i * ysegp;
							arrc[1] = uclosed ? 0 : i * ysegp;
						}
						if (vsame0 && j == 1) {
							arrc[0] = 0;
							arrc[2] = 0;
						}
						if (vsamet && j == yseg) {
							arrc[1] = vclosed ? 0 : ysegp - 1;
							arrc[3] = vclosed ? 0 : ysegp - 1;
						}
					}

					indices[l] = arrc[0];
					indices[l + 1] = arrc[1];
					indices[l + 2] = arrc[2];
					indices[l + 3] = arrc[3];
					indices[l + 4] = arrc[2];
					indices[l + 5] = arrc[1];
					if (hasNaNPoint) {
						indicesAvailable = true;

						if (myNaNMode > 0 && (std::isnan(point[arrc[0]].Y) || std::isnan(point[arrc[0]].Z) || std::isnan(point[arrc[0]].X) || std::isnan(point[arrc[1]].Y) || std::isnan(point[arrc[1]].Z) || std::isnan(point[arrc[1]].X) || std::isnan(point[arrc[2]].Y) || std::isnan(point[arrc[2]].Z) || std::isnan(point[arrc[2]].X) || std::isnan(point[arrc[3]].Y) || std::isnan(point[arrc[3]].Z) || std::isnan(point[arrc[3]].X))) {
							indicesAvailable = false;
						}
						if (indicesAvailable) {
							l += 6;
						}
					}
					else {
						l += 6;
					}
				}
				if (l >= indicesCount) {
					break;
				}
			}


			j++;
			if (j >= yseg1) {
				j = 0;
				i++;
			}
		}
		if (myNaNMode != 0 && hasNaNPoint) {
			for (k = 0; k < pointTotal; k++) {
				if (std::isnan(point[k].Y)) {
					point[k].Y = 0;
				}
				if (std::isnan(point[k].Z))
				{
					point[k].Z = 0;
				}
				if (std::isnan(point[k].X))
				{
					point[k].X = 0;
				}
			}
		}
	}

	i = 0;
	j = 0;
	if (drawType == 2) {
		hasNaNPoint = false;
		for (k = pointTotal; k < (pointTotal << 1); k++) {
			point[k].Y = ordinaryFunctionScale * (i - xseg0) / zseg;
			point[k].X = ordinaryFunctionScale * (j - yseg0) / zseg;
			point[k].Z = ordinaryFunctionScale * CalculateArray((i - xseg0) / zseg * ordinaryFunctionRange, (j - yseg0) / zseg * ordinaryFunctionRange, num2, oper2, con2, 1) / ordinaryFunctionRange;
			FVector p = point[k];
			if (std::isnan(point[k].X)) {
				if (NaNMode == 0) {
					point[k].X = 0;
				}
				hasNaNPoint = true;
				updateCoord = true;
			}
			j++;
			if (j >= yseg1) {
				j = 0;
				i++;
			}
		}
		i = 0;
		j = 0;
		for (k = pointTotal; k < (pointTotal << 1); k++) {
			if (updateCoord) {
				if (i != 0 && j != 0 && l + 5 < indicesCount) {
					arrc[0] = k - 1;
					arrc[1] = k;
					arrc[2] = k - yseg1 - 1;
					arrc[3] = k - yseg1;

					indices[l] = arrc[0];
					indices[l + 1] = arrc[1];
					indices[l + 2] = arrc[2];
					indices[l + 3] = arrc[3];
					indices[l + 4] = arrc[2];
					indices[l + 5] = arrc[1];
					if (hasNaNPoint) {
						indicesAvailable = true;
						if (NaNMode > 0 && (std::isnan(point[arrc[0]].Z) || std::isnan(point[arrc[1]].Z) || std::isnan(point[arrc[2]].Z) || std::isnan(point[arrc[3]].Z))) {
							indicesAvailable = false;
						}

						if (indicesAvailable) {
							l += 6;
						}
					}
					else {
						l += 6;
					}
				}
			}
			j++;
			if (j >= yseg1) {
				j = 0;
				i++;
			}
			if (l >= indicesCount) {
				break;
			}
		}
		if (NaNMode != 0 && hasNaNPoint) {
			for (k = pointTotal; k < (pointTotal << 1); k++) {
				if (std::isnan(point[k].Z)) {
					point[k].Z = 0;
				}
			}
		}
	}
	if (l < indices.Num())
	{
		indices.SetNum(l);
	}
	if (indicesTex.Num() > 0 && l < indicesTex.Num())
	{
		indicesTex.SetNum(l);
	}
	if (textDebug != nullptr) {
		textDebug->SetText(FText::FromString(textDebug->Text.ToString() + FString::Printf(TEXT("\nBefore MeshUti"))));
	}
	MeshUtil::CreateNormal(m_rawMesh);
	MeshUtil::SetUV(m_rawMesh, uv, indicesTex);
}



UModel3d* UModel3d::Sphere(float r, int jseg, int wseg, bool invertU)
{
	std::string str = "";
	if (!invertU) {
		str = std::to_string(r) + "*sin(y/2)cos(x)," + std::to_string(r) + "*sin(y/2)sin(x),-" + std::to_string(r) + "*cos(y/2),%**";
	}
	else {
		str = std::to_string(r) + "*sin(y/2)cos(-x)," + std::to_string(r) + "*sin(y/2)sin(-x),-" + std::to_string(r) + "*cos(y/2),%**";
	}
	UModel3d *model3d = NewObject<UModel3d>();
	model3d->UpdateFunction(str, jseg, wseg);
	return model3d;
}

UModel3d* UModel3d::Cone(float r, float h, int jseg, int wseg)
{
	std::string str = std::to_string(r / (2 * PI)) + "(2P-y)cos(x)," + std::to_string((r / (2 * PI))) + "(2P-y)sin(x)," + std::to_string((h / (2 * PI))) + "(y-P),%";
	UModel3d* model3d = NewObject<UModel3d>();
	model3d->UpdateFunction(str, jseg, wseg);
	return model3d;
}

UModel3d* UModel3d::ConeEx(float rtop, float rbottom, float h, int jseg, int wseg)
{
	float k = (rtop - rbottom) / (2 * PI);
	float b = rbottom;
	std::string str = "(" + std::to_string(k) + "*y+" + std::to_string(b) + ")*cos(x)," + "(" + std::to_string(k) + "*y+" + std::to_string(b) + ")*sin(x)," + std::to_string((h / (2 * PI))) + "*(y-P),%";
	UModel3d* model3d = NewObject<UModel3d>();
	model3d->UpdateFunction(str, jseg, wseg);
	return model3d;
}

UModel3d* UModel3d::Cylinder(float r, float h, int jseg, int wseg)
{
	std::string str = std::to_string(r) + "cos(x)," + std::to_string(r) + "sin(x)," + std::to_string((h / (2 * PI))) + "(y-P),%";
	UModel3d* model3d = NewObject<UModel3d>();
	model3d->UpdateFunction(str, jseg, wseg);
	return model3d;
}

UModel3d* UModel3d::Ring(float r1, float r2, int jseg, int wseg)
{
	std::string str = "(" + std::to_string(r1) + "+" + std::to_string(r2) + "cos(y))*cos(x),(" + std::to_string(r1) + "+" + std::to_string(r2) + "cos(y))*sin(x)," + std::to_string(r2) + "sin(y),%%";
	UModel3d* model3d = NewObject<UModel3d>();
	model3d->UpdateFunction(str, jseg, wseg);
	return model3d;
}

void UModel3d::Plane(FRawMesh &rawMesh, FVector a, FVector b, FVector c, FVector d, int xseg0, int yseg0, float scaleU0, float scaleV0)
{
	int xseg01 = xseg0 + 1;
	int yseg01 = yseg0 + 1;
	int pointTotal = xseg01 * yseg01;
	int indicesTotal = xseg0 * yseg0 * 6;
	int i = 0;
	int j = 0;
	int k = 0;
	int l = 0;
	TArray<FVector>& point = rawMesh.VertexPositions;
	TArray<uint32>& indices = rawMesh.WedgeIndices;
	TArray<FVector2D> uv;
	point.SetNumZeroed(pointTotal);
	indices.SetNumZeroed(indicesTotal);
	uv.SetNumZeroed(point.Num());

	for (k = 0; k < pointTotal; k++) {
		point[k].Y = a.Y * i * j / (float)indicesTotal + b.Y * (xseg0 - i) * j / (float)indicesTotal + c.Y * (xseg0 - i) * (yseg0 - j) / indicesTotal + d.Y * i * (yseg0 - j) / indicesTotal;
		point[k].Z = a.Z * i * j / (float)indicesTotal + b.Z * (xseg0 - i) * j / (float)indicesTotal + c.Z * (xseg0 - i) * (yseg0 - j) / indicesTotal + d.Z * i * (yseg0 - j) / indicesTotal;
		point[k].X = a.X * i * j / (float)indicesTotal + b.X * (xseg0 - i) * j / (float)indicesTotal + c.X * (xseg0 - i) * (yseg0 - j) / indicesTotal + d.X * i * (yseg0 - j) / indicesTotal;
		uv[k].Set(scaleU0 >= 0 ? i / (float)xseg0 * scaleU0 : 1 - i / (float)xseg0 * scaleU0, scaleV0 >= 0 ? j / (float)yseg0 * scaleV0 : 1 - j / (float)yseg0 * scaleV0);

		if (i != 0 && j != 0 && l + 5 < indicesTotal) {
			indices[l] = k - yseg01 - 1;
			indices[l + 1] = k;
			indices[l + 2] = k - 1;
			indices[l + 3] = k;
			indices[l + 4] = k - yseg01 - 1;
			indices[l + 5] = k - yseg01;
			l += 6;
		}
		j++;
		if (j >= yseg01) {
			j = 0;
			i++;
		}
	}


	MeshUtil::CreateNormal(rawMesh);
	MeshUtil::SetUV(rawMesh, uv, TArray<uint32>());
}

void UModel3d::RoundPlane(FRawMesh& rawMesh, float r, int seg)
{
	int i;
	int j;
	int k;
	int l;
	int lprev;
	TArray<FVector>& point = rawMesh.VertexPositions;
	TArray<TArray<int>> indices0;
	TArray<FVector2D> uv;
	if (seg % 2 != 0) {
		seg++;
	}
	float dseg = 2 * PI / seg;
	k = 0;
	l = 0;
	for (i = 0; i <= (seg >> 1); i++) {
		lprev = l;
		l = i <= (seg >> 2) ? 2 * i + 1 : 2 * ((seg >> 1) - i) + 1;//每行点数
		//trace(i,l,-(l>>1));
		for (j = 0; j < l; j++) {
			point.Push(FVector((r * FMath::Cos(i * dseg)), (r * FMath::Sin((j - (l >> 1)) * dseg)), 0));
			uv.Push(FVector2D(0.5f * FMath::Sin((j - (l >> 1)) * dseg) + 0.5f, -0.5f * FMath::Cos(i * dseg) + 0.5f));

			if (i != 0) {
				if (j == 1 && i <= (seg >> 2)) {
					indices0.Push(TArray<int>{k - lprev - 1, k - 1, k, -1});
				}
				else if (j == 0 && i > (seg >> 1) - (seg >> 2)) {
					indices0.Push(TArray<int>{k - lprev, k, k - lprev + 1, -1});
					if (j == l - 1) {
						indices0.Push(TArray<int>{k - l - 1, k, k - l, -1});
					}
				}
				else if (j == l - 1 && i <= (seg >> 2)) {
					indices0.Push(TArray<int>{k - l, k - 1, k, -1});
				}
				else if (j == l - 1 && i > (seg >> 1) - (seg >> 2)) {
					indices0.Push(TArray<int>{k - lprev, k - 1, k - lprev + 1, k});
					indices0.Push(TArray<int>{k - l - 1, k, k - l, -1});
				}
				else if (j > 1 && i <= (seg >> 2)) {
					indices0.Push(TArray<int>{k - l, k - 1, k - l + 1, k});
				}
				else if (i > (seg >> 1) - (seg >> 2)) {
					indices0.Push(TArray<int>{k - lprev, k - 1, k - lprev + 1, k});
				}
				else if (j > 0) {
					indices0.Push(TArray<int>{k - l - 1, k - 1, k - l, k});
				}
			}
			k++;
		}
	}
	TArray<uint32>& indices = rawMesh.WedgeIndices;
	indices.SetNum(0);
	int count = indices0.Num();
	for (i = 0; i < count; i++) {
		TArray<int>& arrc = indices0[i];
		if (arrc.Num() < 4) {
			continue;
		}
		indices.Push((uint32)arrc[2]);
		indices.Push((uint32)arrc[1]);
		indices.Push((uint32)arrc[0]);
		if (arrc[3] >= 0) {
			indices.Push((uint32)arrc[1]);
			indices.Push((uint32)arrc[2]);
			indices.Push((uint32)arrc[3]);
		}
	}
	MeshUtil::CreateNormal(rawMesh);
	MeshUtil::SetUV(rawMesh, uv, TArray<uint32>());
}

void UModel3d::Cuboid(FRawMesh& rawMesh, float a0, float b0, float c0, bool verticalUV, bool invertU)
{
	float a = a0 * 0.5f;
	float b = b0 * 0.5f;
	float c = c0 * 0.5f;
	TArray<FVector> point0{ FVector((-c),(-a),(-b)),FVector((-c),a,(-b)),FVector(c,a,(-b)),FVector(c,(-a),(-b)),FVector((-c),(-a),b),FVector((-c),a,b),FVector(c,a,b),FVector(c,(-a),b) };

	TArray<TArray<int>> indices0{ TArray<int>{2,3,6,7},TArray<int>{1,2,5,6},TArray<int>{0,1,4,5},TArray<int>{3,0,7,4},TArray<int>{4,5,7,6},TArray<int>{3,2,0,1} };

	TArray<FVector>& point = rawMesh.VertexPositions;
	TArray<uint32>& indices = rawMesh.WedgeIndices;
	TArray<FVector2D> uv;
	point.SetNumZeroed(24);
	indices.SetNumZeroed(36);
	uv.SetNumZeroed(24);


	for (int i = 0; i < 6; i++) {
		if (!invertU) {
			point[i << 2].Set(point0[indices0[i][0]].Y, point0[indices0[i][0]].Z, point0[indices0[i][0]].X);
			point[i << 2 | 1].Set(point0[indices0[i][1]].Y, point0[indices0[i][1]].Z, point0[indices0[i][1]].X);
			point[i << 2 | 2].Set(point0[indices0[i][2]].Y, point0[indices0[i][2]].Z, point0[indices0[i][2]].X);
			point[i << 2 | 3].Set(point0[indices0[i][3]].Y, point0[indices0[i][3]].Z, point0[indices0[i][3]].X);
		}
		else {
			point[i << 2].Set(point0[indices0[i][1]].Y, point0[indices0[i][1]].Z, point0[indices0[i][1]].X);
			point[i << 2 | 1].Set(point0[indices0[i][0]].Y, point0[indices0[i][0]].Z, point0[indices0[i][0]].X);
			point[i << 2 | 2].Set(point0[indices0[i][3]].Y, point0[indices0[i][3]].Z, point0[indices0[i][3]].X);
			point[i << 2 | 3].Set(point0[indices0[i][2]].Y, point0[indices0[i][2]].Z, point0[indices0[i][2]].X);
		}


		indices[i * 6] = i << 2 | 2;
		indices[i * 6 + 1] = i << 2 | 1;
		indices[i * 6 + 2] = i << 2;
		indices[i * 6 + 3] = i << 2 | 1;
		indices[i * 6 + 4] = i << 2 | 2;
		indices[i * 6 + 5] = i << 2 | 3;


		if (!verticalUV) {
			uv[i << 2].Set(i / 6.0f, 1);
			uv[i << 2 | 1].Set((i + 1) / 6.0f, 1);
			uv[i << 2 | 2].Set(i / 6.0f, 0);
			uv[i << 2 | 3].Set((i + 1) / 6.0f, 0);
		}
		else {
			uv[i << 2].Set(0, (i + 1) / 6.0f);
			uv[i << 2 | 1].Set(1, (i + 1) / 6.0f);
			uv[i << 2 | 2].Set(0, i / 6.0f);
			uv[i << 2 | 3].Set(1, i / 6.0f);
		}

	}

	MeshUtil::CreateNormal(rawMesh);
	MeshUtil::SetUV(rawMesh, uv, TArray<uint32>());
}

