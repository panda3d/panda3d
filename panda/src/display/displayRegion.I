// Filename: displayRegion.I
// Created by:  frang (07Mar99)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////
//     Function: DisplayRegion::operator <
//       Access: Public
//  Description: Returns true if this DisplayRegion should be sorted
//               before the other one, false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool DisplayRegion::
operator < (const DisplayRegion &other) const {
  return _sort < other._sort;
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayRegion::is_active
//       Access: Published
//  Description: Returns the active flag associated with the
//               DisplayRegion.
////////////////////////////////////////////////////////////////////
INLINE bool DisplayRegion::
is_active() const {
  return _active;
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayRegion::get_sort
//       Access: Published
//  Description: Returns the sort value associated with the
//               DisplayRegion.
////////////////////////////////////////////////////////////////////
INLINE int DisplayRegion::
get_sort() const {
  return _sort;
}


INLINE ostream &operator << (ostream &out, const DisplayRegion &dr) {
  dr.output(out);
  return out;
}
