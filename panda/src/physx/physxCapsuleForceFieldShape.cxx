/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physxCapsuleForceFieldShape.cxx
 * @author enn0x
 * @date 2009-11-15
 */

#include "physxCapsuleForceFieldShape.h"
#include "physxCapsuleForceFieldShapeDesc.h"

TypeHandle PhysxCapsuleForceFieldShape::_type_handle;

/**
 *
 */
void PhysxCapsuleForceFieldShape::
link(NxForceFieldShape *shapePtr) {

  _ptr = shapePtr->isCapsule();
  _ptr->userData = this;
  _error_type = ET_ok;

  set_name(shapePtr->getName());

  PhysxForceFieldShapeGroup *group = (PhysxForceFieldShapeGroup *)_ptr->getShapeGroup().userData;
  group->_shapes.add(this);
}

/**
 *
 */
void PhysxCapsuleForceFieldShape::
unlink() {

  _ptr->userData = nullptr;
  _error_type = ET_released;

  PhysxForceFieldShapeGroup *group = (PhysxForceFieldShapeGroup *)_ptr->getShapeGroup().userData;
  group->_shapes.remove(this);
}

/**
 * Saves the state of the shape object to a descriptor.
 */
void PhysxCapsuleForceFieldShape::
save_to_desc(PhysxCapsuleForceFieldShapeDesc &shapeDesc) const {

  nassertv(_error_type == ET_ok);
  _ptr->saveToDesc(shapeDesc._desc);
}

/**
 * Alters the radius of the capsule.
 */
void PhysxCapsuleForceFieldShape::
set_radius(float radius) {

  nassertv(_error_type == ET_ok);
  _ptr->setRadius(radius);
}

/**
 * Alters the height of the capsule.
 */
void PhysxCapsuleForceFieldShape::
set_height(float height) {

  nassertv(_error_type == ET_ok);
  _ptr->setHeight(height);
}

/**
 * Retrieves the radius of the capsule.
 */
float PhysxCapsuleForceFieldShape::
get_radius() const {

  nassertr(_error_type == ET_ok, 0.0f);
  return _ptr->getRadius();
}

/**
 * Retrieves the height of the capsule.
 */
float PhysxCapsuleForceFieldShape::
get_height() const {

  nassertr(_error_type == ET_ok, 0.0f);
  return _ptr->getHeight();
}
