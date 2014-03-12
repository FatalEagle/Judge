#include "resource.h"

void Judge::givePartialOutput(HTMLBuilder& log,
                              std::string filename)
{
    log<<"Partial Output:<br>";
    log.push("div class=\"partialoutput\"");
    std::ifstream output(filename);
    char partialOutput[11];
    memset(partialOutput, 0, sizeof(partialOutput));
    output.read(partialOutput, 10);
    output.close();
    std::string partialOutputString(partialOutput);
    HTMLBuilder::HTMLencode(partialOutputString);
    if(partialOutputString.empty())
        log<<"(no output)";
    else
        log<<partialOutputString;
    if(output.good())
        log<<" ...";
    log.pop();
}

std::string Judge::getResult(HTMLBuilder& log,
                             std::string executablename,
                             std::string args,
                             std::string inputfilename,
                             std::string outputfilename,
                             int timelimit,
                             SIZE_T memlimit,
                             int pnum)
{
    std::ofstream(outputfilename, std::ofstream::trunc).close();
    SECURITY_ATTRIBUTES sa;
    STARTUPINFO info;
    PROCESS_INFORMATION processInfo;
    ZeroMemory(&info, sizeof(info));
    info.cb=sizeof(info);
    ZeroMemory(&processInfo, sizeof(processInfo));
    sa.nLength=sizeof(sa);
    sa.lpSecurityDescriptor=NULL;
    sa.bInheritHandle=TRUE;
    HANDLE g_hChildStd_OUT_Wr=CreateFile(outputfilename.c_str(),
                                FILE_APPEND_DATA,
                                FILE_SHARE_WRITE | FILE_SHARE_READ,
                                &sa,
                                OPEN_ALWAYS,
                                FILE_ATTRIBUTE_NORMAL,
                                NULL);
    info.cb=sizeof(STARTUPINFO);
    info.dwFlags |= STARTF_USESTDHANDLES;
    HANDLE g_hChildStd_IN_Rd=NULL;
    HANDLE g_hChildStd_IN_Wr=NULL;
    if(!CreatePipe(&g_hChildStd_IN_Rd, &g_hChildStd_IN_Wr, &sa, 0))
        throw CreationError(out, GetLastError());
    if(!g_hChildStd_IN_Rd || !g_hChildStd_OUT_Wr)
        throw CreationError(out, "null child pipe handles");
    SetHandleInformation(g_hChildStd_IN_Wr, HANDLE_FLAG_INHERIT, 0);
    std::ifstream inputfile(inputfilename);
    std::string input=std::string(std::istreambuf_iterator<char>(inputfile), std::istreambuf_iterator<char>());
    inputfile.close();
    info.hStdInput=g_hChildStd_IN_Rd;
    info.hStdError=g_hChildStd_OUT_Wr;
    info.hStdOutput=g_hChildStd_OUT_Wr;
    int failure=0;
    bool exitflag=false;
    SIZE_T memoryusage=0;
    std::mutex memorylock;
    {
        out<<"Starting test case "<<pnum<<"..."<<std::endl;
        char cargs[args.length()+1];
        strcpy(cargs, args.c_str());
        cargs[args.length()]='\0';
        log.push("strong");
        log<<"Test case "<<pnum<<":";
        log.pop();
        log<<"<br>";
        accurate_timer processtimer((std::string("test case ")+toStr(pnum)).c_str(), out);
        if(!CreateProcess(executablename.c_str(),
                      cargs,
                      NULL,
                      NULL,
                      TRUE,
                      0,
                      NULL,
                      NULL,
                      &info,
                      &processInfo
                      ))
        {
            throw CreationError(out, "failed to create child process\n"+executablename+"\n"+args, GetLastError());
        }
        std::thread(inputSender, std::ref(input), std::ref(g_hChildStd_IN_Wr)).detach();
        std::thread memoryMonitorThread(memoryMonitor, std::ref(memorylock), std::ref(processInfo.hProcess), std::ref(memoryusage), std::ref(exitflag));
        while(std::chrono::duration_cast<std::chrono::milliseconds>(processtimer.refresh()).count()-timelimit<=1e-6)
        {
            memorylock.lock();
            if(memoryusage>memlimit)
            {
                failure=2;
                break;
            }
            memorylock.unlock();
            DWORD ret=WaitForSingleObject(processInfo.hProcess, 50);
            if(ret==WAIT_OBJECT_0)
                break;
            else if(ret==WAIT_FAILED)
                throw RuntimeError(out, GetLastError());
        }
        processtimer.stop();
        exitflag=true;
        if(!failure && timelimit-std::chrono::duration_cast<std::chrono::milliseconds>(processtimer.elapsed).count()<=1e-6)
            failure=1;
        switch(failure)
        {
        case 1:
        {
            log<<"Time  : over "<<timelimit<<" ms<br>";
            break;
        }
        case 2:
        {
            log<<"Time  : --<br>";
            break;
        }
        default:
        {
            log<<"Time  : "<<std::chrono::duration_cast<std::chrono::milliseconds>(processtimer.elapsed).count()<<" ms<br>";
            break;
        }
        }
        log<<"Memory: "<<static_cast<double>(memoryusage)/1048576.0<<" MiB<br>";
        log<<"Status: ";
        switch(failure)
        {
        case 1:
        {
            log<<"TLE<br>";
            givePartialOutput(log, outputfilename);
            flags|=TLE_FLAG;
            break;
        }
        case 2:
        {
            log<<"MLE";
            flags|=MLE_FLAG;
            break;
        }
        }
        memoryMonitorThread.join();
    }
    out<<"Memory used by test case "<<pnum<<": "<<std::fixed<<std::setprecision(5)<<static_cast<double>(memoryusage)/1048576.0<<" mebibytes"<<std::endl;
    CloseHandle(g_hChildStd_IN_Rd);
    CloseHandle(g_hChildStd_OUT_Wr);
    if(failure)
    {
        CloseHandle(g_hChildStd_IN_Wr);
        if(!TerminateProcess(processInfo.hProcess, 1))
            out<<"Warning: Process not terminated"<<std::endl;
        if(failure==1)
            out<<"Error: Timeout"<<std::endl;
        else if(failure==2)
            out<<"Error: Memory Limit Exceeded"<<std::endl;
        WaitForSingleObject(processInfo.hProcess, INFINITE);
        CloseHandle(processInfo.hProcess);
        CloseHandle(processInfo.hThread);
        out<<"Process Terminated."<<std::endl;
        switch(failure)
        {
        case 1:
            throw TimeoutError(out, timelimit);
        case 2:
            throw MemoryError(out, memlimit);
        }
    }
    std::ifstream output(outputfilename);
    std::string outputData=std::string(std::istreambuf_iterator<char>(output), std::istreambuf_iterator<char>());
    output.close();
    return outputData;
}

