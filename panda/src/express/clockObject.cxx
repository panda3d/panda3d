// Filename: clockObject.cxx
// Created by:  drose (17Feb00)
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


#include "clockObject.h"
#include "config_express.h"

ClockObject *ClockObject::_global_clock = (ClockObject *)NULL;


////////////////////////////////////////////////////////////////////
//     Function: ClockObject::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
ClockObject::
ClockObject() {
  _true_clock = TrueClock::get_ptr();
  _mode = M_normal;
  _start_short_time = _true_clock->get_short_time();
  _start_long_time = _true_clock->get_long_time();
  _frame_count = 0;
  _actual_frame_time = 0.0;
  _reported_frame_time = 0.0;
  _dt = 0.0;
  _max_dt = -1.0;
}

////////////////////////////////////////////////////////////////////
//     Function: ClockObject::set_real_time
//       Access: Published
//  Description: Resets the clock to the indicated time.  This
//               changes only the real time of the clock as reported
//               by get_real_time(), but does not immediately change
//               the time reported by get_frame_time()--that will
//               change after the next call to tick().  Also see
//               reset(), set_frame_time(), and set_frame_count().
////////////////////////////////////////////////////////////////////
void ClockObject::
set_real_time(double time) {
#ifdef NOTIFY_DEBUG
  if (this == _global_clock) {
    express_cat.warning()
      << "Adjusting global clock's real time by " << time - get_real_time()
      << " seconds.\n";
  }
#endif  // NOTIFY_DEBUG
  _start_short_time = _true_clock->get_short_time() - time;
  _start_long_time = _true_clock->get_long_time() - time;
}

////////////////////////////////////////////////////////////////////
//     Function: ClockObject::set_frame_time
//       Access: Published
//  Description: Changes the time as reported for the current frame to
//               the indicated time.  Normally, the way to adjust the
//               frame time is via tick(); this function is provided
//               only for occasional special adjustments.
////////////////////////////////////////////////////////////////////
void ClockObject::
set_frame_time(double time) {
#ifdef NOTIFY_DEBUG
  if (this == _global_clock) {
    express_cat.warning()
      << "Adjusting global clock's frame time by " << time - get_frame_time()
      << " seconds.\n";
  }
#endif  // NOTIFY_DEBUG
  _actual_frame_time = time;
  _reported_frame_time = time;
}

////////////////////////////////////////////////////////////////////
//     Function: ClockObject::set_frame_count
//       Access: Published
//  Description: Resets the number of frames counted to the indicated
//               number.  Also see reset(), set_real_time(), and
//               set_frame_time().
////////////////////////////////////////////////////////////////////
void ClockObject::
set_frame_count(int frame_count) {
#ifdef NOTIFY_DEBUG
  if (this == _global_clock) {
    express_cat.warning()
      << "Adjusting global clock's frame count by " 
      << frame_count - get_frame_count() << " frames.\n";
  }
#endif  // NOTIFY_DEBUG
  _frame_count = frame_count;
}

////////////////////////////////////////////////////////////////////
//     Function: ClockObject::tick
//       Access: Published
//  Description: Instructs the clock that a new frame has just begun.
//               In normal, real-time mode, get_frame_time() will
//               henceforth report the time as of this instant as the
//               current start-of-frame time.  In non-real-time mode,
//               get_frame_time() will be incremented by the value of
//               dt.
////////////////////////////////////////////////////////////////////
void ClockObject::
tick() {
  double old_time = _actual_frame_time;
  _actual_frame_time = get_real_time();

  switch (_mode) {
  case M_normal:
    _dt = _actual_frame_time - old_time;
    if (_max_dt > 0.0) {
      _dt = min(_max_dt, _dt);
    }
    _reported_frame_time = _actual_frame_time;
    break;

  case M_non_real_time:
    _reported_frame_time += _dt;
    break;
  }

  _frame_count++;
}

////////////////////////////////////////////////////////////////////
//     Function: ClockObject::sync_frame_time
//       Access: Published
//  Description: Resets the frame time to the current real time.  This
//               is similar to tick(), except that it does not advance
//               the frame counter and does not affect dt.  This is
//               intended to be used in the middle of a particularly
//               long frame to compensate for the time that has
//               already elapsed.
//
//               In non-real-time mode, this function has no effect
//               (because in this mode all frames take the same length
//               of time).
////////////////////////////////////////////////////////////////////
void ClockObject::
sync_frame_time() {
  if (_mode == M_normal) {
    _reported_frame_time = get_real_time();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: get_time_of_day
//  Description:
////////////////////////////////////////////////////////////////////
void get_time_of_day(TimeVal &tv) {
  get_true_time_of_day(tv.tv[0], tv.tv[1]);
}
