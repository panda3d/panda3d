/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physxRay.I
 * @author enn0x
 * @date 2009-10-21
 */

/**
 *
 */
INLINE PhysxRay::
PhysxRay() {

  _ray.orig = NxVec3(0.0f, 0.0f, 0.0f);
  _ray.dir = NxVec3(1.0f, 0.0f, 0.0f);

  _length = NX_MAX_F32;
}

/**
 *
 */
INLINE PhysxRay::
~PhysxRay() {

}
