#include "resource.h"

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
        memoryusage=pmc.PeakPagefileUsage;
        mtx.unlock();
        Sleep(100);
    }
}

DWORD uploadContent(std::string ftp_prefix,
                    std::string domain,
                    std::string directory,
                    std::string username,
                    std::string password,
                    std::string filename,
                    std::string content,
                    DWORD& errorcode,
                    DWORD& ierrorcode,
                    std::string& errormessage)
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

bool uploadFile(std::string ftp_prefix,
                std::string domain,
                std::string username,
                std::string password,
                std::string localfilename,
                std::string filename,
                DWORD& errorcode,
                DWORD& ierrorcode,
                std::string& errormessage)
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

bool request(std::string http_prefix,
             std::string domain,
             std::string filename,
             std::string& out)
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
