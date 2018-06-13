/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file trueClock.cxx
 * @author drose
 * @date 2000-07-04
 */

#include "trueClock.h"
#include "config_express.h"
#include "numeric_types.h"

#include <math.h>  // for fabs()

using std::max;
using std::min;

TrueClock *TrueClock::_global_ptr = nullptr;

#if defined(WIN32_VC) || defined(WIN64_VC)

// The Win32 implementation.

#include <sys/timeb.h>
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN 1
#endif
#include <windows.h>

static const double _0001 = 1.0 / 1000.0;
static const double _00000001 = 1.0 / 10000000.0;

// This is the interval of time, in seconds, over which to measure the high-
// precision clock rate vs.  the time-of-day rate, when paranoid-clock is in
// effect.  Reducing it makes the clock respond more quickly to changes in
// rate, but setting it too small may introduce erratic behavior, especially
// if the user has ntp configured.
static const double paranoid_clock_interval = 3.0;

// It will be considered a clock jump error if either the high-precision clock
// or the time-of-day clock change by this number of seconds without the other
// jumping by a similar amount.
static const double paranoid_clock_jump_error = 2.0;

// If the we detect a clock jump error but the corrected clock skew is
// currently more than this amount, we hack the clock scale to try to
// compensate.
static const double paranoid_clock_jump_error_max_delta = 1.0;

// If the measured time_scale appears to change by more than this factor, it
// will be reported to the log.  Changes to time_scale less than this factor
// are assumed to be within the margin of error.
static const double paranoid_clock_report_scale_factor = 0.1;

// If the high-precision clock, after applying time_scale correction, is still
// more than this number of seconds above or below the time-of-day clock, it
// will be sped up or slowed down slightly until it is back in sync.
static const double paranoid_clock_chase_threshold = 0.5;

// This is the minimum factor by which the high-precision clock will be sped
// up or slowed down when it gets out of sync by paranoid-clock-chase-
// threshold.
static const double paranoid_clock_chase_factor = 0.1;

/**
 *
 */
double TrueClock::
get_long_time() {
  int tc = GetTickCount();
  return (double)(tc - _init_tc) * _0001;
}

/**
 *
 */
double TrueClock::
get_short_raw_time() {
  double time;

  if (_has_high_res) {
/*
 * Use the high-resolution clock.  This is of questionable value, since (a) on
 * some OS's and hardware, the low 24 bits can occasionally roll over without
 * setting the carry bit, causing the time to jump backwards, and (b)
 * reportedly it can set the carry bit incorrectly sometimes, causing the time
 * to jump forwards, and (c) even when it doesn't do that, it's not very
 * accurate and seems to lose seconds of time per hour, and (d) someone could
 * be running a program such as Speed Gear which munges this value anyway.
 */
    int64_t count;
    QueryPerformanceCounter((LARGE_INTEGER *)&count);

    time = (double)(count - _init_count) * _recip_frequency;

  } else {
    // No high-resolution clock; return the best information we have.  This
    // doesn't suffer from the rollover problems that QueryPerformanceCounter
    // does, but it's not very precise--only precise to 50ms on Win98, and
    // 10ms on XP-based systems--and Speed Gear still munges it.
    int tc = GetTickCount();
    time = (double)(tc - _init_tc) * _0001;
  }

  return time;
}

/**
 *
 */
typedef BOOL (WINAPI * PFNSETPROCESSAFFINITYMASK)(HANDLE, DWORD_PTR);
typedef BOOL (WINAPI * PFNGETPROCESSAFFINITYMASK)(HANDLE, DWORD_PTR*, DWORD_PTR*);

