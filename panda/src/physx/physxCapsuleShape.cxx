/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physxCapsuleShape.cxx
 * @author enn0x
 * @date 2009-09-16
 */

#include "physxCapsuleShape.h"
#include "physxCapsuleShapeDesc.h"

TypeHandle PhysxCapsuleShape::_type_handle;

/**
 *
 */
void PhysxCapsuleShape::
link(NxShape *shapePtr) {

  _ptr = shapePtr->isCapsule();
  _ptr->userData = this;
  _error_type = ET_ok;

  set_name(shapePtr->getName());

  PhysxActor *actor = (PhysxActor *)_ptr->getActor().userData;
  actor->_shapes.add(this);
}

/**
 *
 */
void PhysxCapsuleShape::
unlink() {

  _ptr->userData = nullptr;
  _error_type = ET_released;

  PhysxActor *actor = (PhysxActor *)_ptr->getActor().userData;
  actor->_shapes.remove(this);
}

/**
 * Saves the state of the shape object to a descriptor.
 */
void PhysxCapsuleShape::
save_to_desc(PhysxCapsuleShapeDesc &shapeDesc) const {

  nassertv(_error_type == ET_ok);
  _ptr->saveToDesc(shapeDesc._desc);
}

/**
 * Alters the radius of the capsule.
 */
void PhysxCapsuleShape::
set_radius(float radius) {

  nassertv(_error_type == ET_ok);
  _ptr->setRadius(radius);
}

/**
 * Alters the height of the capsule.
 */
void PhysxCapsuleShape::
set_height(float height) {

  nassertv(_error_type == ET_ok);
  _ptr->setHeight(height);
}

/**
 * Retrieves the radius of the capsule.
 */
float PhysxCapsuleShape::
get_radius() const {

  nassertr(_error_type == ET_ok, 0.0f);
  return _ptr->getRadius();
}

/**
 * Retrieves the height of the capsule.
 */
float PhysxCapsuleShape::
get_height() const {

  nassertr(_error_type == ET_ok, 0.0f);
  return _ptr->getHeight();
}
