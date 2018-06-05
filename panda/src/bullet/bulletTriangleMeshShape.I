/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file bulletTriangleMeshShape.I
 * @author enn0x
 * @date 2010-02-09
 */

/**
 *
 */
INLINE BulletTriangleMeshShape::
~BulletTriangleMeshShape() {

  if (_bvh_shape) {
    delete _bvh_shape;
  }

  if (_gimpact_shape) {
    delete _gimpact_shape;
  }
}

/**
 *
 */
INLINE bool BulletTriangleMeshShape::
is_static() const {

  return (_bvh_shape != nullptr);
}

/**
 *
 */
INLINE bool BulletTriangleMeshShape::
is_dynamic() const {

  return (_gimpact_shape != nullptr);
}
