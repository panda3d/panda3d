/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physxClothMesh.cxx
 * @author enn0x
 * @date 2010-03-28
 */

#include "physxClothMesh.h"
#include "physxMeshPool.h"

TypeHandle PhysxClothMesh::_type_handle;

/**
 *
 */
void PhysxClothMesh::
link(NxClothMesh *meshPtr) {

  // Link self
  PhysxManager::get_global_ptr()->_cloth_meshes.add(this);
  _ptr = meshPtr;
  _error_type = ET_ok;
}

/**
 *
 */
void PhysxClothMesh::
unlink() {

  // Unlink self
  _error_type = ET_released;
  PhysxManager::get_global_ptr()->_cloth_meshes.remove(this);
}

/**
 *
 */
void PhysxClothMesh::
release() {

  nassertv(_error_type == ET_ok);

  unlink();
  NxGetPhysicsSDK()->releaseClothMesh(*_ptr);
  _ptr = nullptr;

  PhysxMeshPool::release_cloth_mesh(this);
}

/**
 * Returns the reference count for shared meshes.
 */
unsigned int PhysxClothMesh::
get_reference_count() const {

  nassertr(_error_type == ET_ok, 0);

  return _ptr->getReferenceCount();
}
