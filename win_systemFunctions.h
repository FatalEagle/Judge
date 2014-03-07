#ifndef WIN_SYSTEMFUNCTIONS_H_INCLUDED
#define WIN_SYSTEMFUNCTIONS_H_INCLUDED

#ifndef toInt
#define toInt toType<int>
#endif

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

void HTMLencode(std::string& data)
{
    std::string buffer;
    buffer.reserve(data.size()+10);
    for(size_t pos=0; pos!=data.size(); ++pos)
    {
        switch(data[pos])
        {
        case '&':
            buffer.append("&amp;");
            break;
        case '\"':
            buffer.append("&quot;");
            break;
        case '\'':
            buffer.append("&apos;");
            break;
        case '<':
            buffer.append("&lt;");
            break;
        case '>':
            buffer.append("&gt;");
            break;
        case '\n':
            buffer.append("<br>");
            break;
        default:
            buffer.append(&data[pos], 1);
            break;
        }
    }
    data.swap(buffer);
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

void inputSender(std::string& input, HANDLE& g_hChildStd_IN_Wr)
{
    DWORD dwWritten;
    WriteFile(g_hChildStd_IN_Wr, input.c_str(), input.length(), &dwWritten, NULL);
    CloseHandle(g_hChildStd_IN_Wr);
}

void memoryMonitor(std::mutex& mtx, HANDLE& process, SIZE_T& memoryusage, bool& exitflag)
{
    PROCESS_MEMORY_COUNTERS_EX pmc;
    while(!exitflag)
    {
        GetProcessMemoryInfo(process, (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc));
        mtx.lock();
        if(pmc.PrivateUsage>memoryusage)
            memoryusage=pmc.PrivateUsage;
        mtx.unlock();
        Sleep(100);
    }
}

template<class T1, class... Args>
int runMultiple(T1 runType, int total, Args... args)
{
    int number=0;
    while(total--)
        number+=runType(args...);
    return number;
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

std::string getName(std::vector<std::pair<std::string, std::unordered_map<std::string, std::string>>>& dict, int index)
{
    return dict[index].first;
}

std::string getAttr(std::vector<std::pair<std::string, std::unordered_map<std::string, std::string>>>& dict, int index, std::string key, std::string defaultvalue=std::string())
{
    auto it=dict[index].second.find(key);
    if(it==dict[index].second.end())
        return defaultvalue;
    return it->second;
}

std::pair<std::string, std::string> getEvent(std::list<std::pair<int, std::pair<std::string, std::string>>>& event, int currentcase)
{
    if(!event.empty())
    {
        while(currentcase>event.front().first)
            event.pop_front();
        if(currentcase==event.front().first)
        {
            std::pair<std::string, std::string> ret=event.front().second;
            event.pop_front();
            return ret;
        }
    }
    return make_pair(std::string(), std::string());
}

std::pair<std::vector<std::pair<std::string, std::unordered_map<std::string, std::string>>>, std::list<std::pair<int, std::pair<std::string, std::string>>>> parseData(std::string filename)
{
    std::vector<std::pair<std::string, std::unordered_map<std::string, std::string>>> dict;
    std::list<std::pair<int, std::pair<std::string, std::string>>> event;
    dict.push_back(make_pair(std::string(), std::unordered_map<std::string, std::string>()));
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
                std::string command;
                if(cpos==std::string::npos)
                {
                    command=line.substr(1, line.length()-2);
                    event.push_back(make_pair(static_cast<int>(dict.size()-1), make_pair(trim(command), std::string())));
                }
                else
                {
                    command=line.substr(1, cpos-1);
                    std::string arguments=line.substr(cpos+1, line.length()-(cpos+2));
                    event.push_back(make_pair(static_cast<int>(dict.size()-1), make_pair(trim(command), trim(arguments))));
                }
            }
            else if(line[0]=='[' && line[line.length()-1]==']')
                dict.push_back(make_pair(line.substr(1, line.length()-2), std::unordered_map<std::string, std::string>()));
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

DWORD uploadContent(std::string ftp_prefix, std::string domain, std::string directory, std::string username, std::string password, std::string filename, std::string content, DWORD& errorcode, DWORD& ierrorcode, std::string& errormessage)
{
    if(InternetAttemptConnect(0)==ERROR_SUCCESS)
    {
        HINTERNET hInternet=InternetOpen((ftp_prefix+domain).c_str(), INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
        if(hInternet!=NULL)
        {
            HINTERNET hConnect=InternetConnect(hInternet, domain.c_str(), INTERNET_DEFAULT_FTP_PORT, username.c_str(), password.c_str(), INTERNET_SERVICE_FTP, 0, 0);
            if(hConnect!=NULL)
            {
                HINTERNET ftpfile=FtpOpenFile(hConnect, (directory+"/"+filename).c_str(), GENERIC_WRITE, FTP_TRANSFER_TYPE_BINARY, 0);
                if(ftpfile!=NULL)
                {
                    DWORD dwWritten=0;
                    if(!InternetWriteFile(ftpfile, content.c_str(), content.length(), &dwWritten))
                    {
                        char szBuffer[4097];
                        memset(szBuffer, 0, sizeof(szBuffer));
                        DWORD dwBufferLength=4096;
                        InternetGetLastResponseInfo(&ierrorcode, szBuffer, &dwBufferLength);
                        errormessage=szBuffer;
                        errorcode=GetLastError();
                    }
                    else
                    {
                        errorcode=0;
                        ierrorcode=0;
                        errormessage.clear();
                    }
                    InternetCloseHandle(ftpfile);
                    InternetCloseHandle(hConnect);
                    InternetCloseHandle(hInternet);
                    return dwWritten;
                }
                if(errormessage.empty())
                    errormessage="ftpfile is NULL";
                InternetCloseHandle(ftpfile);
            }
            if(errormessage.empty())
                errormessage="hConnect is NULL";
            InternetCloseHandle(hConnect);
        }
        if(errormessage.empty())
            errormessage="hInternet is NULL";
        InternetCloseHandle(hInternet);
    }
    errorcode=GetLastError();
    return 0;
}

bool uploadFile(std::string ftp_prefix, std::string domain, std::string username, std::string password, std::string localfilename, std::string filename, DWORD& errorcode, DWORD& ierrorcode, std::string& errormessage)
{
    bool bSuccess=false;
    if(InternetAttemptConnect(0)==ERROR_SUCCESS)
    {
        HINTERNET hInternet=InternetOpen((ftp_prefix+domain).c_str(), INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
        if(hInternet!=NULL)
        {
            HINTERNET hConnect=InternetConnect(hInternet, domain.c_str(), INTERNET_DEFAULT_FTP_PORT, username.c_str(), password.c_str(), INTERNET_SERVICE_FTP, 0, 0);
            if(hConnect!=NULL)
            {
                if(!FtpPutFile(hConnect, localfilename.c_str(), filename.c_str(), FTP_TRANSFER_TYPE_BINARY, 0))
                {
                    char szBuffer[4097];
                    memset(szBuffer, 0, sizeof(szBuffer));
                    DWORD dwBufferLength=4096;
                    InternetGetLastResponseInfo(&ierrorcode, szBuffer, &dwBufferLength);
                    errormessage=szBuffer;
                    errorcode=GetLastError();
                }
                else
                {
                    errorcode=0;
                    ierrorcode=0;
                    errormessage.clear();
                    bSuccess=true;
                }
            }
            InternetCloseHandle(hConnect);
        }
        InternetCloseHandle(hInternet);
    }
    return bSuccess;
}

bool request(std::string http_prefix, std::string domain, std::string filename, std::string& out)
{
    bool bSuccess=false;
    LPCTSTR rgpszAcceptTypes[]= {TEXT("text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8"), NULL};
    if(InternetAttemptConnect(0)==ERROR_SUCCESS)
    {
        HINTERNET hInternet=InternetOpen((http_prefix+domain).c_str(), INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
        if(hInternet!=NULL)
        {
            HINTERNET hConnect=InternetConnect(hInternet, domain.c_str(), INTERNET_DEFAULT_HTTP_PORT, NULL, NULL, INTERNET_SERVICE_HTTP, 0, 0);
            if(hConnect!=NULL)
            {
                HINTERNET hRequest=HttpOpenRequest(hConnect, TEXT("GET"), filename.c_str(), NULL, NULL, rgpszAcceptTypes, INTERNET_FLAG_RESYNCHRONIZE, 0);
                if(hRequest!=NULL)
                {
                    if(HttpSendRequest(hRequest, (TCHAR*)"Content-type:text/html", -1L, NULL, 0))
                    {
                        char ch;
                        DWORD dwBytes;
                        while(InternetReadFile(hRequest, &ch, 1, &dwBytes) && dwBytes==1)
                            out+=ch;
                        bSuccess=true;
                    }
                }
                InternetCloseHandle(hRequest);
            }
            InternetCloseHandle(hConnect);
        }
        InternetCloseHandle(hInternet);
    }
    return bSuccess;
}

#endif // WIN_SYSTEMFUNCTIONS_H_INCLUDED
