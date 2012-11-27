// Filename: physxControllersHit.I
// Created by:  enn0x (28Nov12)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
//     Function: PhysxControllersHit::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE PhysxControllersHit::
PhysxControllersHit(const NxControllersHit &hit) :
  _hit(hit) {

}

////////////////////////////////////////////////////////////////////
//     Function: PhysxControllersHit::get_controller
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE PhysxController *PhysxControllersHit::
get_controller() const {

  NxController *controllerPtr = _hit.controller;
  PhysxController *controller = controllerPtr ? (PhysxController *)controllerPtr : NULL;

  return controller;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxControllersHit::get_other
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE PhysxController *PhysxControllersHit::
get_other() const {

  NxController *otherPtr = _hit.other;
  PhysxController *other = otherPtr ? (PhysxController *)otherPtr : NULL;

  return other;
}

