// Filename: cInterval.I
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


////////////////////////////////////////////////////////////////////
//     Function: CInterval::get_name
//       Access: Published
//  Description: Returns the interval's name.
////////////////////////////////////////////////////////////////////
INLINE const string &CInterval::
get_name() const {
  return _name;
}

////////////////////////////////////////////////////////////////////
//     Function: CInterval::get_duration
//       Access: Published
//  Description: Returns the duration of the interval in seconds.
////////////////////////////////////////////////////////////////////
INLINE double CInterval::
get_duration() const {
  recompute();
  return _duration;
}

////////////////////////////////////////////////////////////////////
//     Function: CInterval::get_open_ended
//       Access: Published
//  Description: Returns the state of the "open_ended" flag.  This is
//               primarily intended for instantaneous intervals like
//               FunctionIntervals; it indicates true if the interval
//               has some lasting effect that should be applied even
//               if the interval doesn't get started until after its
//               finish time, or false if the interval is a transitive
//               thing that doesn't need to be called late.
////////////////////////////////////////////////////////////////////
INLINE bool CInterval::
get_open_ended() const {
  return _open_ended;
}

////////////////////////////////////////////////////////////////////
//     Function: CInterval::get_state
//       Access: Published
//  Description: Indicates the state the interval believes it is in:
//               whether it has been started, is currently in the
//               middle, or has been finalized.
////////////////////////////////////////////////////////////////////
INLINE CInterval::State CInterval::
get_state() const {
  return _state;
}

////////////////////////////////////////////////////////////////////
//     Function: CInterval::is_stopped
//       Access: Published
//  Description: Returns true if the interval is in either its initial
//               or final states (but not in a running or paused
//               state).
////////////////////////////////////////////////////////////////////
INLINE bool CInterval::
is_stopped() const {
  return (_state == S_initial || _state == S_final);
}

////////////////////////////////////////////////////////////////////
//     Function: CInterval::get_t
//       Access: Published
//  Description: Returns the current time of the interval: the last
//               value of t passed to priv_initialize(), priv_step(), or
//               priv_finalize().
////////////////////////////////////////////////////////////////////
INLINE double CInterval::
get_t() const {
  return _curr_t;
}

////////////////////////////////////////////////////////////////////
//     Function: CInterval::recompute
//       Access: Protected
//  Description: Calls do_recompute() if the dirty flag has been set.
////////////////////////////////////////////////////////////////////
INLINE void CInterval::
recompute() const {
  if (_dirty) {
    ((CInterval *)this)->do_recompute();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CInterval::check_stopped
//       Access: Protected
//  Description: Issues a warning if our internal state is not in
//               one of the stopped states.
////////////////////////////////////////////////////////////////////
INLINE void CInterval::
check_stopped(const char *method_name) const {
  if (_state == S_started) {
    interval_cat.warning()
      << get_name() << "." << method_name << "() called in state "
      << _state << ".\n";
    nassertv(!verify_intervals);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CInterval::check_started
//       Access: Protected
//  Description: Issues a warning if our internal state is not in
//               one of the started states.
////////////////////////////////////////////////////////////////////
INLINE void CInterval::
check_started(const char *method_name) const {
  if (_state != S_started && _state != S_paused) {
    interval_cat.warning()
      << get_name() << "." << method_name << "() called in state "
      << _state << ".\n";
    nassertv(!verify_intervals);
  }
}

INLINE ostream &
operator << (ostream &out, const CInterval &ival) {
  ival.output(out);
  return out;
}

