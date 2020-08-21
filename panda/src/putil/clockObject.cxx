/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file clockObject.cxx
 * @author drose
 * @date 2000-02-17
 */

#include "clockObject.h"
#include "config_putil.h"
#include "configVariableEnum.h"
#include "string_utils.h"
#include "thread.h"

using std::istream;
using std::ostream;
using std::string;

void (*ClockObject::_start_clock_wait)() = ClockObject::dummy_clock_wait;
void (*ClockObject::_start_clock_busy_wait)() = ClockObject::dummy_clock_wait;
void (*ClockObject::_stop_clock_wait)() = ClockObject::dummy_clock_wait;

AtomicAdjust::Pointer ClockObject::_global_clock = nullptr;
TypeHandle ClockObject::_type_handle;

/**
 *
 */
ClockObject::
ClockObject(Mode mode) : _ticks(get_class_type()), _mode(mode) {
  _true_clock = TrueClock::get_global_ptr();

  _start_short_time = _true_clock->get_short_time();
  _start_long_time = _true_clock->get_long_time();
  _actual_frame_time = 0.0;

  ConfigVariableDouble max_dt
    ("max-dt", -1.0,
     PRC_DESC("Sets a limit on the value returned by ClockObject::get_dt().  If "
              "this value is less than zero, no limit is imposed; "
              "otherwise, this is the maximum value that will ever "
              "be returned by get_dt(), regardless of how much time "
              "has actually elapsed between frames.  See ClockObject::set_dt()."));
  ConfigVariableDouble clock_frame_rate
    ("clock-frame-rate", 1.0,
     PRC_DESC("In non-real-time clock mode, sets the number of frames per "
              "second that we should appear to be running.  In forced "
              "mode or limited mode, sets our target frame rate.  In "
              "normal mode, this has no effect.  See ClockObject::set_frame_rate()."));
  ConfigVariableDouble clock_degrade_factor
    ("clock-degrade-factor", 1.0,
     PRC_DESC("In degrade clock mode, returns the ratio by which the "
              "performance is degraded.  A value of 2.0 causes the "
              "clock to be slowed down by a factor of two (reducing "
              "performance to 1/2 what would be otherwise).  See ClockObject::set_degrade_factor()."));
  ConfigVariableDouble average_frame_rate_interval
    ("average-frame-rate-interval", 1.0,
     PRC_DESC("See ClockObject::set_average_frame_rate_interval()."));

  _max_dt = max_dt;
  _user_frame_rate = clock_frame_rate;
  _degrade_factor = clock_degrade_factor;
  _average_frame_rate_interval = average_frame_rate_interval;

  _error_count = _true_clock->get_error_count();
}

/**
 *
 */
ClockObject::
ClockObject(const ClockObject &copy) :
  _true_clock(copy._true_clock),
  _mode(copy._mode),
  _start_short_time(copy._start_short_time),
  _start_long_time(copy._start_long_time),
  _actual_frame_time(copy._actual_frame_time),
  _max_dt(copy._max_dt),
  _user_frame_rate(copy._user_frame_rate),
  _degrade_factor(copy._degrade_factor),
  _error_count(copy._error_count),
  _average_frame_rate_interval(copy._average_frame_rate_interval),
  _ticks(copy._ticks),
  _cycler(copy._cycler)
{
}

/**
 * Changes the mode of the clock.  Normally, the clock is in mode M_normal.
 * In this mode, each call to tick() will set the value returned by
 * get_frame_time() to the current real time; thus, the clock simply reports
 * time advancing.
 *
 * Other possible modes:
 *
 * M_non_real_time - the clock ignores real time completely; at each call to
 * tick(), it pretends that exactly dt seconds have elapsed since the last
 * call to tick().  You may set the value of dt with set_dt() or
 * set_frame_rate().
 *
 * M_limited - the clock will run as fast as it can, as in M_normal, but will
 * not run faster than the rate specified by set_frame_rate().  If the
 * application would run faster than this rate, the clock will slow down the
 * application.
 *
 * M_integer - the clock will run as fast as it can, but the rate will be
 * constrained to be an integer multiple or divisor of the rate specified by
 * set_frame_rate().  The clock will slow down the application a bit to
 * guarantee this.
 *
 * M_integer_limited - a combination of M_limited and M_integer; the clock
 * will not run faster than set_frame_rate(), and if it runs slower, it will
 * run at a integer divisor of that rate.
 *
 * M_forced - the clock forces the application to run at the rate specified by
 * set_frame_rate().  If the application would run faster than this rate, the
 * clock will slow down the application; if the application would run slower
 * than this rate, the clock slows down time so that the application believes
 * it is running at the given rate.
 *
 * M_degrade - the clock runs at real time, but the application is slowed down
 * by a set factor of its frame rate, specified by set_degrade_factor().
 *
 * M_slave - the clock does not advance, but relies on the user to call
 * set_frame_time() and/or set_frame_count() each frame.
 */
