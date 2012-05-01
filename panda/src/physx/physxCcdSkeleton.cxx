// Filename: physxCcdSkeleton.cxx
// Created by:  enn0x (01May12)
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

#include "physxCcdSkeleton.h"
#include "physxMeshPool.h"

TypeHandle PhysxCcdSkeleton::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: PhysxCcdSkeleton::link
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void PhysxCcdSkeleton::
link(NxCCDSkeleton *skeletonPtr) {

  // Link self
  PhysxManager::get_global_ptr()->_ccd_skeletons.add(this);
  _ptr = skeletonPtr;
  _error_type = ET_ok;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxCcdSkeleton::unlink
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void PhysxCcdSkeleton::
unlink() {

  // Unlink self
  _error_type = ET_released;
  PhysxManager::get_global_ptr()->_ccd_skeletons.remove(this);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxCcdSkeleton::release
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
void PhysxCcdSkeleton::
release() {

  nassertv(_error_type == ET_ok);

  unlink();
  NxGetPhysicsSDK()->releaseCCDSkeleton(*_ptr);
  _ptr = NULL;

  //TODO PhysxMeshPool::release_ccd_skeleton(this);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxCcdSkeleton::get_reference_count
//       Access: Published
//  Description: Returns the reference count for shared meshes.
////////////////////////////////////////////////////////////////////
unsigned int PhysxCcdSkeleton::
get_reference_count() const {

  nassertr(_error_type == ET_ok, 0);

  return _ptr->getReferenceCount();
}

