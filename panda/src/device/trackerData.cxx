// Filename: trackerData.cxx
// Created by:  jason (04Aug00)
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

#include "trackerData.h"

////////////////////////////////////////////////////////////////////
//     Function: TrackerData::Copy Assignment Operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void TrackerData::
operator = (const TrackerData &copy) {
  _flags = copy._flags;

  _time = copy._time;
  _pos = copy._pos;
  _orient = copy._orient;
  _dt = copy._dt;
}
