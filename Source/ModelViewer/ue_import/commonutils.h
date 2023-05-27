#ifndef COMMONUTILS_H
#define COMMONUTILS_H
#endif // COMMONUTILS_H

#include <iostream>
#include <string>
#include <vector>
#include <random>

#ifndef _WIN32
#define _wtoi(a) wcstol(a,nullptr,10)
#define _wtof(a) wcstod(a,nullptr)
#endif 

double randomValue();
double randomRange(double min,double max);
int randomRange(int min,int max);

std::string substring(const std::string& str0, int begin=0, int end=0x7fffffff);
std::string stringReplace(std::string strBase,const std::string& strSrc, const std::string& strDes);
std::string trimString(const std::string& str);
void splitString(std::vector<std::string> &arr, const std::string& str,const std::string& splitter);
template<typename T>
std::string joinString(const std::vector<T>& arr, const std::string& splitter);

std::string stringLowerCase(std::string strt);
std::string stringUpperCase(std::string strt);
int vectorStringIndexOf(const std::vector<std::string> &arr, const std::string& value);
int vectorStringLastIndexOf(const std::vector<std::string> &arr, const std::string& value);
void concatVectorStringWith(std::vector<std::string> &arrConcated,const std::vector<std::string> &arrConcatWith);
void copyVectorString(const std::vector<std::string> &arr0,std::vector<std::string> &arrt);

int strnumcmp(const char *s1,const char *s2);
int strnumcmp(std::string &s1,std::string &s2);
void strNumSort(std::vector<std::string> &strArrSrc,std::vector<std::string> &strArrOut);
void strNumSort(std::vector<std::string> &strArr);

template<typename T>
class vectorex : public std::vector<T> {
public:
	vectorex() : std::vector<T>() {};
	vectorex(std::vector<int>::size_type size) : std::vector<T>(size) {};
	vectorex(const std::vector<T>& another) : std::vector<T>(another) {};
	vectorex& operator=(const std::vector<T>& another){
		return vectorex(another);
	}

	T& operator [](int index)
	{
		T size0 = std::vector<T>::size();
		if (index < 0)
		{
			return std::vector<T>::at(index);
		}
		if (index >= size0)
		{
			std::vector<T>::resize(index + 1);
			return std::vector<T>::operator[](index);
		}
		else
		{
			return std::vector<T>::operator[](index);
		}
	}
};