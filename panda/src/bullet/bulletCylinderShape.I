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
//     Function: BulletCylinderShape::get_radius
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE float BulletCylinderShape::
get_radius() const {

  return _shape->getRadius();
}

////////////////////////////////////////////////////////////////////
//     Function: BulletCylinderShape::get_half_extents_without_marging
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE LVecBase3f BulletCylinderShape::
get_half_extents_without_marging() const {

  return btVector3_to_LVecBase3f(_shape->getHalfExtentsWithoutMargin());
}

////////////////////////////////////////////////////////////////////
//     Function: BulletCylinderShape::get_half_extents_with_marging
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE LVecBase3f BulletCylinderShape::
get_half_extents_with_marging() const {

  return btVector3_to_LVecBase3f(_shape->getHalfExtentsWithMargin());
}

