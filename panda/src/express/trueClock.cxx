// Filename: trueClock.cxx
// Created by:  drose (04Jul00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////


#include "trueClock.h"
#include "config_express.h"
#include "numeric_types.h"

#include <math.h>  // for fabs()

TrueClock *TrueClock::_global_ptr = NULL;

#if defined(WIN32_VC)

////////////////////////////////////////////////////////////////////
//
// The Win32 implementation.
//
////////////////////////////////////////////////////////////////////

#include <sys/timeb.h>

#define WINDOWS_LEAN_AND_MEAN
#include <windows.h>
#undef WINDOWS_LEAN_AND_MEAN

static BOOL _has_high_res;
static PN_int64 _frequency;
static PN_int64 _init_count;
static double _fFrequency,_recip_fFrequency;
static DWORD _init_tc;
static bool _paranoid_clock;
static const double _0001 = 1.0/1000.0;

void get_true_time_of_day(ulong &sec, ulong &usec) {
  struct timeb tb;
  ftime(&tb);
  sec = tb.time;
  usec = (ulong)(tb.millitm * 1000.0);
}

double TrueClock::
get_long_time() const {
  DWORD tc = GetTickCount();
  return (double)(tc - _init_tc) * _0001;
}

double TrueClock::
get_short_time() const {
  if (_has_high_res) {
    LARGE_INTEGER count;
    QueryPerformanceCounter(&count);

    double time = (double)(count.QuadPart - _init_count) * _recip_fFrequency;

    if (_paranoid_clock) {
      // Now double-check the high-resolution clock against the system
      // clock.
      DWORD tc = GetTickCount();
      double sys_time = (double)(tc - _init_tc) * _0001;
      if (fabs(time - sys_time) > 0.5) {
        // Too much variance!
        express_cat.info()
          << "Clock error!  High resolution clock reads " << time 
          << " while system clock reads " << sys_time << ".\n";
        _init_count = 
          (PN_int64)(count.QuadPart - sys_time * _fFrequency);
        time = (double)(count.QuadPart - _init_count) * _recip_fFrequency;
        express_cat.info()
          << "High resolution clock reset to " << time << ".\n";
      }
    }

    return time;

  } else {
    // No high-resolution clock; return the best information we have.
    DWORD tc = GetTickCount();
    return (double)(tc - _init_tc) * _0001;
  }
}

TrueClock::
TrueClock() {
  _has_high_res = false;
  if (get_use_high_res_clock()) {
    _has_high_res = QueryPerformanceFrequency((LARGE_INTEGER *)&_frequency);
    _fFrequency = (double) _frequency;
    _recip_fFrequency = 1.0/_fFrequency;
  }

  _paranoid_clock = get_paranoid_clock();

  if (_has_high_res) {
    LARGE_INTEGER count;
    QueryPerformanceCounter(&count);
    _init_count = count.QuadPart;

    if (_frequency <= 0) {
      express_cat.error()
        << "TrueClock::get_real_time() - frequency is negative!" << endl;
      _has_high_res = false;
    }
  }

  // Also store the initial tick count.  We'll need this for
  // get_long_time(), as well as for get_short_time() if we're not
  // using the high resolution clock, or to cross-check the high
  // resolution clock if we are using it.
  _init_tc = GetTickCount();

  if (!_has_high_res) {
    express_cat.warning()
      << "No high resolution clock available." << endl;

  } else if (_paranoid_clock) {
    express_cat.info()
      << "Not trusting the high resolution clock." << endl;
  }
}


#elif defined(PENV_PS2)

////////////////////////////////////////////////////////////////////
//
// The PS2 implementation.
//
////////////////////////////////////////////////////////////////////


#include <eeregs.h>
#include <eekernel.h>

static unsigned int _msec;
static unsigned int _sec;

// PS2 timer interrupt, as the RTC routines don't exist unless you're
// using the .irx iop compiler, which scares us.  A lot.
static int
timer_handler(int) {
  _msec++;

  if (_msec >= 1000) {
    _msec = 0;
    _sec++;
  }

  return -1;
}

void get_true_time_of_day(ulong &sec, ulong &msec) {
  cerr << "get_true_time_of_day() not implemented!" << endl;
}

double TrueClock::
get_long_time() const {
  return (double) _sec + ((double) _msec / 1000.0);
}

double TrueClock::
get_short_time() const {
  return (double) _sec + ((double) _msec / 1000.0);
}

TrueClock::
TrueClock() {
  _init_sec = 0;
  _msec = 0;
  _sec = 0;

  tT_MODE timer_mode;
  *(unsigned int *) &timer_mode = 0;

  timer_mode.cxxLKS = 1;
  timer_mode.ZRET = 1;
  timer_mode.cxxUE = 1;
  timer_mode.cxxMPE = 1;
  timer_mode.EQUF = 1;

  *T0_COMP = 9375;
  *T0_MODE = *(unsigned int *) &timer_mode;

  EnableIntc(INTC_TIM0);
  AddIntcHandler(INTC_TIM0, timer_handler, -1);
}


#elif !defined(WIN32_VC)

////////////////////////////////////////////////////////////////////
//
// The Posix implementation.
//
////////////////////////////////////////////////////////////////////

#include <sys/time.h>
#include <stdio.h>  // for perror

static long _init_sec;

void get_true_time_of_day(ulong &sec, ulong &msec) {
  struct timeval tv;
  int result;

#ifdef GETTIMEOFDAY_ONE_PARAM
  result = gettimeofday(&tv);
#else
  result = gettimeofday(&tv, (struct timezone *)NULL);
#endif

  if (result < 0) {
    sec = 0;
    msec = 0;
    // Error in gettimeofday().
    return;
  }
  sec = tv.tv_sec;
  msec = tv.tv_usec;
}

double TrueClock::
get_long_time() const {
  struct timeval tv;

  int result;

#ifdef GETTIMEOFDAY_ONE_PARAM
  result = gettimeofday(&tv);
#else
  result = gettimeofday(&tv, (struct timezone *)NULL);
#endif

  if (result < 0) {
    // Error in gettimeofday().
    return 0.0;
  }

  // We subtract out the time at which the clock was initialized,
  // because we don't care about the number of seconds all the way
  // back to 1970, and we want to leave the double with as much
  // precision as it can get.
  return (double)(tv.tv_sec - _init_sec) + (double)tv.tv_usec / 1000000.0;
}

double TrueClock::
get_short_time() const {
  struct timeval tv;

  int result;

#ifdef GETTIMEOFDAY_ONE_PARAM
  result = gettimeofday(&tv);
#else
  result = gettimeofday(&tv, (struct timezone *)NULL);
#endif

  if (result < 0) {
    // Error in gettimeofday().
    return 0.0;
  }

  // We subtract out the time at which the clock was initialized,
  // because we don't care about the number of seconds all the way
  // back to 1970, and we want to leave the double with as much
  // precision as it can get.
  return (double)(tv.tv_sec - _init_sec) + (double)tv.tv_usec / 1000000.0;
}

TrueClock::
TrueClock() {
  struct timeval tv;

  int result;
#ifdef GETTIMEOFDAY_ONE_PARAM
  result = gettimeofday(&tv);
#else
  result = gettimeofday(&tv, (struct timezone *)NULL);
#endif

  if (result < 0) {
    perror("gettimeofday");
    _init_sec = 0;
  } else {
    _init_sec = tv.tv_sec;
  }
}

#endif
