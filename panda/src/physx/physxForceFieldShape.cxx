// Filename: physxForceFieldShape.cxx
// Created by:  enn0x (15Nov09)
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

#include "physxForceFieldShape.h"
#include "physxForceFieldShapeGroup.h"
#include "physxForceField.h"
#include "physxBoxForceFieldShape.h"
#include "physxCapsuleForceFieldShape.h"
#include "physxSphereForceFieldShape.h"
#include "physxConvexForceFieldShape.h"
#include "physxManager.h"

TypeHandle PhysxForceFieldShape::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: PhysxForceFieldShape::release
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
void PhysxForceFieldShape::
release() {

  nassertv(_error_type == ET_ok);

  unlink();
  ptr()->getShapeGroup().releaseShape(*ptr());
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxForceFieldShape::factory
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
PhysxForceFieldShape *PhysxForceFieldShape::
factory(NxShapeType shapeType) {

  switch (shapeType) {

  case NX_SHAPE_SPHERE:
    return new PhysxSphereForceFieldShape();

  case NX_SHAPE_BOX:
    return new PhysxBoxForceFieldShape();

  case NX_SHAPE_CAPSULE:
    return new PhysxCapsuleForceFieldShape();

  case NX_SHAPE_CONVEX:
    return new PhysxConvexForceFieldShape();
  }

  physx_cat.error() << "Unknown shape type.\n";
  return NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxForceFieldShape::get_force_field
//       Access: Published
//  Description: Returns the owning force field if this is a shape
//               of an include group, else NULL will be returned.
////////////////////////////////////////////////////////////////////
PhysxForceField *PhysxForceFieldShape::
get_force_field() const {

  nassertr(_error_type == ET_ok, NULL);

  NxForceField *fieldPtr = ptr()->getForceField();
  if (fieldPtr == NULL) {
    return NULL;
  }
  return (PhysxForceField *)(fieldPtr->userData);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxForceFieldShape::get_shape_group
//       Access: Published
//  Description: Returns the owning force field shape group.
////////////////////////////////////////////////////////////////////
PhysxForceFieldShapeGroup *PhysxForceFieldShape::
get_shape_group() const {

  nassertr(_error_type == ET_ok, NULL);
  return (PhysxForceFieldShapeGroup *)(ptr()->getShapeGroup().userData);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxForceFieldShape::set_name
//       Access: Published
//  Description: Sets a name string for this object. The name can
//               be retrieved again with get_name().
////////////////////////////////////////////////////////////////////
void PhysxForceFieldShape::
set_name(const char *name) {

  nassertv(_error_type == ET_ok);

  _name = name ? name : "";
  ptr()->setName(_name.c_str());
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxForceFieldShape::get_name
//       Access: Published
//  Description: Returns the name string. 
////////////////////////////////////////////////////////////////////
const char *PhysxForceFieldShape::
get_name() const {

  nassertr(_error_type == ET_ok, "");
  return ptr()->getName();
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxForceFieldShape::set_mat
//       Access: Published
//  Description: Sets the force field shape's transform.
////////////////////////////////////////////////////////////////////
void PhysxForceFieldShape::
set_mat(const LMatrix4f &mat) {

  nassertv(_error_type == ET_ok);

  ptr()->setPose(PhysxManager::mat4_to_nxMat34(mat));
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxForceFieldShape::get_mat
//       Access: Published
//  Description: Returns the force field shape's transform. 
////////////////////////////////////////////////////////////////////
LMatrix4f PhysxForceFieldShape::
get_mat() const {

  nassertr(_error_type == ET_ok, LMatrix4f::zeros_mat());

  return PhysxManager::nxMat34_to_mat4(ptr()->getPose());
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxForceFieldShape::set_pos
//       Access: Published
//  Description: Sets the force field shape's translation.
////////////////////////////////////////////////////////////////////
void PhysxForceFieldShape::
set_pos(const LPoint3f &pos) {

  nassertv(_error_type == ET_ok);

  NxMat34 pose = ptr()->getPose();
  pose.t = PhysxManager::point3_to_nxVec3(pos);
  ptr()->setPose(pose);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxForceFieldShape::get_pos
//       Access: Published
//  Description: Returns the force field shape's translation.
////////////////////////////////////////////////////////////////////
LPoint3f PhysxForceFieldShape::
get_pos() const {

  nassertr(_error_type == ET_ok, LPoint3f::zero());

  return PhysxManager::nxVec3_to_point3(ptr()->getPose().t);
}

