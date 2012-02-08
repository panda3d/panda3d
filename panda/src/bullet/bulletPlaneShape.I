// Filename: bulletPlaneShape.I
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

////////////////////////////////////////////////////////////////////
//     Function: BulletPlaneShape::Destructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE BulletPlaneShape::
~BulletPlaneShape() {

  delete _shape;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletPlaneShape::Copy Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE BulletPlaneShape::
BulletPlaneShape(const BulletPlaneShape &copy) :
  _shape(copy._shape) {
}

////////////////////////////////////////////////////////////////////
//     Function: BulletPlaneShape::Copy Assignment Operator
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void BulletPlaneShape::
operator = (const BulletPlaneShape &copy) {
  _shape = copy._shape;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletPlaneShape::get_plane_constant
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE PN_stdfloat BulletPlaneShape::
get_plane_constant() const {

  return (PN_stdfloat)_shape->getPlaneConstant();
}

////////////////////////////////////////////////////////////////////
//     Function: BulletPlaneShape::get_plane_normal
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE LVector3 BulletPlaneShape::
get_plane_normal() const {

  return btVector3_to_LVector3(_shape->getPlaneNormal());
}

