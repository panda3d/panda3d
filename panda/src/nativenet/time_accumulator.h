#ifndef __TIME_ACCUMULATOR_H__
#define __TIME_ACCUMULATOR_H__
// Think of this as a stopwatch that can be restarted.
class Time_Accumulator
{
public:
    Time_Accumulator();
    ~Time_Accumulator();

    void Start();
    void Stop();
    void Reset();
    void Set(const Time_Span & in);

    Time_Span  Report();
private:

    Time_Span       _total_time;    // the collected time from previous start/stops
    Time_Clock      *_accum_start;  // the time of day the clock started
};

// you can set the internal accumilator to a value..
inline void Time_Accumulator::Set(const Time_Span & in)
{
    _total_time = in;
    // this seems to make the most since .. if you are running the clock right
    // know... assume the timespane you are passing in is inclusive.. but keep
    // clock running.. May need to rethink this...
    if(_accum_start != nullptr)
    {
        Stop();
        Start();
    }
}
/**
 *
 */
inline Time_Accumulator::Time_Accumulator() : _total_time(0,0,0,0,0), _accum_start(nullptr)
{
}
/**
 *
 */
inline Time_Accumulator::~Time_Accumulator()
{
    if(_accum_start != nullptr)
        delete _accum_start;
}
/**
 *
 */
inline     void Time_Accumulator::Start()
{
    if(_accum_start == nullptr)
        _accum_start = new Time_Clock();
}
/**
 *
 */
inline     void Time_Accumulator::Stop()
{
    if(_accum_start != nullptr)
    {
         Time_Span   work1(Time_Clock::GetCurrentTime() - *_accum_start);
        _total_time += work1;
        delete _accum_start;
        _accum_start = nullptr;
    }
}
/**
 *
 */
void Time_Accumulator::Reset()
{
    if(_accum_start != nullptr)
    {
        delete _accum_start;
        _accum_start = nullptr;
    }
    _total_time.Set(0,0,0,0,0);
}
/**
 *
 */
inline Time_Span  Time_Accumulator::Report()
{
    Time_Span   answer(_total_time);
    if(_accum_start != nullptr)
    {
        Time_Span ww(Time_Clock::GetCurrentTime() - *_accum_start);
        answer += ww;
    }
    return answer;
}

#endif //__TIME_ACCUMULATOR_H__