bool TrueClock::
set_cpu_affinity(uint32_t mask) const {
  HMODULE hker = GetModuleHandle("kernel32");
  if (hker != 0) {
    PFNGETPROCESSAFFINITYMASK gp = (PFNGETPROCESSAFFINITYMASK)
      GetProcAddress(hker, "GetProcessAffinityMask");
    PFNSETPROCESSAFFINITYMASK sp = (PFNSETPROCESSAFFINITYMASK)
      GetProcAddress(hker, "SetProcessAffinityMask");
    if (gp != 0 && sp != 0) {
      DWORD proc_mask;
      DWORD sys_mask;
      if (gp(GetCurrentProcess(), (PDWORD_PTR)&proc_mask, (PDWORD_PTR)&sys_mask)) {
        // make sure we don't reference CPUs that don't exist
        proc_mask = mask & sys_mask;
        if (proc_mask) {
          return sp(GetCurrentProcess(), proc_mask) != 0;
        }
      }
    }
  }
  return false;
}

/**
 *
 */
TrueClock::
TrueClock() {
  _error_count = 0;
  _has_high_res = false;

  _time_scale = 1.0;
  _time_offset = 0.0;
  _tod_offset = 0.0;
  _time_scale_changed = false;
  _last_reported_time_scale = 1.0;
  _report_time_scale_time = 0.0;

  ConfigVariableBool lock_to_one_cpu
    ("lock-to-one-cpu", false,
     PRC_DESC("Set this to true if you want the entire process to use one "
              "CPU, even on multi-core and multi-CPU workstations. This is "
              "mainly a hack to solve a bug in which QueryPerformanceCounter "
              "returns inconsistent results on multi-core machines. "));

  if (lock_to_one_cpu) {
    set_cpu_affinity(0x01);
  }

  if (get_use_high_res_clock()) {
    int64_t int_frequency;
    _has_high_res =
      (QueryPerformanceFrequency((LARGE_INTEGER *)&int_frequency) != 0);
    if (_has_high_res) {
      if (int_frequency <= 0) {
        clock_cat.error()
          << "TrueClock::get_real_time() - frequency is negative!" << std::endl;
        _has_high_res = false;

      } else {
        _frequency = (double)int_frequency;
        _recip_frequency = 1.0 / _frequency;

        QueryPerformanceCounter((LARGE_INTEGER *)&_init_count);
      }
    }
  }

  // Also store the initial tick count.  We'll need this for get_long_time(),
  // as well as for get_short_time() if we're not using the high resolution
  // clock.
  _init_tc = GetTickCount();

  // And we will need the current time of day to cross-check either of the
  // above clocks if paranoid-clock is enabled.
  GetSystemTimeAsFileTime((FILETIME *)&_init_tod);

  _chase_clock = CC_keep_even;

  // In case we'll be cross-checking the clock, we'd better start out with at
  // least one timestamp, so we'll know if the clock jumps just after startup.
  _timestamps.push_back(Timestamp(0.0, 0.0));

  if (!_has_high_res) {
    clock_cat.warning()
      << "No high resolution clock available." << std::endl;
  }
}

/**
 * Ensures that the reported timestamp from the high-precision (or even the
 * low-precision) clock is valid by verifying against the time-of-day clock.
 *
 * This attempts to detect sudden jumps in time that might be caused by a
 * failure of the high-precision clock to roll over properly.
 *
 * It also corrects for long-term skew of the clock by measuring the timing
 * discrepency against the wall clock and projecting that discrepency into the
 * future.  This also should defeat programs such as Speed Gear that work by
 * munging the value returned by QueryPerformanceCounter() and GetTickCount(),
 * but not the wall clock time.
 *
 * However, relying on wall clock time presents its own set of problems, since
 * the time of day might be adjusted slightly forward or back from time to
 * time in response to ntp messages, or it might even be suddenly reset at any
 * time by the user.  So we do the best we can.
 */
