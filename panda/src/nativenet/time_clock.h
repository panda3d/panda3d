#ifndef __Time_H__
#define __Time_H__ 
//////////////////////////////////////////////////////
// Class : Time_Clock
//
// Description:
//  This class is to provide a consistant interface and storage to
//  clock time .. Epoch based time to the second
//
// jan-2000 .. rhh changinging all time to use sub second timing...
//
//
//////////////////////////////////////////////////////


#include <stdio.h>

class Time_Span;

class Time_Clock
{
    friend class Time_Span;
    
public:
    // Constructors
    static Time_Clock GetCurrentTime();
    void ToCurrentTime();
    Time_Clock( timeval &in_mytime)
    {
        _my_time = in_mytime;
    };
    Time_Clock();
    Time_Clock(time_t time);
    Time_Clock(long secs, long usecs);
    Time_Clock(int nYear, int nMonth, int nDay, int nHour, int nMin, int nSec, unsigned long microseconds = 0, int nDST = -1);
    Time_Clock(const Time_Clock& timeSrc);
    
    inline const Time_Clock& operator=(const Time_Clock& timeSrc);
    inline const Time_Clock& operator=(time_t t);
    
    // Attributes
    struct tm* GetGmtTm(struct tm* ptm = NULL) const;
    struct tm* GetLocalTm(struct tm* ptm = NULL) const;
    
    time_t GetTime() const;
    int GetYear() const;
    int GetMonth() const;       // month of year (1 = Jan)
    int GetDay() const;         // day of month
    int GetHour() const;
    int GetMinute() const;
    int GetSecond() const;
    int GetDayOfWeek() const;   // 1=Sun, 2=Mon, ..., 7=Sat
    
    void Set(int nYear, int nMonth, int nDay, int nHour, int nMin, int nSec, unsigned long microseconds = 0, int nDST = -1);
    
    
    // Operations
    // time math
    const Time_Clock& operator+=(const Time_Span &Time_Span);
    const Time_Clock& operator-=(const Time_Span &Time_Span);
    bool operator==(const Time_Clock &time) const;
    bool operator!=(const Time_Clock &time) const;
    bool operator<(const Time_Clock &time) const;
    bool operator>(const Time_Clock &time) const;
    bool operator<=(const Time_Clock &time) const;
    bool operator>=(const Time_Clock &time) const;
    
    
    time_t GetTime_t()
    {
        return _my_time.tv_sec;
    };
    unsigned long GetUsecPart()
    {
        return _my_time.tv_usec;
    };
    
    // formatting using "C" strftime
    std::string Format(const char * pFormat) const;
    std::string FormatGmt(const char * pFormat) const;
    const timeval & GetTval()
    {
        return _my_time;
    };
    const timeval & GetTval() const
    {
        return _my_time;
    } ;
    private:
        struct timeval _my_time;
};
/////////////////////////////////////////////////////////////////////////////
// Time_Clock - absolute time

