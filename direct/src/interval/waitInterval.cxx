// Filename: waitInterval.cxx
// Created by:  drose (12Sep02)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#include "waitInterval.h"
#include "config_interval.h"

TypeHandle WaitInterval::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: WaitInterval::priv_step
//       Access: Published, Virtual
//  Description: Advances the time on the interval.  The time may
//               either increase (the normal case) or decrease
//               (e.g. if the interval is being played by a slider).
////////////////////////////////////////////////////////////////////
void WaitInterval::
priv_step(double t) {
  // The WaitInterval is normally not run directly; it just fills up
  // time when constructing a MetaInterval (specifically, a Sequence).
#ifndef NDEBUG
  if (verify_intervals) {
    interval_cat.info() 
      << "running WaitInterval.  Intentional?\n";
  }
#endif
  check_started(get_class_type(), "priv_step");
  _state = S_started;
  _curr_t = t;
}
