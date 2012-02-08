// Filename: bulletConvexHullShape.I
// Created by:  enn0x (26Jan10)
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
//     Function: BulletConvexHullShape::Destructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE BulletConvexHullShape::
~BulletConvexHullShape() {

  delete _shape;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletConvexHullShape::Copy Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE BulletConvexHullShape::
BulletConvexHullShape(const BulletConvexHullShape &copy) :
  _shape(copy._shape) {
}

////////////////////////////////////////////////////////////////////
//     Function: BulletConvexHullShape::Copy Assignment Operator
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void BulletConvexHullShape::
operator = (const BulletConvexHullShape &copy) {
  _shape = copy._shape;
}

