// Filename: clockObject.h
// Created by:  drose (19Feb99)
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

#ifndef CLOCKOBJECT_H
#define CLOCKOBJECT_H

#include "pandabase.h"

#include "trueClock.h"
#include "pdeque.h"

class EXPCL_PANDAEXPRESS TimeVal {
PUBLISHED:
  INLINE TimeVal();
  INLINE ulong get_sec() const;
  INLINE ulong get_usec() const;
  ulong tv[2];
};

BEGIN_PUBLISH

EXPCL_PANDAEXPRESS void get_time_of_day(TimeVal &tv);

END_PUBLISH

////////////////////////////////////////////////////////////////////
//       Class : ClockObject
// Description : A ClockObject keeps track of elapsed real time and
//               discrete time.  It can run in two modes: In normal
//               mode, get_frame_time() returns the time as of the
//               last time tick() was called.  This is the "discrete"
//               time, and is usually used to get the time as of, for
//               instance, the beginning of the current frame.  In
//               non-real-time mode, get_frame_time() returns a
//               constant increment since the last time tick() was
//               called; this is useful when it is desirable to fake
//               the clock out, for instance for non-real-time
//               animation rendering.
//
//               In both modes, get_real_time() always returns the
//               elapsed real time in seconds since the ClockObject
//               was constructed, or since it was last reset.
//
//               You can create your own ClockObject whenever you want
//               to have your own local timer.  There is also a
//               default, global ClockObject intended to represent
//               global time for the application; this is normally set
//               up to tick every frame so that its get_frame_time()
//               will return the time for the current frame.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAEXPRESS ClockObject {
PUBLISHED:
  enum Mode {
    M_normal,
    M_non_real_time,
    M_forced,
    M_degrade,
  };

  ClockObject();
  INLINE ~ClockObject();

  INLINE void set_mode(Mode mode);
  INLINE Mode get_mode() const;

  INLINE double get_frame_time() const;
  INLINE double get_real_time() const;
  INLINE double get_long_time() const;

  INLINE void reset();
  void set_real_time(double time);
  void set_frame_time(double time);
  void set_frame_count(int frame_count);

  INLINE int get_frame_count() const;
  INLINE double get_net_frame_rate() const;

  INLINE double get_dt() const;
  INLINE void set_dt(double dt);

  INLINE double get_max_dt() const;
  INLINE void set_max_dt(double max_dt);

  INLINE double get_degrade_factor() const;
  INLINE void set_degrade_factor(double degrade_factor);

  INLINE void set_average_frame_rate_interval(double time);
  INLINE double get_average_frame_rate_interval() const;
  INLINE double get_average_frame_rate() const;

  void tick();
  void sync_frame_time();

  INLINE static ClockObject *get_global_clock();

private:
  void wait_until(double want_time);
  static void make_global_clock();

  TrueClock *_true_clock;
  Mode _mode;
  double _start_short_time;
  double _start_long_time;
  int _frame_count;
  double _actual_frame_time;
  double _reported_frame_time;
  double _dt;
  double _max_dt;
  double _degrade_factor;

  // For tracking the average frame rate over a certain interval of
  // time.
  double _average_frame_rate_interval;
  typedef pdeque<double> Ticks;
  Ticks _ticks;

  static ClockObject *_global_clock;
};

EXPCL_PANDAEXPRESS ostream &
operator << (ostream &out, ClockObject::Mode mode);
EXPCL_PANDAEXPRESS istream &
operator >> (istream &in, ClockObject::Mode &mode);

#include "clockObject.I"

#endif

