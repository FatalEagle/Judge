#ifndef SYSTEMFUNCTIONS_H_INCLUDED
#define SYSTEMFUNCTIONS_H_INCLUDED

#include "basicresource.h"

typedef std::string name_t;
typedef std::string key_t;
typedef std::string value_t;
typedef std::unordered_map<key_t, value_t> attributes_t;
typedef std::pair<name_t,attributes_t> entry_t;
typedef std::vector<entry_t> dictionary_t;
typedef std::string arguments_t;
typedef std::pair<name_t, arguments_t> event_t;
typedef std::pair<int, event_t> queued_event_t;
typedef std::list<queued_event_t> event_queue_t;

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
name_t getName(dictionary_t& dict, int index);
value_t getAttr(dictionary_t& dict, int index, key_t key, value_t defaultvalue);
event_t getEvent(event_queue_t& event, int currentcase);
std::pair<dictionary_t, event_queue_t> parseData(std::string filename);

#ifdef _WIN32
#include "win_systemFunctions.h"
#else
#include "nix_systemFunctions.h"
#endif // _WIN32

#endif // SYSTEMFUNCTIONS_H_INCLUDED
