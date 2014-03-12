#ifndef TIMER_H_INCLUDED
#define TIMER_H_INCLUDED

#include "basicresource.h"

template<class ClockType>
class basic_timer
{
private:
    bool stopped;
    std::string _s;
    typename ClockType::time_point _b, _e;
    std::ostream& out;
public:
    std::chrono::duration<double> elapsed;
    basic_timer(const char* &_s_arg, std::ostream& out_arg):
        stopped(false),
        _s(_s_arg),
        _b(ClockType::now()),
        out(out_arg)
    {
        //
    }
    basic_timer(const char* &&_s_arg="timer", std::ostream& out_arg=std::cout):
        stopped(false),
        _s(_s_arg),
        _b(ClockType::now()),
        out(out_arg)
    {
        //
    }
    ~basic_timer()
    {
        if(!stopped)
            _e=ClockType::now();
        out<<"Time used by "<<_s<<": "<<(refresh().count())<<" seconds"<<std::endl;
    }
    std::chrono::duration<double>& refresh()
    {
        if(!stopped)
            elapsed=std::chrono::duration_cast<std::chrono::duration<double>>(ClockType::now()-_b);
        return elapsed;
    }
    void stop()
    {
        _e=ClockType::now();
        elapsed=std::chrono::duration_cast<std::chrono::duration<double>>(_e-_b);
        stopped=true;
    }
};

typedef basic_timer<std::chrono::steady_clock> timer;
typedef basic_timer<std::chrono::high_resolution_clock> accurate_timer;

#endif // TIMER_H_INCLUDED
