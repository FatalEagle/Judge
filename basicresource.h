#ifndef BASICRESOURCE_H_INCLUDED
#define BASICRESOURCE_H_INCLUDED

#include <algorithm>
#include <cctype>
#include <chrono>
#include <cstring>
#include <ctime>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <list>
#include <locale>
#include <mutex>
#include <random>
#include <sstream>
#include <stack>
#include <string>
#include <thread>
#include <unordered_map>
#include <utility>
#include <vector>
#ifdef _WIN32
#include <windows.h>
#include <wininet.h>
#include <psapi.h>
#endif // _WIN32

//#define FTP_PHP
/*
#define FTP_USERNAME "placeholder"
#define FTP_PASSWORD "placeholder"
#define FTP_PREFIX "ftp://"
#define FTP_DOMAIN "placeholder.placeholder"
*/
#define FTP_USERNAME "dmopc@dmci.me"
#define FTP_PASSWORD "SecondTuesday!"
#define FTP_PREFIX "ftp://"
#define FTP_DOMAIN "dmci.me"

#define FTP_RETRY 10
#define FTP_RANDOM_CHARS 2

#define CONSOLE_WIDTH 80

extern std::mt19937 DEFAULT_RNG;

#endif // BASICRESOURCE_H_INCLUDED
