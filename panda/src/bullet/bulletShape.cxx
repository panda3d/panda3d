// Filename: bulletShape.cxx
// Created by:  enn0x (23Jan10)
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

#include "bulletShape.h"
#include "bullet_utils.h"

TypeHandle BulletShape::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: BulletShape::get_name
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
const char *BulletShape::
get_name() const {

  return ptr()->getName();
}

////////////////////////////////////////////////////////////////////
//     Function: BulletShape::get_margin
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
PN_stdfloat BulletShape::
get_margin() const {

  return ptr()->getMargin();
}

////////////////////////////////////////////////////////////////////
//     Function: BulletShape::set_margin
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void BulletShape::
set_margin(PN_stdfloat margin) {

  ptr()->setMargin(margin);
}

////////////////////////////////////////////////////////////////////
//     Function: BulletShape::get_local_scale
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
LVecBase3 BulletShape::
get_local_scale() const {

  return btVector3_to_LVecBase3(ptr()->getLocalScaling());
}

////////////////////////////////////////////////////////////////////
//     Function: BulletShape::set_local_scale
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void BulletShape::
set_local_scale(const LVecBase3 &scale) {

  nassertv(!scale.is_nan());
  ptr()->setLocalScaling(LVecBase3_to_btVector3(scale));
}

////////////////////////////////////////////////////////////////////
//     Function: BulletShape::get_shape_bounds
//       Access: Published
//  Description: Returns the current bounds of this collision shape.
////////////////////////////////////////////////////////////////////
BoundingSphere BulletShape::
get_shape_bounds() const {

/*
  btTransform tr;
  tr.setIdentity();
  btVector3 aabbMin,aabbMax;
  ptr()->getAabb(tr,aabbMin,aabbMax);
  btVector3 o = tr.getOrigin();
cout << "aabbMin " << aabbMin.x() << " " << aabbMin.y() << " " << aabbMin.z() << endl;
cout << "aabbMax " << aabbMax.x() << " " << aabbMax.y() << " " << aabbMax.z() << endl;
cout << "origin " << aabbMin.x() << " " << aabbMin.y() << " " << aabbMin.z() << endl;
*/

  btVector3 center;
  btScalar radius;

  ptr()->getBoundingSphere(center, radius);
  BoundingSphere bounds(btVector3_to_LPoint3(center), (PN_stdfloat)radius);

  return bounds;
}

