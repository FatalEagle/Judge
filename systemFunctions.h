#ifndef SYSTEMFUNCTIONS_H_INCLUDED
#define SYSTEMFUNCTIONS_H_INCLUDED

#ifdef _WIN32
#include "win_systemFunctions.h"
#else
#include "nix_systemFunctions.h"
#endif // _WIN32

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

inline std::string& ltrim(std::string& s)
{
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), std::not1(std::ptr_fun<int, int>(std::isspace))));
    return s;
}

inline std::string& rtrim(std::string& s)
{
    s.erase(std::find_if(s.rbegin(), s.rend(), std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
    return s;
}

inline std::string& trim(std::string &str)
{
    ltrim(rtrim(str));
    return str;
}

inline std::string& mergeSpaces(std::string &str)
{
    trim(str);
    std::string::iterator new_end=std::unique(str.begin(), str.end(), [](char lhs, char rhs)
    {
        return isspace(lhs) && isspace(rhs);
    });
    str.erase(new_end, str.end());
    std::replace(str.begin(), str.end(), '\n', ' ');
    return str;
}

void replaceAll(std::string& str, const std::string& from, const std::string& to)
{
    if(from.empty())
        return;
    size_t start_pos = 0;
    while((start_pos = str.find(from, start_pos)) != std::string::npos)
    {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length(); // In case 'to' contains 'from', like replacing 'x' with 'yx'
    }
}

std::vector<std::string> split(std::string original, std::string delimiter)
{
    std::vector<std::string> ret;
    std::string component;
    size_t lpos=0, pos=original.find(delimiter);
    while(pos!=std::string::npos)
    {
        component=original.substr(lpos, pos-lpos);
        ret.push_back(trim(component));
        lpos=pos+delimiter.length();
        pos=original.find(delimiter, lpos);
    }
    component=original.substr(lpos, pos-lpos);
    ret.push_back(trim(component));
    return ret;
}

#endif // SYSTEMFUNCTIONS_H_INCLUDED