double TrueClock::
correct_time(double time) {
  // First, get the current time of day measurement.
  uint64_t int_tod;
  GetSystemTimeAsFileTime((FILETIME *)&int_tod);
  double tod = (double)(int_tod - _init_tod) * _00000001;

  nassertr(!_timestamps.empty(), time);

  // Make sure we didn't experience a sudden jump from the last measurement.
  double time_delta = (time - _timestamps.back()._time) * _time_scale;
  double tod_delta = (tod - _timestamps.back()._tod);

  if (time_delta < -0.0001 ||
      fabs(time_delta - tod_delta) > paranoid_clock_jump_error) {
    // A step backward in the high-precision clock, or more than a small jump
    // on only one of the clocks, is cause for alarm.  We allow a trivial step
    // backward in the high-precision clock, since this does appear to happen
    // in a threaded environment.

    clock_cat.debug()
      << "Clock error detected; elapsed time " << time_delta
      << "s on high-resolution counter, and " << tod_delta
      << "s on time-of-day clock.\n";
    ++_error_count;

    // If both are negative, we call it 0.  If one is negative, we trust the
    // other one (up to paranoid_clock_jump_error).  If both are nonnegative,
    // we trust the smaller of the two.
    double time_adjust = 0.0;
    double tod_adjust = 0.0;

    if (time_delta < 0.0 && tod < 0.0) {
      // Trust neither.
      time_adjust = -time_delta;
      tod_adjust = -tod_delta;

    } else if (time_delta < 0.0 || (tod_delta >= 0.0 && tod_delta < time_delta)) {
      // Trust tod, up to a point.
      double new_tod_delta = min(tod_delta, paranoid_clock_jump_error);
      time_adjust = new_tod_delta - time_delta;
      tod_adjust = new_tod_delta - tod_delta;

    } else {
      // Trust time, up to a point.
      double new_time_delta = min(time_delta, paranoid_clock_jump_error);
      time_adjust = new_time_delta - time_delta;
      tod_adjust = new_time_delta - tod_delta;
    }

    _time_offset += time_adjust;
    time_delta += time_adjust;
    _tod_offset += tod_adjust;
    tod_delta += tod_adjust;

    // Apply the adjustments to the timestamp queue.  We could just completely
    // empty the timestamp queue, but that makes it hard to catch up if we are
    // getting lots of these "momentary" errors in a row.
    Timestamps::iterator ti;
    for (ti = _timestamps.begin(); ti != _timestamps.end(); ++ti) {
      (*ti)._time -= time_adjust / _time_scale;
      (*ti)._tod -= tod_adjust;
    }

    // And now we can record this timestamp, which is now consistent with the
    // previous timestamps in the queue.
    _timestamps.push_back(Timestamp(time, tod));

/*
 * Detecting and filtering this kind of momentary error can help protect us
 * from legitimate problems cause by OS or BIOS bugs (which might introduce
 * errors into the high precision clock), or from sudden changes to the time-
 * of-day by the user, but we have to be careful because if the user uses a
 * Speed Gear-type program to speed up the clock by an extreme amount, it can
 * look like a lot of such "momentary" errors in a row--and if we throw them
 * all out, we won't compute _time_scale correctly.  To avoid this, we hack
 * _time_scale here if we seem to be getting out of sync.
 */
    double corrected_time = time * _time_scale + _time_offset;
    double corrected_tod = tod + _tod_offset;
    if (corrected_time - corrected_tod > paranoid_clock_jump_error_max_delta &&
        _time_scale > 0.00001) {
      clock_cat.info()
        << "Force-adjusting time_scale to catch up to errors.\n";
      set_time_scale(time, _time_scale * 0.5);
    }

  } else if (tod_delta < 0.0) {
    // A small backwards jump on the time-of-day clock is not a concern, since
    // this is technically allowed with ntp enabled.  We simply ignore the
    // event.

  } else {
    // Ok, we don't think there was a sudden jump, so carry on.

    // The timestamp queue here records the measured timestamps over the past
    // _priority_interval seconds.  Its main purpose is to keep a running
    // observation of _time_scale, so we can detect runtime changes of the
    // clock's scale, for instance if the user is using a program like Speed
    // Gear and pulls the slider during runtime.

    // Consider the oldest timestamp in our queue.
    Timestamp oldest = _timestamps.front();
    double time_age = (time - oldest._time);
    double tod_age = (tod - oldest._tod);

    double keep_interval = paranoid_clock_interval;

    if (tod_age > keep_interval / 2.0 && time_age > 0.0) {
      // Adjust the _time_scale value to match the ratio between the elapsed
      // time on the high-resolution clock, and the time-of-day clock.
      double new_time_scale = tod_age / time_age;

      // When we adjust _time_scale, we have to be careful to adjust
      // _time_offset at the same time, so we don't introduce a sudden jump in
      // time.
      set_time_scale(time, new_time_scale);

      // Check to see if the time scale has changed significantly since we
      // last reported it.
      double ratio = _time_scale / _last_reported_time_scale;
      if (fabs(ratio - 1.0) > paranoid_clock_report_scale_factor) {
        _time_scale_changed = true;
        _last_reported_time_scale = _time_scale;
        // Actually report it a little bit later, to give the time scale a
        // chance to settle down.
        _report_time_scale_time = tod + _tod_offset + keep_interval;
        if (clock_cat.is_debug()) {
          clock_cat.debug()
            << "Will report time scale, now " << 100.0 / _time_scale
            << "%, tod_age = " << tod_age << ", time_age = " << time_age
            << ", ratio = " << ratio << "\n";
        }
      }
    }

    // Clean out old entries in the timestamps queue.
    if (tod_age > keep_interval) {
      while (!_timestamps.empty() &&
             tod - _timestamps.front()._tod > keep_interval) {
        _timestamps.pop_front();
      }
    }

    // Record this timestamp.
    _timestamps.push_back(Timestamp(time, tod));
  }

  double corrected_time = time * _time_scale + _time_offset;
  double corrected_tod = tod + _tod_offset;

  if (_time_scale_changed && corrected_tod >= _report_time_scale_time) {
    double percent = 100.0 / _time_scale;
    // Round percent to the nearest 5% to reduce confusion in the logs.
    percent = floor(percent / 20.0 + 0.5) * 20.0;
    clock_cat.info()
      << "Clock appears to be running at " << percent << "% real time.\n";
    _last_reported_time_scale = _time_scale;
    _time_scale_changed = false;
  }

  // By the time we get here, we have a corrected_time and a corrected_tod
  // value, both of which should be advancing at about the same rate.
  // However, there might be accumulated skew between them, since there is
  // some lag in the above algorithm that corrects the _time_scale, and clock
  // skew can accumulate while the algorithm is catching up.

  // Therefore, we have one more line of defense: we check at this point for
  // skew, and correct for it by slowing the clock down or speeding it up a
  // bit as needed, until we even out the clocks again.  Rather than adjusting
  // the clock speed with _time_scale here, we simply slide _time_offset
  // forward and back as needed--that way we don't interfere with the above
  // algorithm, which is trying to compute _time_scale accurately.

  switch (_chase_clock) {
  case CC_slow_down:
    if (corrected_time < corrected_tod) {
      // We caught up.
      _chase_clock = CC_keep_even;
      if (clock_cat.is_debug()) {
        clock_cat.debug()
          << "Clock back down to real time.\n";
        // Let's report the clock error now, so an app can resync now that
        // we're at a good time.
        ++_error_count;
      }

    } else {
      // Slow down the clock by sliding the offset a bit backward.
      double fixup = 1.0 - (1.0 / (corrected_time - corrected_tod));
      double correction = time_delta * max(fixup, paranoid_clock_chase_factor);
      _time_offset -= correction;
      corrected_time -= correction;
    }
    break;

  case CC_keep_even:
    if ((corrected_tod - corrected_time) > paranoid_clock_chase_threshold) {
      // Oops, we're dropping behind; need to speed up.
      _chase_clock = CC_speed_up;

      if (clock_cat.is_debug()) {
        clock_cat.debug()
          << "Clock is behind by " << (corrected_tod - corrected_time)
          << "s; speeding up to correct.\n";
      }
    } else if ((corrected_time - corrected_tod) > paranoid_clock_chase_threshold) {
      // Oops, we're going too fast; need to slow down.
      _chase_clock = CC_slow_down;

      if (clock_cat.is_debug()) {
        clock_cat.debug()
          << "Clock is ahead by " << (corrected_time - corrected_tod)
          << "s; slowing down to correct.\n";
      }
    }
    break;

  case CC_speed_up:
    if (corrected_time > corrected_tod) {
      // We caught up.
      _chase_clock = CC_keep_even;
      if (clock_cat.is_debug()) {
        clock_cat.debug()
          << "Clock back up to real time.\n";
        // Let's report the clock error now, so an app can resync now that
        // we're at a good time.
        ++_error_count;
      }

    } else {
      // Speed up the clock by sliding the offset a bit forward.
      double fixup = 1.0 - (1.0 / (corrected_tod - corrected_time));
      double correction = time_delta * max(fixup, paranoid_clock_chase_factor);
      _time_offset += correction;
      corrected_time += correction;
    }
    break;
  }

  if (clock_cat.is_spam()) {
    clock_cat.spam()
      << "time " << time << " tod " << corrected_tod
      << " corrected time " << corrected_time << "\n";
  }

  return corrected_time;
}

