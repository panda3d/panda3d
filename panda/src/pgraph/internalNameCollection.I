// Filename: internalNameCollection.I
// Created by:  drose (16Mar02)
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
//     Function: InternalNameCollection::Destructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE InternalNameCollection::
~InternalNameCollection() {
}

////////////////////////////////////////////////////////////////////
//     Function: InternalNameCollection::operator +=
//       Access: Published
//  Description: Appends the other list onto the end of this one.
////////////////////////////////////////////////////////////////////
INLINE void InternalNameCollection::
operator += (const InternalNameCollection &other) {
  add_names_from(other);
}

////////////////////////////////////////////////////////////////////
//     Function: InternalNameCollection::operator +
//       Access: Published
//  Description: Returns a InternalNameCollection representing the
//               concatenation of the two lists.
////////////////////////////////////////////////////////////////////
INLINE InternalNameCollection InternalNameCollection::
operator + (const InternalNameCollection &other) const {
  InternalNameCollection a(*this);
  a += other;
  return a;
}
