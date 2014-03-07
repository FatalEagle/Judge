#include "resource.h"

std::string& ltrim(std::string& s)
{
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), std::not1(std::ptr_fun<int, int>(std::isspace))));
    return s;
}

std::string& rtrim(std::string& s)
{
    s.erase(std::find_if(s.rbegin(), s.rend(), std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
    return s;
}

std::string& trim(std::string &str)
{
    ltrim(rtrim(str));
    return str;
}

std::string& mergeSpaces(std::string &str)
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
