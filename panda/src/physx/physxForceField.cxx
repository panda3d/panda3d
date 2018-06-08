/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physxForceField.cxx
 * @author enn0x
 * @date 2009-11-06
 */

#include "physxForceField.h"
#include "physxForceFieldDesc.h"
#include "physxForceFieldShapeGroup.h"
#include "physxScene.h"

TypeHandle PhysxForceField::_type_handle;

/**
 *
 */
void PhysxForceField::
link(NxForceField *fieldPtr) {

  // Link self
  _ptr = fieldPtr;
  _ptr->userData = this;
  _error_type = ET_ok;

  set_name(fieldPtr->getName());

  PhysxScene *scene = (PhysxScene *)_ptr->getScene().userData;
  scene->_forcefields.add(this);

  // Link include shape group
  PhysxForceFieldShapeGroup *group = new PhysxForceFieldShapeGroup();
  group->link(&(_ptr->getIncludeShapeGroup()));
}

/**
 *
 */
void PhysxForceField::
unlink() {

  // Unlink inlcude shape group
  PhysxForceFieldShapeGroup *group = (PhysxForceFieldShapeGroup *)(_ptr->getIncludeShapeGroup().userData);
  group->unlink();

  // Unlink self
  _ptr->userData = nullptr;
  _error_type = ET_released;

  PhysxScene *scene = (PhysxScene *)_ptr->getScene().userData;
  scene->_forcefields.remove(this);
}

/**
 *
 */
void PhysxForceField::
release() {

  nassertv(_error_type == ET_ok);

  unlink();
  _ptr->getScene().releaseForceField(*_ptr);
  _ptr = nullptr;
}

/**
 *
 */
void PhysxForceField::
set_name(const char *name) {

  nassertv(_error_type == ET_ok);

  _name = name ? name : "";
  _ptr->setName(_name.c_str());
}

/**
 *
 */
const char *PhysxForceField::
get_name() const {

  nassertr(_error_type == ET_ok, "");
  return _ptr->getName();
}

/**
 *
 */
PhysxScene *PhysxForceField::
get_scene() const {

  nassertr(_error_type == ET_ok, nullptr);
  return (PhysxScene *)(_ptr->getScene().userData);
}

/**
 *
 */
PhysxForceFieldShapeGroup *PhysxForceField::
get_include_shape_group() const {

  nassertr(_error_type == ET_ok, nullptr);
  return (PhysxForceFieldShapeGroup *)(_ptr->getIncludeShapeGroup().userData);
}

/**
 *
 */
unsigned int PhysxForceField::
get_num_shape_groups() const {

  nassertr(_error_type == ET_ok, 0);
  return _ptr->getNbShapeGroups();
}

/**
 *
 */
PhysxForceFieldShapeGroup *PhysxForceField::
get_shape_group(unsigned int idx) const {

  nassertr(_error_type == ET_ok, nullptr);
  nassertr_always(idx < _ptr->getNbShapeGroups(), nullptr);

  NxForceFieldShapeGroup *groupPtr;
  NxU32 nGroups = _ptr->getNbShapeGroups();

  _ptr->resetShapeGroupsIterator();
  for (NxU32 i=0; i <= idx; i++) {
    groupPtr = _ptr->getNextShapeGroup();
  }

  return (PhysxForceFieldShapeGroup *)(groupPtr->userData);
}
