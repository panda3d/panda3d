// Filename: waitInterval.cxx
// Created by:  drose (12Sep02)
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
