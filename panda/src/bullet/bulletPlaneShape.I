/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file bulletPlaneShape.I
 * @author enn0x
 * @date 2010-01-23
 */

/**

 */
INLINE BulletPlaneShape::
~BulletPlaneShape() {

  delete _shape;
}

/**

 */
INLINE BulletPlaneShape::
BulletPlaneShape(const BulletPlaneShape &copy) :
  _shape(copy._shape) {
}

/**

 */
INLINE void BulletPlaneShape::
operator = (const BulletPlaneShape &copy) {
  _shape = copy._shape;
}

/**

 */
INLINE PN_stdfloat BulletPlaneShape::
get_plane_constant() const {

  return (PN_stdfloat)_shape->getPlaneConstant();
}

/**

 */
INLINE LVector3 BulletPlaneShape::
get_plane_normal() const {

  return btVector3_to_LVector3(_shape->getPlaneNormal());
}
