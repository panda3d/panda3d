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
//     Function: BulletPlaneShape::get_plane_constant
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE float BulletPlaneShape::
get_plane_constant() const {

  return _shape->getPlaneConstant();
}

////////////////////////////////////////////////////////////////////
//     Function: BulletPlaneShape::get_plane_normal
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE LVector3f BulletPlaneShape::
get_plane_normal() const {

  return btVector3_to_LVector3f(_shape->getPlaneNormal());
}

