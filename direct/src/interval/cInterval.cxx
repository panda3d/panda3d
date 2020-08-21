/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file cInterval.cxx
 * @author drose
 * @date 2002-08-27
 */

#include "cInterval.h"
#include "cIntervalManager.h"
#include "indent.h"
#include "clockObject.h"
#include "event.h"
#include "eventQueue.h"
#include "pStatTimer.h"

using std::ostream;
using std::string;

PStatCollector CInterval::_root_pcollector("App:Show code:ivalLoop");
TypeHandle CInterval::_type_handle;

static inline string
get_pstats_name(const string &name) {
  string pname = name;
  size_t hyphen = pname.find('-');
  if (hyphen != string::npos) {
    pname = pname.substr(0, hyphen);
  }
  return pname;
}

/**
 *
 */
CInterval::
CInterval(const string &name, double duration, bool open_ended) :
  _state(S_initial),
  _curr_t(0.0),
  _name(name),
  _pname(get_pstats_name(name)),
  _duration(std::max(duration, 0.0)),
  _open_ended(open_ended),
  _dirty(false),
  _ival_pcollector(_root_pcollector, _pname)
{
  _auto_pause = false;
  _auto_finish = false;
  _wants_t_callback = false;
  _last_t_callback = -1.0;
  _manager = CIntervalManager::get_global_ptr();
  _clock_start = 0.0;
  _start_t = 0.0;
  _end_t = _duration;
  _start_t_at_start = true;
  _end_t_at_end = true;
  _play_rate = 1.0;
  _do_loop = false;
  _loop_count = 0;

  if (interval_cat.is_spam()) {
    interval_cat.spam()
      << "Constructing interval " << (void *)this << ", duration = "
      << _duration << "\n";
  }
}

/**
 *
 */
CInterval::
~CInterval() {
  if (interval_cat.is_spam()) {
    interval_cat.spam()
      << "Destructing interval " << (void *)this << "\n";
  }
}

/**
 * Explicitly sets the time within the interval.  Normally, you would use
 * start() .. finish() to let the time play normally, but this may be used to
 * set the time to some particular value.
 */
void CInterval::
set_t(double t) {
  // There doesn't seem to be any reason to clamp this, and it breaks looping
  // intervals.  The interval code should properly handle t values outside the
  // proper range.  t = min(max(t, 0.0), get_duration());

  switch (get_state()) {
  case S_initial:
    priv_initialize(t);
    if (is_playing()) {
      setup_resume();
    } else {
      priv_interrupt();
    }
    break;

  case S_started:
    // Support modifying t while the interval is playing.  We assume
    // is_playing() will be true in this state.
    nassertv(is_playing());
    priv_interrupt();
    priv_step(t);
    setup_resume();
    break;

  case S_paused:
    // Support modifying t while the interval is paused.  In this case, we
    // simply step to the new value of t; but this will change the state to
    // S_started, so we must then change it back to S_paused by hand (because
    // we're still paused).
    priv_step(t);
    priv_interrupt();
    break;

  case S_final:
    priv_reverse_initialize(t);
    if (is_playing()) {
      setup_resume();
    } else {
      priv_interrupt();
    }
    break;
  }
}

/**
 * Starts the interval playing by registering it with the current
 * CIntervalManager.  The interval will play to the end and stop.
 *
 * If end_t is less than zero, it indicates the end of the interval.
 */
void CInterval::
start(double start_t, double end_t, double play_rate) {
  setup_play(start_t, end_t, play_rate, false);
  _manager->add_c_interval(this, false);
}

/**
 * Starts the interval playing by registering it with the current
 * CIntervalManager.  The interval will play until it is interrupted with
 * finish() or pause(), looping back to start_t when it reaches end_t.
 *
 * If end_t is less than zero, it indicates the end of the interval.
 */
void CInterval::
loop(double start_t, double end_t, double play_rate) {
  setup_play(start_t, end_t, play_rate, true);
  _manager->add_c_interval(this, false);
}

/**
 * Stops the interval from playing but leaves it in its current state.  It may
 * later be resumed from this point by calling resume().
 */
double CInterval::
pause() {
  if (get_state() == S_started) {
    priv_interrupt();
  }
  int index = _manager->find_c_interval(this->get_name());
  if (index >= 0) {
    _manager->remove_c_interval(index);
  }
  return get_t();
}

