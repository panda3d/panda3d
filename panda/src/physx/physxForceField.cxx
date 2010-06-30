// Filename: physxForceField.cxx
// Created by:  enn0x (06Nov09)
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

#include "physxForceField.h"
#include "physxForceFieldDesc.h"
#include "physxForceFieldShapeGroup.h"
#include "physxScene.h"

TypeHandle PhysxForceField::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: PhysxForceField::link
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void PhysxForceField::
link(NxForceField *fieldPtr) {

  // Link self
  _ptr = fieldPtr;
  _ptr->userData = this;
  _error_type = ET_ok;

  set_name(fieldPtr->getName());

  PhysxScene *scene = (PhysxScene *)_ptr->getScene().userData;
  scene->_forcefields.add(this);

  // Link include shape group
  PhysxForceFieldShapeGroup *group = new PhysxForceFieldShapeGroup();
  group->link(&(_ptr->getIncludeShapeGroup()));
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxForceField::unlink
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void PhysxForceField::
unlink() {

  // Unlink inlcude shape group
  PhysxForceFieldShapeGroup *group = (PhysxForceFieldShapeGroup *)(_ptr->getIncludeShapeGroup().userData);
  group->unlink();

  // Unlink self
  _ptr->userData = NULL;
  _error_type = ET_released;

  PhysxScene *scene = (PhysxScene *)_ptr->getScene().userData;
  scene->_forcefields.remove(this);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxForceField::release
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
void PhysxForceField::
release() {

  nassertv(_error_type == ET_ok);

  unlink();
  _ptr->getScene().releaseForceField(*_ptr);
  _ptr = NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxForceField::set_name
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
void PhysxForceField::
set_name(const char *name) {

  nassertv(_error_type == ET_ok);

  _name = name ? name : "";
  _ptr->setName(_name.c_str());
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxForceField::get_name
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
const char *PhysxForceField::
get_name() const {

  nassertr(_error_type == ET_ok, "");
  return _ptr->getName();
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxForceField::get_scene
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
PhysxScene *PhysxForceField::
get_scene() const {

  nassertr(_error_type == ET_ok, NULL);
  return (PhysxScene *)(_ptr->getScene().userData);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxForceField::get_include_shape_group
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
PhysxForceFieldShapeGroup *PhysxForceField::
get_include_shape_group() const {

  nassertr(_error_type == ET_ok, NULL);
  return (PhysxForceFieldShapeGroup *)(_ptr->getIncludeShapeGroup().userData);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxForceField::get_num_shape_groups
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
unsigned int PhysxForceField::
get_num_shape_groups() const {

  nassertr(_error_type == ET_ok, NULL);
  return _ptr->getNbShapeGroups();
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxForceField::get_shape_group
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
PhysxForceFieldShapeGroup *PhysxForceField::
get_shape_group(unsigned int idx) const {

  nassertr(_error_type == ET_ok, NULL);
  nassertr_always(idx < _ptr->getNbShapeGroups(), NULL);

  NxForceFieldShapeGroup *groupPtr;
  NxU32 nGroups = _ptr->getNbShapeGroups();

  _ptr->resetShapeGroupsIterator();
  for (NxU32 i=0; i <= idx; i++) {
    groupPtr = _ptr->getNextShapeGroup();
  }

  return (PhysxForceFieldShapeGroup *)(groupPtr->userData);
}

