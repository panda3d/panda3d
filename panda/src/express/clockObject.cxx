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
  _start_time = _true_clock->get_real_time();
  _frame_count = 0;
  _actual_frame_time = 0.0;
  _reported_frame_time = 0.0;
  _dt = 0.0;
  _max_dt = -1.0;
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
//     Function: get_time_of_day
//  Description:
////////////////////////////////////////////////////////////////////
void get_time_of_day(TimeVal &tv) {
  get_true_time_of_day(tv.tv[0], tv.tv[1]);
}
