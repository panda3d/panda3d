// Filename: bulletCylinderShape.I
// Created by:  enn0x (17Feb10)
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

////////////////////////////////////////////////////////////////////
//     Function: BulletCylinderShape::Destructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE BulletCylinderShape::
~BulletCylinderShape() {

  delete _shape;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletCylinderShape::Copy Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE BulletCylinderShape::
BulletCylinderShape(const BulletCylinderShape &copy) :
  _shape(copy._shape) {
}

////////////////////////////////////////////////////////////////////
//     Function: BulletCylinderShape::Copy Assignment Operator
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void BulletCylinderShape::
operator = (const BulletCylinderShape &copy) {
  _shape = copy._shape;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletCylinderShape::get_radius
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE PN_stdfloat BulletCylinderShape::
get_radius() const {

  return (PN_stdfloat)_shape->getRadius();
}

////////////////////////////////////////////////////////////////////
//     Function: BulletCylinderShape::get_half_extents_without_margin
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE LVecBase3 BulletCylinderShape::
get_half_extents_without_margin() const {

  return btVector3_to_LVecBase3(_shape->getHalfExtentsWithoutMargin());
}

////////////////////////////////////////////////////////////////////
//     Function: BulletCylinderShape::get_half_extents_with_margin
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE LVecBase3 BulletCylinderShape::
get_half_extents_with_margin() const {

  return btVector3_to_LVecBase3(_shape->getHalfExtentsWithMargin());
}

