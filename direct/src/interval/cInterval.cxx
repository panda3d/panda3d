// Filename: cInterval.cxx
// Created by:  drose (27Aug02)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#include "cInterval.h"
#include "indent.h"
#include "clockObject.h"
#include "throw_event.h"
#include <math.h>

TypeHandle CInterval::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: CInterval::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
CInterval::
CInterval(const string &name, double duration, bool open_ended) :
  _state(S_initial),
  _curr_t(0.0),
  _name(name),
  _duration(duration),
  _open_ended(open_ended),
  _dirty(false)
{
  _clock_start = 0.0;
  _start_t = 0.0;
  _end_t = _duration;
  _start_t_at_start = true;
  _end_t_at_end = true;
  _play_rate = 1.0;
  _loop_count = 0;
}

////////////////////////////////////////////////////////////////////
//     Function: CInterval::setup_play
//       Access: Published
//  Description: Called to prepare the interval for automatic timed
//               playback, e.g. via a Python task.  The interval will
//               be played from start_t to end_t, at a time factor
//               specified by play_rate.  start_t must always be less
//               than end_t (except for the exception for end_t == -1,
//               below), but if play_rate is negative the interval
//               will be played backwards.
//
//               Specify end_t of -1 to play the entire interval from
//               start_t.
//
//               Call step_play() repeatedly to execute the interval.
////////////////////////////////////////////////////////////////////
void CInterval::
setup_play(double start_t, double end_t, double play_rate) {
  nassertv(start_t < end_t || end_t < 0.0);
  nassertv(play_rate != 0.0);

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
  _loop_count = 0;
}

////////////////////////////////////////////////////////////////////
//     Function: CInterval::setup_resume
//       Access: Published
//  Description: Called to prepare the interval for restarting at the
//               current point within the interval after an
//               interruption.
////////////////////////////////////////////////////////////////////
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

