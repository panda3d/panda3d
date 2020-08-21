/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file clockObject.I
 * @author drose
 * @date 2000-02-17
 */

/**
 *
 */
INLINE ClockObject::
~ClockObject() {
}

/**
 * Returns the current mode of the clock.  See set_mode().
 */
INLINE ClockObject::Mode ClockObject::
get_mode() const {
  return _mode;
}

/**
 * Returns the time in seconds as of the last time tick() was called
 * (typically, this will be as of the start of the current frame).
 *
 * This is generally the kind of time you want to ask for in most rendering
 * and animation contexts, since it's important that all of the animation for
 * a given frame remains in sync with each other.
 */
INLINE double ClockObject::
get_frame_time(Thread *current_frame) const {
  CDReader cdata(_cycler, current_frame);
  return cdata->_reported_frame_time;
}

/**
 * Returns the actual number of seconds elapsed since the ClockObject was
 * created, or since it was last reset.  This is useful for doing real timing
 * measurements, e.g.  for performance statistics.
 *
 * This returns the most precise timer we have for short time intervals, but
 * it may tend to drift over the long haul.  If more accurate timekeeping is
 * needed over a long period of time, use get_long_time() instead.
 */
INLINE double ClockObject::
get_real_time() const {
  return (_true_clock->get_short_time() - _start_short_time);
}

/**
 * Returns the actual number of seconds elapsed since the ClockObject was
 * created, or since it was last reset.
 *
 * This is similar to get_real_time(), except that it uses the most accurate
 * counter we have over a long period of time, and so it is less likely to
 * drift.  However, it may not be very precise for measuring short intervals.
 * On Windows, for instace, this is only accurate to within about 55
 * milliseconds.
 */
INLINE double ClockObject::
get_long_time() const {
  return (_true_clock->get_long_time() - _start_long_time);
}

/**
 * Simultaneously resets both the time and the frame count to zero.
 */
INLINE void ClockObject::
reset() {
  set_real_time(0.0);
  set_frame_time(0.0);
  set_frame_count(0);
}

/**
 * Returns the number of times tick() has been called since the ClockObject
 * was created, or since it was last reset.  This is generally the number of
 * frames that have been rendered.
 */
INLINE int ClockObject::
get_frame_count(Thread *current_thread) const {
  CDReader cdata(_cycler, current_thread);
  return cdata->_frame_count;
}

/**
 * Returns the average frame rate since the last reset.  This is simply the
 * total number of frames divided by the total elapsed time.  This reports the
 * virtual frame rate if the clock is in (or has been in) M_non_real_time
 * mode.
 */
INLINE double ClockObject::
get_net_frame_rate(Thread *current_thread) const {
  CDReader cdata(_cycler, current_thread);
  return (double)cdata->_frame_count / cdata->_reported_frame_time;
}

/**
 * Returns the elapsed time for the previous frame: the number of seconds
 * elapsed between the last two calls to tick().
 */
INLINE double ClockObject::
get_dt(Thread *current_thread) const {
  CDReader cdata(_cycler, current_thread);
  if (_max_dt > 0.0) {
    return std::min(_max_dt, cdata->_dt);
  }
  return cdata->_dt;
}

/**
 * Returns the current maximum allowable time elapsed between any two frames.
 * See set_max_dt().
 */
INLINE double ClockObject::
get_max_dt() const {
  return _max_dt;
}

/**
 * Sets a limit on the value returned by get_dt().  If this value is less than
 * zero, no limit is imposed; otherwise, this is the maximum value that will
 * ever be returned by get_dt(), regardless of how much time has actually
 * elapsed between frames.
 *
 * This limit is only imposed in real-time mode; in non-real-time mode, the dt
 * is fixed anyway and max_dt is ignored.
 *
 * This is generally used to guarantee reasonable behavior even in the
 * presence of a very slow or chuggy frame rame.
 */
INLINE void ClockObject::
set_max_dt(double max_dt) {
  _max_dt = max_dt;
}

/**
 * In degrade mode, returns the ratio by which the performance is degraded.  A
 * value of 2.0 causes the clock to be slowed down by a factor of two
 * (reducing performance to 1/2 what would be otherwise).
 *
 * This has no effect if mode is not M_degrade.
 */
INLINE double ClockObject::
get_degrade_factor() const {
  return _degrade_factor;
}

/**
 * In degrade mode, sets the ratio by which the performance is degraded.  A
 * value of 2.0 causes the clock to be slowed down by a factor of two
 * (reducing performance to 1/2 what would be otherwise).
 *
 * This has no effect if mode is not M_degrade.
 */
INLINE void ClockObject::
set_degrade_factor(double degrade_factor) {
  _degrade_factor = degrade_factor;
}

/**
 * Specifies the interval of time (in seconds) over which
 * get_average_frame_rate() averages the number of frames per second to
 * compute the frame rate.  Changing this does not necessarily immediately
 * change the result of get_average_frame_rate(), until this interval of time
 * has elapsed again.
 *
 * Setting this to zero disables the computation of get_average_frame_rate().
 */
INLINE void ClockObject::
set_average_frame_rate_interval(double time) {
  _average_frame_rate_interval = time;
  if (_average_frame_rate_interval == 0.0) {
    _ticks.clear();
  }
}

/**
 * Returns the interval of time (in seconds) over which
 * get_average_frame_rate() averages the number of frames per second to
 * compute the frame rate.
 */
INLINE double ClockObject::
get_average_frame_rate_interval() const {
  return _average_frame_rate_interval;
}

/**
 * Returns true if a clock error was detected since the last time
 * check_errors() was called.  A clock error means that something happened, an
 * OS or BIOS bug, for instance, that makes the current value of the clock
 * somewhat suspect, and an application may wish to resynchronize with any
 * external clocks.
 */
INLINE bool ClockObject::
check_errors(Thread *current_thread) {
  CDReader cdata(_cycler, current_thread);  // Just to hold a mutex.
  int orig_error_count = _error_count;
  _error_count = _true_clock->get_error_count();
  return (_error_count != orig_error_count);
}

/**
 * Returns a pointer to the global ClockObject.  This is the ClockObject that
 * most code should use for handling scene graph rendering and animation.
 */
INLINE ClockObject *ClockObject::
get_global_clock() {
  ClockObject *clock = (ClockObject *)AtomicAdjust::get_ptr(_global_clock);
  if (UNLIKELY(clock == nullptr)) {
    make_global_clock();
    clock = (ClockObject *)_global_clock;
  }
  return clock;
}

/**
 *
 */
INLINE ClockObject::CData::
CData(const ClockObject::CData &copy) :
  _frame_count(copy._frame_count),
  _reported_frame_time(copy._reported_frame_time),
  _dt(copy._dt)
{
}

/**
 *
 */
INLINE TimeVal::
TimeVal() {
}

/**
 *
 */
INLINE ulong TimeVal::
get_sec() const {
  return tv[0];
}

/**
 *
 */
INLINE ulong TimeVal::
get_usec() const {
  return tv[1];
}
