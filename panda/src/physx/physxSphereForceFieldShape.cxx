// Filename: physxSphereForceFieldShape.cxx
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

#include "physxSphereForceFieldShape.h"
#include "physxSphereForceFieldShapeDesc.h"

TypeHandle PhysxSphereForceFieldShape::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: PhysxSphereForceFieldShape::link
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void PhysxSphereForceFieldShape::
link(NxForceFieldShape *shapePtr) {

  _ptr = shapePtr->isSphere();
  _ptr->userData = this;
  _error_type = ET_ok;

  set_name(shapePtr->getName());

  PhysxForceFieldShapeGroup *group = (PhysxForceFieldShapeGroup *)_ptr->getShapeGroup().userData;
  group->_shapes.add(this);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxSphereForceFieldShape::unlink
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void PhysxSphereForceFieldShape::
unlink() {

  _ptr->userData = NULL;
  _error_type = ET_released;

  PhysxForceFieldShapeGroup *group = (PhysxForceFieldShapeGroup *)_ptr->getShapeGroup().userData;
  group->_shapes.remove(this);
}

////////////////////////////////////////////////////////////////////
//     Function : PhysxSphereForceFieldShape::save_to_desc
//       Access : Published
//  Description : Saves the state of the shape object to a 
//                descriptor.
////////////////////////////////////////////////////////////////////
void PhysxSphereForceFieldShape::
save_to_desc(PhysxSphereForceFieldShapeDesc &shapeDesc) const {

  nassertv(_error_type == ET_ok);
  _ptr->saveToDesc(shapeDesc._desc);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxSphereForceFieldShape::set_radius
//       Access: Published
//  Description: Sets the sphere radius. 
////////////////////////////////////////////////////////////////////
void PhysxSphereForceFieldShape::
set_radius(float radius) {

  nassertv(_error_type == ET_ok);
  _ptr->setRadius(radius);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxSphereForceFieldShape::get_radius
//       Access: Published
//  Description: Returns the radius of the sphere.
////////////////////////////////////////////////////////////////////
float PhysxSphereForceFieldShape::
get_radius() const {

  nassertr(_error_type == ET_ok, 0.0f);
  return _ptr->getRadius();
}

