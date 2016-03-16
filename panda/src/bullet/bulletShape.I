/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file bulletShape.I
 * @author enn0x
 * @date 2010-01-23
 */

/**
 *
 */
INLINE BulletShape::
~BulletShape() {

}

/**
 *
 */
INLINE bool BulletShape::
is_polyhedral() const {

  return ptr()->isPolyhedral();
}

/**
 *
 */
INLINE bool BulletShape::
is_convex() const {

  return ptr()->isConvex();
}

/**
 *
 */
INLINE bool BulletShape::
is_convex_2d() const {

  return ptr()->isConvex2d();
}

/**
 *
 */
INLINE bool BulletShape::
is_concave() const {

  return ptr()->isConcave();
}

/**
 *
 */
INLINE bool BulletShape::
is_infinite() const {

  return ptr()->isInfinite();
}

/**
 *
 */
INLINE bool BulletShape::
is_non_moving() const {

  return ptr()->isNonMoving();
}

/**
 *
 */
INLINE bool BulletShape::
is_soft_body() const {

  return ptr()->isSoftBody();
}