/**
 * Restarts the interval from its current point after a previous call to
 * pause().
 */
void CInterval::
resume() {
  setup_resume();
  _manager->add_c_interval(this, false);
}

/**
 * Restarts the interval from the indicated point after a previous call to
 * pause().
 */
void CInterval::
resume(double start_t) {
  set_t(start_t);
  setup_resume();
  _manager->add_c_interval(this, false);
}

/**
 * Restarts the interval from the current point after a previous call to
 * pause() (or a previous play-to-point-and-stop), to play until the indicated
 * point and then stop.
 */
void CInterval::
resume_until(double end_t) {
  setup_resume_until(end_t);
  _manager->add_c_interval(this, false);
}

/**
 * Stops the interval from playing and sets it to its final state.
 */
void CInterval::
finish() {
  switch (get_state()) {
  case S_initial:
    priv_instant();
    break;

  case S_final:
    break;

  default:
    priv_finalize();
  }

  int index = _manager->find_c_interval(this->get_name());
  if (index >= 0) {
    _manager->remove_c_interval(index);
  }
}

/**
 * Pauses the interval, if it is playing, and resets its state to its initial
 * state, abandoning any state changes already in progress in the middle of
 * the interval.  Calling this is like pausing the interval and discarding it,
 * creating a new one in its place.
 */
void CInterval::
clear_to_initial() {
  pause();

  _state = S_initial;
  _curr_t = 0.0;
}

/**
 * Returns true if the interval is currently playing, false otherwise.
 */
bool CInterval::
is_playing() const {
  int index = _manager->find_c_interval(this->get_name());
  return (index >= 0);
}

/**
 * Returns the play rate as set by the last call to start(), loop(), or
 * set_play_rate().
 */
double CInterval::
get_play_rate() const {
  return _play_rate;
}

/**
 * Changes the play rate of the interval.  If the interval is already started,
 * this changes its speed on-the-fly.  Note that since play_rate is a
 * parameter to start() and loop(), the next call to start() or loop() will
 * reset this parameter.
 */
void CInterval::
set_play_rate(double play_rate) {
  if (is_playing()) {
    pause();
    _play_rate = play_rate;
    resume();
  } else {
    _play_rate = play_rate;
  }
}

/**
 * Calls the appropriate event function indicated by the EventType.
 */
void CInterval::
priv_do_event(double t, EventType event) {
  PStatTimer timer(_ival_pcollector);
  switch (event) {
  case ET_initialize:
    priv_initialize(t);
    return;

  case ET_instant:
    priv_instant();
    return;

  case ET_step:
    priv_step(t);
    return;

  case ET_finalize:
    priv_finalize();
    return;

  case ET_reverse_initialize:
    priv_reverse_initialize(t);
    return;

  case ET_reverse_instant:
    priv_reverse_instant();
    return;

  case ET_reverse_finalize:
    priv_reverse_finalize();
    return;

  case ET_interrupt:
    priv_interrupt();
    return;
  }

  interval_cat.warning()
    << "Invalid event type: " << (int)event << "\n";
}

/**
 * This replaces the first call to priv_step(), and indicates that the
 * interval has just begun.  This may be overridden by derived classes that
 * need to do some explicit initialization on the first call.
 */
void CInterval::
priv_initialize(double t) {
  check_stopped(get_class_type(), "priv_initialize");
  recompute();
  _state = S_started;
  priv_step(t);
}

/**
 * This is called in lieu of priv_initialize() .. priv_step() ..
 * priv_finalize(), when everything is to happen within one frame.  The
 * interval should initialize itself, then leave itself in the final state.
 */
void CInterval::
priv_instant() {
  check_stopped(get_class_type(), "priv_instant");
  recompute();
  _state = S_started;
  priv_step(get_duration());
  _state = S_final;
  interval_done();
}

/**
 * Advances the time on the interval.  The time may either increase (the
 * normal case) or decrease (e.g.  if the interval is being played by a
 * slider).
 */
void CInterval::
priv_step(double t) {
  check_started(get_class_type(), "priv_step");
  _state = S_started;
  _curr_t = t;
}

/**
 * This is called to stop an interval, forcing it to whatever state it would
 * be after it played all the way through.  It's generally invoked by
 * set_final_t().
 */
