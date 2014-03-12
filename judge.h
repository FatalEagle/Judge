#ifndef JUDGE_H_INCLUDED
#define JUDGE_H_INCLUDED

#include "basicresource.h"
#include "HTMLBuilder.h"

class Judge
{
protected:
    std::ostream& out;
public:
    static const int TLE_FLAG=0x1;
    static const int WA_FLAG=0x2;
    static const int MLE_FLAG=0x4;
    int flags;
    std::string runPath;
    std::string javaPath;
    class CreationError
    {
    public:
        CreationError(std::ostream& out)
        {
            out<<"A creation error occured."<<std::endl;
        }
        CreationError(std::ostream& out, int id)
        {
            out<<"A creation error occured with id: "<<id<<std::endl;
        }
        CreationError(std::ostream& out, std::string message)
        {
            out<<"A creation error occured: "<<message<<std::endl;
        }
        CreationError(std::ostream& out, std::string message, int id)
        {
            out<<"A creation error occured with id "<<id<<": "<<message<<std::endl;
        }
    };
    class TimeoutError
    {
    public:
        TimeoutError(std::ostream& out, int timelimit)
        {
            out<<"A timeout error occured because the program exceeded the time limit: "<<timelimit<<"ms"<<std::endl;
        }
    };
    class MemoryError
    {
    public:
        MemoryError(std::ostream& out, SIZE_T memlimit)
        {
            out<<"A memory error occured because the program exceeded the memory limit: "<<memlimit<<"b"<<std::endl;
        }
    };
    class RuntimeError
    {
    public:
        RuntimeError(std::ostream& out, int id)
        {
            out<<"A runtime error occured with id: "<<id<<std::endl;
        }
    };
    enum class language: int
    {
        NONE=0,
        EXECUTABLE,
        CPP,
        JAVA,
        PYTHON
    };
    enum class mode: int
    {
        NORMAL=0,
        SORT_OUTPUT
    };
    enum class logoutputmode: int
    {
        SAVE_TO_FILE=0,
        UPLOAD_TO_WEB,
        SAVE_AND_UPLOAD
    };
    Judge(std::ostream& out_arg):
        out(out_arg),
        flags(0)
    {
        //
    }
    ~Judge()
    {
        //
    }
    void givePartialOutput(HTMLBuilder& log,
                           std::string filename);
    std::string getResult(HTMLBuilder& log,
                          std::string executablename,
                          std::string args,
                          std::string inputfilename,
                          std::string outputfilename,
                          int timelimit,
                          SIZE_T memlimit,
                          int pnum);
    bool runtest(HTMLBuilder& log,
                 std::string executablename,
                 std::string args,
                 std::string inputfilename,
                 std::string outputfilename,
                 std::string answerfilename,
                 int timelimit,
                 SIZE_T memlimit,
                 int pnum,
                 mode runmode);
#ifdef _WIN32
    bool runExe(HTMLBuilder& log,
                std::string executable,
                std::string in,
                std::string out,
                std::string ans,
                int timelimit,
                SIZE_T memlimit,
                int pnum,
                mode runmode);
#endif // _WIN32
    bool runCpp(HTMLBuilder& log,
                std::string cppFile,
                std::string in,
                std::string out,
                std::string ans,
                int timelimit,
                SIZE_T memlimit,
                int pnum,
                mode runmode);
    bool runJava(HTMLBuilder& log,
                 std::string javaDir,
                 std::string javaFile,
                 std::string in,
                 std::string out,
                 std::string ans,
                 int timelimit,
                 SIZE_T memlimit,
                 int pnum,
                 mode runmode);
    bool runPython(HTMLBuilder& log,
                   std::string pythonVersion,
                   std::string pythonFile,
                   std::string in,
                   std::string out,
                   std::string ans,
                   int timelimit,
                   SIZE_T memlimit,
                   int pnum,
                   mode runmode);
    bool compile_cpp(std::string cpp_filename, std::string output_filename);
    bool compile_java(std::string java_filename);
    void finalize(HTMLBuilder& log, logoutputmode logmode, std::string logfilename, std::string problemCode, std::ostringstream& _logss, std::ofstream& _logfs);
    int run(std::string program,
            std::string problemName,
            std::string problemCode,
            std::string timestamp);
};

#endif // JUDGE_H_INCLUDED