bool Judge::runtest(HTMLBuilder& log,
                    std::string executablename,
                    std::string args,
                    std::string inputfilename,
                    std::string outputfilename,
                    std::string answerfilename,
                    int timelimit,
                    SIZE_T memlimit,
                    int pnum,
                    mode runmode)
{
    switch(runmode)
    {
    case mode::NORMAL:
    {
        std::ifstream answer(answerfilename);
        std::string answerData=std::string(std::istreambuf_iterator<char>(answer), std::istreambuf_iterator<char>());
        answer.close();
        std::string outputData;
        log.push("div class=\"caseresult\"");
        try
        {
            outputData=getResult(log, executablename, args, inputfilename, outputfilename, timelimit, memlimit, pnum);
        }
        catch(...)
        {
            log<<"<br>";
            log.pop();
            return false;
        }
        auto outputLines=split(trim(outputData), "\n");
        auto answerLines=split(trim(answerData), "\n");
        bool good=(outputLines==answerLines);
        out<<"Test case completed. Passed? "<<std::boolalpha<<good<<std::endl;
        if(good)
            log<<"OK";
        else
        {
            log<<"WA<br>";
            givePartialOutput(log, outputfilename);
            flags|=WA_FLAG;
        }
        log<<"<br>";
        log.pop();
        return good;
    }
    case mode::SORT_OUTPUT:
    {
        std::ifstream answer(answerfilename);
        std::string answerData=std::string(std::istreambuf_iterator<char>(answer), std::istreambuf_iterator<char>());
        answer.close();
        std::string outputData;
        log.push("div class=\"caseresult\"");
        try
        {
            outputData=getResult(log, executablename, args, inputfilename, outputfilename, timelimit, memlimit, pnum);
        }
        catch(...)
        {
            log<<"<br>";
            log.pop();
            return false;
        }
        auto outputLines=split(trim(outputData), "\n");
        auto answerLines=split(trim(answerData), "\n");
        sort(outputLines.begin(), outputLines.end());
        sort(answerLines.begin(), answerLines.end());
        bool good=(outputLines==answerLines);
        out<<"Test case completed. Passed? "<<std::boolalpha<<good<<std::endl;
        if(good)
            log<<"OK";
        else
        {
            log<<"WA<br>";
            givePartialOutput(log, outputfilename);
            flags|=WA_FLAG;
        }
        log<<"<br>";
        log.pop();
        return good;
    }
    }
    return false;
}

