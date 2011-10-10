// Filename: boundingLine.I
// Created by:  drose (04Jul00)
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
//     Function: BoundingLine::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE_MATHUTIL BoundingLine::
BoundingLine() {
}

////////////////////////////////////////////////////////////////////
//     Function: BoundingLine::Constructor
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE_MATHUTIL BoundingLine::
BoundingLine(const LPoint3 &a, const LPoint3 &b) :
  _origin(a), _vector(b - a)
{
  _vector.normalize();
  _flags = 0;
  nassertd(!_origin.is_nan() && !_vector.is_nan()) {
    _flags = F_empty;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: BoundingLine::get_point_a
//       Access: Published
//  Description: Returns the first point that defines the line.
////////////////////////////////////////////////////////////////////
INLINE_MATHUTIL const LPoint3 &BoundingLine::
get_point_a() const {
  nassertr(!is_empty(), _origin);
  nassertr(!is_infinite(), _origin);
  return _origin;
}

////////////////////////////////////////////////////////////////////
//     Function: BoundingLine::get_point_b
//       Access: Published
//  Description: Returns the second point that defines the line.
////////////////////////////////////////////////////////////////////
INLINE_MATHUTIL LPoint3 BoundingLine::
get_point_b() const {
  nassertr(!is_empty(), _origin);
  nassertr(!is_infinite(), _origin);
  return _origin + _vector;
}
