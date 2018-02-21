/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file bulletSphereShape.I
 * @author enn0x
 * @date 2010-01-23
 */

/**
 *
 */
INLINE BulletSphereShape::
~BulletSphereShape() {

  delete _shape;
}

/**
 * Returns the radius that was used to construct this sphere.
 */
INLINE PN_stdfloat BulletSphereShape::
get_radius() const {
  return _radius;
}