/////////////////////////////////////////////////////////////
// Function name : Time_Clock::Time_Clock
// Description     : Construction from parts
// Argument         : int nYear
// Argument         : int nMonth
// Argument         : int nDay
// Argument         : int nHour
// Argument         : int nMin
// Argument         : int nSec
// Argument         : int nDST
//////////////////////////////////////////////////////////
inline Time_Clock::Time_Clock(int nYear, int nMonth, int nDay, int nHour, int nMin, int nSec, unsigned long microseconds , int nDST)
{
    struct tm atm;
    atm.tm_sec = nSec;
    atm.tm_min = nMin;
    atm.tm_hour = nHour;
    assert(nDay >= 1 && nDay <= 31);
    atm.tm_mday = nDay;
    assert(nMonth >= 1 && nMonth <= 12);
    atm.tm_mon = nMonth - 1;        // tm_mon is 0 based
    assert(nYear >= 1900);
    atm.tm_year = nYear - 1900;     // tm_year is 1900 based
    atm.tm_isdst = nDST;
    _my_time.tv_sec = (long)mktime(&atm);
    assert(_my_time.tv_sec != -1);       // indicates an illegal input time
    _my_time.tv_usec = microseconds;
    assert(_my_time.tv_usec < 1000000);
}
//////////////////////////////////////////////////////////////
// Function name : Time_Clock::Set
// Description     :
// Return type  : inline
// Argument         : int nYear
// Argument         : int nMonth
// Argument         : int nDay
// Argument         : int nHour
// Argument         : int nMin
// Argument         : int nSec
// Argument         : unsigned long microseconds
// Argument         : int nDST
//////////////////////////////////////////////////////////////
inline void Time_Clock::Set(int nYear, int nMonth, int nDay, int nHour, int nMin, int nSec, unsigned long microseconds , int nDST)
{
    struct tm atm;
    atm.tm_sec = nSec;
    atm.tm_min = nMin;
    atm.tm_hour = nHour;
    assert(nDay >= 1 && nDay <= 31);
    atm.tm_mday = nDay;
    assert(nMonth >= 1 && nMonth <= 12);
    atm.tm_mon = nMonth - 1;        // tm_mon is 0 based
    assert(nYear >= 1900);
    atm.tm_year = nYear - 1900;     // tm_year is 1900 based
    atm.tm_isdst = nDST;
    _my_time.tv_sec = (long)mktime(&atm);
    assert(_my_time.tv_sec != -1);       // indicates an illegal input time
    _my_time.tv_usec = microseconds;
    assert(_my_time.tv_usec < 1000000);
}
/////////////////////////////////////////////////////////////
// Function name : Time_Clock::GetCurrentTime
// Description     : The Default no param constructor.. Will set time to current system time
// Return type  : Time_Clock
//////////////////////////////////////////////////////////
inline Time_Clock Time_Clock::GetCurrentTime()
{
    return Time_Clock();
}
//////////////////////////////////////////////////////////////
// Function name : Time_Clock::Time_Clock
// Description     :
// Return type  : inline
//////////////////////////////////////////////////////////////
inline Time_Clock::Time_Clock()
{
    gettimeofday(&_my_time, NULL);
}
//////////////////////////////////////////////////////////////
// Function name : Time_Clock::ToCurrentTime
// Description     : Load this object with the current OS time
// Return type  : inline void
// Argument         : void
//////////////////////////////////////////////////////////////
inline void Time_Clock::ToCurrentTime()
{
    gettimeofday(&_my_time, NULL);
}
/////////////////////////////////////////////////////////////
// Function name : Time_Clock::GetGmtTm
// Description     :  Access the stored time and convers to a struct tm format
//      If storage location is specified then it will stor information in the
//      provided buffer else it will use the library's internal buffer space
// Return type  : struct tm*
// Argument         : struct tm* ptm
//////////////////////////////////////////////////////////
inline struct tm* Time_Clock::GetGmtTm(struct tm* ptm) const
{
    if (ptm != NULL)
    {
        *ptm = *gmtime((const time_t *)&_my_time.tv_sec);
        return ptm;
    } else
        return gmtime((const time_t *)&_my_time.tv_sec);
}

