#include "commonutils.h"
#include <sstream>

#ifndef _WIN32
#endif 

std::default_random_engine _rand;
std::uniform_real_distribution<double> _randReal(0.0, 1.0);

double randomValue(){
    return _randReal(_rand);
}
double randomRange(double min, double max) {
    return std::uniform_real_distribution<double>(min, max)(_rand);
}
int randomRange(int min, int max) {
    return std::uniform_int_distribution<int>(min, max-1)(_rand);
}


std::string substring(const std::string& str0,int begin,int end){
    std::string strt="";
    int leng0=str0.length();

    if(leng0<0){
        strt=str0;
        return strt;
    }

    if(end>leng0){
        end=leng0;
    }
    if(begin<0){
        begin=0;
    }
    if(begin>=end){
        return std::string();
    }

    if(begin==0 && end==leng0){
        return str0;
    }
    return str0.substr(begin, (size_t)end - (size_t)begin);
}

std::string stringReplace(std::string strBase, const std::string& strSrc, const std::string& strDes){
    std::string::size_type pos = 0;
    std::string::size_type srcLen = strSrc.size();
    std::string::size_type desLen = strDes.size();

    pos=strBase.find(strSrc, pos);
    while (pos != std::string::npos){
        strBase.replace(pos, srcLen, strDes);
        pos=strBase.find(strSrc, (pos+desLen));
    }
    return strBase;
}

std::string trimString(const std::string& str){
    long long i=0,j=str.length();
    if(j<=0){
        return std::string();
    }
    for(i=0;i<j;i++){
        if(str[i]!=' ' && str[i]!='\t' && str[i]!='\r' && str[i]!='\n'){
            break;
        }
    }
    if(i>=j){
        return std::string();
    }
    j--;
    while(j>=i){
        if(str[j]!=' ' && str[j]!='\t' && str[j]!='\r' && str[j]!='\n'){
            break;
        }
        j--;
    }
    if(i>j){
        return std::string();
    }else{
        return str.substr(i,j+1-i);
    }
}

void splitString(std::vector<std::string> &arr,const std::string &str,const std::string& splitter){
    arr.clear();
    if(str.empty() || splitter.empty()){
        arr.push_back(str);
        return;
    }

    size_t i=0,j=0,strl=str.length();
    do{

        j=str.find(splitter.c_str(),i,splitter.length());
        if(j!=std::string::npos){
            arr.push_back(str.substr(i,j-i));
        }else{
            arr.push_back(str.substr(i,str.length()));
        }
        i=j+splitter.length();
    }while(j!=std::string::npos && i<strl);

    if(j!=std::string::npos && i==strl){
        arr.push_back(std::string());
    }
}

template<typename T>
std::string joinString(const std::vector<T> &arr,const std::string& splitter){
    std::ostringstream stream;
    int i=0;
    int arrl=(int)(arr.size());
    for(i=0;i<arrl;i++){
        stream<<arr[i];
        if(i<arrl-1){
            stream << splitter;
        }
    }
    return stream.str();
}

template std::string joinString(const std::vector<std::string>&, const std::string&);
template std::string joinString(const std::vector<bool>&, const std::string&);
template std::string joinString(const std::vector<char>&, const std::string&);
template std::string joinString(const std::vector<unsigned char>&, const std::string&);
template std::string joinString(const std::vector<short>&, const std::string&);
template std::string joinString(const std::vector<int>&, const std::string&);
template std::string joinString(const std::vector<long>&, const std::string&);
template std::string joinString(const std::vector<long long>&, const std::string&);
template std::string joinString(const std::vector<unsigned short>&, const std::string&);
template std::string joinString(const std::vector<unsigned int>&, const std::string&);
template std::string joinString(const std::vector<unsigned long>&, const std::string&);
template std::string joinString(const std::vector<unsigned long long>&, const std::string&);
template std::string joinString(const std::vector<float>&, const std::string&);
template std::string joinString(const std::vector<double>&, const std::string&);




std::string stringLowerCase(std::string strt){
    int i=0;
    int strl=strt.length();
    int dchar='a'-'A';
    for(i=0;i<strl;i++){
        if(strt[i]>='A' && strt[i]<='Z'){
            strt[i]+=dchar;
        }
    }
    return strt;
}

std::string stringUpperCase(std::string strt){
    int i=0;
    int strl=strt.length();
    int dchar='a'-'A';
    for(i=0;i<strl;i++){
        if(strt[i]>='a' && strt[i]<='z'){
            strt[i]-=dchar;
        }
    }
    return strt;
}

int vectorStringIndexOf(const std::vector<std::string> &arr, const std::string& value){
    int i=0,leng0=arr.size();
    if(leng0<=0){
        return -1;
    }
    for(i=0;i<leng0;i++){
        if(arr[i]==value){
            return i;
        }
    }
    return -1;
}

int vectorStringLastIndexOf(const std::vector<std::string> &arr, const std::string& value) {
    int i = 0, leng0 = arr.size();
    if (leng0 <= 0) {
        return -1;
    }
    for (i = leng0-1; i >= 0; i--) {
        if (arr[i] == value) {
            return i;
        }
    }
    return -1;
}



