#ifndef TIMER_H_INCLUDED
#define TIMER_H_INCLUDED

#include "basicresource.h"

class timer
{
private:
    bool stopped;
    std::string _s;
    std::chrono::steady_clock::time_point _b, _e;
    std::ostream& out;
public:
    timer(const char* &_s_arg, std::ostream& out_arg):
        stopped(false),
        _s(_s_arg),
        _b(std::chrono::steady_clock::now()),
        out(out_arg)
    {
        //
    }
    timer(const char* &&_s_arg="timer", std::ostream& out_arg=std::cout):
        stopped(false),
        _s(_s_arg),
        _b(std::chrono::steady_clock::now()),
        out(out_arg)
    {
        //
    }
    ~timer()
    {
        if(!stopped)
            _e=std::chrono::steady_clock::now();
        out<<"Time used by "<<_s<<": "<<std::chrono::duration_cast<std::chrono::duration<double>>(_e-_b).count()<<" seconds"<<std::endl;
    }
    void stop()
    {
        _e=std::chrono::steady_clock::now();
        stopped=true;
    }
};

#endif // TIMER_H_INCLUDED
