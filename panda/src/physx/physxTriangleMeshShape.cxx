// Filename: physxTriangleMeshShape.cxx
// Created by:  enn0x (14Oct09)
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

#include "physxTriangleMeshShape.h"
#include "physxTriangleMeshShapeDesc.h"

TypeHandle PhysxTriangleMeshShape::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: PhysxTriangleMeshShape::link
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void PhysxTriangleMeshShape::
link(NxShape *shapePtr) {

  _ptr = shapePtr->isTriangleMesh();
  _ptr->userData = this;
  _error_type = ET_ok;

  set_name(shapePtr->getName());

  PhysxActor *actor = (PhysxActor *)_ptr->getActor().userData;
  actor->_shapes.add(this);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxTriangleMeshShape::unlink
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void PhysxTriangleMeshShape::
unlink() {

  _ptr->userData = NULL;
  _error_type = ET_released;

  PhysxActor *actor = (PhysxActor *)_ptr->getActor().userData;
  actor->_shapes.remove(this);
}

////////////////////////////////////////////////////////////////////
//     Function : PhysxTriangleMeshShape::save_to_desc
//       Access : Published
//  Description : Saves the state of the shape object to a 
//                descriptor.
////////////////////////////////////////////////////////////////////
void PhysxTriangleMeshShape::
save_to_desc(PhysxTriangleMeshShapeDesc &shapeDesc) const {

  nassertv(_error_type == ET_ok);
  _ptr->saveToDesc(shapeDesc._desc);
}

