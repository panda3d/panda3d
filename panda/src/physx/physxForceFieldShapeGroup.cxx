/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physxForceFieldShapeGroup.cxx
 * @author enn0x
 * @date 2009-11-11
 */

#include "physxForceFieldShapeGroup.h"
#include "physxForceFieldShapeGroupDesc.h"
#include "physxForceField.h"
#include "physxForceFieldShape.h"

TypeHandle PhysxForceFieldShapeGroup::_type_handle;

/**
 *
 */
void PhysxForceFieldShapeGroup::
link(NxForceFieldShapeGroup *groupPtr) {

  // Link self
  _ptr = groupPtr;
  _ptr->userData = this;
  _error_type = ET_ok;

  set_name(groupPtr->getName());

  PhysxScene *scene = (PhysxScene *)_ptr->getScene().userData;
  scene->_ffgroups.add(this);

  // Link shapes
  NxU32 nShapes = _ptr->getNbShapes();
  _ptr->resetShapesIterator();
  for (NxU32 i=0; i < nShapes; i++) {
    NxForceFieldShape *shapePtr = _ptr->getNextShape();
    PhysxForceFieldShape *shape = PhysxForceFieldShape::factory(shapePtr->getType());
    shape->link(shapePtr);
  }
}

/**
 *
 */
void PhysxForceFieldShapeGroup::
unlink() {

  // Unlink shapes
  NxU32 nShapes = _ptr->getNbShapes();
  _ptr->resetShapesIterator();
  for (NxU32 i=0; i < nShapes; i++) {
    NxForceFieldShape *shapePtr = _ptr->getNextShape();
    PhysxForceFieldShape *shape = (PhysxForceFieldShape *)shapePtr->userData;
    shape->unlink();
  }

  // Unlink self
  _ptr->userData = nullptr;
  _error_type = ET_released;

  PhysxScene *scene = (PhysxScene *)_ptr->getScene().userData;
  scene->_ffgroups.remove(this);
}

/**
 * Releases the force field shape.
 */
void PhysxForceFieldShapeGroup::
release() {

  nassertv(_error_type == ET_ok);

  unlink();
  _ptr->getScene().releaseForceFieldShapeGroup(*_ptr);
  _ptr = nullptr;
}

/**
 * Returns the scene that owns this force field shape group.
 */
PhysxScene *PhysxForceFieldShapeGroup::
get_scene() const {

  nassertr(_error_type == ET_ok, nullptr);
  return (PhysxScene *)(_ptr->getScene().userData);
}

/**
 * Returns the force field of this group if this is an include group.  If not
 * NULL will be returned.
 */
PhysxForceField *PhysxForceFieldShapeGroup::
get_force_field() const {

  nassertr(_error_type == ET_ok, nullptr);

  if (_ptr->getForceField() == nullptr) {
    return nullptr;
  }
  else {
    return (PhysxForceField *)(_ptr->getForceField()->userData);
  }
}

/**
 * Saves the state of the force field shape group object to a  descriptor.
 */
void PhysxForceFieldShapeGroup::
save_to_desc(PhysxForceFieldShapeGroupDesc &groupDesc) const {

  nassertv(_error_type == ET_ok);
  _ptr->saveToDesc(groupDesc._desc);
}

/**
 * Sets a name string for the object that can be retrieved with get_name().
 * This is for debugging and is not used by the engine.
 */
void PhysxForceFieldShapeGroup::
set_name(const char *name) {

  nassertv(_error_type == ET_ok);

  _name = name ? name : "";
  _ptr->setName(_name.c_str());
}

/**
 * Returns the name string.
 */
const char *PhysxForceFieldShapeGroup::
get_name() const {

  nassertr(_error_type == ET_ok, "");
  return _ptr->getName();
}

/**
 * Returns the number of shapes assigned to the force field shape group.
 */
unsigned int PhysxForceFieldShapeGroup::
get_num_shapes() const {

  nassertr(_error_type == ET_ok, -1);

  return _ptr->getNbShapes();
}

/**
 * Creates a force field shape and adds it to the group.
 */
PhysxForceFieldShape *PhysxForceFieldShapeGroup::
create_shape(PhysxForceFieldShapeDesc &desc) {

  nassertr(_error_type == ET_ok, nullptr);
  nassertr(desc.is_valid(),nullptr);

  PhysxForceFieldShape *shape = PhysxForceFieldShape::factory(desc.ptr()->getType());
  nassertr(shape, nullptr);

  NxForceFieldShape *shapePtr = _ptr->createShape(*desc.ptr());
  nassertr(shapePtr, nullptr);

  shape->link(shapePtr);

  return shape;
}

/**
 * Returns the i-th shape in the force field group.
 */
PhysxForceFieldShape *PhysxForceFieldShapeGroup::
get_shape(unsigned int idx) const {

  nassertr(_error_type == ET_ok, nullptr);
  nassertr_always(idx < _ptr->getNbShapes(), nullptr);

  NxForceFieldShape *shapePtr;
  NxU32 nShapes = _ptr->getNbShapes();

  _ptr->resetShapesIterator();
  for (NxU32 i=0; i <= idx; i++) {
    shapePtr = _ptr->getNextShape();
  }

  return (PhysxForceFieldShape *)(shapePtr->userData);
}
