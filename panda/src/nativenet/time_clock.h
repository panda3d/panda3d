#ifndef __Time_H__
#define __Time_H__

#include <stdio.h>

class Time_Span;

/**
 * This class is to provide a consistant interface and storage to clock time
 * .. Epoch based time to the second
 *
 * jan-2000 .. rhh changing all time to use sub second timing...
 */
class Time_Clock {
  friend class Time_Span;

public:
  // Constructors
  static Time_Clock GetCurrentTime();
  void ToCurrentTime();
  inline Time_Clock(const timeval &in_mytime) {
    _my_time = in_mytime;
  }
  Time_Clock();
  Time_Clock(time_t time);
  Time_Clock(long secs, long usecs);
  Time_Clock(int nYear, int nMonth, int nDay, int nHour, int nMin, int nSec, long microseconds = 0, int nDST = -1);
  Time_Clock(const Time_Clock& timeSrc);

  inline const Time_Clock &operator=(const Time_Clock& timeSrc);
  inline const Time_Clock &operator=(time_t t);

  // Attributes
  struct tm *GetGmtTm(struct tm *ptm) const;
  struct tm *GetLocalTm(struct tm *ptm) const;

  time_t GetTime() const;
  int GetYear() const;
  int GetMonth() const;       // month of year (1 = Jan)
  int GetDay() const;         // day of month
  int GetHour() const;
  int GetMinute() const;
  int GetSecond() const;
  int GetDayOfWeek() const;   // 1=Sun, 2=Mon, ..., 7=Sat

  void Set(int nYear, int nMonth, int nDay, int nHour, int nMin, int nSec, long microseconds = 0, int nDST = -1);

  // Operations time math
  const Time_Clock& operator+=(const Time_Span &Time_Span);
  const Time_Clock& operator-=(const Time_Span &Time_Span);
  bool operator==(const Time_Clock &time) const;
  bool operator!=(const Time_Clock &time) const;
  bool operator<(const Time_Clock &time) const;
  bool operator>(const Time_Clock &time) const;
  bool operator<=(const Time_Clock &time) const;
  bool operator>=(const Time_Clock &time) const;

  inline time_t GetTime_t() {
    return _my_time.tv_sec;
  }
  inline long GetUsecPart() {
    return _my_time.tv_usec;
  }

  // formatting using "C" strftime
  std::string Format(const char *pFormat) const;
  std::string FormatGmt(const char *pFormat) const;

  const timeval &GetTval() {
    return _my_time;
  }
  const timeval &GetTval() const {
    return _my_time;
  }

private:
  struct timeval _my_time;
};

/**
 * Construction from parts
 */
