#ifndef __TIME_GENERAL_H__
#define __TIME_GENERAL_H__

Time_Span TimeDifference(const Time_Clock &time1, const Time_Clock &time2);
Time_Clock TimeDifference(const Time_Clock &time1, const Time_Span &Time_Span);
Time_Clock TimeAddition(const Time_Clock &time1, Time_Span &Time_Span);

Time_Clock operator+(const Time_Clock &tm, const Time_Span &ts);
Time_Clock operator-(const Time_Clock &tm, const Time_Span &ts);

/**
 *
 */
inline Time_Span TimeDifference(const Time_Clock &time1, const Time_Clock &time2) {
  timeval ans;
  TimeDif(time2.GetTval(), time1.GetTval(), ans);
  return Time_Span(ans);
}

/**
 *
 */
inline Time_Clock TimeDifference(const Time_Clock &time1, const Time_Span &Time_Span) {
  timeval ans;
  TimeDif(Time_Span.GetTval(), time1.GetTval(), ans);
  return Time_Clock(ans);
}

/**
 *
 */
inline Time_Clock TimeAddition(const Time_Clock &time1, Time_Span &Time_Span)
{
  timeval ans;
  TimeAdd(time1.GetTval(), Time_Span.GetTval(), ans);
  return Time_Clock(ans);
}

/**
 *
 */
inline const Time_Clock &Time_Clock::operator+=(const Time_Span &Time_Span) {
  _my_time.tv_usec += Time_Span._my_time.tv_usec;
  _my_time.tv_sec += Time_Span._my_time.tv_sec;
  NormalizeTime(_my_time);
  return *this;
}

/**
 *
 */
inline Time_Clock operator+(const Time_Clock &tm, const Time_Span &ts) {
  Time_Clock work(tm);
  work += ts;
  return work;
}

/**
 *
 */
inline Time_Clock operator-(const Time_Clock &tm, const Time_Span &ts) {
  return TimeDifference(tm, ts);
}

/**
 *
 */
inline const Time_Clock& Time_Clock::operator-=(const Time_Span &Time_Span) {
  _my_time.tv_usec -= Time_Span._my_time.tv_usec;
  _my_time.tv_sec -= Time_Span._my_time.tv_sec;
  NormalizeTime(_my_time);
  return *this;
}

/**
 *
 */
inline Time_Span operator-(const Time_Clock &tm1, const Time_Clock &tm2) {
  return TimeDifference(tm1, tm2);
}

#endif //__TIME_GENERAL_H__
