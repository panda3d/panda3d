/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physxSoftBodyMesh.cxx
 * @author enn0x
 * @date 2010-09-12
 */

#include "physxSoftBodyMesh.h"
#include "physxMeshPool.h"

TypeHandle PhysxSoftBodyMesh::_type_handle;

/**
 *
 */
void PhysxSoftBodyMesh::
link(NxSoftBodyMesh *meshPtr) {

  // Link self
  PhysxManager::get_global_ptr()->_softbody_meshes.add(this);
  _ptr = meshPtr;
  _error_type = ET_ok;
}

/**
 *
 */
void PhysxSoftBodyMesh::
unlink() {

  // Unlink self
  _error_type = ET_released;
  PhysxManager::get_global_ptr()->_softbody_meshes.remove(this);
}

/**
 *
 */
void PhysxSoftBodyMesh::
release() {

  nassertv(_error_type == ET_ok);

  unlink();
  NxGetPhysicsSDK()->releaseSoftBodyMesh(*_ptr);
  _ptr = nullptr;

  PhysxMeshPool::release_soft_body_mesh(this);
}

/**
 * Returns the reference count for shared meshes.
 */
unsigned int PhysxSoftBodyMesh::
get_reference_count() const {

  nassertr(_error_type == ET_ok, 0);

  return _ptr->getReferenceCount();
}
