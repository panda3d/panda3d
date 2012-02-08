// Filename: bulletCapsuleShape.I
// Created by:  enn0x (27Jan10)
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
//     Function: BulletCapsuleShape::Destructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE BulletCapsuleShape::
~BulletCapsuleShape() {

  delete _shape;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletCapsuleShape::Copy Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE BulletCapsuleShape::
BulletCapsuleShape(const BulletCapsuleShape &copy) :
  _shape(copy._shape) {
}

////////////////////////////////////////////////////////////////////
//     Function: BulletCapsuleShape::Copy Assignment Operator
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void BulletCapsuleShape::
operator = (const BulletCapsuleShape &copy) {
  _shape = copy._shape;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletCapsuleShape::get_radius
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE PN_stdfloat BulletCapsuleShape::
get_radius() const {

  return (PN_stdfloat)_shape->getRadius();
}

////////////////////////////////////////////////////////////////////
//     Function: BulletCapsuleShape::get_half_height
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE PN_stdfloat BulletCapsuleShape::
get_half_height() const {

  return (PN_stdfloat)_shape->getHalfHeight();
}

