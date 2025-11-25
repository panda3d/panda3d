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
 * Adds a point to the path.
 */
void NavPath::
add_point(const LPoint3 &point) {
  _points.push_back(point);
}

/**
 * Output method
 */
void NavPath::
output(std::ostream &out) const {
  out << "NavPath(" << _points.size() << " points)";
}

