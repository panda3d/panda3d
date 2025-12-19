/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file navPath.cxx
 * @author Ashwani / 
 * @date 2024
 */

#include "navPath.h"

TypeHandle NavPath::_type_handle;

/**
 * Constructor
 */
NavPath::
NavPath() {
}

/**
 * Destructor
 */
NavPath::
~NavPath() {
}

/**
 * Returns the total length of the path.
 */
PN_stdfloat NavPath::
get_length() const {
  if (_waypoints.empty()) {
    return 0.0f;
  }
  
  PN_stdfloat length = 0.0f;
  for (size_t i = 1; i < _waypoints.size(); ++i) {
    length += (_waypoints[i] - _waypoints[i-1]).length();
  }
  return length;
}

/**
 * Adds a point to the path.
 */
void NavPath::
add_point(const LPoint3 &point) {
  _waypoints.push_back(point);
}