void ClockObject::
set_mode(ClockObject::Mode mode) {
  Thread *current_thread = Thread::get_current_thread();
  nassertv(current_thread->get_pipeline_stage() == 0);
  CDWriter cdata(_cycler, current_thread);

  _mode = mode;

  // In case we have set the clock to one of the modes that uses
  // _reported_frame_time_epoch, recompute the epoch.
  switch (_mode) {
  case M_non_real_time:
  case M_forced:
    cdata->_reported_frame_time_epoch = cdata->_reported_frame_time -
      cdata->_frame_count / _user_frame_rate;
    cdata->_dt = 1.0 / _user_frame_rate;

  default:
    break;
  }
}

/**
 * Resets the clock to the indicated time.  This changes only the real time of
 * the clock as reported by get_real_time(), but does not immediately change
 * the time reported by get_frame_time()--that will change after the next call
 * to tick().  Also see reset(), set_frame_time(), and set_frame_count().
 */
void ClockObject::
set_real_time(double time) {
#ifdef NOTIFY_DEBUG
  // This is only a debug message, since it happens during normal development,
  // particularly at startup, or whenever you break into the task loop.
  if (util_cat.is_debug() && this == _global_clock) {
    util_cat.debug()
      << "Adjusting global clock's real time by " << time - get_real_time()
      << " seconds.\n";
  }
#endif  // NOTIFY_DEBUG
  _start_short_time = _true_clock->get_short_time() - time;
  _start_long_time = _true_clock->get_long_time() - time;
}

/**
 * Changes the time as reported for the current frame to the indicated time.
 * Normally, the way to adjust the frame time is via tick(); this function is
 * provided only for occasional special adjustments.
 */
void ClockObject::
set_frame_time(double time, Thread *current_thread) {
  nassertv(current_thread->get_pipeline_stage() == 0);
#ifdef NOTIFY_DEBUG
  if (this == _global_clock && _mode != M_slave) {
    util_cat.warning()
      << "Adjusting global clock's frame time by " << time - get_frame_time()
      << " seconds.\n";
  }
#endif  // NOTIFY_DEBUG
  CDWriter cdata(_cycler, current_thread);
  _actual_frame_time = time;
  cdata->_reported_frame_time = time;

  // Recompute the epoch in case we are in a mode that relies on this.
  cdata->_reported_frame_time_epoch = cdata->_reported_frame_time -
    cdata->_frame_count / _user_frame_rate;
}

/**
 * Resets the number of frames counted to the indicated number.  Also see
 * reset(), set_real_time(), and set_frame_time().
 */
void ClockObject::
set_frame_count(int frame_count, Thread *current_thread) {
  nassertv(current_thread->get_pipeline_stage() == 0);
#ifdef NOTIFY_DEBUG
  if (this == _global_clock && _mode != M_slave) {
    util_cat.warning()
      << "Adjusting global clock's frame count by "
      << frame_count - get_frame_count() << " frames.\n";
  }
#endif  // NOTIFY_DEBUG
  CDWriter cdata(_cycler, current_thread);
  cdata->_frame_count = frame_count;

  // Recompute the epoch in case we are in a mode that relies on this.
  cdata->_reported_frame_time_epoch = cdata->_reported_frame_time -
    cdata->_frame_count / _user_frame_rate;
}

/**
 * In non-real-time mode, sets the number of seconds that should appear to
 * elapse between frames.  In forced mode or limited mode, sets our target dt.
 * In normal mode, this has no effect.
 *
 * Also see set_frame_rate(), which is a different way to specify the same
 * quantity.
 */
