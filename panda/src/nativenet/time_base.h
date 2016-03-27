#ifndef __TIME_BASE_H__
#define __TIME_BASE_H__
/*
 * Functions To support General Time Managment.  And to allow for cross
 * platform use.  Today Really Two Base classes and one convience class
 * Time_Clock = The clock time down to micro seconds.. Time_Span  = Delta Time
 * to the Mico Second.. Time_Out   = Help timer ............count down a
 * duration.  I realize TimeClock is really an implied delta to EPOCH. I have
 * chosen to implement it this way.  it may be apropriate to convert it all to
 * delta times with an EPOCk constant and functions that can handle the EPOCK
 * to current time.  All though this is probably the "right" implementation
 * most coders do not think of clock time in this fashon.  General
 * Observation.. Windows 2k and Linux  are really slow (~250k a sec) at
 * returning the current system time ?? So use time functions that grab the
 * current system time sparingly ??
 */
#ifdef WIN32
#include <winsock2.h>
#include <wtypes.h>
#include <sys/types.h>
#include  <sys/timeb.h>
#else
#include  <sys/time.h>
#endif
#include <time.h>
#include <string>
#include <assert.h>

enum { USEC = 1000000 };
/**
 *
 */
inline void NormalizeTime(timeval &in)
{
    while (in.tv_usec >= USEC)
    {
        in.tv_usec -= USEC;
        in.tv_sec++;
    }

    while (in.tv_usec < 0)
    {
        in.tv_usec += USEC;
        in.tv_sec--;
    }
}

/**
 *
 */
inline void TimeDif(const struct timeval &start, const struct timeval &fin, struct timeval &answer)
{
    answer.tv_usec = fin.tv_usec - start.tv_usec;
    answer.tv_sec = fin.tv_sec - start.tv_sec;
    NormalizeTime(answer);
}
/**
 *
 */
inline void TimeAdd(const struct timeval &start, const struct timeval &delta, struct timeval &answer)
{
    answer.tv_usec = start.tv_usec + delta.tv_usec;
    answer.tv_sec = start.tv_sec + delta.tv_sec;
    NormalizeTime(answer);
}

#ifdef WIN32
// Lets make Windows think it is a unix machine :)

/**
 *
 */
inline int gettimeofday(struct timeval *tv, void * trash)
{
    struct timeb timeb;
    ftime( &timeb);
    tv->tv_sec = (long)timeb.time;
    tv->tv_usec = (unsigned int)timeb.millitm * 1000;
    return 0;
}
#endif

#include "time_clock.h"
#include "time_span.h"
#include "time_general.h"
#include "time_out.h"

#endif //__TIME_BASE_H__
