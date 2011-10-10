// Filename: boundingPlane.I
// Created by:  drose (19Aug05)
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
BoundingPlane(const LPlane &plane) :
  _plane(plane)
{
  _flags = 0;
}

////////////////////////////////////////////////////////////////////
//     Function: BoundingPlane::get_plane
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE_MATHUTIL const LPlane &BoundingPlane::
get_plane() const {
  nassertr(!is_empty(), _plane);
  nassertr(!is_infinite(), _plane);
  return _plane;
}
