// Filename: physicsObjectCollection.I
// Created by:  joswilso (12Jul06)
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
//     Function: PhysicsObjectCollection::Destructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE PhysicsObjectCollection::
~PhysicsObjectCollection() {
}

////////////////////////////////////////////////////////////////////
//     Function: PhysicsObjectCollection::operator +=
//       Access: Published
//  Description: Appends the other list onto the end of this one.
////////////////////////////////////////////////////////////////////
INLINE void PhysicsObjectCollection::
operator += (const PhysicsObjectCollection &other) {
  add_physics_objects_from(other);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysicsObjectCollection::operator +
//       Access: Published
//  Description: Returns a PhysicsObjectCollection representing the
//               concatenation of the two lists.
////////////////////////////////////////////////////////////////////
INLINE PhysicsObjectCollection PhysicsObjectCollection::
operator + (const PhysicsObjectCollection &other) const {
  PhysicsObjectCollection a(*this);
  a += other;
  return a;
}
