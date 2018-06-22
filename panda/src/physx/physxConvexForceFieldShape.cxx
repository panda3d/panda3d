/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physxConvexForceFieldShape.cxx
 * @author enn0x
 * @date 2009-11-15
 */

#include "physxConvexForceFieldShape.h"
#include "physxConvexForceFieldShapeDesc.h"

TypeHandle PhysxConvexForceFieldShape::_type_handle;

/**
 *
 */
void PhysxConvexForceFieldShape::
link(NxForceFieldShape *shapePtr) {

  _ptr = shapePtr->isConvex();
  _ptr->userData = this;
  _error_type = ET_ok;

  set_name(shapePtr->getName());

  PhysxForceFieldShapeGroup *group = (PhysxForceFieldShapeGroup *)_ptr->getShapeGroup().userData;
  group->_shapes.add(this);
}

/**
 *
 */
void PhysxConvexForceFieldShape::
unlink() {

  _ptr->userData = nullptr;
  _error_type = ET_released;

  PhysxForceFieldShapeGroup *group = (PhysxForceFieldShapeGroup *)_ptr->getShapeGroup().userData;
  group->_shapes.remove(this);
}

/**
 * Saves the state of the shape object to a descriptor.
 */
void PhysxConvexForceFieldShape::
save_to_desc(PhysxConvexForceFieldShapeDesc &shapeDesc) const {

  nassertv(_error_type == ET_ok);
  _ptr->saveToDesc(shapeDesc._desc);
}
