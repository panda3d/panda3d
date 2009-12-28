// Filename: physxHeightField.cxx
// Created by:  enn0x (15Oct09)
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

#include "physxHeightField.h"

TypeHandle PhysxHeightField::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: PhysxHeightField::link
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void PhysxHeightField::
link(NxHeightField *hfPtr) {

  PhysxManager::get_global_ptr()->_heightfields.add(this);
  _ptr = hfPtr;
  _error_type = ET_ok;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxHeightField::unlink
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void PhysxHeightField::
unlink() {

  _error_type = ET_released;
  PhysxManager::get_global_ptr()->_heightfields.remove(this);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxHeightField::release
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void PhysxHeightField::
release() {

  nassertv(_error_type == ET_ok);

  unlink();
  NxGetPhysicsSDK()->releaseHeightField(*_ptr);
  _ptr = NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxHeightField::get_height
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
float PhysxHeightField::
get_height(float x, float y) const {

  nassertr(_error_type == ET_ok, 0.0f);
  return _ptr->getHeight(x, y);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxHeightField::get_reference_count
//       Access: Published
//  Description: Returns the reference count for shared meshes.
////////////////////////////////////////////////////////////////////
unsigned int PhysxHeightField::
get_reference_count() const {

  nassertr(_error_type == ET_ok, 0);
  return _ptr->getReferenceCount();
}

