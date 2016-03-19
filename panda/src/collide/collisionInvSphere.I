/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file collisionInvSphere.I
 * @author drose
 * @date 2005-01-05
 */

/**
 *
 */
INLINE CollisionInvSphere::
CollisionInvSphere(const LPoint3 &center, PN_stdfloat radius) :
  CollisionSphere(center, radius)
{
}

/**
 *
 */
INLINE CollisionInvSphere::
CollisionInvSphere(PN_stdfloat cx, PN_stdfloat cy, PN_stdfloat cz, PN_stdfloat radius) :
  CollisionSphere(cx, cy, cz, radius)
{
}

/**
 * Creates an invalid sphere.  Only used when reading from a bam file.
 */
INLINE CollisionInvSphere::
CollisionInvSphere() {
}

/**
 *
 */
INLINE CollisionInvSphere::
CollisionInvSphere(const CollisionInvSphere &copy) :
  CollisionSphere(copy)
{
}
