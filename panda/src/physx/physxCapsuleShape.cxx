// Filename: physxCapsuleShape.cxx
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

#include "physxCapsuleShape.h"
#include "physxCapsuleShapeDesc.h"

TypeHandle PhysxCapsuleShape::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: PhysxCapsuleShape::link
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void PhysxCapsuleShape::
link(NxShape *shapePtr) {

  _ptr = shapePtr->isCapsule();
  _ptr->userData = this;
  _error_type = ET_ok;

  set_name(shapePtr->getName());

  PhysxActor *actor = (PhysxActor *)_ptr->getActor().userData;
  actor->_shapes.add(this);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxCapsuleShape::unlink
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void PhysxCapsuleShape::
unlink() {

  _ptr->userData = NULL;
  _error_type = ET_released;

  PhysxActor *actor = (PhysxActor *)_ptr->getActor().userData;
  actor->_shapes.remove(this);
}

////////////////////////////////////////////////////////////////////
//     Function : PhysxCapsuleShape::save_to_desc
//       Access : Published
//  Description : Saves the state of the shape object to a 
//                descriptor.
////////////////////////////////////////////////////////////////////
void PhysxCapsuleShape::
save_to_desc(PhysxCapsuleShapeDesc &shapeDesc) const {

  nassertv(_error_type == ET_ok);
  _ptr->saveToDesc(shapeDesc._desc);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxCapsuleShape::set_radius
//       Access: Published
//  Description: Alters the radius of the capsule.
////////////////////////////////////////////////////////////////////
void PhysxCapsuleShape::
set_radius(float radius) {

  nassertv(_error_type == ET_ok);
  _ptr->setRadius(radius);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxCapsuleShape::set_height
//       Access: Published
//  Description: Alters the height of the capsule.
////////////////////////////////////////////////////////////////////
void PhysxCapsuleShape::
set_height(float height) {

  nassertv(_error_type == ET_ok);
  _ptr->setHeight(height);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxCapsuleShape::get_radius
//       Access: Published
//  Description: Retrieves the radius of the capsule.
////////////////////////////////////////////////////////////////////
float PhysxCapsuleShape::
get_radius() const {

  nassertr(_error_type == ET_ok, 0.0f);
  return _ptr->getRadius();
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxCapsuleShape::get_height
//       Access: Published
//  Description: Retrieves the height of the capsule.
////////////////////////////////////////////////////////////////////
float PhysxCapsuleShape::
get_height() const {

  nassertr(_error_type == ET_ok, 0.0f);
  return _ptr->getHeight();
}

