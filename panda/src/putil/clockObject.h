/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file clockObject.h
 * @author drose
 * @date 1999-02-19
 */

#ifndef CLOCKOBJECT_H
#define CLOCKOBJECT_H

#include "pandabase.h"

#include "trueClock.h"
#include "pdeque.h"
#include "cycleData.h"
#include "cycleDataReader.h"
#include "cycleDataWriter.h"
#include "cycleDataStageReader.h"
#include "pipelineCycler.h"
#include "thread.h"
#include "referenceCount.h"
#include "pointerTo.h"
#include "vector_double.h"  // needed to see exported allocators for pdeque

class EXPCL_PANDA_PUTIL TimeVal {
PUBLISHED:
  INLINE TimeVal();
  INLINE ulong get_sec() const;
  INLINE ulong get_usec() const;
  ulong tv[2];
};

/**
 * A ClockObject keeps track of elapsed real time and discrete time.  In
 * normal mode, get_frame_time() returns the time as of the last time tick()
 * was called.  This is the "discrete" time, and is usually used to get the
 * time as of, for instance, the beginning of the current frame.
 *
 * In other modes, as set by set_mode() or the clock-mode config variable,
 * get_frame_time() may return other values to simulate different timing
 * effects, for instance to perform non-real-time animation.  See set_mode().
 *
 * In all modes, get_real_time() always returns the elapsed real time in
 * seconds since the ClockObject was constructed, or since it was last reset.
 *
 * You can create your own ClockObject whenever you want to have your own
 * local timer.  There is also a default, global ClockObject intended to
 * represent global time for the application; this is normally set up to tick
 * every frame so that its get_frame_time() will return the time for the
 * current frame.
 */
class EXPCL_PANDA_PUTIL ClockObject : public ReferenceCount {
PUBLISHED:
  enum Mode {
    M_normal,
    M_non_real_time,
    M_forced,
    M_degrade,
    M_slave,
    M_limited,
    M_integer,
    M_integer_limited,
  };

  ClockObject(Mode mode = M_normal);
  ClockObject(const ClockObject &copy);
  INLINE ~ClockObject();

  void set_mode(Mode mode);
  INLINE Mode get_mode() const;
  MAKE_PROPERTY(mode, get_mode, set_mode);

  INLINE double get_frame_time(Thread *current_thread = Thread::get_current_thread()) const;
  INLINE double get_real_time() const;
  INLINE double get_long_time() const;

  INLINE void reset();
  void set_real_time(double time);
  void set_frame_time(double time, Thread *current_thread = Thread::get_current_thread());
  void set_frame_count(int frame_count, Thread *current_thread = Thread::get_current_thread());

  INLINE int get_frame_count(Thread *current_thread = Thread::get_current_thread()) const;
  INLINE double get_net_frame_rate(Thread *current_thread = Thread::get_current_thread()) const;

  MAKE_PROPERTY(frame_time, get_frame_time, set_frame_time);
  MAKE_PROPERTY(real_time, get_real_time, set_real_time);
  MAKE_PROPERTY(long_time, get_long_time);
  MAKE_PROPERTY(frame_count, get_frame_count, set_frame_count);

  INLINE double get_dt(Thread *current_thread = Thread::get_current_thread()) const;
  void set_dt(double dt);
  void set_frame_rate(double frame_rate);
  MAKE_PROPERTY(dt, get_dt, set_dt);

  INLINE double get_max_dt() const;
  INLINE void set_max_dt(double max_dt);
  MAKE_PROPERTY(max_dt, get_max_dt, set_max_dt);

  INLINE double get_degrade_factor() const;
  INLINE void set_degrade_factor(double degrade_factor);
  MAKE_PROPERTY(degrade_factor, get_degrade_factor, set_degrade_factor);

  INLINE void set_average_frame_rate_interval(double time);
  INLINE double get_average_frame_rate_interval() const;
  MAKE_PROPERTY(average_frame_rate_interval,
            get_average_frame_rate_interval,
            set_average_frame_rate_interval);

  double get_average_frame_rate(Thread *current_thread = Thread::get_current_thread()) const;
  double get_max_frame_duration(Thread *current_thread = Thread::get_current_thread()) const;
  double calc_frame_rate_deviation(Thread *current_thread = Thread::get_current_thread()) const;
  MAKE_PROPERTY(average_frame_rate, get_average_frame_rate);
  MAKE_PROPERTY(max_frame_duration, get_max_frame_duration);

  void tick(Thread *current_thread = Thread::get_current_thread());
  void sync_frame_time(Thread *current_thread = Thread::get_current_thread());

  INLINE bool check_errors(Thread *current_thread);

  INLINE static ClockObject *get_global_clock();

public:
  static void (*_start_clock_wait)();
  static void (*_start_clock_busy_wait)();
  static void (*_stop_clock_wait)();

private:
  void wait_until(double want_time);
  static void make_global_clock();
  static void dummy_clock_wait();

  TrueClock *_true_clock;
  Mode _mode;
  double _start_short_time;
  double _start_long_time;
  double _actual_frame_time;
  double _max_dt;
  double _user_frame_rate;
  double _degrade_factor;
  int _error_count;

  // For tracking the average frame rate over a certain interval of time.
  double _average_frame_rate_interval;
  typedef pdeque<double> Ticks;
  Ticks _ticks;

  // This is the data that needs to be cycled each frame.
  class EXPCL_PANDA_PUTIL CData : public CycleData {
  public:
    CData();
    INLINE CData(const CData &copy);

    virtual CycleData *make_copy() const;
    virtual TypeHandle get_parent_type() const {
      return ClockObject::get_class_type();
    }

    int _frame_count;
    double _reported_frame_time;
    double _reported_frame_time_epoch;
    double _dt;
  };

  PipelineCycler<CData> _cycler;
  typedef CycleDataReader<CData> CDReader;
  typedef CycleDataWriter<CData> CDWriter;
  typedef CycleDataStageReader<CData> CDStageReader;

  static AtomicAdjust::Pointer _global_clock;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    ReferenceCount::init_type();
    register_type(_type_handle, "ClockObject",
                  ReferenceCount::get_class_type());
  }

private:
  static TypeHandle _type_handle;
};

EXPCL_PANDA_PUTIL std::ostream &
operator << (std::ostream &out, ClockObject::Mode mode);
EXPCL_PANDA_PUTIL std::istream &
operator >> (std::istream &in, ClockObject::Mode &mode);

#include "clockObject.I"

#endif
