/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physxHeightField.cxx
 * @author enn0x
 * @date 2009-10-15
 */

#include "physxHeightField.h"

TypeHandle PhysxHeightField::_type_handle;

/**
 *
 */
void PhysxHeightField::
link(NxHeightField *hfPtr) {

  PhysxManager::get_global_ptr()->_heightfields.add(this);
  _ptr = hfPtr;
  _error_type = ET_ok;
}

/**
 *
 */
void PhysxHeightField::
unlink() {

  _error_type = ET_released;
  PhysxManager::get_global_ptr()->_heightfields.remove(this);
}

/**
 *
 */
void PhysxHeightField::
release() {

  nassertv(_error_type == ET_ok);

  unlink();
  NxGetPhysicsSDK()->releaseHeightField(*_ptr);
  _ptr = nullptr;
}

/**
 *
 */
float PhysxHeightField::
get_height(float x, float y) const {

  nassertr(_error_type == ET_ok, 0.0f);
  return _ptr->getHeight(x, y);
}

/**
 * Returns the reference count for shared meshes.
 */
unsigned int PhysxHeightField::
get_reference_count() const {

  nassertr(_error_type == ET_ok, 0);
  return _ptr->getReferenceCount();
}
