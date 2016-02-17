/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physxContactPair.I
 * @author enn0x
 * @date 2009-12-19
 */

/**
 *
 */
INLINE PhysxContactPair::
PhysxContactPair(const NxContactPair pair) {

  _pair = pair;
}

/**
 *
 */
INLINE PhysxContactPair::
~PhysxContactPair() {

}
