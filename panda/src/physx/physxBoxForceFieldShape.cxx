/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physxBoxForceFieldShape.cxx
 * @author enn0x
 * @date 2009-11-15
 */

#include "physxBoxForceFieldShape.h"
#include "physxBoxForceFieldShapeDesc.h"
#include "physxManager.h"

TypeHandle PhysxBoxForceFieldShape::_type_handle;

/**
 *
 */
void PhysxBoxForceFieldShape::
link(NxForceFieldShape *shapePtr) {

  _ptr = shapePtr->isBox();
  _ptr->userData = this;
  _error_type = ET_ok;

  set_name(shapePtr->getName());

  PhysxForceFieldShapeGroup *group = (PhysxForceFieldShapeGroup *)_ptr->getShapeGroup().userData;
  group->_shapes.add(this);
}

/**
 *
 */
void PhysxBoxForceFieldShape::
unlink() {

  _ptr->userData = nullptr;
  _error_type = ET_released;

  PhysxForceFieldShapeGroup *group = (PhysxForceFieldShapeGroup *)_ptr->getShapeGroup().userData;
  group->_shapes.remove(this);
}

/**
 * Saves the state of the shape object to a descriptor.
 */
void PhysxBoxForceFieldShape::
save_to_desc(PhysxBoxForceFieldShapeDesc &shapeDesc) const {

  nassertv(_error_type == ET_ok);
  _ptr->saveToDesc(shapeDesc._desc);
}

/**
 * Sets the box dimensions.
 *
 * The dimensions are the 'radii' of the box, meaning 1/2 extents in x
 * dimension, 1/2 extents in y dimension, 1/2 extents in z dimension.
 */
void PhysxBoxForceFieldShape::
set_dimensions(const LVector3f &vec) {

  nassertv(_error_type == ET_ok);
  _ptr->setDimensions(PhysxManager::vec3_to_nxVec3(vec));
}

/**
 * Retrieves the dimensions of the box.
 *
 * The dimensions are the 'radii' of the box, meaning 1/2 extents in x
 * dimension, 1/2 extents in y dimension, 1/2 extents in z dimension.
 */
LVector3f PhysxBoxForceFieldShape::
get_dimensions() const {

  nassertr(_error_type == ET_ok, LVector3f::zero());
  return PhysxManager::nxVec3_to_vec3(_ptr->getDimensions());
}
