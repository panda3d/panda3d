// Filename: trueClock.h
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

#ifndef TRUECLOCK_H
#define TRUECLOCK_H

#include "pandabase.h"
#include "typedef.h"
#include "pdeque.h"

////////////////////////////////////////////////////////////////////
//       Class : TrueClock
// Description : An interface to whatever real-time clock we might
//               have available in the current environment.  There is
//               only one TrueClock in existence, and it constructs
//               itself.
//
//               The TrueClock returns elapsed real time in seconds
//               since some undefined epoch.  Since it is not defined
//               at what time precisely the clock indicates zero, this
//               value can only be meaningfully used to measure
//               elapsed time, by sampling it at two different times
//               and subtracting.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAEXPRESS TrueClock {
public:
  INLINE static TrueClock *get_ptr();

  // get_long_time() returns the most accurate timer we have over a
  // long interval.  It may not be very precise for measuring short
  // intervals, but it should not drift substantially over the long
  // haul.
  double get_long_time();

  // get_short_time() returns the most precise timer we have over a
  // short interval.  It may tend to drift over the long haul, but it
  // should have lots of digits to measure short intervals very
  // precisely.
  double get_short_time();

  INLINE int get_error_count() const;

protected:
  TrueClock();
  INLINE ~TrueClock();

  int _error_count;

  static TrueClock *_global_ptr;

#ifdef WIN32
  double correct_time(double time);
  void set_time_scale(double time, double new_time_scale);

  bool _has_high_res;
  PN_int64 _init_count;
  double _frequency, _recip_frequency;
  int _init_tc;
  PN_uint64 _init_tod;

  bool _paranoid_clock;

  // The rest of the data structures in this block are strictly for
  // implementing paranoid_clock: they are designed to allow us to
  // cross-check the high-resolution clock against the time-of-day
  // clock, and smoothly correct for deviations.
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
#endif  // WIN32
};

#include "trueClock.I"

#endif