void CInterval::
priv_finalize() {
  check_started(get_class_type(), "priv_finalize");
  double duration = get_duration();
  priv_step(duration);
  _state = S_final;
  interval_done();
}

/**
 * Similar to priv_initialize(), but this is called when the interval is being
 * played backwards; it indicates that the interval should start at the
 * finishing state and undo any intervening intervals.
 */
void CInterval::
priv_reverse_initialize(double t) {
  check_stopped(get_class_type(), "priv_reverse_initialize");
  recompute();
  _state = S_started;
  priv_step(t);
}

/**
 * This is called in lieu of priv_reverse_initialize() .. priv_step() ..
 * priv_reverse_finalize(), when everything is to happen within one frame.
 * The interval should initialize itself, then leave itself in the initial
 * state.
 */
void CInterval::
priv_reverse_instant() {
  check_stopped(get_class_type(), "priv_reverse_instant");
  recompute();
  _state = S_started;
  priv_step(0.0);
  _state = S_initial;
}

/**
 * Called generally following a priv_reverse_initialize(), this indicates the
 * interval should set itself to the initial state.
 */
void CInterval::
priv_reverse_finalize() {
  check_started(get_class_type(), "priv_reverse_finalize");
  priv_step(0.0);
  _state = S_initial;
}

/**
 * This is called while the interval is playing to indicate that it is about
 * to be interrupted; that is, priv_step() will not be called for a length of
 * time.  But the interval should remain in its current state in anticipation
 * of being eventually restarted when the calls to priv_step() eventually
 * resume.
 *
 * The purpose of this function is to allow self-running intervals like sound
 * intervals to stop the actual sound playback during the pause.
 */
void CInterval::
priv_interrupt() {
  check_started(get_class_type(), "priv_interrupt");
  _state = S_paused;
}

/**
 *
 */
void CInterval::
output(ostream &out) const {
  out << get_name();
  if (get_duration() != 0.0) {
    out << " dur " << get_duration();
  }
}

/**
 *
 */
void CInterval::
write(ostream &out, int indent_level) const {
  indent(out, indent_level) << *this << "\n";
}

/**
 * Called to prepare the interval for automatic timed playback, e.g.  via a
 * Python task.  The interval will be played from start_t to end_t, at a time
 * factor specified by play_rate.  start_t must always be less than end_t
 * (except for the exception for end_t == -1, below), but if play_rate is
 * negative the interval will be played backwards.
 *
 * Specify end_t of -1 to play the entire interval from start_t.
 *
 * Call step_play() repeatedly to execute the interval.
 */
void CInterval::
setup_play(double start_t, double end_t, double play_rate, bool do_loop) {
  nassertv(start_t < end_t || end_t < 0.0);
  nassertv(play_rate != 0.0);
  PStatTimer timer(_ival_pcollector);

  double duration = get_duration();

  if (start_t <= 0.0) {
    _start_t = 0.0;
    _start_t_at_start = true;
  } else if (start_t > duration) {
    _start_t = duration;
    _start_t_at_start = false;
  } else {
    _start_t = start_t;
    _start_t_at_start = false;
  }
  if (end_t < 0.0 || end_t >= duration) {
    _end_t = duration;
    _end_t_at_end = true;
  } else {
    _end_t = end_t;
    _end_t_at_end = false;
  }

  _clock_start = ClockObject::get_global_clock()->get_frame_time();
  _play_rate = play_rate;
  _do_loop = do_loop;
  _loop_count = 0;
}

/**
 * Called to prepare the interval for restarting at the current point within
 * the interval after an interruption.
 */
void CInterval::
setup_resume() {
  double now = ClockObject::get_global_clock()->get_frame_time();
  if (_play_rate > 0.0) {
    _clock_start = now - ((get_t() - _start_t) / _play_rate);

  } else if (_play_rate < 0.0) {
    _clock_start = now - ((get_t() - _end_t) / _play_rate);
  }
  _loop_count = 0;
}

/**
 * Called to prepare the interval for restarting from the current point after
 * a previous call to pause() (or a previous play-to-point-and-stop), to play
 * until the indicated point and then stop.
 */
void CInterval::
setup_resume_until(double end_t) {
  double duration = get_duration();

  if (end_t < 0.0 || end_t >= duration) {
    _end_t = duration;
    _end_t_at_end = true;
  } else {
    _end_t = end_t;
    _end_t_at_end = false;
  }

  setup_resume();
}

