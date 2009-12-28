// Filename: physxConvexMesh.cxx
// Created by:  enn0x (13Oct09)
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

#include "physxConvexMesh.h"
#include "physxMeshPool.h"

TypeHandle PhysxConvexMesh::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: PhysxConvexMesh::link
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void PhysxConvexMesh::
link(NxConvexMesh *meshPtr) {

  // Link self
  PhysxManager::get_global_ptr()->_convex_meshes.add(this);
  _ptr = meshPtr;
  _error_type = ET_ok;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxConvexMesh::unlink
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void PhysxConvexMesh::
unlink() {

  // Unlink self
  _error_type = ET_released;
  PhysxManager::get_global_ptr()->_convex_meshes.remove(this);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxConvexMesh::release
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
void PhysxConvexMesh::
release() {

  nassertv(_error_type == ET_ok);

  unlink();
  NxGetPhysicsSDK()->releaseConvexMesh(*_ptr);
  _ptr = NULL;

  PhysxMeshPool::release_convex_mesh(this);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxConvexMesh::get_reference_count
//       Access: Published
//  Description: Returns the reference count for shared meshes.
////////////////////////////////////////////////////////////////////
unsigned int PhysxConvexMesh::
get_reference_count() const {

  nassertr(_error_type == ET_ok, 0);

  return _ptr->getReferenceCount();
}

