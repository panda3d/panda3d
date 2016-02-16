/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file bulletConvexPointCloudShape.I
 * @author enn0x
 * @date 2010-01-30
 */

////////////////////////////////////////////////////////////////////
//     Function: BulletConvexPointCloudShape::Destructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE BulletConvexPointCloudShape::
~BulletConvexPointCloudShape() {

  delete _shape;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletConvexPointCloudShape::Copy Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE BulletConvexPointCloudShape::
BulletConvexPointCloudShape(const BulletConvexPointCloudShape &copy) :
  _shape(copy._shape) {
}

////////////////////////////////////////////////////////////////////
//     Function: BulletConvexPointCloudShape::Copy Assignment Operator
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void BulletConvexPointCloudShape::
operator = (const BulletConvexPointCloudShape &copy) {
  _shape = copy._shape;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletConvexPointCloudShape::get_num_points
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE int BulletConvexPointCloudShape::
get_num_points() const {

  return _shape->getNumPoints();
}

