// Filename: clockObject.h
// Created by:  drose (19Feb99)
//
////////////////////////////////////////////////////////////////////

#ifndef CLOCKOBJECT_H
#define CLOCKOBJECT_H

#include <pandabase.h>

#include "trueClock.h"
#include "config_express.h"

////////////////////////////////////////////////////////////////////
//       Class : ClockObject
// Description : A ClockObject keeps track of elapsed real time and
//               discrete time.  It can run in two modes: In normal
//               mode, get_time() returns the time as of the last time
//               tick() was called.  This is the "discrete" time, and
//               is usually used to get the time as of, for instance,
//               the beginning of the current frame.  In non-real-time
//               mode, get_time() returns a constant increment since
//               the last time tick() was called; this is useful when
//               it is desirable to fake the clock out, for instance
//               for non-real-time animation rendering.
//
//               In both modes, get_real_time() always returns the
//               elapsed real time in seconds since the ClockObject
//               was constructed, or since it was last reset.
//
//               You can create your own ClockObject whenever you want
//               to have your own local timer.  There is also a
//               default, global ClockObject intended to represent
//               global time for the application; this is normally set
//               up to tick every frame so that its get_time() will
//               return the time for the current frame.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAEXPRESS ClockObject {
public:
  enum Mode {
    M_normal,
    M_non_real_time,
  };

  ClockObject();
  INLINE ~ClockObject();

  INLINE void set_mode(Mode mode);
  INLINE Mode get_mode() const;

  INLINE double get_time() const;
  INLINE double get_intra_frame_time() const;
  INLINE double get_real_time() const;

  INLINE void reset();

  void set_time(double time);
  INLINE void set_frame_count(int frame_count);

  INLINE int get_frame_count() const;
  INLINE double get_frame_rate() const;

  INLINE double get_dt() const;
  INLINE void set_dt(double dt);

  void tick();

  INLINE static ClockObject *get_global_clock();

private:
  TrueClock *_true_clock;
  Mode _mode;
  double _start_time;
  int _frame_count;
  double _frame_time;
  double _reported_time;
  double _dt;

  static ClockObject *_global_clock;
};

#include "clockObject.I"

#endif

