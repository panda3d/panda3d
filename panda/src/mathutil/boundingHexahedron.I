// Filename: boundingHexahedron.I
// Created by:  drose (03Oct99)
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

INLINE_MATHUTIL BoundingHexahedron::
BoundingHexahedron() {
}

INLINE_MATHUTIL int BoundingHexahedron::
get_num_points() const {
  return num_points;
}

INLINE_MATHUTIL LPoint3f BoundingHexahedron::
get_point(int n) const {
  nassertr(n >= 0 && n < num_points, LPoint3f(0.0f, 0.0f, 0.0f));
  return _points[n];
}

INLINE_MATHUTIL int BoundingHexahedron::
get_num_planes() const {
  return num_planes;
}

INLINE_MATHUTIL Planef BoundingHexahedron::
get_plane(int n) const {
  nassertr(n >= 0 && n < num_planes, Planef());
  return _planes[n];
}
