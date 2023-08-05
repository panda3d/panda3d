#ifndef TIME_SPAN_H
#define TIME_SPAN_H

/**
 *
 */
class Time_Span {
public:
  // Constructors
  Time_Span() {}

  Time_Span(struct timeval time) {
    _my_time = time;
    NormalizeTime(_my_time);
  }

  Time_Span(time_t time);
  Time_Span(long lDays, int nHours, int nMins, int nSecs, int usecs);
  Time_Span(long seconds, int usecs);
  Time_Span(const Time_Span &Time_SpanSrc);
  Time_Span(const Time_Clock &Time_SpanSrc);
  Time_Span(PN_stdfloat Seconds);

  const Time_Span &operator=(const Time_Span &Time_SpanSrc);

  // Attributes extract parts
  long GetDays() const;   // total # of days
  long GetTotalHours() const;
  int GetHours() const;
  long GetTotalMinutes() const;
  int GetMinutes() const;
  long GetTotalSeconds() const;
  int GetSeconds() const;
  long GetTotalMSeconds() const;
  long GetTotal100Seconds() const;
  long GetMSeconds() const;

  // Operations time math
  const Time_Span &operator+=(Time_Span &Time_Span);
  const Time_Span &operator-=(Time_Span &Time_Span);
  bool operator==(Time_Span &Time_Span) const;
  bool operator!=(Time_Span &Time_Span) const;
  bool operator<(Time_Span &Time_Span) const;
  bool operator>(Time_Span &Time_Span) const;
  bool operator<=(Time_Span &Time_Span) const;
  bool operator>=(Time_Span &Time_Span) const;
  const timeval &GetTval() const {
    return _my_time;
  }

  void Set(long lDays, int nHours, int nMins, int nSecs, int usecs);

  std::string Format(char *pFormat) const;

private:
  struct timeval _my_time;
  friend class Time_Clock;
};

/**
 *
 */
inline Time_Span::
Time_Span(long seconds, int usecs) {
  _my_time.tv_sec = seconds;
  _my_time.tv_usec = usecs;
  NormalizeTime(_my_time);
}

/**
 *
 */
inline Time_Span::
Time_Span(time_t time) {
  _my_time.tv_usec = 0;
  _my_time.tv_sec = (long)time;
}

/**
 *
 */
inline Time_Span::
Time_Span(PN_stdfloat Seconds) {
  _my_time.tv_sec = (long)Seconds; // this truncats .. desired result..
  _my_time.tv_usec = (long)((Seconds - (double)_my_time.tv_sec) * (double)USEC);
}

/**
 *
 */
inline Time_Span::
Time_Span(long lDays, int nHours, int nMins, int nSecs, int usecs) {
  _my_time.tv_sec = nSecs + 60 * (nMins + 60 * (nHours + 24 * lDays));
  _my_time.tv_usec = usecs;

}

/**
 *
 */
inline void Time_Span::
Set(long lDays, int nHours, int nMins, int nSecs, int usecs) {
  _my_time.tv_sec = nSecs + 60 * (nMins + 60 * (nHours + 24 * lDays));
  _my_time.tv_usec = usecs;

}

/**
 *
 */
inline Time_Span::
Time_Span(const Time_Span &Time_SpanSrc) {
  _my_time = Time_SpanSrc._my_time;
}

/**
 *
 */
inline Time_Span::
Time_Span(const Time_Clock &Time_SpanSrc) {
  _my_time = Time_SpanSrc._my_time;
}

/**
 *
 */
inline const Time_Span &Time_Span::
operator=(const Time_Span &Time_SpanSrc) {
  if (&Time_SpanSrc == this) {
    return *this;
  }
  _my_time = Time_SpanSrc._my_time;
  return *this;
}

/**
 *
 */
inline long Time_Span::
GetDays() const {
  return _my_time.tv_sec / (24*3600L);
}

/**
 *
 */
inline long Time_Span::
GetTotalHours() const {
  return _my_time.tv_sec / 3600;
}

/**
 *
 */
inline int Time_Span::
GetHours() const {
  return (int)(GetTotalHours() - GetDays()*24);
}

/**
 *
 */
inline long Time_Span::
GetTotalMinutes() const {
  return _my_time.tv_sec / 60;
}

/**
 *
 */
inline int Time_Span::
GetMinutes() const {
  return (int)(GetTotalMinutes() - GetTotalHours()*60);
}

/**
 *
 */
inline long Time_Span::
GetTotalSeconds() const {
  return _my_time.tv_sec;
}

/**
 *
 */
inline long Time_Span::
GetTotalMSeconds() const {
  return (_my_time.tv_sec * 1000) + (_my_time.tv_usec / 1000);
}

inline long Time_Span::
GetTotal100Seconds() const {
  return (_my_time.tv_sec * 100) + (_my_time.tv_usec / 10000);
}

/**
 *
 */
inline long Time_Span::
GetMSeconds() const {
  return (_my_time.tv_usec / 1000);
}

/**
 *
 */
inline int Time_Span::
GetSeconds() const {
  return (int)(GetTotalSeconds() - GetTotalMinutes()*60);
}