////////////////////////////////////////////////////////////////////
// Function name : Time_Clock::GetLocalTm
// Description     :  Gets The local time in a tm structre from the internal time value
//
// Return type  : struct tm*
// Argument         : struct tm* ptm
////////////////////////////////////////////////////////////////////
inline struct tm* Time_Clock::GetLocalTm(struct tm* ptm) const
{
    if (ptm != NULL)
    {
        struct tm* ptmTemp = localtime((const time_t *)&_my_time.tv_sec);
        if (ptmTemp == NULL)
            return NULL;    // indicates the _my_time.tv_sec was not initialized!
        *ptm = *ptmTemp;
        return ptm;
    } else
        return localtime((const time_t *)&_my_time.tv_sec);
}
/////////////////////////////////////////////////////////////////////////////
// String formatting
#define maxTimeBufferSize       4096
// Verifies will fail if the needed buffer size is too large
/////////////////////////////////////////////////////////////
// Function name :  Time_Clock::Format
// Description     :  Used to allow access to the "C" library strftime functions..
//
// Return type  : std::string
// Argument         : char * pFormat
//////////////////////////////////////////////////////////
inline std::string Time_Clock::Format(const char * pFormat) const
{
    
    char szBuffer[maxTimeBufferSize];
    char ch, ch1;
    char * pch = szBuffer;
    
    while ((ch = *pFormat++) != '\0') 
    {
        assert(pch < &szBuffer[maxTimeBufferSize]);
        if (ch == '%') 
        {
            switch (ch1 = *pFormat++) 
            {
            default:
                *pch++ = ch;
                *pch++ = ch1;
                break;
            case 'N':
                pch += sprintf(pch, "%03ld", (long)(_my_time.tv_usec / 1000));
                break;
            }
        }
        else 
        {
            *pch++ = ch;
        }
    }
    
    *pch = '\0';
    
    char szBuffer1[maxTimeBufferSize];
    
    struct tm* ptmTemp = localtime((const time_t *)&_my_time.tv_sec);
    if (ptmTemp == NULL ||
        !strftime(szBuffer1, sizeof(szBuffer1), szBuffer, ptmTemp))
        szBuffer1[0] = '\0';
    return std::string(szBuffer1);
}
//////////////////////////////////////////////////////////////
// Function name :   Time_Clock::FormatGmt
// Description     : A Wraper to
//
//   size_t strftime( char *strDest, size_t maxsize, const char *format, const struct tm *timeptr );
//
// Return type  : inline std::string
// Argument         : char * pFormat
//////////////////////////////////////////////////////////////
inline std::string Time_Clock::FormatGmt(const char * pFormat) const
{
    
    char szBuffer[maxTimeBufferSize];
    char ch, ch1;
    char * pch = szBuffer;
    
    while ((ch = *pFormat++) != '\0') 
    {
        assert(pch < &szBuffer[maxTimeBufferSize]);
        if (ch == '%') 
        {
            switch (ch1 = *pFormat++) 
            {
            default:
                *pch++ = ch;
                *pch++ = ch1;
                break;
            case 'N':
                pch += sprintf(pch, "%03ld", (long)(_my_time.tv_usec / 1000));
                break;
            }
        }
        else 
        {
            *pch++ = ch;
        }
    }
    *pch = '\0';
    
    char szBuffer1[maxTimeBufferSize];
    
    struct tm* ptmTemp = gmtime((const time_t *)&_my_time.tv_sec);
    if (ptmTemp == NULL ||
        !strftime(szBuffer1, sizeof(szBuffer1), szBuffer, ptmTemp))
        szBuffer1[0] = '\0';
    return std::string(szBuffer1);
}
//////////////////////////////////////////////////////////////
// Function name : Time_Clock::Time_Clock
// Description     : The Constructor that take a time_t objext
// Return type  : inline
// Argument         : time_t time
//////////////////////////////////////////////////////////////
inline Time_Clock::Time_Clock(time_t time)
{
    _my_time.tv_sec = (long)time;
    _my_time.tv_usec = 0;
};
//////////////////////////////////////////////////////////////
// Function name : Time_Clock::Time_Clock
// Description     : Constructor that takes in sec and usecs..
// Return type  : inline
// Argument         : long secs
// Argument         : long usecs
//////////////////////////////////////////////////////////////
inline Time_Clock::Time_Clock(long secs, long usecs)
{
    _my_time.tv_sec = secs;
    _my_time.tv_usec = usecs;
    NormalizeTime(_my_time);
};
//////////////////////////////////////////////////////////////
// Function name : Time_Clock::Time_Clock
// Description     :  yet another constructor
// Return type  : inline
// Argument         : const Time_Clock& timeSrc
//////////////////////////////////////////////////////////////
inline Time_Clock::Time_Clock(const Time_Clock& timeSrc)
{
    _my_time.tv_sec = timeSrc._my_time.tv_sec;
    _my_time.tv_usec = timeSrc._my_time.tv_usec;
}
//////////////////////////////////////////////////////////////
// Function name : Time_Clock::operator==
// Description     : .. is time  equal
// Return type  : inline bool
// Argument         : const Time_Clock &time
//////////////////////////////////////////////////////////////
inline bool Time_Clock::operator==(const Time_Clock &time) const
{
    return ((_my_time.tv_sec == time._my_time.tv_sec) && (_my_time.tv_usec == time._my_time.tv_usec));
}
//////////////////////////////////////////////////////////////
// Function name : Time_Clock::operator!=
// Description     :  .is time !=
// Return type  : inline bool
// Argument         : const Time_Clock &time
//////////////////////////////////////////////////////////////
inline bool Time_Clock::operator!=(const Time_Clock &time) const
{
    return ((_my_time.tv_sec != time._my_time.tv_sec) || (_my_time.tv_usec != time._my_time.tv_usec));
}