#ifdef _WIN32
bool Judge::runExe(HTMLBuilder& log,
                   std::string executable,
                   std::string in,
                   std::string out,
                   std::string ans,
                   int timelimit,
                   SIZE_T memlimit,
                   int pnum,
                   mode runmode)
{
    return runtest(log, executable, "", in, out, ans, timelimit, memlimit, pnum, runmode);
}
#endif // _WIN32

bool Judge::runCpp(HTMLBuilder& log,
                   std::string cppFile,
                   std::string in,
                   std::string out,
                   std::string ans,
                   int timelimit,
                   SIZE_T memlimit,
                   int pnum,
                   mode runmode)
{
    return runtest(log, cppFile.substr(0, cppFile.rfind('.'))+std::string(".exe"), "", in, out, ans, timelimit, memlimit, pnum, runmode);
}

bool Judge::runJava(HTMLBuilder& log,
                    std::string javaDir,
                    std::string javaFile,
                    std::string in,
                    std::string out,
                    std::string ans,
                    int timelimit,
                    SIZE_T memlimit,
                    int pnum,
                    mode runmode)
{
    return runtest(log, "C:\\Windows\\System32\\cmd.exe", "/c java -cp "+javaDir+" "+javaFile, in, out, ans, timelimit, memlimit, pnum, runmode);
}

bool Judge::runPython(HTMLBuilder& log,
                      std::string pythonVersion,
                      std::string pythonFile,
                      std::string in,
                      std::string out,
                      std::string ans,
                      int timelimit,
                      SIZE_T memlimit,
                      int pnum,
                      mode runmode)
{
    return runtest(log, "C:"+runPath+"\\Python"+pythonVersion+"\\python.exe", "\"C:"+runPath+"\\Python"+pythonVersion+"\\python.exe\" \""+pythonFile+"\"", in, out, ans, timelimit, memlimit, pnum, runmode);
}

bool Judge::compile_cpp(std::string cpp_filename, std::string output_filename)
{
    const std::string gplusplus=std::string("\"C:")+runPath+std::string("\\g++.exe");
    // setting the stack size is probably Windows only
    const std::string arguments="-Wl,--stack,16777216 -std=c++11 -O2 -static";
    enquote(cpp_filename);
    enquote(output_filename);
    return system(join(" ", gplusplus, cpp_filename, arguments, std::string("-o ")+output_filename).c_str())==0;
}

bool Judge::compile_java(std::string java_filename)
{
    const std::string javac="javac";
    enquote(java_filename);
    return system(join(" ", javac, java_filename).c_str())==0;
}

