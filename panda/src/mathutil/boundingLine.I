// Filename: boundingLine.I
// Created by:  drose (04Jul00)
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

INLINE_MATHUTIL BoundingLine::
BoundingLine() {
}

INLINE_MATHUTIL BoundingLine::
BoundingLine(const LPoint3f &a, const LPoint3f &b) :
  _origin(a), _vector(b - a)
{
  _vector.normalize();
  _flags = 0;
  nassertd(!_origin.is_nan() && !_vector.is_nan()) {
    _flags = F_empty;
  }
}

INLINE_MATHUTIL const LPoint3f &BoundingLine::
get_point_a() const {
  nassertr(!is_empty(), _origin);
  nassertr(!is_infinite(), _origin);
  return _origin;
}

INLINE_MATHUTIL LPoint3f BoundingLine::
get_point_b() const {
  nassertr(!is_empty(), _origin);
  nassertr(!is_infinite(), _origin);
  return _origin + _vector;
}