/**
 * Should be called once per frame to execute the automatic timed playback
 * begun with setup_play().
 *
 * Returns true if the interval should continue, false if it is done and
 * should stop.
 */
bool CInterval::
step_play() {
  PStatTimer timer(_ival_pcollector);
  double now = ClockObject::get_global_clock()->get_frame_time();

  if (_play_rate >= 0.0) {
    double t = (now - _clock_start) * _play_rate + _start_t;

    if (_end_t_at_end) {
      _end_t = get_duration();
    }

    if (t < _end_t) {
      // In the middle of the interval, not a problem.
      if (is_stopped()) {
        priv_initialize(t);
      } else {
        priv_step(t);
      }

    } else {
      // Past the ending point; time to finalize.
      if (_end_t_at_end) {
        // Only finalize if the playback cycle includes the whole interval.
        if (is_stopped()) {
          if (get_open_ended() || _loop_count != 0) {
            priv_instant();
          }
        } else {
          priv_finalize();
        }
      } else {
        if (is_stopped()) {
          priv_initialize(_end_t);
        } else {
          priv_step(_end_t);
        }
      }

      // Advance the clock for the next loop cycle.  We might have to advance
      // multiple times if we skipped several cycles in the past frame.

      if (_end_t == _start_t) {
        // If the interval has no length, we loop exactly once each time.
        _loop_count++;

      } else {
        // Otherwise, figure out how many loops we need to skip.
        double time_per_loop = (_end_t - _start_t) / _play_rate;
        double num_loops = floor((now - _clock_start) / time_per_loop);
        _loop_count += (int)num_loops;
        _clock_start += num_loops * time_per_loop;
      }
    }

  } else {
    // Playing backwards.
    double t = (now - _clock_start) * _play_rate + _end_t;

    if (t >= _start_t) {
      // In the middle of the interval, not a problem.
      if (is_stopped()) {
        priv_reverse_initialize(t);
      } else {
        priv_step(t);
      }

    } else {
      // Past the ending point; time to finalize.
      if (_start_t_at_start) {
        // Only finalize if the playback cycle includes the whole interval.
        if (is_stopped()) {
          if (get_open_ended() || _loop_count != 0) {
            priv_reverse_instant();
          }
        } else {
          priv_reverse_finalize();
        }
      } else {
        if (is_stopped()) {
          priv_reverse_initialize(_start_t);
        } else {
          priv_step(_start_t);
        }
      }

      // Advance the clock for the next loop cycle.  We might have to advance
      // multiple times if we skipped several cycles in the past frame.

      if (_end_t == _start_t) {
        // If the interval has no length, we loop exactly once each time.
        _loop_count++;

      } else {
        // Otherwise, figure out how many loops we need to skip.
        double time_per_loop = (_end_t - _start_t) / -_play_rate;
        double num_loops = floor((now - _clock_start) / time_per_loop);
        _loop_count += (int)num_loops;
        _clock_start += num_loops * time_per_loop;
      }
    }
  }

  bool should_continue = (_loop_count == 0 || _do_loop);

  if (!should_continue && _state == S_started) {
    priv_interrupt();
  }

  return should_continue;
}

/**
 * Called by a derived class to indicate the interval has been changed
 * internally and must be recomputed before its duration may be returned.
 */
void CInterval::
mark_dirty() {
  if (!_dirty) {
    _dirty = true;
    Parents::iterator pi;
    for (pi = _parents.begin(); pi != _parents.end(); ++pi) {
      (*pi)->mark_dirty();
    }
  }
}

/**
 * Called internally whenever the interval reaches its final state.
 */
void CInterval::
interval_done() {
  if (!_done_event.empty()) {
    _manager->get_event_queue()->queue_event(new Event(_done_event));
  }
}

/**
 * Does whatever processing is necessary to recompute the interval after a
 * call to mark_dirty() has indicated a recomputation is necessary.
 */
void CInterval::
do_recompute() {
  _dirty = false;
}

ostream &
operator << (ostream &out, CInterval::State state) {
  switch (state) {
  case CInterval::S_initial:
    return out << "initial";

  case CInterval::S_started:
    return out << "started";

  case CInterval::S_paused:
    return out << "paused";

  case CInterval::S_final:
    return out << "final";
  }

  return out << "**invalid state(" << (int)state << ")**";
}
