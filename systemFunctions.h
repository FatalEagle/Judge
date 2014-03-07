#ifndef SYSTEMFUNCTIONS_H_INCLUDED
#define SYSTEMFUNCTIONS_H_INCLUDED

#include "basicresource.h"

template<class T1>
std::string toStr(T1 t)
{
    std::stringstream ss;
    ss<<t;
    return ss.str();
}

template<class T1>
T1 toType(std::string s)
{
    std::stringstream ss(s);
    T1 i;
    ss>>i;
    return i;
}

template<class T1, class... Args>
int runMultiple(T1 runType, int total, Args... args)
{
    int number=0;
    while(total--)
        number+=runType(args...);
    return number;
}

std::string& ltrim(std::string& s);
std::string& rtrim(std::string& s);
std::string& trim(std::string &str);
std::string& mergeSpaces(std::string &str);
void replaceAll(std::string& str, const std::string& from, const std::string& to);
std::vector<std::string> split(std::string original, std::string delimiter);

#ifdef _WIN32
#include "win_systemFunctions.h"
#else
#include "nix_systemFunctions.h"
#endif // _WIN32

#endif // SYSTEMFUNCTIONS_H_INCLUDED
