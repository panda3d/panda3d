/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physxControllersHit.I
 * @author enn0x
 * @date 2012-11-28
 */

/**
 *
 */
INLINE PhysxControllersHit::
PhysxControllersHit(const NxControllersHit &hit) :
  _hit(hit) {

}

/**
 *
 */
INLINE PhysxController *PhysxControllersHit::
get_controller() const {

  NxController *controllerPtr = _hit.controller;
  PhysxController *controller = controllerPtr ? (PhysxController *)controllerPtr : nullptr;

  return controller;
}

/**
 *
 */
INLINE PhysxController *PhysxControllersHit::
get_other() const {

  NxController *otherPtr = _hit.other;
  PhysxController *other = otherPtr ? (PhysxController *)otherPtr : nullptr;

  return other;
}