////////////////////////////////////////////////////////////////////
//     Function: CInterval::step_play
//       Access: Published
//  Description: Should be called once per frame to execute the
//               automatic timed playback begun with setup_play().
//               The return value is the number of times the interval
//               is about to repeat; stop when this reaches one to
//               play the interval through exactly once.
////////////////////////////////////////////////////////////////////
int CInterval::
step_play() {
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
        // Only finalize if the playback cycle includes the whole
        // interval.
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
      
      // Advance the clock for the next loop cycle.  We might have to
      // advance multiple times if we skipped several cycles in the past
      // frame.
      
      if (_end_t == _start_t) {
        // If the interval has no length, we loop exactly once each
        // time.
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
        // Only finalize if the playback cycle includes the whole
        // interval.
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
      
      // Advance the clock for the next loop cycle.  We might have to
      // advance multiple times if we skipped several cycles in the past
      // frame.
      
      if (_end_t == _start_t) {
        // If the interval has no length, we loop exactly once each
        // time.
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

  return _loop_count;
}

////////////////////////////////////////////////////////////////////
//     Function: CInterval::priv_do_event
//       Access: Published
//  Description: Calls the appropriate event function indicated by the
//               EventType.
////////////////////////////////////////////////////////////////////
void CInterval::
priv_do_event(double t, EventType event) {
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

////////////////////////////////////////////////////////////////////
//     Function: CInterval::priv_initialize
//       Access: Published, Virtual
//  Description: This replaces the first call to priv_step(), and indicates
//               that the interval has just begun.  This may be
//               overridden by derived classes that need to do some
//               explicit initialization on the first call.
////////////////////////////////////////////////////////////////////
void CInterval::
priv_initialize(double t) {
  check_stopped("priv_initialize");
  recompute();
  _state = S_started;
  priv_step(t);
}

////////////////////////////////////////////////////////////////////
//     Function: CInterval::priv_instant
//       Access: Published, Virtual
//  Description: This is called in lieu of priv_initialize() .. priv_step()
//               .. priv_finalize(), when everything is to happen within
//               one frame.  The interval should initialize itself,
//               then leave itself in the final state.
////////////////////////////////////////////////////////////////////
void CInterval::
priv_instant() {
  check_stopped("priv_instant");
  recompute();
  _state = S_started;
  priv_step(get_duration());
  _state = S_final;
  interval_done();
}

////////////////////////////////////////////////////////////////////
//     Function: CInterval::priv_step
//       Access: Published, Virtual
//  Description: Advances the time on the interval.  The time may
//               either increase (the normal case) or decrease
//               (e.g. if the interval is being played by a slider).
////////////////////////////////////////////////////////////////////
void CInterval::
priv_step(double t) {
  check_started("priv_step");
  _state = S_started;
  _curr_t = t;
}

////////////////////////////////////////////////////////////////////
//     Function: CInterval::priv_finalize
//       Access: Published, Virtual
//  Description: This is called to stop an interval, forcing it to
//               whatever state it would be after it played all the
//               way through.  It's generally invoked by
//               set_final_t().
////////////////////////////////////////////////////////////////////
void CInterval::
priv_finalize() {
  check_started("priv_step");
  double duration = get_duration();
  priv_step(duration);
  _state = S_final;
  interval_done();
}

////////////////////////////////////////////////////////////////////
//     Function: CInterval::reverse_initialize
//       Access: Published, Virtual
//  Description: Similar to priv_initialize(), but this is called when the
//               interval is being played backwards; it indicates that
//               the interval should start at the finishing state and
//               undo any intervening intervals.
////////////////////////////////////////////////////////////////////
void CInterval::
priv_reverse_initialize(double t) {
  check_stopped("priv_reverse_initialize");
  recompute();
  _state = S_started;
  priv_step(t);
}

////////////////////////////////////////////////////////////////////
//     Function: CInterval::reverse_instant
//       Access: Published, Virtual
//  Description: This is called in lieu of priv_reverse_initialize()
//               .. priv_step() .. priv_reverse_finalize(), when everything is
//               to happen within one frame.  The interval should
//               initialize itself, then leave itself in the initial
//               state.
////////////////////////////////////////////////////////////////////
void CInterval::
priv_reverse_instant() {
  check_stopped("priv_reverse_instant");
  recompute();
  _state = S_started;
  priv_step(0.0);
  _state = S_initial;
}

////////////////////////////////////////////////////////////////////
//     Function: CInterval::reverse_finalize
//       Access: Published, Virtual
//  Description: Called generally following a priv_reverse_initialize(),
//               this indicates the interval should set itself to the
//               initial state.
////////////////////////////////////////////////////////////////////
void CInterval::
priv_reverse_finalize() {
  check_started("priv_reverse_finalize");
  priv_step(0.0);
  _state = S_initial;
}

////////////////////////////////////////////////////////////////////
//     Function: CInterval::priv_interrupt
//       Access: Published, Virtual
//  Description: This is called while the interval is playing to
//               indicate that it is about to be interrupted; that is,
//               priv_step() will not be called for a length of time.  But
//               the interval should remain in its current state in
//               anticipation of being eventually restarted when the
//               calls to priv_step() eventually resume.
//
//               The purpose of this function is to allow self-running
//               intervals like sound intervals to stop the actual
//               sound playback during the pause.
////////////////////////////////////////////////////////////////////
void CInterval::
priv_interrupt() {
  check_started("priv_interrupt");
  _state = S_paused;
}

////////////////////////////////////////////////////////////////////
//     Function: CInterval::output
//       Access: Published, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void CInterval::
output(ostream &out) const {
  out << get_name();
  if (get_duration() != 0.0) {
    out << " dur " << get_duration();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CInterval::write
//       Access: Published, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void CInterval::
write(ostream &out, int indent_level) const {
  indent(out, indent_level) << *this << "\n";
}

////////////////////////////////////////////////////////////////////
//     Function: CInterval::mark_dirty
//       Access: Public
//  Description: Called by a derived class to indicate the interval has
//               been changed internally and must be recomputed before
//               its duration may be returned.
////////////////////////////////////////////////////////////////////
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

////////////////////////////////////////////////////////////////////
//     Function: CInterval::interval_done
//       Access: Protected
//  Description: Called internally whenever the interval reaches its
//               final state.
////////////////////////////////////////////////////////////////////
void CInterval::
interval_done() {
  if (!_done_event.empty()) {
    throw_event(_done_event);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CInterval::do_recompute
//       Access: Protected, Virtual
//  Description: Does whatever processing is necessary to recompute
//               the interval after a call to mark_dirty() has
//               indicated a recomputation is necessary.
////////////////////////////////////////////////////////////////////
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

