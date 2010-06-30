// Filename: physxForceFieldShapeGroup.cxx
// Created by:  enn0x (11Nov09)
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

#include "physxForceFieldShapeGroup.h"
#include "physxForceFieldShapeGroupDesc.h"
#include "physxForceField.h"
#include "physxForceFieldShape.h"

TypeHandle PhysxForceFieldShapeGroup::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: PhysxForceFieldShapeGroup::link
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void PhysxForceFieldShapeGroup::
link(NxForceFieldShapeGroup *groupPtr) {

  // Link self
  _ptr = groupPtr;
  _ptr->userData = this;
  _error_type = ET_ok;

  set_name(groupPtr->getName());

  PhysxScene *scene = (PhysxScene *)_ptr->getScene().userData;
  scene->_ffgroups.add(this);

  // Link shapes
  NxU32 nShapes = _ptr->getNbShapes();
  _ptr->resetShapesIterator();
  for (NxU32 i=0; i < nShapes; i++) {
    NxForceFieldShape *shapePtr = _ptr->getNextShape();
    PhysxForceFieldShape *shape = PhysxForceFieldShape::factory(shapePtr->getType());
    shape->link(shapePtr);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxForceFieldShapeGroup::unlink
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void PhysxForceFieldShapeGroup::
unlink() {

  // Unlink shapes
  NxU32 nShapes = _ptr->getNbShapes();
  _ptr->resetShapesIterator();
  for (NxU32 i=0; i < nShapes; i++) {
    NxForceFieldShape *shapePtr = _ptr->getNextShape();
    PhysxForceFieldShape *shape = (PhysxForceFieldShape *)shapePtr->userData;
    shape->unlink();
  }

  // Unlink self
  _ptr->userData = NULL;
  _error_type = ET_released;

  PhysxScene *scene = (PhysxScene *)_ptr->getScene().userData;
  scene->_ffgroups.remove(this);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxForceFieldShapeGroup::release
//       Access: Published
//  Description: Releases the force field shape.
////////////////////////////////////////////////////////////////////
void PhysxForceFieldShapeGroup::
release() {

  nassertv(_error_type == ET_ok);

  unlink();
  _ptr->getScene().releaseForceFieldShapeGroup(*_ptr);
  _ptr = NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxForceFieldShapeGroup::get_scene
//       Access: Published
//  Description: Returns the scene that owns this force field shape
//               group.
////////////////////////////////////////////////////////////////////
PhysxScene *PhysxForceFieldShapeGroup::
get_scene() const {

  nassertr(_error_type == ET_ok, NULL);
  return (PhysxScene *)(_ptr->getScene().userData);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxForceFieldShapeGroup::get_force_field
//       Access: Published
//  Description: Returns the force field of this group if this is
//               an include group. If not NULL will be returned.
////////////////////////////////////////////////////////////////////
PhysxForceField *PhysxForceFieldShapeGroup::
get_force_field() const {

  nassertr(_error_type == ET_ok, NULL);

  if (_ptr->getForceField() == NULL) {
    return NULL;
  }
  else {
    return (PhysxForceField *)(_ptr->getForceField()->userData);
  }
}

////////////////////////////////////////////////////////////////////
//     Function : PhysxForceFieldShapeGroup::save_to_desc
//       Access : Published
//  Description : Saves the state of the force field shape group
//                object to a  descriptor.
////////////////////////////////////////////////////////////////////
void PhysxForceFieldShapeGroup::
save_to_desc(PhysxForceFieldShapeGroupDesc &groupDesc) const {

  nassertv(_error_type == ET_ok);
  _ptr->saveToDesc(groupDesc._desc);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxForceFieldShapeGroup::set_name
//       Access: Published
//  Description: Sets a name string for the object that can be
//               retrieved with get_name(). 
//               This is for debugging and is not used by the
//               engine.
////////////////////////////////////////////////////////////////////
void PhysxForceFieldShapeGroup::
set_name(const char *name) {

  nassertv(_error_type == ET_ok);

  _name = name ? name : "";
  _ptr->setName(_name.c_str());
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxForceFieldShapeGroup::get_name
//       Access: Published
//  Description: Returns the name string.
////////////////////////////////////////////////////////////////////
const char *PhysxForceFieldShapeGroup::
get_name() const {

  nassertr(_error_type == ET_ok, "");
  return _ptr->getName();
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxForceFieldShapeGroup::get_num_shapes
//       Access: Published
//  Description: Returns the number of shapes assigned to the
//               force field shape group.
////////////////////////////////////////////////////////////////////
unsigned int PhysxForceFieldShapeGroup::
get_num_shapes() const {

  nassertr(_error_type == ET_ok, -1);

  return _ptr->getNbShapes();
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxForceFieldShapeGroup::create_shape
//       Access: Published
//  Description: Creates a force field shape and adds it to the
//               group.
////////////////////////////////////////////////////////////////////
PhysxForceFieldShape *PhysxForceFieldShapeGroup::
create_shape(PhysxForceFieldShapeDesc &desc) {

  nassertr(_error_type == ET_ok, NULL);
  nassertr(desc.is_valid(),NULL);

  PhysxForceFieldShape *shape = PhysxForceFieldShape::factory(desc.ptr()->getType());
  nassertr(shape, NULL);

  NxForceFieldShape *shapePtr = _ptr->createShape(*desc.ptr());
  nassertr(shapePtr, NULL);

  shape->link(shapePtr);

  return shape;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxForceFieldShapeGroup::get_shape
//       Access: Published
//  Description: Returns the i-th shape in the force field group.
////////////////////////////////////////////////////////////////////
PhysxForceFieldShape *PhysxForceFieldShapeGroup::
get_shape(unsigned int idx) const {

  nassertr(_error_type == ET_ok, NULL);
  nassertr_always(idx < _ptr->getNbShapes(), NULL);

  NxForceFieldShape *shapePtr;
  NxU32 nShapes = _ptr->getNbShapes();

  _ptr->resetShapesIterator();
  for (NxU32 i=0; i <= idx; i++) {
    shapePtr = _ptr->getNextShape();
  }

  return (PhysxForceFieldShape *)(shapePtr->userData);
}

