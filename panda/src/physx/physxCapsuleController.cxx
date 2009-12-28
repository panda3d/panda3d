// Filename: physxCapsuleController.cxx
// Created by:  enn0x (24Sep09)
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

#include "physxCapsuleController.h"

TypeHandle PhysxCapsuleController::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: PhysxCapsuleController::link
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void PhysxCapsuleController::
link(NxController *controllerPtr) {

  nassertv(controllerPtr->getType() == NX_CONTROLLER_CAPSULE);

  // Link self
  _ptr = (NxCapsuleController *)controllerPtr;
  _error_type = ET_ok;

  PhysxScene *scene = (PhysxScene *)_ptr->getActor()->getScene().userData;
  scene->_controllers.add(this);

  // Link actor
  PT(PhysxActor) actor = new PhysxActor();
  actor->link(_ptr->getActor());
  actor->link_controller(this);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxCapsuleController::unlink
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void PhysxCapsuleController::
unlink() {

  // Unlink actor
  PT(PhysxActor) actor = (PhysxActor *)ptr()->getActor()->userData;
  actor->unlink();

  // Unlink self
  _error_type = ET_released;

  PhysxScene *scene = (PhysxScene *)_ptr->getActor()->getScene().userData;
  scene->_controllers.remove(this);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxCapsuleController::set_radius
//       Access: Published
//  Description: Resets the controller's radius.
////////////////////////////////////////////////////////////////////
void PhysxCapsuleController::
set_radius(float radius) {

  nassertv(_error_type == ET_ok);
  _ptr->setRadius(radius);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxCapsuleController::set_height
//       Access: Published
//  Description: Resets the controller's height.
////////////////////////////////////////////////////////////////////
void PhysxCapsuleController::
set_height(float height) {

  nassertv(_error_type == ET_ok);
  _ptr->setHeight(height);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxCapsuleController::get_radius
//       Access: Published
//  Description: Returns the controller's radius.
////////////////////////////////////////////////////////////////////
float PhysxCapsuleController::
get_radius() const {

  nassertr(_error_type == ET_ok, 0.0f);
  return _ptr->getRadius();
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxCapsuleController::get_height
//       Access: Published
//  Description: Returns the controller's height.
////////////////////////////////////////////////////////////////////
float PhysxCapsuleController::
get_height() const {

  nassertr(_error_type == ET_ok, 0.0f);
  return _ptr->getHeight();
}

