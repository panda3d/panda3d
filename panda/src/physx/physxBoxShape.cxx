// Filename: physxBoxShape.cxx
// Created by:  enn0x (16Sep09)
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

#include "physxBoxShape.h"
#include "physxBoxShapeDesc.h"
#include "physxManager.h"

TypeHandle PhysxBoxShape::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: PhysxBoxShape::link
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void PhysxBoxShape::
link(NxShape *shapePtr) {

  _ptr = shapePtr->isBox();
  _ptr->userData = this;
  _error_type = ET_ok;

  set_name(shapePtr->getName());

  PhysxActor *actor = (PhysxActor *)_ptr->getActor().userData;
  actor->_shapes.add(this);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxBoxShape::unlink
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void PhysxBoxShape::
unlink() {

  _ptr->userData = NULL;
  _error_type = ET_released;

  PhysxActor *actor = (PhysxActor *)_ptr->getActor().userData;
  actor->_shapes.remove(this);
}

////////////////////////////////////////////////////////////////////
//     Function : PhysxBoxShape::save_to_desc
//       Access : Published
//  Description : Saves the state of the shape object to a 
//                descriptor.
////////////////////////////////////////////////////////////////////
void PhysxBoxShape::
save_to_desc(PhysxBoxShapeDesc &shapeDesc) const {

  nassertv(_error_type == ET_ok);
  _ptr->saveToDesc(shapeDesc._desc);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxBoxShape::set_dimensions
//       Access: Published
//  Description: Sets the box dimensions. 
//
//               The dimensions are the 'radii' of the box,
//               meaning 1/2 extents in x dimension, 1/2 extents
//               in y dimension, 1/2 extents in z dimension.
////////////////////////////////////////////////////////////////////
void PhysxBoxShape::
set_dimensions(const LVector3f &vec) {

  nassertv(_error_type == ET_ok);
  _ptr->setDimensions(PhysxManager::vec3_to_nxVec3(vec));
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxBoxShape::get_dimensions
//       Access: Published
//  Description: Retrieves the dimensions of the box. 
//
//               The dimensions are the 'radii' of the box,
//               meaning 1/2 extents in x dimension, 1/2 extents
//               in y dimension, 1/2 extents in z dimension.
////////////////////////////////////////////////////////////////////
LVector3f PhysxBoxShape::
get_dimensions() const {

  nassertr(_error_type == ET_ok, LVector3f::zero());
  return PhysxManager::nxVec3_to_vec3(_ptr->getDimensions());
}

