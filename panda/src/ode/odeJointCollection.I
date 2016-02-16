/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file odeJointCollection.I
 * @author drose
 * @date 2008-11-10
 */

////////////////////////////////////////////////////////////////////
//     Function: OdeJointCollection::Destructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE OdeJointCollection::
~OdeJointCollection() {
}

////////////////////////////////////////////////////////////////////
//     Function: OdeJointCollection::operator +=
//       Access: Published
//  Description: Appends the other list onto the end of this one.
////////////////////////////////////////////////////////////////////
INLINE void OdeJointCollection::
operator += (const OdeJointCollection &other) {
  add_joints_from(other);
}

////////////////////////////////////////////////////////////////////
//     Function: OdeJointCollection::operator +
//       Access: Published
//  Description: Returns a OdeJointCollection representing the
//               concatenation of the two lists.
////////////////////////////////////////////////////////////////////
INLINE OdeJointCollection OdeJointCollection::
operator + (const OdeJointCollection &other) const {
  OdeJointCollection a(*this);
  a += other;
  return a;
}
