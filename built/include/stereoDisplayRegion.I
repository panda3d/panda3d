/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file stereoDisplayRegion.I
 * @author drose
 * @date 2009-02-19
 */

/**
 * Returns a pointer to the left DisplayRegion managed by this stereo object.
 */
INLINE DisplayRegion *StereoDisplayRegion::
get_left_eye() {
  return _left_eye;
}

/**
 * Returns a pointer to the right DisplayRegion managed by this stereo object.
 */
INLINE DisplayRegion *StereoDisplayRegion::
get_right_eye() {
  return _right_eye;
}
