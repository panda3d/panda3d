// Filename: physxConvexForceFieldShape.cxx
// Created by:  enn0x (15Nov09)
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

#include "physxConvexForceFieldShape.h"
#include "physxConvexForceFieldShapeDesc.h"

TypeHandle PhysxConvexForceFieldShape::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: PhysxConvexForceFieldShape::link
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void PhysxConvexForceFieldShape::
link(NxForceFieldShape *shapePtr) {

  _ptr = shapePtr->isConvex();
  _ptr->userData = this;
  _error_type = ET_ok;

  set_name(shapePtr->getName());

  PhysxForceFieldShapeGroup *group = (PhysxForceFieldShapeGroup *)_ptr->getShapeGroup().userData;
  group->_shapes.add(this);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxConvexForceFieldShape::unlink
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void PhysxConvexForceFieldShape::
unlink() {

  _ptr->userData = NULL;
  _error_type = ET_released;

  PhysxForceFieldShapeGroup *group = (PhysxForceFieldShapeGroup *)_ptr->getShapeGroup().userData;
  group->_shapes.remove(this);
}

////////////////////////////////////////////////////////////////////
//     Function : PhysxConvexForceFieldShape::save_to_desc
//       Access : Published
//  Description : Saves the state of the shape object to a 
//                descriptor.
////////////////////////////////////////////////////////////////////
void PhysxConvexForceFieldShape::
save_to_desc(PhysxConvexForceFieldShapeDesc &shapeDesc) const {

  nassertv(_error_type == ET_ok);
  _ptr->saveToDesc(shapeDesc._desc);
}

