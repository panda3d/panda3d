// Filename: clockObject.cxx
// Created by:  drose (17Feb00)
// 
////////////////////////////////////////////////////////////////////

#include "clockObject.h"

ClockObject *ClockObject::_global_clock = (ClockObject *)NULL;


////////////////////////////////////////////////////////////////////
//     Function: ClockObject::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
ClockObject::
ClockObject() {
  _true_clock = TrueClock::get_ptr();
  _mode = M_normal;
  _start_time = _true_clock->get_real_time();
  _frame_count = 0;
  _frame_time = 0.0;
  _reported_time = 0.0;
  _dt = 0.0;
}

////////////////////////////////////////////////////////////////////
//     Function: ClockObject::set_time
//       Access: Public
//  Description: Resets the clock to the indicated time.  Also see
//               reset() and set_frame_count().
////////////////////////////////////////////////////////////////////
void ClockObject::
set_time(double time) {
  double true_time = _true_clock->get_real_time();
  _start_time = true_time - time;
  _frame_time = time;

  switch (_mode) {
  case M_normal:
    _reported_time = _frame_time;
    break;

  case M_non_real_time:
    _reported_time = time;
    break;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: ClockObject::tick
//       Access: Public
//  Description: Instructs the clock that a new frame has just begun.
//               In normal, real-time mode, get_time() will henceforth
//               report the time as of this instant as the current
//               start-of-frame time.  In non-real-time mode,
//               get_time() will be incremented by the value of dt.
////////////////////////////////////////////////////////////////////
void ClockObject::
tick() {
  double old_time = _frame_time;
  _frame_time = get_real_time();

  switch (_mode) {
  case M_normal:
    _dt = _frame_time - old_time;
    _reported_time = _frame_time;
    break;

  case M_non_real_time:
    _reported_time += _dt;
    break;
  }

  _frame_count++;
}
