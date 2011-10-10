// Filename: boundingHexahedron.I
// Created by:  drose (03Oct99)
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
//     Function: BoundingHexahedron::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE_MATHUTIL BoundingHexahedron::
BoundingHexahedron() {
}

////////////////////////////////////////////////////////////////////
//     Function: BoundingHexahedron::get_num_points
//       Access: Published
//  Description: Returns 8: the number of vertices of a hexahedron.
////////////////////////////////////////////////////////////////////
INLINE_MATHUTIL int BoundingHexahedron::
get_num_points() const {
  return num_points;
}

////////////////////////////////////////////////////////////////////
//     Function: BoundingHexahedron::get_point
//       Access: Published
//  Description: Returns the nth vertex of the hexahedron.
////////////////////////////////////////////////////////////////////
INLINE_MATHUTIL LPoint3 BoundingHexahedron::
get_point(int n) const {
  nassertr(n >= 0 && n < num_points, LPoint3::zero());
  return _points[n];
}

////////////////////////////////////////////////////////////////////
//     Function: BoundingHexahedron::get_num_planes
//       Access: Published
//  Description: Returns 6: the number of faces of a hexahedron.
////////////////////////////////////////////////////////////////////
INLINE_MATHUTIL int BoundingHexahedron::
get_num_planes() const {
  return num_planes;
}

////////////////////////////////////////////////////////////////////
//     Function: BoundingHexahedron::get_plane
//       Access: Published
//  Description: Returns the nth face of the hexahedron.
////////////////////////////////////////////////////////////////////
INLINE_MATHUTIL LPlane BoundingHexahedron::
get_plane(int n) const {
  nassertr(n >= 0 && n < num_planes, LPlane());
  return _planes[n];
}