/**
 *
 */
inline Time_Span TimeDifference(const Time_Span &Time_Span1, const Time_Span &Time_Span2) {
  timeval ans;
  TimeDif(Time_Span2.GetTval(), Time_Span1.GetTval(), ans);
  return Time_Span(ans);
}

/**
 *
 */
inline Time_Span TimeAddition(const Time_Span &Time_Span1, const Time_Span &Time_Span2) {
  timeval ans;
  TimeAdd(Time_Span2.GetTval(), Time_Span1.GetTval(), ans);
  return Time_Span(ans);
}

/**
 *
 */
inline const Time_Span &Time_Span::
operator+=(Time_Span &Time_Span) {
  _my_time.tv_usec += Time_Span._my_time.tv_usec;
  _my_time.tv_sec += Time_Span._my_time.tv_sec;
  NormalizeTime(_my_time);
  return *this;
}

/**
 *
 */
inline const Time_Span &Time_Span::
operator-=(Time_Span &Time_Span) {
  _my_time.tv_usec -= Time_Span._my_time.tv_usec;
  _my_time.tv_sec -= Time_Span._my_time.tv_sec;
  NormalizeTime(_my_time);
  return *this;
}

/**
 *
 */
inline bool Time_Span::
operator==(Time_Span &Time_Span) const {
  return ((_my_time.tv_sec == Time_Span._my_time.tv_sec) && (_my_time.tv_usec == Time_Span._my_time.tv_usec));
}

/**
 *
 */
inline bool Time_Span::
operator!=(Time_Span &Time_Span) const {
  return ((_my_time.tv_sec != Time_Span._my_time.tv_sec) || (_my_time.tv_usec != Time_Span._my_time.tv_usec));
}

/**
 *
 */
inline bool Time_Span::
operator<(Time_Span &Time_Span) const {
  return ((_my_time.tv_sec < Time_Span._my_time.tv_sec) ||
    ((_my_time.tv_sec == Time_Span._my_time.tv_sec) && (_my_time.tv_usec < Time_Span._my_time.tv_usec)));
}

/**
 *
 */
inline bool Time_Span::
operator>(Time_Span &Time_Span) const {
  return ((_my_time.tv_sec > Time_Span._my_time.tv_sec) ||
    ((_my_time.tv_sec == Time_Span._my_time.tv_sec) && (_my_time.tv_usec > Time_Span._my_time.tv_usec)));
}

/**
 *
 */
inline bool Time_Span::
operator<=(Time_Span &Time_Span) const {
  return ((_my_time.tv_sec < Time_Span._my_time.tv_sec) ||
    ((_my_time.tv_sec == Time_Span._my_time.tv_sec) && (_my_time.tv_usec <= Time_Span._my_time.tv_usec)));
}

/**
 *
 */
inline bool Time_Span::
operator>=(Time_Span &Time_Span) const {
  return ((_my_time.tv_sec > Time_Span._my_time.tv_sec) ||
    ((_my_time.tv_sec == Time_Span._my_time.tv_sec) && (_my_time.tv_usec >= Time_Span._my_time.tv_usec)));
}

/**
 *
 */
inline std::string Time_Span::
Format(char *pFormat) const {
/*
* formatting Time_Spans is a little trickier than formatting * we are only
* interested in relative time formats, ie.  it is illegal to format anything
* dealing with absolute time (i.e.  years, months, day of week, day of year,
* timezones, ...) * the only valid formats: %D - # of days -- NEW !!! %H -
* hour in 24 hour format %M - minute (0-59) %S - seconds (0-59) %% - percent
* sign %N - nanosecs
*/
  char szBuffer[maxTimeBufferSize];
  char ch;
  char *pch = szBuffer;

  while ((ch = *pFormat++) != '\0') {
    assert(pch < &szBuffer[maxTimeBufferSize]);
    if (ch == '%') {
      switch (ch = *pFormat++) {
      default:
        assert(false);      // probably a bad format character
      case '%':
        *pch++ = ch;
        break;
      case 'D':
#ifdef _WIN32
        pch += sprintf_s(pch, maxTimeBufferSize, "%ld", GetDays());
#else
        pch += snprintf(pch, maxTimeBufferSize, "%ld", GetDays());
#endif
        break;
      case 'H':
#ifdef _WIN32
        pch += sprintf_s(pch, maxTimeBufferSize, "%02d", GetHours());
#else
        pch += snprintf(pch, maxTimeBufferSize, "%02d", GetHours());
#endif
        break;
      case 'M':
#ifdef _WIN32
        pch += sprintf_s(pch, maxTimeBufferSize, "%02d", GetMinutes());
#else
        pch += snprintf(pch, maxTimeBufferSize, "%02d", GetMinutes());
#endif
        break;
      case 'S':
#ifdef _WIN32
        pch += sprintf_s(pch, maxTimeBufferSize, "%02d", GetSeconds());
#else
        pch += snprintf(pch, maxTimeBufferSize, "%02d", GetSeconds());
#endif
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
  return std::string(szBuffer);
}

#endif //TIME_SPAN_H