//////////////////////////////////////////////////////////////
// Function name : Time_Clock::operator<
// Description     :
// Return type  : inline bool
// Argument         : const Time_Clock &time
//////////////////////////////////////////////////////////////
inline bool Time_Clock::operator<(const Time_Clock &time) const
{
    return ((_my_time.tv_sec < time._my_time.tv_sec) ||
        ((_my_time.tv_sec == time._my_time.tv_sec) && (_my_time.tv_usec < time._my_time.tv_usec)));
}
//////////////////////////////////////////////////////////////
// Function name : Time_Clock::operator>
// Description     :
// Return type  : inline bool
// Argument         : const Time_Clock &time
//////////////////////////////////////////////////////////////
inline bool Time_Clock::operator>(const Time_Clock &time) const
{
    return ((_my_time.tv_sec > time._my_time.tv_sec) ||
        ((_my_time.tv_sec == time._my_time.tv_sec) && (_my_time.tv_usec > time._my_time.tv_usec)));
}
//////////////////////////////////////////////////////////////
// Function name : Time_Clock::operator<=
// Description     :
// Return type  : inline bool
// Argument         : const Time_Clock &time
//////////////////////////////////////////////////////////////
inline bool Time_Clock::operator<=(const Time_Clock &time) const
{
    return ((_my_time.tv_sec < time._my_time.tv_sec) ||
        ((_my_time.tv_sec == time._my_time.tv_sec) && (_my_time.tv_usec <= time._my_time.tv_usec)));
}
//////////////////////////////////////////////////////////////
// Function name : Time_Clock::operator>=
// Description     :
// Return type  : inline bool
// Argument         : const Time_Clock &time
//////////////////////////////////////////////////////////////
inline bool Time_Clock::operator>=(const Time_Clock &time) const
{
    return ((_my_time.tv_sec > time._my_time.tv_sec) ||
        ((_my_time.tv_sec == time._my_time.tv_sec) && (_my_time.tv_usec >= time._my_time.tv_usec)));
}
//////////////////////////////////////////////////////////////
// Function name : Time_Clock& Time_Clock::operator=
// Description     :
// Return type  : inline const
// Argument         : const Time_Clock& timeSrc
//////////////////////////////////////////////////////////////
inline const Time_Clock& Time_Clock::operator=(const Time_Clock& timeSrc)
{
    if (&timeSrc == this)
        return * this;
    
    _my_time = timeSrc._my_time;
    return *this;
}
//////////////////////////////////////////////////////////////
// Function name : Time_Clock& Time_Clock::operator=
// Description     :
// Return type  : inline const
// Argument         : time_t t
//////////////////////////////////////////////////////////////
inline const Time_Clock& Time_Clock::operator=(time_t t)
{
    _my_time.tv_sec = (long)t;
    _my_time.tv_usec = 0;
    return *this;
}
//////////////////////////////////////////////////////////////
// Function name : Time_Clock::GetTime
// Description     :
// Return type  : inline time_t
//////////////////////////////////////////////////////////////
inline time_t Time_Clock::GetTime() const
{
    return _my_time.tv_sec;
}
//////////////////////////////////////////////////////////////
// Function name : Time_Clock::GetYear
// Description     :
// Return type  : inline int
//////////////////////////////////////////////////////////////
inline int Time_Clock::GetYear() const
{
    return (GetLocalTm(NULL)->tm_year) + 1900;
}
//////////////////////////////////////////////////////////////
// Function name : Time_Clock::GetMonth
// Description     :
// Return type  : inline int
//////////////////////////////////////////////////////////////
inline int Time_Clock::GetMonth() const
{
    return GetLocalTm(NULL)->tm_mon + 1;
}
//////////////////////////////////////////////////////////////
// Function name : Time_Clock::GetDay
// Description     :
// Return type  : inline int
//////////////////////////////////////////////////////////////
inline int Time_Clock::GetDay() const
{
    return GetLocalTm(NULL)->tm_mday;
}
//////////////////////////////////////////////////////////////
// Function name : Time_Clock::GetHour
// Description     :
// Return type  : inline int
//////////////////////////////////////////////////////////////
inline int Time_Clock::GetHour() const
{
    return GetLocalTm(NULL)->tm_hour;
}
//////////////////////////////////////////////////////////////
// Function name : Time_Clock::GetMinute
// Description     :
// Return type  : inline int
//////////////////////////////////////////////////////////////
inline int Time_Clock::GetMinute() const
{
    return GetLocalTm(NULL)->tm_min;
}
//////////////////////////////////////////////////////////////
// Function name : Time_Clock::GetSecond
// Description     :
// Return type  : inline int
//////////////////////////////////////////////////////////////
inline int Time_Clock::GetSecond() const
{
    return GetLocalTm(NULL)->tm_sec;
}
//////////////////////////////////////////////////////////////
// Function name : Time_Clock::GetDayOfWeek
// Description     :
// Return type  : inline int
//////////////////////////////////////////////////////////////
inline int Time_Clock::GetDayOfWeek() const
{
    return GetLocalTm(NULL)->tm_wday + 1;
}

#endif //__Time_H__