inline Time_Clock::
Time_Clock(int nYear, int nMonth, int nDay, int nHour, int nMin, int nSec, long microseconds, int nDST) {
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

/**
 *
 */
inline void Time_Clock::
Set(int nYear, int nMonth, int nDay, int nHour, int nMin, int nSec, long microseconds , int nDST) {
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

/**
 * The Default no param constructor.. Will set time to current system time
 */
inline Time_Clock Time_Clock::
GetCurrentTime() {
  return Time_Clock();
}

/**
 *
 */
inline Time_Clock::
Time_Clock() {
  gettimeofday(&_my_time, nullptr);
}

/**
 * Load this object with the current OS time
 */
inline void Time_Clock::
ToCurrentTime() {
  gettimeofday(&_my_time, nullptr);
}

/**
 * Access the stored time and converts to a struct tm format If storage
 * location is specified then it will stor information in the provided buffer
 * else it will use the library's internal buffer space
 */
inline struct tm *Time_Clock::
GetGmtTm(struct tm *ptm) const {
  nassertr(ptm != nullptr, nullptr);
#ifdef _WIN32
  return (gmtime_s(ptm, (const time_t *)&_my_time.tv_sec) == 0) ? ptm : nullptr;
#else
  return gmtime_r((const time_t *)&_my_time.tv_sec, ptm);
#endif
}

/**
 * Gets The local time in a tm structre from the internal time value
 */
inline struct tm *Time_Clock::
GetLocalTm(struct tm *ptm) const {
  nassertr(ptm != nullptr, nullptr);
#ifdef _WIN32
  return (localtime_s(ptm, (const time_t *)&_my_time.tv_sec) == 0) ? ptm : nullptr;
#else
  return localtime_r((const time_t *)&_my_time.tv_sec, ptm);
#endif
}
// String formatting Verifies will fail if the needed buffer size is too large
#define maxTimeBufferSize       4096

/**
 * Used to allow access to the "C" library strftime functions..
 */
inline std::string Time_Clock::
Format(const char *pFormat) const {
  char szBuffer[maxTimeBufferSize];
  char ch, ch1;
  char *pch = szBuffer;

  while ((ch = *pFormat++) != '\0') {
    assert(pch < &szBuffer[maxTimeBufferSize]);
    if (ch == '%') {
      switch (ch1 = *pFormat++) {
      default:
        *pch++ = ch;
        *pch++ = ch1;
        break;
      case 'N':
#ifdef _WIN32
        pch += sprintf_s(pch, maxTimeBufferSize, "%03ld", (long)(_my_time.tv_usec / 1000));
#else
        pch += snprintf(pch, maxTimeBufferSize, "%03ld", (long)(_my_time.tv_usec / 1000));
#endif
        break;
      }
    } else {
      *pch++ = ch;
    }
  }

  *pch = '\0';

  char szBuffer1[maxTimeBufferSize];

  struct tm tmTemp;
  if (GetLocalTm(&tmTemp) == nullptr ||
      !strftime(szBuffer1, sizeof(szBuffer1), szBuffer, &tmTemp)) {
    szBuffer1[0] = '\0';
  }
  return std::string(szBuffer1);
}

/**
 * A Wraper to size_t strftime( char *strDest, size_t maxsize, const char
 * *format, const struct tm *timeptr );
 *
 */
inline std::string Time_Clock::
FormatGmt(const char *pFormat) const {
  char szBuffer[maxTimeBufferSize];
  char ch, ch1;
  char *pch = szBuffer;

  while ((ch = *pFormat++) != '\0') {
    assert(pch < &szBuffer[maxTimeBufferSize]);
    if (ch == '%') {
      switch (ch1 = *pFormat++) {
      default:
        *pch++ = ch;
        *pch++ = ch1;
        break;
      case 'N':
#ifdef _WIN32
        pch += sprintf_s(pch, maxTimeBufferSize, "%03ld", (long)(_my_time.tv_usec / 1000));
#else
        pch += snprintf(pch, maxTimeBufferSize, "%03ld", (long)(_my_time.tv_usec / 1000));
#endif
        break;
      }
    } else {
      *pch++ = ch;
    }
  }
  *pch = '\0';

  char szBuffer1[maxTimeBufferSize];

  struct tm tmTemp;
  if (GetGmtTm(&tmTemp) == nullptr ||
      !strftime(szBuffer1, sizeof(szBuffer1), szBuffer, &tmTemp)) {
    szBuffer1[0] = '\0';
  }
  return std::string(szBuffer1);
}

/**
 * The Constructor that take a time_t objext
 */
inline Time_Clock::
Time_Clock(time_t time) {
  _my_time.tv_sec = (long)time;
  _my_time.tv_usec = 0;
}

/**
 * Constructor that takes in sec and usecs..
 */
inline Time_Clock::
Time_Clock(long secs, long usecs) {
  _my_time.tv_sec = secs;
  _my_time.tv_usec = usecs;
  NormalizeTime(_my_time);
}

/**
 * yet another constructor
 */
inline Time_Clock::
Time_Clock(const Time_Clock& timeSrc) {
  _my_time.tv_sec = timeSrc._my_time.tv_sec;
  _my_time.tv_usec = timeSrc._my_time.tv_usec;
}

/**
 * .. is time  equal
 */
inline bool Time_Clock::
operator==(const Time_Clock &time) const {
  return ((_my_time.tv_sec == time._my_time.tv_sec) && (_my_time.tv_usec == time._my_time.tv_usec));
}

/**
 * .is time !=
 */
inline bool Time_Clock::
operator!=(const Time_Clock &time) const {
  return ((_my_time.tv_sec != time._my_time.tv_sec) || (_my_time.tv_usec != time._my_time.tv_usec));
}

/**
 *
 */
inline bool Time_Clock::
operator<(const Time_Clock &time) const {
  return ((_my_time.tv_sec < time._my_time.tv_sec) ||
    ((_my_time.tv_sec == time._my_time.tv_sec) && (_my_time.tv_usec < time._my_time.tv_usec)));
}

/**
 *
 */
inline bool Time_Clock::
operator>(const Time_Clock &time) const {
  return ((_my_time.tv_sec > time._my_time.tv_sec) ||
    ((_my_time.tv_sec == time._my_time.tv_sec) && (_my_time.tv_usec > time._my_time.tv_usec)));
}

/**
 *
 */
inline bool Time_Clock::
operator<=(const Time_Clock &time) const {
  return ((_my_time.tv_sec < time._my_time.tv_sec) ||
    ((_my_time.tv_sec == time._my_time.tv_sec) && (_my_time.tv_usec <= time._my_time.tv_usec)));
}

/**
 *
 */
inline bool Time_Clock::
operator>=(const Time_Clock &time) const {
  return ((_my_time.tv_sec > time._my_time.tv_sec) ||
    ((_my_time.tv_sec == time._my_time.tv_sec) && (_my_time.tv_usec >= time._my_time.tv_usec)));
}

/**
 *
 */
inline const Time_Clock& Time_Clock::
operator=(const Time_Clock& timeSrc) {
  if (&timeSrc == this) {
    return *this;
  }

  _my_time = timeSrc._my_time;
  return *this;
}

/**
 *
 */
inline const Time_Clock& Time_Clock::
operator=(time_t t) {
  _my_time.tv_sec = (long)t;
  _my_time.tv_usec = 0;
  return *this;
}

/**
 *
 */
inline time_t Time_Clock::
GetTime() const {
  return _my_time.tv_sec;
}

/**
 *
 */
inline int Time_Clock::
GetYear() const {
  struct tm atm;
  return (GetLocalTm(&atm)->tm_year) + 1900;
}

/**
 *
 */
inline int Time_Clock::
GetMonth() const {
  struct tm atm;
  return GetLocalTm(&atm)->tm_mon + 1;
}

/**
 *
 */
inline int Time_Clock::
GetDay() const {
  struct tm atm;
  return GetLocalTm(&atm)->tm_mday;
}

/**
 *
 */
inline int Time_Clock::
GetHour() const {
  struct tm atm;
  return GetLocalTm(&atm)->tm_hour;
}

/**
 *
 */
inline int Time_Clock::
GetMinute() const {
  struct tm atm;
  return GetLocalTm(&atm)->tm_min;
}

/**
 *
 */
inline int Time_Clock::
GetSecond() const {
  struct tm atm;
  return GetLocalTm(&atm)->tm_sec;
}

/**
 *
 */
inline int Time_Clock::
GetDayOfWeek() const {
  struct tm atm;
  return GetLocalTm(&atm)->tm_wday + 1;
}

#endif //__Time_H__
