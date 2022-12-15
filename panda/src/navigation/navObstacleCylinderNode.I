/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file navObstacleCylinderNode.I
 * @author Maxwell175
 * @date 2022-12-15
 */

/**
 * Returns the radius of the cylinder.
 */
INLINE float NavObstacleCylinderNode::
get_radius() const {
  return _radius;
}

/**
 * Sets the radius of the cylinder.
 */
INLINE void NavObstacleCylinderNode::
set_radius(float radius) {
  _radius = radius;
}

/**
 * Returns the height of the cylinder.
 */
INLINE float NavObstacleCylinderNode::
get_height() const {
  return _height;
}

/**
 * Sets the height of the cylinder.
 */
INLINE void NavObstacleCylinderNode::
set_height(float height) {
  _height = height;
}
