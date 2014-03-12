#include "resource.h"

std::string join(std::string separator, std::string str)
{
    return str;
}

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

std::string& mergeSpaces(std::string& str)
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

std::string& enquote(std::string& str)
{
    const std::string quote="\"";
    str=quote+str;
    str+=quote;
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

name_t getName(dictionary_t& dict, int index)
{
    return dict[index].first;
}

value_t getAttr(dictionary_t& dict, int index, key_t key, value_t defaultvalue)
{
    auto it=dict[index].second.find(key);
    if(it==dict[index].second.end())
        return defaultvalue;
    return it->second;
}

event_t getEvent(event_queue_t& event, int currentcase)
{
    if(!event.empty())
    {
        while(currentcase>event.front().first)
            event.pop_front();
        if(currentcase==event.front().first)
        {
            event_t ret=event.front().second;
            event.pop_front();
            return ret;
        }
    }
    return event_t();
}

std::pair<dictionary_t, event_queue_t> parseData(std::string filename)
{
    dictionary_t dict;
    event_queue_t event;
    dict.push_back(entry_t());
    std::ifstream fin(filename);
    std::string line;
    while(fin.good())
    {
        std::getline(fin, line);
        trim(line);
        line=line.substr(0, line.find('#'));
        if(!line.empty())
        {
            if(line[0]=='<' && line[line.length()-1]=='>')
            {
                size_t cpos=line.find(':');
                if(cpos==std::string::npos)
                {
                    std::string command=line.substr(1, line.length()-2);
                    event.push_back(make_pair(static_cast<int>(dict.size()-1), make_pair(trim(command), arguments_t())));
                }
                else
                {
                    std::string command=line.substr(1, cpos-1);
                    std::string arguments=line.substr(cpos+1, line.length()-(cpos+2));
                    event.push_back(make_pair(static_cast<int>(dict.size()-1), make_pair(trim(command), trim(arguments))));
                }
            }
            else if(line[0]=='[' && line[line.length()-1]==']')
                dict.push_back(make_pair(line.substr(1, line.length()-2), attributes_t()));
            else
            {
                size_t cpos=line.find(':');
                if(cpos==std::string::npos)
                    continue;
                std::string variable=line.substr(0, cpos);
                std::string value=line.substr(cpos+1, std::string::npos);
                dict.back().second[trim(variable)]=trim(value);
            }
        }
    }
    fin.close();
    return make_pair(dict, event);
}