void Judge::finalize(HTMLBuilder& log, logoutputmode logmode, std::string logfilename, std::string problemCode, std::ostringstream& _logss, std::ofstream& _logfs)
{
    log.close();
    // BUGFIX: Does not upload sometimes. Use SAVE_AND_UPLOAD until further notice.
    if(logmode==logoutputmode::UPLOAD_TO_WEB)
    {
        std::string submissionID;
        if(problemCode.empty())
        {
            out<<"Request for submission number success? "<<std::boolalpha<<request("http://", FTP_DOMAIN, "DMOPC/next.php", submissionID)<<std::endl;
            std::uniform_int_distribution<char> uniformDistribution('a', 'z');
            for(int i=0; i<FTP_RANDOM_CHARS; i++)
                submissionID+=uniformDistribution(DEFAULT_RNG);
        }
        else
            submissionID+=problemCode;
        out<<std::endl<<"Your submission ID is:"<<std::endl<<std::endl;
        for(int i=(CONSOLE_WIDTH-submissionID.length())/2; i>=0; i--)
            out<<' ';
        out<<submissionID<<std::endl<<std::endl;
#ifdef FTP_PHP
        std::string _out;
        out<<"Upload success? "<<std::boolalpha<<request("http://", FTP_DOMAIN, "DMOPC/submit/write.php?subid="+submissionID+"&data="+_logss.str(), _out)<<std::endl;
        out<<_out<<std::endl;
#else
        DWORD errorcode;
        DWORD ierrorcode;
        std::string errormessage;
        int retry=FTP_RETRY+1;
        DWORD bytesUploaded=0;
        while(bytesUploaded==0 && retry>0)
        {
            bytesUploaded=uploadContent(FTP_PREFIX, FTP_DOMAIN, "public_html/DMOPC/submissions", FTP_USERNAME, FTP_PASSWORD, submissionID+".html", _logss.str(), errorcode, ierrorcode, errormessage);
            out<<"Uploaded "<<bytesUploaded<<" bytes."<<std::endl;
            out<<"Returned with error code "<<errorcode<<" and Internet error code "<<ierrorcode<<std::endl;
            out<<"Error message:"<<std::endl<<errormessage<<std::endl;
            retry--;
            if(bytesUploaded==0 && retry>0)
                out<<"Retrying..."<<std::endl<<std::endl;
        }
#endif
    }
    else if(logmode==logoutputmode::SAVE_TO_FILE)
        _logfs.close();
    else if(logmode==logoutputmode::SAVE_AND_UPLOAD)
    {
        _logfs.open(logfilename);
        _logfs<<_logss.str();
        _logfs.close();
        std::string submissionID;
        if(problemCode.empty())
        {
            out<<"Request for submission number success? "<<std::boolalpha<<request("http://", FTP_DOMAIN, "DMOPC/next.php", submissionID)<<std::endl;
            std::uniform_int_distribution<char> uniformDistribution('a', 'z');
            for(int i=0; i<FTP_RANDOM_CHARS; i++)
                submissionID+=uniformDistribution(DEFAULT_RNG);
        }
        else
            submissionID+=problemCode;
        out<<std::endl<<"Your submission ID is:"<<std::endl<<std::endl;
        for(int i=(CONSOLE_WIDTH-submissionID.length())/2; i>=0; i--)
            out<<' ';
        out<<submissionID<<std::endl<<std::endl;
        DWORD errorcode;
        DWORD ierrorcode;
        std::string errormessage;
        int retry=FTP_RETRY+1;
        bool bSuccess=false;
        while(!bSuccess && retry>0)
        {
            bSuccess=uploadFile(FTP_PREFIX, FTP_DOMAIN, FTP_USERNAME, FTP_PASSWORD, logfilename, "public_html/DMOPC/submissions/"+submissionID+".html", errorcode, ierrorcode, errormessage);
            out<<"Upload success? "<<std::boolalpha<<bSuccess<<std::endl;
            out<<"Returned with error code "<<errorcode<<" and Internet error code "<<ierrorcode<<std::endl;
            out<<"Error message:"<<std::endl<<errormessage<<std::endl;
            retry--;
            if(!bSuccess && retry>0)
                out<<"Retrying..."<<std::endl<<std::endl;
        }
    }
}