void concatVectorStringWith(std::vector<std::string> &arrConcated,const std::vector<std::string> &arrConcatWith){
    unsigned int a=arrConcated.size(),b=arrConcatWith.size();
    unsigned int c=a+b;
    unsigned int i;
    arrConcated.resize(c);
    for(i=a;i<c;i++){
        arrConcated[i]=arrConcatWith[i-a];
    }
}

void copyVectorString(const std::vector<std::string> &arr0,std::vector<std::string> &arrt){
    unsigned int i=0;
    unsigned int length=arr0.size();
    arrt.resize(length);
    for(i=0;i<length;i++){
        arrt[i]=arr0[i];
    }
}

int strnumcmp(const char *s1,const char *s2){
    //比较可能有编号的字符串s1和s2，不优先区分大小写。
    int i=0;
    int nummode1=0;
    int nummode2=0;
    int numcmp=-2;
    char c01;
    char c02;
    char c1;
    char c2;
    int casecmp=0;
    for(i=0;s1[i]!=0 && s2[i]!=0;i++){
        c01=s1[i];
        c02=s2[i];
        if(c01>='0' && c01<='9'){
            nummode1=true;
        }else{
            nummode1=false;
        }
        if(c02>='0' && c02<='9'){
            nummode2=true;
        }else{
            nummode2=false;
        }
        if(nummode1 && nummode2 && numcmp==-2){
            numcmp=0;
        }
        if(numcmp==0){
            if(c01>c02){
                numcmp=1;
            }else if(c01<c02){
                numcmp=-1;
            }
        }
        if(numcmp>-2){
            if(nummode1 && !nummode2){
                return 1;
            }else if(!nummode1 && nummode2){
                return -1;
            }else if(!nummode1 && !nummode2){
                if(numcmp!=0){
                    return numcmp;
                }
            }else{
                continue;
            }
        }
        if(!nummode1 || !nummode2){
            numcmp=-2;
        }
        if(c01>='A' && c01<='Z'){
            c1=c01+32;
        }else{
            c1=c01;
        }
        if(c02>='A' && c02<='Z'){
            c2=c02+32;
        }else{
            c2=c02;
        }
        if(c1>c2){
            return 1;
        }else if(c1<c2){
            return -1;
        }else if(casecmp==0){
            if(c01>c02){
                casecmp=1;
            }else if(c01<c02){
                casecmp=-1;
            }
        }
    }
    if(nummode1 && nummode2){
        c01=s1[i];
        c02=s2[i];
        if(c01!=0){
            return 1;
        }else if(c02!=0){
            return -1;
        }else if(numcmp==1){
            return 1;
        }else if(numcmp==-1){
            return -1;
        }else{
            return casecmp;
        }
    }else{
        c01=s1[i];
        c02=s2[i];
        if(c01!=0){
            return 1;
        }else if(c02!=0){
            return -1;
        }else{
            return casecmp;
        }
    }
    return 0;
}

int strnumcmp(std::string &s1,std::string &s2){
    //比较可能有编号的字符串s1和s2，不优先区分大小写。
    return strnumcmp(s1.c_str(),s2.c_str());
}

void strNumSort(std::vector<std::string> &strArrSrc,std::vector<std::string> &strArrOut){
    //将vector<std::wstring>用strnumcmp的比较方法排序
    int length=strArrSrc.size();
    if(length<=0){
        strArrOut.clear();
        return;
    }

    int *arr=new int[length];
    int *arr2=new int[length];

    int i=0;


    for(i=0;i<length;i++){
        arr[i]=i;
        arr2[i]=i;
    }
    i=0;

    int j=0;
    int k=0;
    int l=0;
    int m=0;
    int n=0;
    int a=1;
    int b=a<<1;

    while(a<length){
        i=0;
        j=a;
        k=a;
        l=b;
        m=0;
        if(k>length){
            k=length;
        }
        if(l>length){
            l=length;
        }
        while(m<length){
            if(n==0){
                if(i>=k || j<l && strnumcmp(strArrSrc[arr[i]],strArrSrc[arr[j]])>0){
                    arr2[m]=arr[j];
                    j++;
                }else if(i<k){
                    arr2[m]=arr[i];
                    i++;
                }
            }else{
                if(i>=k || j<l && strnumcmp(strArrSrc[arr2[i]],strArrSrc[arr2[j]])>0){
                    arr[m]=arr2[j];
                    j++;
                }else if(i<k){
                    arr[m]=arr2[i];
                    i++;
                }
            }
            m++;
            if(m>=l){
                i=l;
                j=l+a;
                k+=b;
                l+=b;
                if(k>length){
                    k=length;
                }
                if(l>length){
                    l=length;
                }
            }
        }
        n=1-n;
        a=b;
        b=b<<1;
    }

    if((int)(strArrOut.size())!=length){
        strArrOut.resize(length);
    }
    for(i=0;i<length;i++){
        if(n==0){
            strArrOut[i]=strArrSrc[arr[i]];
        }else{
            strArrOut[i]=strArrSrc[arr2[i]];
        }
    }
    delete []arr;
    delete []arr2;
}

void strNumSort(std::vector<std::string> &strArr){
    std::vector<std::string> strArrTemp;
    strArrTemp.resize(strArr.size());
    strNumSort(strArr,strArrTemp);
    unsigned int i=0,length=strArr.size();
    for(i=0;i<length;i++){
        strArr[i]=strArrTemp[i];
    }
}
