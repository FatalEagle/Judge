#ifndef WIN_SYSTEMFUNCTIONS_H_INCLUDED
#define WIN_SYSTEMFUNCTIONS_H_INCLUDED

#ifndef toInt
#define toInt toType<int>
#endif

void inputSender(std::string& input,
                 HANDLE& g_hChildStd_IN_Wr);
void memoryMonitor(std::mutex& mtx,
                   HANDLE& process,
                   SIZE_T& memoryusage,
                   bool& exitflag);
std::string getName(std::vector<
                                std::pair<
                                          std::string,
                                          std::unordered_map<std::string, std::string>
                                          >
                                >& dict,
                    int index);
std::string getAttr(std::vector<
                                std::pair<
                                          std::string,
                                          std::unordered_map<std::string, std::string>
                                          >
                                >& dict,
                    int index,
                    std::string key,
                    std::string defaultvalue=std::string());
std::pair<
          std::string,
          std::string
          >
          getEvent(std::list<
                               std::pair<
                                         int,
                                         std::pair<
                                                   std::string,
                                                   std::string
                                                  >
                                        >
                              >& event,
          int currentcase);
std::pair<
          std::vector<
                      std::pair<
                                std::string,
                                std::unordered_map<std::string, std::string>
                                >
                     >,
          std::list<
                    std::pair<
                              int,
                              std::pair<
                                        std::string,
                                        std::string
                                       >
                             >
                   >
         > parseData(std::string filename);
DWORD uploadContent(std::string ftp_prefix,
                    std::string domain,
                    std::string directory,
                    std::string username,
                    std::string password,
                    std::string filename,
                    std::string content,
                    DWORD& errorcode,
                    DWORD& ierrorcode,
                    std::string& errormessage);
bool uploadFile(std::string ftp_prefix,
                std::string domain,
                std::string username,
                std::string password,
                std::string localfilename,
                std::string filename,
                DWORD& errorcode,
                DWORD& ierrorcode,
                std::string& errormessage);
bool request(std::string http_prefix,
             std::string domain,
             std::string filename,
             std::string& out);

#endif // WIN_SYSTEMFUNCTIONS_H_INCLUDED
