/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file trackerData.cxx
 * @author jason
 * @date 2000-08-04
 */

#include "trackerData.h"

/**
 *
 */
void TrackerData::
operator = (const TrackerData &copy) {
  _flags = copy._flags;

  _time = copy._time;
  _pos = copy._pos;
  _orient = copy._orient;
  _dt = copy._dt;
}
