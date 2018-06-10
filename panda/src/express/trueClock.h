/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file trueClock.h
 * @author drose
 * @date 2000-07-04
 */

#ifndef TRUECLOCK_H
#define TRUECLOCK_H

#include "pandabase.h"
#include "typedef.h"
#include "pdeque.h"
#include "mutexImpl.h"
#include "config_express.h"

/**
 * An interface to whatever real-time clock we might have available in the
 * current environment.  There is only one TrueClock in existence, and it
 * constructs itself.
 *
 * The TrueClock returns elapsed real time in seconds since some undefined
 * epoch.  Since it is not defined at what time precisely the clock indicates
 * zero, this value can only be meaningfully used to measure elapsed time, by
 * sampling it at two different times and subtracting.
 */
class EXPCL_PANDA_EXPRESS TrueClock {
PUBLISHED:
  // get_long_time() returns the most accurate timer we have over a long
  // interval.  It may not be very precise for measuring short intervals, but
  // it should not drift substantially over the long haul.
  double get_long_time();
  MAKE_PROPERTY(long_time, get_long_time);

  // get_short_time() returns the most precise timer we have over a short
  // interval.  It may tend to drift over the long haul, but it should have
  // lots of digits to measure short intervals very precisely.
  INLINE double get_short_time();
  MAKE_PROPERTY(short_time, get_short_time);

  // get_short_raw_time() is like get_short_time(), but does not apply any
  // corrections (e.g.  paranoid-clock) to the result returned by the OS.
  double get_short_raw_time();
  MAKE_PROPERTY(short_raw_time, get_short_raw_time);

  INLINE int get_error_count() const;
  MAKE_PROPERTY(error_count, get_error_count);

  INLINE static TrueClock *get_global_ptr();

  bool set_cpu_affinity(uint32_t mask) const;

protected:
  TrueClock();
  INLINE ~TrueClock();

  int _error_count;

  static TrueClock *_global_ptr;

#ifdef WIN32
  double correct_time(double time);
  void set_time_scale(double time, double new_time_scale);

  bool _has_high_res;
  int64_t _init_count;
  double _frequency, _recip_frequency;
  int _init_tc;
  uint64_t _init_tod;

  // The rest of the data structures in this block are strictly for
  // implementing paranoid_clock: they are designed to allow us to cross-check
  // the high-resolution clock against the time-of-day clock, and smoothly
  // correct for deviations.
  class Timestamp {
  public:
    Timestamp(double time, double tod) : _time(time), _tod(tod) { }
    double _time;
    double _tod;
  };
  typedef pdeque<Timestamp> Timestamps;
  Timestamps _timestamps;
  double _time_scale;
  double _time_offset;
  double _tod_offset;
  int _num_jump_errors;
  bool _time_scale_changed;
  double _last_reported_time_scale;
  double _report_time_scale_time;
  enum ChaseClock {
    CC_slow_down,
    CC_keep_even,
    CC_speed_up,
  };
  ChaseClock _chase_clock;
  MutexImpl _lock;
#endif  // WIN32
};

#include "trueClock.I"

#endif