/**
 * Changes the _time_scale value, recomputing _time_offset at the same time so
 * we don't introduce a sudden jump in time.
 */
void TrueClock::
set_time_scale(double time, double new_time_scale) {
  nassertv(new_time_scale > 0.0);
  _time_offset = time * _time_scale + _time_offset - (time * new_time_scale);
  _time_scale = new_time_scale;
}

#else  // !WIN32_VC

// The Posix implementation.

#include <sys/time.h>
#include <stdio.h>  // for perror

static long _init_sec;

/**
 *
 */
double TrueClock::
get_long_time() {
  struct timeval tv;

  int result;

#ifdef GETTIMEOFDAY_ONE_PARAM
  result = gettimeofday(&tv);
#else
  result = gettimeofday(&tv, nullptr);
#endif

  if (result < 0) {
    // Error in gettimeofday().
    return 0.0;
  }

  // We subtract out the time at which the clock was initialized, because we
  // don't care about the number of seconds all the way back to 1970, and we
  // want to leave the double with as much precision as it can get.
  return (double)(tv.tv_sec - _init_sec) + (double)tv.tv_usec / 1000000.0;
}

/**
 *
 */
double TrueClock::
get_short_raw_time() {
  struct timeval tv;

  int result;

#ifdef GETTIMEOFDAY_ONE_PARAM
  result = gettimeofday(&tv);
#else
  result = gettimeofday(&tv, nullptr);
#endif

  if (result < 0) {
    // Error in gettimeofday().
    return 0.0;
  }

  // We subtract out the time at which the clock was initialized, because we
  // don't care about the number of seconds all the way back to 1970, and we
  // want to leave the double with as much precision as it can get.
  return (double)(tv.tv_sec - _init_sec) + (double)tv.tv_usec / 1000000.0;
}

/**
 *
 */
bool TrueClock::
set_cpu_affinity(uint32_t mask) const {
  return false;
}

/**
 *
 */
TrueClock::
TrueClock() {
  _error_count = 0;
  struct timeval tv;

  int result;
#ifdef GETTIMEOFDAY_ONE_PARAM
  result = gettimeofday(&tv);
#else
  result = gettimeofday(&tv, nullptr);
#endif

  if (result < 0) {
    perror("gettimeofday");
    _init_sec = 0;
  } else {
    _init_sec = tv.tv_sec;
  }
}

#endif
