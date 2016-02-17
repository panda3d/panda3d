/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file collisionHandlerFloor.I
 * @author drose
 * @date 2002-03-16
 */

/**
 * Sets the linear offset to add to (or subtract from) the highest detected
 * collision point to determine the actual height at which to set the
 * collider.
 */
INLINE void CollisionHandlerFloor::
set_offset(PN_stdfloat offset) {
  _offset = offset;
}

/**
 * Returns the linear offset to add to (or subtract from) the highest detected
 * collision point to determine the actual height at which to set the
 * collider.
 */
INLINE PN_stdfloat CollisionHandlerFloor::
get_offset() const {
  return _offset;
}

/**
 * Sets the reach to add to (or subtract from) the highest collision point
 */
INLINE void CollisionHandlerFloor::
set_reach(PN_stdfloat reach) {
  _reach = reach;
}

/**
 * Returns the reach to add to (or subtract from) the highest collision point
 */
INLINE PN_stdfloat CollisionHandlerFloor::
get_reach() const {
  return _reach;
}

/**
 * Sets the maximum speed at which the object will be allowed to descend
 * towards a floor below it, in units per second.  Set this to zero to allow
 * it to instantly teleport any distance.
 */
INLINE void CollisionHandlerFloor::
set_max_velocity(PN_stdfloat max_velocity) {
  _max_velocity = max_velocity;
}

/**
 * Retrieves the maximum speed at which the object will be allowed to descend
 * towards a floor below it, in units per second.  See set_max_velocity().
 */
INLINE PN_stdfloat CollisionHandlerFloor::
get_max_velocity() const {
  return _max_velocity;
}
