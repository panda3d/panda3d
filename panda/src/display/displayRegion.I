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

////////////////////////////////////////////////////////////////////
//     Function: DisplayRegion::set_cube_map_index
//       Access: Published
//  Description: This is a special parameter that is only used when
//               rendering the faces of a cube map.  Normally you
//               should not need to set it directly.  This sets up the
//               DisplayRegion to render to the nth cube map face; the
//               value must be between 0 and 5, inclusive.  A normal
//               DisplayRegion that is not associated with any
//               particular cube map should be set to -1.
////////////////////////////////////////////////////////////////////
INLINE void DisplayRegion::
set_cube_map_index(int cube_map_index) {
  _cube_map_index = cube_map_index;
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayRegion::get_cube_map_index
//       Access: Published
//  Description: Returns the cube map face index associated with this
//               particular DisplayRegion, or -1 if it is not
//               associated with a cube map.  See
//               set_cube_map_index().
////////////////////////////////////////////////////////////////////
INLINE int DisplayRegion::
get_cube_map_index() const {
  return _cube_map_index;
}

INLINE ostream &operator << (ostream &out, const DisplayRegion &dr) {
  dr.output(out);
  return out;
}
