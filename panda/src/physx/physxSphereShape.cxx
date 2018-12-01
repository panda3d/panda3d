/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physxSphereShape.cxx
 * @author enn0x
 * @date 2009-09-16
 */

#include "physxSphereShape.h"
#include "physxSphereShapeDesc.h"

TypeHandle PhysxSphereShape::_type_handle;

/**
 *
 */
void PhysxSphereShape::
link(NxShape *shapePtr) {

  _ptr = shapePtr->isSphere();
  _ptr->userData = this;
  _error_type = ET_ok;

  set_name(shapePtr->getName());

  PhysxActor *actor = (PhysxActor *)_ptr->getActor().userData;
  actor->_shapes.add(this);
}

/**
 *
 */
void PhysxSphereShape::
unlink() {

  _ptr->userData = nullptr;
  _error_type = ET_released;

  PhysxActor *actor = (PhysxActor *)_ptr->getActor().userData;
  actor->_shapes.remove(this);
}

/**
 * Saves the state of the shape object to a descriptor.
 */
void PhysxSphereShape::
save_to_desc(PhysxSphereShapeDesc &shapeDesc) const {

  nassertv(_error_type == ET_ok);
  _ptr->saveToDesc(shapeDesc._desc);
}

/**
 * Sets the sphere radius.
 */
void PhysxSphereShape::
set_radius(float radius) {

  nassertv(_error_type == ET_ok);
  _ptr->setRadius(radius);
}

/**
 * Returns the radius of the sphere.
 */
float PhysxSphereShape::
get_radius() const {

  nassertr(_error_type == ET_ok, 0.0f);
  return _ptr->getRadius();
}
