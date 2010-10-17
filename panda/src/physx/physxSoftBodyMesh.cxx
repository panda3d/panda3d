// Filename: physxSoftBodyMesh.cxx
// Created by:  enn0x (12Sep10)
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

#include "physxSoftBodyMesh.h"
#include "physxMeshPool.h"

TypeHandle PhysxSoftBodyMesh::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: PhysxSoftBodyMesh::link
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void PhysxSoftBodyMesh::
link(NxSoftBodyMesh *meshPtr) {

  // Link self
  PhysxManager::get_global_ptr()->_softbody_meshes.add(this);
  _ptr = meshPtr;
  _error_type = ET_ok;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxSoftBodyMesh::unlink
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void PhysxSoftBodyMesh::
unlink() {

  // Unlink self
  _error_type = ET_released;
  PhysxManager::get_global_ptr()->_softbody_meshes.remove(this);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxSoftBodyMesh::release
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
void PhysxSoftBodyMesh::
release() {

  nassertv(_error_type == ET_ok);

  unlink();
  NxGetPhysicsSDK()->releaseSoftBodyMesh(*_ptr);
  _ptr = NULL;

  PhysxMeshPool::release_soft_body_mesh(this);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxSoftBodyMesh::get_reference_count
//       Access: Published
//  Description: Returns the reference count for shared meshes.
////////////////////////////////////////////////////////////////////
unsigned int PhysxSoftBodyMesh::
get_reference_count() const {

  nassertr(_error_type == ET_ok, 0);

  return _ptr->getReferenceCount();
}

