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
 * Returns the number of waypoints in the path.
 */
INLINE int NavPath::
get_num_points() const {
  return _points.size();
}

/**
 * Returns the point at the given index.
 */
INLINE LPoint3 NavPath::
get_point(int n) const {
  nassertr(n >= 0 && n < (int)_points.size(), LPoint3::zero());
  return _points[n];
}

/**
 * Returns true if the path is valid (has points).
 */
INLINE bool NavPath::
is_valid() const {
  return !_points.empty();
}