void ClockObject::
set_dt(double dt) {
  if (_mode == M_slave) {
    // In M_slave mode, we can set any dt we like.
    CDWriter cdata(_cycler, Thread::get_current_thread());
    cdata->_dt = dt;
    if (dt != 0.0) {
      set_frame_rate(1.0 / dt);
    }

  } else {
    // In any other mode, we can only set non-zero dt.
    nassertv(dt != 0.0);
    set_frame_rate(1.0 / dt);
  }
}

/**
 * In non-real-time mode, sets the number of frames per second that we should
 * appear to be running.  In forced mode or limited mode, sets our target
 * frame rate.  In normal mode, this has no effect.
 *
 * Also see set_dt(), which is a different way to specify the same quantity.
 */
void ClockObject::
set_frame_rate(double frame_rate) {
  nassertv(frame_rate != 0.0);

  Thread *current_thread = Thread::get_current_thread();
  nassertv(current_thread->get_pipeline_stage() == 0);

  CDWriter cdata(_cycler, current_thread);
  _user_frame_rate = frame_rate;

  switch (_mode) {
  case M_non_real_time:
  case M_forced:
    cdata->_reported_frame_time_epoch = cdata->_reported_frame_time -
      cdata->_frame_count / _user_frame_rate;
    cdata->_dt = 1.0 / _user_frame_rate;

  default:
    break;
  }
}

/**
 * Returns the average frame rate in number of frames per second over the last
 * get_average_frame_rate_interval() seconds.  This measures the virtual frame
 * rate if the clock is in M_non_real_time mode.
 */
double ClockObject::
get_average_frame_rate(Thread *current_thread) const {
  CDStageReader cdata(_cycler, 0, current_thread);
  if (_ticks.size() <= 1) {
    return 0.0;
  } else {
    return _ticks.size() / (cdata->_reported_frame_time - _ticks.front());
  }
}

/**
 * Returns the maximum frame duration over the last
 * get_average_frame_rate_interval() seconds.
 */
double ClockObject::
get_max_frame_duration(Thread *current_thread) const {
  CDStageReader cdata(_cycler, 0, current_thread);
  double max_duration = 0.0;
  double cur_duration = 0.0;
  size_t i;
  for (i = 0; i < _ticks.size() - 1; i++) {
    cur_duration = _ticks[i + 1] - _ticks[i];
    if (cur_duration > max_duration) {
      max_duration = cur_duration;
    }
  }
  return max_duration;
}

/**
 * Returns the standard deviation of the frame times of the frames rendered
 * over the past get_average_frame_rate_interval() seconds.  This number gives
 * an estimate of the chugginess of the frame rate; if it is large, there is a
 * large variation in the frame rate; if is small, all of the frames are
 * consistent in length.
 *
 * A large value might also represent just a recent change in frame rate, for
 * instance, because the camera has just rotated from looking at a simple
 * scene to looking at a more complex scene.
 */
double ClockObject::
calc_frame_rate_deviation(Thread *current_thread) const {
  CDStageReader cdata(_cycler, 0, current_thread);
  if (_ticks.size() <= 1) {
    return 0.0;
  } else {
    double mean = (_ticks.back() - _ticks.front()) / (_ticks.size() - 1);
    size_t i;
    double sum_squares = 0.0;
    for (i = 0; i < _ticks.size() - 1; ++i) {
      double delta = _ticks[i + 1] - _ticks[i];
      double diff = (delta - mean);
      sum_squares += (diff * diff);
    }
    double deviation_2 = sum_squares / (_ticks.size() - 1);
    return sqrt(deviation_2);
  }
}

/**
 * Instructs the clock that a new frame has just begun.  In normal, real-time
 * mode, get_frame_time() will henceforth report the time as of this instant
 * as the current start-of-frame time.  In non-real-time mode,
 * get_frame_time() will be incremented by the value of dt.
 */
