/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physxSphereForceFieldShape.cxx
 * @author enn0x
 * @date 2009-11-15
 */

#include "physxSphereForceFieldShape.h"
#include "physxSphereForceFieldShapeDesc.h"

TypeHandle PhysxSphereForceFieldShape::_type_handle;

/**
 *
 */
void PhysxSphereForceFieldShape::
link(NxForceFieldShape *shapePtr) {

  _ptr = shapePtr->isSphere();
  _ptr->userData = this;
  _error_type = ET_ok;

  set_name(shapePtr->getName());

  PhysxForceFieldShapeGroup *group = (PhysxForceFieldShapeGroup *)_ptr->getShapeGroup().userData;
  group->_shapes.add(this);
}

/**
 *
 */
void PhysxSphereForceFieldShape::
unlink() {

  _ptr->userData = nullptr;
  _error_type = ET_released;

  PhysxForceFieldShapeGroup *group = (PhysxForceFieldShapeGroup *)_ptr->getShapeGroup().userData;
  group->_shapes.remove(this);
}

/**
 * Saves the state of the shape object to a descriptor.
 */
void PhysxSphereForceFieldShape::
save_to_desc(PhysxSphereForceFieldShapeDesc &shapeDesc) const {

  nassertv(_error_type == ET_ok);
  _ptr->saveToDesc(shapeDesc._desc);
}

/**
 * Sets the sphere radius.
 */
void PhysxSphereForceFieldShape::
set_radius(float radius) {

  nassertv(_error_type == ET_ok);
  _ptr->setRadius(radius);
}

/**
 * Returns the radius of the sphere.
 */
float PhysxSphereForceFieldShape::
get_radius() const {

  nassertr(_error_type == ET_ok, 0.0f);
  return _ptr->getRadius();
}
