/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file navPath.I
 * @author Ashwani / 
 * @date 2024
 */

/**
 * Returns the number of points in the path.
 */
INLINE int NavPath::
get_num_points() const {
  return _waypoints.size();
}

/**
 * Returns the point at the given index.
 */
INLINE LPoint3 NavPath::
get_point(int i) const {
  nassertr(i >= 0 && i < (int)_waypoints.size(), LPoint3::zero());
  return _waypoints[i];
}
