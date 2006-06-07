// Filename: clockObject.cxx
// Created by:  drose (17Feb00)
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

#include "clockObject.h"
#include "config_util.h"
#include "configVariableEnum.h"
#include "thread.h"

ClockObject *ClockObject::_global_clock = (ClockObject *)NULL;
TypeHandle ClockObject::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: ClockObject::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
ClockObject::
ClockObject() {
  _true_clock = TrueClock::get_global_ptr();

  // Each clock except for the application global clock is created in
  // M_normal mode.  The application global clock is later reset to
  // respect clock_mode, which comes from the Config.prc file.
  _mode = M_normal;

  _start_short_time = _true_clock->get_short_time();
  _start_long_time = _true_clock->get_long_time();
  _actual_frame_time = 0.0;
  _max_dt = max_dt;
  _degrade_factor = clock_degrade_factor;
  _average_frame_rate_interval = average_frame_rate_interval;

  _error_count = _true_clock->get_error_count();

  CDReader cdata(_cycler);
  _set_dt = cdata->_dt;
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
    util_cat.warning()
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
}

////////////////////////////////////////////////////////////////////
//     Function: ClockObject::set_frame_count
//       Access: Published
//  Description: Resets the number of frames counted to the indicated
//               number.  Also see reset(), set_real_time(), and
//               set_frame_time().
////////////////////////////////////////////////////////////////////
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
tick(Thread *current_thread) {
  nassertv(current_thread->get_pipeline_stage() == 0);
  CDWriter cdata(_cycler, current_thread);
  double old_reported_time = cdata->_reported_frame_time;

  if (_mode != M_slave) {
    double old_time = _actual_frame_time;
    _actual_frame_time = get_real_time();
    
    switch (_mode) {
    case M_normal:
      // Time runs as it will; we simply report time elapsing.
      cdata->_dt = _actual_frame_time - old_time;
      cdata->_reported_frame_time = _actual_frame_time;
      break;
      
    case M_non_real_time:
      // Ignore real time.  We always report the same interval having
      // elapsed each frame.
      cdata->_reported_frame_time += _set_dt;
      break;
      
    case M_limited:
      // If we are running faster than the desired interval, slow down.
      wait_until(old_time + _set_dt);
      cdata->_dt = _actual_frame_time - old_time;
      cdata->_reported_frame_time = _actual_frame_time;
      break;
      
    case M_forced:
      // If we are running faster than the desired interval, slow down.
      // If we are running slower than the desired interval, ignore that
      // and pretend we're running at the specified rate.
      wait_until(old_time + _set_dt);
      cdata->_reported_frame_time += _set_dt;
      break;
      
    case M_degrade:
      // Each frame, wait a certain fraction of the previous frame's
      // time to degrade performance uniformly.
      cdata->_dt = (_actual_frame_time - old_time) * _degrade_factor;
      
      if (_degrade_factor < 1.0) {
        // If the degrade_factor is less than one, we want to simulate a
        // higher frame rate by incrementing the clock more slowly.
        cdata->_reported_frame_time += cdata->_dt;
        
      } else {
        // Otherwise, we simulate a lower frame rate by waiting until
        // the appropriate time has elapsed.
        wait_until(old_time + cdata->_dt);
        cdata->_reported_frame_time = _actual_frame_time;
      }
      
      break;

    case M_slave:
      // Handled above.
      break;
    }

    cdata->_frame_count++;
  }

  if (_average_frame_rate_interval > 0.0) {
    _ticks.push_back(old_reported_time);
    while (!_ticks.empty() && 
           cdata->_reported_frame_time - _ticks.front() > _average_frame_rate_interval) {
      _ticks.pop_front();
    }
  }
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
sync_frame_time(Thread *current_thread) {
  if (_mode == M_normal) {
    CDWriter cdata(_cycler, current_thread);
    cdata->_reported_frame_time = get_real_time();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: ClockObject::wait_until
//       Access: Private
//  Description: Waits at the end of a frame until the indicated time
//               has arrived.  This is used to implement M_forced and
//               M_degrade.
////////////////////////////////////////////////////////////////////
void ClockObject::
wait_until(double want_time) {
  double wait_interval = (want_time - _actual_frame_time) - sleep_precision;
    
  if (wait_interval > 0.0) {
    Thread::sleep(wait_interval);
  }
  
  // Now busy-wait until the actual time elapses.
  while (_actual_frame_time < want_time) {
    _actual_frame_time = get_real_time();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: ClockObject::make_global_clock
//       Access: Private, Static
//  Description: Called once per application to create the global
//               clock object.
////////////////////////////////////////////////////////////////////
void ClockObject::
make_global_clock() {
  nassertv(_global_clock == (ClockObject *)NULL);

  ConfigVariableEnum<ClockObject::Mode> clock_mode
    ("clock-mode", ClockObject::M_normal,
     PRC_DESC("Specifies the mode of the global clock.  The default mode, normal, "
              "is a real-time clock; other modes allow non-real-time special "
              "effects like simulated reduced frame rate.  See "
              "ClockObject::set_mode()."));

  _global_clock = new ClockObject;
  _global_clock->set_mode(clock_mode);
}

////////////////////////////////////////////////////////////////////
//     Function: ClockObject::CData::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
ClockObject::CData::
CData() {
  _frame_count = 0;
  _reported_frame_time = 0.0;
  _dt = 1.0 / clock_frame_rate;
}

////////////////////////////////////////////////////////////////////
//     Function: ClockObject::CData::make_copy
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
CycleData *ClockObject::CData::
make_copy() const {
  return new CData(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: ClockObject::Mode ostream operator
//  Description:
////////////////////////////////////////////////////////////////////
ostream &
operator << (ostream &out, ClockObject::Mode mode) {
  switch (mode) {
  case ClockObject::M_normal:
    return out << "normal";

  case ClockObject::M_non_real_time:
    return out << "non-real-time";

  case ClockObject::M_limited:
    return out << "limited";

  case ClockObject::M_forced:
    return out << "forced";

  case ClockObject::M_degrade:
    return out << "degrade";

  case ClockObject::M_slave:
    return out << "slave";
  };

  return out << "**invalid ClockObject::Mode(" << (int)mode << ")**";
}

////////////////////////////////////////////////////////////////////
//     Function: ClockObject::Mode istream operator
//  Description:
////////////////////////////////////////////////////////////////////
istream &
operator >> (istream &in, ClockObject::Mode &mode) {
  string word;
  in >> word;

  if (word == "normal") {
    mode = ClockObject::M_normal;
  } else if (word == "non-real-time") {
    mode = ClockObject::M_non_real_time;
  } else if (word == "limited") {
    mode = ClockObject::M_limited;
  } else if (word == "forced") {
    mode = ClockObject::M_forced;
  } else if (word == "degrade") {
    mode = ClockObject::M_degrade;
  } else if (word == "slave") {
    mode = ClockObject::M_slave;
  } else {
    util_cat.error()
      << "Invalid ClockObject::Mode: " << word << "\n";
    mode = ClockObject::M_normal;
  }

  return in;
}
