/************************************************************************************

Filename    :   OVR_Timer.cpp
Content     :   Provides static functions for precise timing
Created     :   September 19, 2012
Notes       : 

Copyright   :   Copyright 2012 Oculus VR, Inc. All Rights reserved.

Use of this software is subject to the terms of the Oculus license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

************************************************************************************/

#include "OVR_Timer.h"

#if defined (OVR_OS_WIN32)
#include <windows.h>

#else
#include <sys/time.h>
#endif

namespace OVR {

//-----------------------------------------------------------------------------------
// ***** Timer Class

UInt64 Timer::GetProfileTicks()
{
    return (GetRawTicks() * MksPerSecond) / GetRawFrequency();
}
double Timer::GetProfileSeconds()
{
    static UInt64 StartTime = GetProfileTicks();
    return TicksToSeconds(GetProfileTicks()-StartTime);
}


//------------------------------------------------------------------------
// *** Win32 Specific Timer

#if (defined (OVR_OS_WIN32))

CRITICAL_SECTION WinAPI_GetTimeCS;
volatile UInt32  WinAPI_OldTime = 0;
volatile UInt32  WinAPI_WrapCounter = 0;


UInt32 Timer::GetTicksMs()
{
    return timeGetTime();
}

UInt64 Timer::GetTicks()
{
    DWORD  ticks = timeGetTime();
    UInt64 result;

    // On Win32 QueryPerformanceFrequency is unreliable due to SMP and
    // performance levels, so use this logic to detect wrapping and track
    // high bits.
    ::EnterCriticalSection(&WinAPI_GetTimeCS);

    if (WinAPI_OldTime > ticks)
        WinAPI_WrapCounter++;
    WinAPI_OldTime = ticks;

    result = (UInt64(WinAPI_WrapCounter) << 32) | ticks;
    ::LeaveCriticalSection(&WinAPI_GetTimeCS);

    return result * MksPerMs;
}

UInt64 Timer::GetRawTicks()
{
    LARGE_INTEGER li;
    QueryPerformanceCounter(&li);
    return li.QuadPart;
}

UInt64 Timer::GetRawFrequency()
{
    static UInt64 perfFreq = 0;
    if (perfFreq == 0)
    {
        LARGE_INTEGER freq;
        QueryPerformanceFrequency(&freq);
        perfFreq = freq.QuadPart;
    }
    return perfFreq;
}

void Timer::initializeTimerSystem()
{
    timeBeginPeriod(1);
    InitializeCriticalSection(&WinAPI_GetTimeCS);

}
void Timer::shutdownTimerSystem()
{
    DeleteCriticalSection(&WinAPI_GetTimeCS);
    timeEndPeriod(1);
}

#else   // !OVR_OS_WIN32


//------------------------------------------------------------------------
// *** Standard OS Timer     

UInt32 Timer::GetTicksMs()
{
    return (UInt32)(GetProfileTicks() / 1000);
}
// The profile ticks implementation is just fine for a normal timer.
UInt64 Timer::GetTicks()
{
    return GetProfileTicks();
}

void Timer::initializeTimerSystem()
{
}
void Timer::shutdownTimerSystem()
{
}

UInt64  Timer::GetRawTicks()
{
    // TODO: prefer rdtsc when available?

    // Return microseconds.
    struct timeval tv;
    UInt64 result;

    gettimeofday(&tv, 0);

    result = (UInt64)tv.tv_sec * 1000000;
    result += tv.tv_usec;

    return result;
}

UInt64 Timer::GetRawFrequency()
{
    return MksPerSecond;
}

#endif  // !OVR_OS_WIN32



} // OVR