int Judge::run(std::string program,
               std::string problemName,
               std::string problemCode,
               std::string timestamp)
{
    std::string programDirectory(program.substr(0, program.rfind('\\')+1));
    std::string programName(program.substr(program.rfind('\\')+1, program.rfind('.')-program.rfind('\\')-1));
    std::string extension=program.substr(program.rfind('.')+1);
    std::string pyVer;
    std::string realdirectory(program.substr(0, program.rfind('\\')));
    std::string directory(realdirectory+"\\Judge\\TestCases\\"+problemName+"\\");
    language lang=language::NONE;
    std::unordered_map<std::string, std::string> variables;
    auto dictevent=parseData(directory+problemName+".dat");
    auto dict=dictevent.first;
    auto event=dictevent.second;
    int timesToRun=toInt(getAttr(dict, 0, "repeat", "1"));
    int timesToPass=toInt(getAttr(dict, 0, "pass rate", "1"));
    int num_problems=dict.size()-2;
    int startquestion=toInt(getAttr(dict, 0, "start question", "0"));
    int points=toInt(getAttr(dict, 0, "initial points", "0"));
    std::string testnames[num_problems+1];
    std::string inputExtension=getAttr(dict, 0, "input extension", ".in");
    std::string outputExtension=getAttr(dict, 0, "output extenston", ".ans");
    std::string answerExtension=getAttr(dict, 0, "answer extension", ".out");
    std::string defaultTimeLimit=getAttr(dict, 0, "default time limit", "1000");
    std::string defaultMemoryLimit=getAttr(dict, 0, "default memory limit", "1073741824");
    std::string _runmode=getAttr(dict, 0, "mode", "normal");
    mode runmode;
    if(_runmode=="normal")
        runmode=mode::NORMAL;
    else if(_runmode=="sort output")
        runmode=mode::SORT_OUTPUT;
    else
        return -0x3f3f3f3f;
    std::vector<int> tlimit(num_problems+1);
    std::vector<SIZE_T> mlimit(num_problems+1);
    std::vector<int> pylimit(num_problems+1);
    std::vector<int> qpoints(num_problems+1);
    std::vector<int> batch(num_problems+2);
    for(int i=0; i<=num_problems; i++)
    {
        testnames[i]=getName(dict, i+1);
        tlimit[i]=toInt(getAttr(dict, i+1, "time limit", defaultTimeLimit));
        mlimit[i]=toType<SIZE_T>(getAttr(dict, i+1, "memory limit", defaultMemoryLimit));
        pylimit[i]=toInt(getAttr(dict, i+1, "python time limit", "-1"));
        if(pylimit[i]==-1)
            pylimit[i]=tlimit[i];
        qpoints[i]=toInt(getAttr(dict, i+1, "point value", "0"));
        batch[i]=toInt(getAttr(dict, i+1, "case number", "-1"));
        if(batch[i]==-1)
            batch[i]=-i-2;
    }
    batch[num_problems+1]=-1;
    std::string _logmode=getAttr(dict, 0, "log mode", "save");
    logoutputmode logmode;
    if(_logmode=="save")
        logmode=logoutputmode::SAVE_TO_FILE;
    else if(_logmode=="upload")
        logmode=logoutputmode::UPLOAD_TO_WEB;
    else if (_logmode=="save and upload")
        logmode=logoutputmode::SAVE_AND_UPLOAD;
    else
        return -0x3f3f3f3f;
    std::string logfilename=getAttr(dict, 0, "log file name", "");
    bool log_enabled=!logfilename.empty();
    std::ostringstream _logss;
    std::ofstream _logfs;
    std::ostream& _logstream=*((logmode==logoutputmode::SAVE_TO_FILE)?static_cast<std::ostream*>(&_logfs):
                             (logmode==logoutputmode::UPLOAD_TO_WEB)?static_cast<std::ostream*>(&_logss):
                             (logmode==logoutputmode::SAVE_AND_UPLOAD)?static_cast<std::ostream*>(&_logss):
                             static_cast<std::ostream*>(&_logfs));
    HTMLBuilder log(_logstream, log_enabled);
    log<<"<link href=\"http://"<<FTP_DOMAIN<<"/css/submissionresult.css\" rel=\"stylesheet\" type=\"text/css\" />";
    log.open();
    if(extension=="exe")
    {
        lang=language::EXECUTABLE;
    }
    else if(extension=="cpp")
    {
        lang=language::CPP;
        runPath=getAttr(dict, 0, "c++ path");
        if(!compile_cpp((programDirectory+programName+std::string(".cpp")), programDirectory+programName+std::string(".exe")))
        {
            out<<"Compilation Error!"<<std::endl;
            log.push("div class=\"finalresult CE\"");
            log.push("strong");
            log<<"Final Score: 0";
            log.pop();
            log<<"<br>Program status: CE<br>";
            if(timestamp.empty())
            {
                auto currealtime=std::time(nullptr);
                char timeString[1025];
                memset(timeString, 0, sizeof(timeString));
                strftime(timeString, 1024, "Time graded: %Y/%m/%d %H:%M:%S", std::localtime(&currealtime));
                log<<timeString;
            }
            else
                log<<"Time submitted: "<<timestamp;
            log<<"<br>Problem name: "<<problemName<<"<br>Submission name: "<<program.substr(program.rfind('\\')+1);
            if(log_enabled)
                finalize(log, logmode, logfilename, problemCode, _logss, _logfs);
            return 0;
        }
    }
    else if(extension=="java")
    {
        lang=language::JAVA;
        javaPath=getAttr(dict, 0, "java path");
        if(!compile_java(programDirectory+programName+std::string(".java")))
        {
            out<<"Compilation Error!"<<std::endl;
            log.push("div class=\"finalresult CE\"");
            log.push("strong");
            log<<"Final Score: 0";
            log.pop();
            log<<"<br>Program status: CE<br>";
            if(timestamp.empty())
            {
                auto currealtime=std::time(nullptr);
                char timeString[1025];
                memset(timeString, 0, sizeof(timeString));
                strftime(timeString, 1024, "Time graded: %Y/%m/%d %H:%M:%S", std::localtime(&currealtime));
                log<<timeString;
            }
            else
                log<<"Time submitted: "<<timestamp;
            log<<"<br>Problem name: "<<problemName<<"<br>Submission name: "<<program.substr(program.rfind('\\')+1);
            if(log_enabled)
                finalize(log, logmode, logfilename, problemCode, _logss, _logfs);
            return 0;
        }
    }
    else if(extension=="py")
    {
        lang=language::PYTHON;
        pyVer=getAttr(dict, 0, "python version", "25");
        runPath=getAttr(dict, 0, "python path");
    }
    variables["counter"]=toStr(startquestion);
    int batchpoints=0;
    for(int pnum=startquestion; pnum<=num_problems; pnum++, variables["counter"]=toStr(toInt(variables["counter"])+1))
    {
        auto ev=getEvent(event, pnum);
        if(!ev.first.empty())
        {
            auto args=split(ev.second, ",");
            if(ev.first=="set")
                variables[args[0]]=args[1];
        }
        out<<std::endl;
        bool passed=false;
        if(testnames[pnum].find('%')!=std::string::npos)
        {
            char buf[65536];
            snprintf(buf, 65536, testnames[pnum].c_str(), toInt(variables["counter"]));
            testnames[pnum]=buf;
        }
        std::string inputFile=directory+getAttr(dict, pnum+1, "input file", testnames[pnum]+inputExtension);
        std::string outputFile=realdirectory+"\\Judge\\Answers\\"+getAttr(dict, pnum+1, "output file", testnames[pnum]+outputExtension);
        std::string answerFile=directory+getAttr(dict, pnum+1, "answer file", testnames[pnum]+answerExtension);
        switch(lang)
        {
        case language::EXECUTABLE:
            passed=runMultiple(std::bind(&Judge::runExe, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5, std::placeholders::_6, std::placeholders::_7, std::placeholders::_8, std::placeholders::_9), timesToRun, std::ref(log), programDirectory+programName+".exe", inputFile, outputFile, answerFile, tlimit[pnum], mlimit[pnum], pnum, runmode)>=timesToPass;
            break;
        case language::CPP:
            passed=runMultiple(std::bind(&Judge::runCpp, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5, std::placeholders::_6, std::placeholders::_7, std::placeholders::_8, std::placeholders::_9), timesToRun, std::ref(log), programDirectory+programName+".cpp", inputFile, outputFile, answerFile, tlimit[pnum], mlimit[pnum], pnum, runmode)>=timesToPass;
            break;
        case language::JAVA:
            passed=runMultiple(std::bind(&Judge::runJava, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5, std::placeholders::_6, std::placeholders::_7, std::placeholders::_8, std::placeholders::_9, std::placeholders::_10), timesToRun, std::ref(log), programDirectory, programName, inputFile, outputFile, answerFile, tlimit[pnum], mlimit[pnum], pnum, runmode)>=timesToPass;
            break;
        case language::PYTHON:
            passed=runMultiple(std::bind(&Judge::runPython, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5, std::placeholders::_6, std::placeholders::_7, std::placeholders::_8, std::placeholders::_9, std::placeholders::_10), timesToRun, std::ref(log), pyVer, programDirectory+programName+".py", inputFile, outputFile, answerFile, pylimit[pnum], mlimit[pnum], pnum, runmode)>=timesToPass;
            break;
        default:
            break;
        }
        if(passed)
        {
            if(batch[pnum]!=batch[pnum+1])
            {
                if(batchpoints>=0)
                    points+=batchpoints+qpoints[pnum];
                batchpoints=0;
            }
            else if(batchpoints!=-1)
                batchpoints+=qpoints[pnum];
            out<<"Test case "<<pnum<<" of \""<<problemName<<"\" solved."<<std::endl;
        }
        else
        {
            if(batch[pnum]==batch[pnum+1])
                batchpoints=-1;
            out<<"Test case "<<pnum<<" of \""<<problemName<<"\" failed."<<std::endl;
        }
        out<<"Current Score: "<<points<<std::endl;
        out<<std::endl;
    }
    log<<"<br>";
    std::string finalstatus;
    if(flags&TLE_FLAG)
        finalstatus="TLE";
    else if(flags&WA_FLAG)
        finalstatus="WA";
    else if(flags&MLE_FLAG)
        finalstatus="MLE";
    else
        finalstatus="AC";
    log.push("div class=\"finalresult "+finalstatus+"\"");
    log.push("strong");
    log<<"Final Score: "<<points;
    log.pop();
    log<<"<br>Program status: "<<finalstatus<<"<br>";
    if(timestamp.empty())
    {
        auto currealtime=std::time(nullptr);
        char timeString[1025];
        memset(timeString, 0, sizeof(timeString));
        strftime(timeString, 1024, "Time graded: %Y/%m/%d %H:%M:%S", std::localtime(&currealtime));
        log<<timeString;
    }
    else
        log<<"Time submitted: "<<timestamp;
    log<<"<br>Problem name: "<<problemName<<"<br>Submission name: "<<program.substr(program.rfind('\\')+1);
    if(log_enabled)
        finalize(log, logmode, logfilename, problemCode, _logss, _logfs);
    return points;
}
