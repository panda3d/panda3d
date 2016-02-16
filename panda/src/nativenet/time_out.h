/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file time_out.h
 */


#ifndef __TIME_OUT_H__
#define __TIME_OUT_H__

////////////////////////////////////////////////////////////////////
//
// think of this class as a time based alarm..
//
// would be nice to have a template implementation of this class .. could avoud some storage and some math ..
//
// I would do this but not sure how to represent the duration in the template ??
//
////////////////////////////////////////////////////////////////////
class Time_Out
{
public:
    Time_Out()
    {
    }

    Time_Out(const Time_Span & dur) : _alarm_time(Time_Clock::GetCurrentTime() + dur) , _duration(dur)
    {
    }
/*
    Time_Out(const Time_Clock & tm, const Time_Span & dur) : _alarm_time(tm + dur) , _duration(dur)
    {
    }
  */
    void ResetAll(const Time_Clock &tm, const Time_Span &sp);
    void ReStart();
    void ResetTime(const Time_Clock & tm);
    void SetTimeOutSec(int sec);

    bool Expired(const Time_Clock &tm, bool reset = false);
    bool Expired(bool reset = false);

    Time_Span Remaining(const Time_Clock & tm) const;
    Time_Span Remaining() const;

    void ForceToExpired()
    {
        _alarm_time.ToCurrentTime();
    }

    bool operator() (bool reset= false)
    {
        return Expired(reset);
    }
    bool operator() (const Time_Clock &tm, bool reset = false)
    {
        return Expired(tm, reset);
    }

    Time_Clock GetAlarm(void)
    {
        return _alarm_time;
    }

    Time_Span Duration() const { return _duration; };

    void NextInStep(Time_Clock  &curtime)
    {
        _alarm_time += _duration;
        if(_alarm_time <=curtime)  // if we fall way behind.. just ratchet it up ...
            _alarm_time = curtime+_duration;
    }
private:
    Time_Clock _alarm_time;
    Time_Span _duration;
};
////////////////////////////////////////////////////////////////////
//     Function: Time_Out::ReStart
//  Description:
////////////////////////////////////////////////////////////////////
inline void Time_Out::ResetAll(const Time_Clock &tm, const Time_Span &sp)
{
    _duration = sp;
    _alarm_time = tm + _duration;
}
////////////////////////////////////////////////////////////////////
//     Function: Time_Span::ReStart
//  Description:
////////////////////////////////////////////////////////////////////
inline void Time_Out::SetTimeOutSec(int sec)
{
    _duration.Set(0, 0, 0, sec, 0);
    ReStart();
}
////////////////////////////////////////////////////////////////////
//     Function: Time_Span::ReStart
//  Description:
////////////////////////////////////////////////////////////////////
inline void Time_Out::ReStart()
{
    _alarm_time = Time_Clock::GetCurrentTime() + _duration;
}
////////////////////////////////////////////////////////////////////
//     Function: ResetTime
//  Description:
////////////////////////////////////////////////////////////////////
inline void Time_Out::ResetTime(const Time_Clock & tm)
{
    _alarm_time = tm + _duration;

}
////////////////////////////////////////////////////////////////////
//     Function: Time_Span::Expired
//  Description:
////////////////////////////////////////////////////////////////////
inline bool Time_Out::Expired(const Time_Clock &tm, bool reset)
{
    bool answer = (_alarm_time <= tm) ;
    if (answer && reset)
        ResetTime(tm);
    return answer;
}
////////////////////////////////////////////////////////////////////
//     Function: Time_Span::Expired
//  Description:
////////////////////////////////////////////////////////////////////
inline bool Time_Out::Expired(bool reset)
{
    return Expired(Time_Clock::GetCurrentTime(), reset);
}
////////////////////////////////////////////////////////////////////
//     Function: Time_Span::Remaining
//  Description:
////////////////////////////////////////////////////////////////////
inline Time_Span Time_Out::Remaining(const Time_Clock & tm) const
{
    return _alarm_time - tm;
}
////////////////////////////////////////////////////////////////////
//     Function: Time_Span::Remaining
//  Description:
////////////////////////////////////////////////////////////////////
inline Time_Span Time_Out::Remaining() const
{
    return Remaining(Time_Clock::GetCurrentTime());
}

#endif //__TIME_OUT_H__
