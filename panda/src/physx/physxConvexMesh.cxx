/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physxConvexMesh.cxx
 * @author enn0x
 * @date 2009-10-13
 */

#include "physxConvexMesh.h"
#include "physxMeshPool.h"

TypeHandle PhysxConvexMesh::_type_handle;

/**
 *
 */
void PhysxConvexMesh::
link(NxConvexMesh *meshPtr) {

  // Link self
  PhysxManager::get_global_ptr()->_convex_meshes.add(this);
  _ptr = meshPtr;
  _error_type = ET_ok;
}

/**
 *
 */
void PhysxConvexMesh::
unlink() {

  // Unlink self
  _error_type = ET_released;
  PhysxManager::get_global_ptr()->_convex_meshes.remove(this);
}

/**
 *
 */
void PhysxConvexMesh::
release() {

  nassertv(_error_type == ET_ok);

  unlink();
  NxGetPhysicsSDK()->releaseConvexMesh(*_ptr);
  _ptr = nullptr;

  PhysxMeshPool::release_convex_mesh(this);
}

/**
 * Returns the reference count for shared meshes.
 */
unsigned int PhysxConvexMesh::
get_reference_count() const {

  nassertr(_error_type == ET_ok, 0);

  return _ptr->getReferenceCount();
}
