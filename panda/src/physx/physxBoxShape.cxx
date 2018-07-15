/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physxBoxShape.cxx
 * @author enn0x
 * @date 2009-09-16
 */

#include "physxBoxShape.h"
#include "physxBoxShapeDesc.h"
#include "physxManager.h"

TypeHandle PhysxBoxShape::_type_handle;

/**
 *
 */
void PhysxBoxShape::
link(NxShape *shapePtr) {

  _ptr = shapePtr->isBox();
  _ptr->userData = this;
  _error_type = ET_ok;

  set_name(shapePtr->getName());

  PhysxActor *actor = (PhysxActor *)_ptr->getActor().userData;
  actor->_shapes.add(this);
}

/**
 *
 */
void PhysxBoxShape::
unlink() {

  _ptr->userData = nullptr;
  _error_type = ET_released;

  PhysxActor *actor = (PhysxActor *)_ptr->getActor().userData;
  actor->_shapes.remove(this);
}

/**
 * Saves the state of the shape object to a descriptor.
 */
void PhysxBoxShape::
save_to_desc(PhysxBoxShapeDesc &shapeDesc) const {

  nassertv(_error_type == ET_ok);
  _ptr->saveToDesc(shapeDesc._desc);
}

/**
 * Sets the box dimensions.
 *
 * The dimensions are the 'radii' of the box, meaning 1/2 extents in x
 * dimension, 1/2 extents in y dimension, 1/2 extents in z dimension.
 */
void PhysxBoxShape::
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
LVector3f PhysxBoxShape::
get_dimensions() const {

  nassertr(_error_type == ET_ok, LVector3f::zero());
  return PhysxManager::nxVec3_to_vec3(_ptr->getDimensions());
}
