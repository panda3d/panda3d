#ifndef __TIME_GENERAL_H__
#define __TIME_GENERAL_H__


Time_Span TimeDifference(const Time_Clock &time1, const Time_Clock &time2);
Time_Clock TimeDifference( const Time_Clock &time1, const Time_Span &Time_Span);
Time_Clock TimeAddition(const Time_Clock &time1, Time_Span &Time_Span);


Time_Clock operator+(const Time_Clock &tm, const Time_Span &ts);
Time_Clock operator-(const Time_Clock &tm, const Time_Span &ts);


bool SetFromTimeStr(const char * str, Time_Clock & outtime);
std::string GetTimeStr(const Time_Clock & intime);

//////////////////////////////////////////////////////////////
// Function name :  TimeDifference
// Description     :
// Return type  : inline Time_Span
// Argument         : const  Time_Clock &time1
// Argument         : const Time_Clock &time2
//////////////////////////////////////////////////////////////
inline Time_Span TimeDifference(const Time_Clock &time1, const Time_Clock &time2)
{
    timeval ans;
    TimeDif(time2.GetTval(), time1.GetTval(), ans);
    return Time_Span(ans);
}
//////////////////////////////////////////////////////////////
// Function name : TimeDifference
// Description     :
// Return type  :
// Argument         :  const Time_Clock &time1
// Argument         : const Time_Span &Time_Span
//////////////////////////////////////////////////////////////
inline Time_Clock TimeDifference( const Time_Clock &time1, const Time_Span &Time_Span)
{
    timeval ans;
    TimeDif(Time_Span.GetTval(), time1.GetTval(), ans);
    return Time_Clock(ans);
}
//////////////////////////////////////////////////////////////
// Function name : TimeAddition
// Description     :
// Return type  :
// Argument         : const Time_Clock &time1
// Argument         : Time_Span &Time_Span
//////////////////////////////////////////////////////////////
inline Time_Clock TimeAddition(const Time_Clock &time1, Time_Span &Time_Span)
{
    timeval ans;
    TimeAdd(time1.GetTval(), Time_Span.GetTval(), ans);
    return Time_Clock(ans);
}
//////////////////////////////////////////////////////////////
// Function name : Time_Clock& Time_Clock::operator+=
// Description     :
// Return type  : inline const
// Argument         : Time_Span &Time_Span
//////////////////////////////////////////////////////////////
inline const Time_Clock& Time_Clock::operator+=(const Time_Span &Time_Span)
{
    _my_time.tv_usec += Time_Span._my_time.tv_usec;
    _my_time.tv_sec += Time_Span._my_time.tv_sec;
    NormalizeTime(_my_time);
    return *this;
}
//////////////////////////////////////////////////////////////
// Function name : operator+
// Description     :
// Return type  : inline Time_Clock
// Argument         : const Time_Clock &tm
// Argument         : const Time_Span &ts
//////////////////////////////////////////////////////////////
inline Time_Clock operator+(const Time_Clock &tm, const Time_Span &ts)
{
    Time_Clock work(tm);
    work += ts;
    return work;
}
//////////////////////////////////////////////////////////////
// Function name : operator-
// Description     :
// Return type  : inline Time_Clock
// Argument         : const Time_Clock &tm
// Argument         : const Time_Span &ts
//////////////////////////////////////////////////////////////
inline Time_Clock operator-(const Time_Clock &tm, const Time_Span &ts)
{
    return TimeDifference(tm, ts);
}
//////////////////////////////////////////////////////////////
// Function name : Time_Clock& Time_Clock::operator-=
// Description     :
// Return type  : inline const
// Argument         : Time_Span &Time_Span
//////////////////////////////////////////////////////////////
inline const Time_Clock& Time_Clock::operator-=(const Time_Span &Time_Span)
{
    _my_time.tv_usec -= Time_Span._my_time.tv_usec;
    _my_time.tv_sec -= Time_Span._my_time.tv_sec;
    NormalizeTime(_my_time);
    return *this;
}
//////////////////////////////////////////////////////////////
// Function name : operator-
// Description     :
// Return type  : inline Time_Span
// Argument         : const Time_Clock &tm1
// Argument         : const Time_Clock &tm2
//////////////////////////////////////////////////////////////
inline Time_Span operator-(const Time_Clock &tm1, const Time_Clock &tm2)
{
    return TimeDifference(tm1, tm2);
}
//////////////////////////////////////////////////////////////
// Function name : char * GetTimeStr
// Description     :
// Return type  : inline const
// Argument         : const Time_Clock & intime
//////////////////////////////////////////////////////////////
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
//////////////////////////////////////////////////////////////
// Function name :  GetTimeStr
// Description     :
// Return type  : inline std::string
// Argument         : void
//////////////////////////////////////////////////////////////
inline std::string GetTimeStr()
{
    return GetTimeStr(Time_Clock::GetCurrentTime());
}
//////////////////////////////////////////////////////////////
// Function name : SetFromTimeStr
// Description     :
// Return type  : inline bool
// Argument         : const char * str
// Argument         : Time_Clock & outtime
//////////////////////////////////////////////////////////////
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
