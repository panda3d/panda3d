/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physxPlaneShape.cxx
 * @author enn0x
 * @date 2009-09-16
 */

#include "physxPlaneShape.h"
#include "physxPlaneShapeDesc.h"
#include "physxManager.h"

TypeHandle PhysxPlaneShape::_type_handle;

/**
 *
 */
void PhysxPlaneShape::
link(NxShape *shapePtr) {

  _ptr = shapePtr->isPlane();
  _ptr->userData = this;
  _error_type = ET_ok;

  set_name(shapePtr->getName());

  PhysxActor *actor = (PhysxActor *)_ptr->getActor().userData;
  actor->_shapes.add(this);
}

/**
 *
 */
void PhysxPlaneShape::
unlink() {

  _ptr->userData = nullptr;
  _error_type = ET_released;

  PhysxActor *actor = (PhysxActor *)_ptr->getActor().userData;
  actor->_shapes.remove(this);
}

/**
 * Saves the state of the shape object to a descriptor.
 */
void PhysxPlaneShape::
save_to_desc(PhysxPlaneShapeDesc &shapeDesc) const {

  nassertv(_error_type == ET_ok);
  _ptr->saveToDesc(shapeDesc._desc);
}

/**
 * Sets the plane equation.  - normal:  Normal for the plane, in the global
 * frame.  Range: direction vector - d: Distance coefficient of the plane
 * equation.  Range: (-inf,inf)
 */
void PhysxPlaneShape::
set_plane(const LVector3f &normal, float d) {

  nassertv(_error_type == ET_ok);
  _ptr->setPlane(PhysxManager::vec3_to_nxVec3(normal), d);
}
