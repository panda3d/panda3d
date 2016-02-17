/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file time_general.h
 */


#ifndef __TIME_GENERAL_H__
#define __TIME_GENERAL_H__


Time_Span TimeDifference(const Time_Clock &time1, const Time_Clock &time2);
Time_Clock TimeDifference( const Time_Clock &time1, const Time_Span &Time_Span);
Time_Clock TimeAddition(const Time_Clock &time1, Time_Span &Time_Span);


Time_Clock operator+(const Time_Clock &tm, const Time_Span &ts);
Time_Clock operator-(const Time_Clock &tm, const Time_Span &ts);


bool SetFromTimeStr(const char * str, Time_Clock & outtime);
std::string GetTimeStr(const Time_Clock & intime);

/**

 */
inline Time_Span TimeDifference(const Time_Clock &time1, const Time_Clock &time2)
{
    timeval ans;
    TimeDif(time2.GetTval(), time1.GetTval(), ans);
    return Time_Span(ans);
}
/**

 */
inline Time_Clock TimeDifference( const Time_Clock &time1, const Time_Span &Time_Span)
{
    timeval ans;
    TimeDif(Time_Span.GetTval(), time1.GetTval(), ans);
    return Time_Clock(ans);
}
/**

 */
inline Time_Clock TimeAddition(const Time_Clock &time1, Time_Span &Time_Span)
{
    timeval ans;
    TimeAdd(time1.GetTval(), Time_Span.GetTval(), ans);
    return Time_Clock(ans);
}
/**

 */
inline const Time_Clock& Time_Clock::operator+=(const Time_Span &Time_Span)
{
    _my_time.tv_usec += Time_Span._my_time.tv_usec;
    _my_time.tv_sec += Time_Span._my_time.tv_sec;
    NormalizeTime(_my_time);
    return *this;
}
/**

 */
inline Time_Clock operator+(const Time_Clock &tm, const Time_Span &ts)
{
    Time_Clock work(tm);
    work += ts;
    return work;
}
/**

 */
inline Time_Clock operator-(const Time_Clock &tm, const Time_Span &ts)
{
    return TimeDifference(tm, ts);
}
/**

 */
inline const Time_Clock& Time_Clock::operator-=(const Time_Span &Time_Span)
{
    _my_time.tv_usec -= Time_Span._my_time.tv_usec;
    _my_time.tv_sec -= Time_Span._my_time.tv_sec;
    NormalizeTime(_my_time);
    return *this;
}
/**

 */
inline Time_Span operator-(const Time_Clock &tm1, const Time_Clock &tm2)
{
    return TimeDifference(tm1, tm2);
}
/**

 */
inline std::string GetTimeStr(const Time_Clock & intime)
{
    static std::string ts;
    static Time_Clock prev_time;

    if (prev_time != intime || ts.empty())
    {
        ts = intime.Format("%Y-%m-%d %H:%M:%S");
        prev_time = intime;
    }
    return ts;
}
/**

 */
inline std::string GetTimeStr()
{
    return GetTimeStr(Time_Clock::GetCurrentTime());
}
/**

 */
inline bool SetFromTimeStr(const char * str, Time_Clock & outtime)
{
    int year = 0;
    int month = 0;
    int day = 0;
    int hour = 0;
    int min = 0;
    int sec = 0;

    if (sscanf(str, "%d-%d-%d %d:%d:%d", &year, &month, &day, &hour, &min, &sec) != 6)
        return false;

    outtime.Set(year, month, day, hour, min, sec);
    return true;
}

#endif //__TIME_GENERAL_H__