void ClockObject::
tick(Thread *current_thread) {
  nassertv(current_thread->get_pipeline_stage() == 0);
  CDWriter cdata(_cycler, current_thread);
  double old_reported_time = cdata->_reported_frame_time;

  if (_mode != M_slave) {
    double old_time = _actual_frame_time;
    _actual_frame_time = get_real_time();

    // In case someone munged the clock last frame and sent us backward in
    // time, clamp the previous time to the current time to make sure we don't
    // report anything strange (or wait interminably).
    old_time = std::min(old_time, _actual_frame_time);

    ++cdata->_frame_count;

    switch (_mode) {
    case M_normal:
      // Time runs as it will; we simply report time elapsing.
      cdata->_dt = _actual_frame_time - old_time;
      cdata->_reported_frame_time = _actual_frame_time;
      break;

    case M_non_real_time:
      // Ignore real time.  We always report the same interval having elapsed
      // each frame.
      cdata->_reported_frame_time = cdata->_reported_frame_time_epoch +
        cdata->_frame_count / _user_frame_rate;
      break;

    case M_limited:
      // If we are running faster than the desired interval, slow down.
      {
        double wait_until_time = old_time + 1.0 / _user_frame_rate;
        wait_until(wait_until_time);
        cdata->_dt = _actual_frame_time - old_time;
        cdata->_reported_frame_time = std::max(_actual_frame_time, wait_until_time);
      }
      break;

    case M_integer:
      {
        double dt = _actual_frame_time - old_time;
        double target_dt = 1.0 / _user_frame_rate;
        if (dt < target_dt) {
          // We're running faster than the desired interval, so slow down to
          // the next integer multiple of the frame rate.
          target_dt = target_dt / floor(target_dt / dt);
        } else {
          // We're running slower than the desired interval, so slow down to
          // the next integer divisor of the frame rate.
          target_dt = target_dt * ceil(dt / target_dt);
        }
        double wait_until_time = old_time + target_dt;
        wait_until(wait_until_time);
        cdata->_dt = target_dt;
        cdata->_reported_frame_time = wait_until_time;
      }
      break;

    case M_integer_limited:
      {
        double dt = _actual_frame_time - old_time;
        double target_dt = 1.0 / _user_frame_rate;
        if (dt < target_dt) {
          // We're running faster than the desired interval, so slow down to
          // the target frame rate.

        } else {
          // We're running slower than the desired interval, so slow down to
          // the next integer divisor of the frame rate.
          target_dt = target_dt * ceil(dt / target_dt);
        }
        double wait_until_time = old_time + target_dt;
        wait_until(wait_until_time);
        cdata->_dt = target_dt;
        cdata->_reported_frame_time = wait_until_time;
      }
      break;

    case M_forced:
      // If we are running faster than the desired interval, slow down.  If we
      // are running slower than the desired interval, ignore that and pretend
      // we're running at the specified rate.
      wait_until(old_time + 1.0 / _user_frame_rate);
      cdata->_reported_frame_time = cdata->_reported_frame_time_epoch +
        cdata->_frame_count / _user_frame_rate;
      break;

    case M_degrade:
      // Each frame, wait a certain fraction of the previous frame's time to
      // degrade performance uniformly.
      cdata->_dt = (_actual_frame_time - old_time) * _degrade_factor;

      if (_degrade_factor < 1.0) {
        // If the degrade_factor is less than one, we want to simulate a
        // higher frame rate by incrementing the clock more slowly.
        cdata->_reported_frame_time += cdata->_dt;

      } else {
        // Otherwise, we simulate a lower frame rate by waiting until the
        // appropriate time has elapsed.
        wait_until(old_time + cdata->_dt);
        cdata->_reported_frame_time = _actual_frame_time;
      }

      break;

    case M_slave:
      // Handled above.
      break;
    }
  }

  if (_average_frame_rate_interval > 0.0) {
    _ticks.push_back(old_reported_time);
    while (_ticks.size() > 2 &&
           cdata->_reported_frame_time - _ticks.front() > _average_frame_rate_interval) {
      _ticks.pop_front();
    }
  }
}

/**
 * Resets the frame time to the current real time.  This is similar to tick(),
 * except that it does not advance the frame counter and does not affect dt.
 * This is intended to be used in the middle of a particularly long frame to
 * compensate for the time that has already elapsed.
 *
 * In non-real-time mode, this function has no effect (because in this mode
 * all frames take the same length of time).
 */
