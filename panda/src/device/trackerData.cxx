// Filename: trackerData.cxx
// Created by:  jason (04Aug00)
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
