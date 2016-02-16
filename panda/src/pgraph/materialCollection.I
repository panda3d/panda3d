/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file materialCollection.I
 * @author drose
 * @date 2002-03-16
 */

////////////////////////////////////////////////////////////////////
//     Function: MaterialCollection::Destructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE MaterialCollection::
~MaterialCollection() {
}

////////////////////////////////////////////////////////////////////
//     Function: MaterialCollection::operator +=
//       Access: Published
//  Description: Appends the other list onto the end of this one.
////////////////////////////////////////////////////////////////////
INLINE void MaterialCollection::
operator += (const MaterialCollection &other) {
  add_materials_from(other);
}

////////////////////////////////////////////////////////////////////
//     Function: MaterialCollection::operator +
//       Access: Published
//  Description: Returns a MaterialCollection representing the
//               concatenation of the two lists.
////////////////////////////////////////////////////////////////////
INLINE MaterialCollection MaterialCollection::
operator + (const MaterialCollection &other) const {
  MaterialCollection a(*this);
  a += other;
  return a;
}