void ClockObject::
sync_frame_time(Thread *current_thread) {
  if (_mode == M_normal) {
    CDWriter cdata(_cycler, current_thread);
    cdata->_reported_frame_time = get_real_time();
  }
}

/**
 * Waits at the end of a frame until the indicated time has arrived.  This is
 * used to implement M_forced and M_degrade.
 */
void ClockObject::
wait_until(double want_time) {
  if (want_time <= _actual_frame_time) {
    return;
  }

#ifdef DO_PSTATS
  (*_start_clock_wait)();
#endif

  double wait_interval = (want_time - _actual_frame_time) - sleep_precision;

  if (wait_interval > 0.0) {
    Thread::sleep(wait_interval);
  }

#ifdef DO_PSTATS
  (*_start_clock_busy_wait)();
#endif

  // Now busy-wait until the actual time elapses.
  while (_actual_frame_time < want_time) {
    _actual_frame_time = get_real_time();
  }

#ifdef DO_PSTATS
  (*_stop_clock_wait)();
#endif
}

/**
 * Called once per application to create the global clock object.
 */
void ClockObject::
make_global_clock() {
  nassertv(_global_clock == nullptr);

  ConfigVariableEnum<ClockObject::Mode> clock_mode
    ("clock-mode", ClockObject::M_normal,
     PRC_DESC("Specifies the mode of the global clock.  The default mode, normal, "
              "is a real-time clock; other modes allow non-real-time special "
              "effects like simulated reduced frame rate.  See "
              "ClockObject::set_mode()."));

  ClockObject *clock = new ClockObject(clock_mode);
  clock->local_object();

  if (AtomicAdjust::compare_and_exchange_ptr(_global_clock, nullptr, clock) != nullptr) {
    // Another thread beat us to it.
    delete clock;
  }
}

/**
 * This no-op function is assigned as the initial pointer for
 * _start_clock_wait and _stop_clock_wait, until the PStatClient comes along
 * and replaces it.
 */
void ClockObject::
dummy_clock_wait() {
}

/**
 *
 */
ClockObject::CData::
CData() {
  _frame_count = 0;
  _reported_frame_time = 0.0;
  _reported_frame_time_epoch = 0.0;
  _dt = 0.0;
}

/**
 *
 */
CycleData *ClockObject::CData::
make_copy() const {
  return new CData(*this);
}

/**
 *
 */
ostream &
operator << (ostream &out, ClockObject::Mode mode) {
  switch (mode) {
  case ClockObject::M_normal:
    return out << "normal";

  case ClockObject::M_non_real_time:
    return out << "non-real-time";

  case ClockObject::M_limited:
    return out << "limited";

  case ClockObject::M_integer:
    return out << "integer";

  case ClockObject::M_integer_limited:
    return out << "integer_limited";

  case ClockObject::M_forced:
    return out << "forced";

  case ClockObject::M_degrade:
    return out << "degrade";

  case ClockObject::M_slave:
    return out << "slave";
  };

  return out << "**invalid ClockObject::Mode(" << (int)mode << ")**";
}

/**
 *
 */
istream &
operator >> (istream &in, ClockObject::Mode &mode) {
  string word;
  in >> word;

  if (cmp_nocase_uh(word, "normal") == 0) {
    mode = ClockObject::M_normal;
  } else if (cmp_nocase_uh(word, "non-real-time") == 0) {
    mode = ClockObject::M_non_real_time;
  } else if (cmp_nocase_uh(word, "limited") == 0) {
    mode = ClockObject::M_limited;
  } else if (cmp_nocase_uh(word, "integer") == 0) {
    mode = ClockObject::M_integer;
  } else if (cmp_nocase_uh(word, "integer_limited") == 0) {
    mode = ClockObject::M_integer_limited;
  } else if (cmp_nocase_uh(word, "forced") == 0) {
    mode = ClockObject::M_forced;
  } else if (cmp_nocase_uh(word, "degrade") == 0) {
    mode = ClockObject::M_degrade;
  } else if (cmp_nocase_uh(word, "slave") == 0) {
    mode = ClockObject::M_slave;
  } else {
    util_cat->error()
      << "Invalid ClockObject::Mode: " << word << "\n";
    mode = ClockObject::M_normal;
  }

  return in;
}
