/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file bulletSphereShape.I
 * @author enn0x
 * @date 2010-01-23
 */

////////////////////////////////////////////////////////////////////
//     Function: BulletSphereShape::Destructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE BulletSphereShape::
~BulletSphereShape() {

  delete _shape;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletSphereShape::Copy Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE BulletSphereShape::
BulletSphereShape(const BulletSphereShape &copy) :
  _shape(copy._shape) {
}

////////////////////////////////////////////////////////////////////
//     Function: BulletSphereShape::Copy Assignment Operator
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void BulletSphereShape::
operator = (const BulletSphereShape &copy) {
  _shape = copy._shape;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletSphereShape::get_radius
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE PN_stdfloat BulletSphereShape::
get_radius() const {

  return _shape->getRadius();
}

