// Filename: boundingPlane.I
// Created by:  drose (19Aug05)
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
//     Function: BoundingPlane::Default Constructor
//       Access: Published
//  Description: Constructs an empty "plane" that has no
//               intersections.
////////////////////////////////////////////////////////////////////
INLINE_MATHUTIL BoundingPlane::
BoundingPlane() {
}

////////////////////////////////////////////////////////////////////
//     Function: BoundingPlane::Constructor
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE_MATHUTIL BoundingPlane::
BoundingPlane(const Planef &plane) :
  _plane(plane)
{
  _flags = 0;
}

////////////////////////////////////////////////////////////////////
//     Function: BoundingPlane::get_plane
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE_MATHUTIL const Planef &BoundingPlane::
get_plane() const {
  nassertr(!is_empty(), _plane);
  nassertr(!is_infinite(), _plane);
  return _plane;
}
