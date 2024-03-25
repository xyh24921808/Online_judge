#pragma once
#include"util.hpp"
#include<iostream>
#include<string>
using namespace std;
//日志功能模块

enum 
{
    INFO,
    DEBUG,
    WARING,
    ERROR,
    FATA
};


inline ostream& Log(const string&Level,const string&File_name,int Line)
{
    string mess;
    mess+='['+Level+']';

    mess+='['+File_name+']';

    mess+='['+to_string(Line)+']';

    mess+='['+TimeUtil::Gettimest()+']';

    cout<<mess;
    return cout;
}

#define LOG(level) Log(#level,__FILE__,__LINE__) 
