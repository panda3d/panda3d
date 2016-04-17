/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physxBoxController.cxx
 * @author enn0x
 * @date 2009-09-24
 */

#include "physxBoxController.h"
#include "physxManager.h"

TypeHandle PhysxBoxController::_type_handle;

/**
 *
 */
void PhysxBoxController::
link(NxController *controllerPtr) {

  nassertv(controllerPtr->getType() == NX_CONTROLLER_BOX);

  // Link self
  _ptr = (NxBoxController *)controllerPtr;
  _error_type = ET_ok;

  PhysxScene *scene = (PhysxScene *)_ptr->getActor()->getScene().userData;
  scene->_controllers.add(this);

  // Link actor
  PT(PhysxActor) actor = new PhysxActor();
  actor->link(_ptr->getActor());
  actor->link_controller(this);
}

/**
 *
 */
void PhysxBoxController::
unlink() {

  // Unlink actor
  PT(PhysxActor) actor = (PhysxActor *)ptr()->getActor()->userData;
  actor->unlink();

  // Unlink self
  _error_type = ET_released;

  PhysxScene *scene = (PhysxScene *)_ptr->getActor()->getScene().userData;
  scene->_controllers.remove(this);
}

/**
 * Sets controller's extents.
 */
void PhysxBoxController::
set_extents(const LVector3f &extents) {

  nassertv(_error_type == ET_ok);
  _ptr->setExtents(PhysxManager::vec3_to_nxVec3(extents));
}

/**
 * Returns controller's extents.
 */
LVector3f PhysxBoxController::
get_extents() const {

  nassertr(_error_type == ET_ok, LVector3f::zero());
  return PhysxManager::nxVec3_to_vec3(_ptr->getExtents());
}
