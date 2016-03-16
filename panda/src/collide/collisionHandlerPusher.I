/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file collisionHandlerPusher.I
 * @author drose
 * @date 2002-03-16
 */

/**
 *
 */
INLINE void CollisionHandlerPusher::
set_horizontal(bool flag) {
  _horizontal = flag;
}

/**
 *
 */
INLINE bool CollisionHandlerPusher::
get_horizontal() const {
  return _horizontal;
}
