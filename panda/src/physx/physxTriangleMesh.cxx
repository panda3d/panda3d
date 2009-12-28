// Filename: physxTriangleMesh.cxx
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

#include "physxTriangleMesh.h"
#include "physxMeshPool.h"

TypeHandle PhysxTriangleMesh::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: PhysxTriangleMesh::link
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void PhysxTriangleMesh::
link(NxTriangleMesh *meshPtr) {

  // Link self
  PhysxManager::get_global_ptr()->_triangle_meshes.add(this);
  _ptr = meshPtr;
  _error_type = ET_ok;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxTriangleMesh::unlink
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void PhysxTriangleMesh::
unlink() {

  // Unlink self
  _error_type = ET_released;
  PhysxManager::get_global_ptr()->_triangle_meshes.remove(this);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxTriangleMesh::release
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
void PhysxTriangleMesh::
release() {

  nassertv(_error_type == ET_ok);

  unlink();
  NxGetPhysicsSDK()->releaseTriangleMesh(*_ptr);
  _ptr = NULL;

  PhysxMeshPool::release_triangle_mesh(this);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxTriangleMesh::get_reference_count
//       Access: Published
//  Description: Returns the reference count for shared meshes.
////////////////////////////////////////////////////////////////////
unsigned int PhysxTriangleMesh::
get_reference_count() const {

  nassertr(_error_type == ET_ok, 0);

  return _ptr->getReferenceCount();
}

