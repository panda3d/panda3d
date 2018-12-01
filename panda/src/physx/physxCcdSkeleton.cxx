/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physxCcdSkeleton.cxx
 * @author enn0x
 * @date 2012-05-01
 */

#include "physxCcdSkeleton.h"
#include "physxMeshPool.h"

TypeHandle PhysxCcdSkeleton::_type_handle;

/**
 *
 */
void PhysxCcdSkeleton::
link(NxCCDSkeleton *skeletonPtr) {

  // Link self
  PhysxManager::get_global_ptr()->_ccd_skeletons.add(this);
  _ptr = skeletonPtr;
  _error_type = ET_ok;
}

/**
 *
 */
void PhysxCcdSkeleton::
unlink() {

  // Unlink self
  _error_type = ET_released;
  PhysxManager::get_global_ptr()->_ccd_skeletons.remove(this);
}

/**
 *
 */
void PhysxCcdSkeleton::
release() {

  nassertv(_error_type == ET_ok);

  unlink();
  NxGetPhysicsSDK()->releaseCCDSkeleton(*_ptr);
  _ptr = nullptr;

  // TODO PhysxMeshPool::release_ccd_skeleton(this);
}

/**
 * Returns the reference count for shared meshes.
 */
unsigned int PhysxCcdSkeleton::
get_reference_count() const {

  nassertr(_error_type == ET_ok, 0);

  return _ptr->getReferenceCount();
}
