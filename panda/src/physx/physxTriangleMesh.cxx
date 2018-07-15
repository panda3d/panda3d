/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physxTriangleMesh.cxx
 * @author enn0x
 * @date 2009-10-14
 */

#include "physxTriangleMesh.h"
#include "physxMeshPool.h"

TypeHandle PhysxTriangleMesh::_type_handle;

/**
 *
 */
void PhysxTriangleMesh::
link(NxTriangleMesh *meshPtr) {

  // Link self
  PhysxManager::get_global_ptr()->_triangle_meshes.add(this);
  _ptr = meshPtr;
  _error_type = ET_ok;
}

/**
 *
 */
void PhysxTriangleMesh::
unlink() {

  // Unlink self
  _error_type = ET_released;
  PhysxManager::get_global_ptr()->_triangle_meshes.remove(this);
}

/**
 *
 */
void PhysxTriangleMesh::
release() {

  nassertv(_error_type == ET_ok);

  unlink();
  NxGetPhysicsSDK()->releaseTriangleMesh(*_ptr);
  _ptr = nullptr;

  PhysxMeshPool::release_triangle_mesh(this);
}

/**
 * Returns the reference count for shared meshes.
 */
unsigned int PhysxTriangleMesh::
get_reference_count() const {

  nassertr(_error_type == ET_ok, 0);

  return _ptr->getReferenceCount();
}
