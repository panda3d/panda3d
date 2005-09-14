// Filename: lpoint3_src.I
// Created by:  drose (25Sep99)
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
//     Function: LPoint3::Default Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LPoint3)::
FLOATNAME(LPoint3)() {
}

////////////////////////////////////////////////////////////////////
//     Function: LPoint3::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LPoint3)::
FLOATNAME(LPoint3)(const FLOATNAME(LVecBase3) &copy) : FLOATNAME(LVecBase3)(copy) {
}

////////////////////////////////////////////////////////////////////
//     Function: LPoint3::Copy Assignment Operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LPoint3) &FLOATNAME(LPoint3)::
operator = (const FLOATNAME(LVecBase3) &copy) {
  FLOATNAME(LVecBase3)::operator = (copy);
  return *this;
}

////////////////////////////////////////////////////////////////////
//     Function: LPoint3::Copy Fill Operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LPoint3) &FLOATNAME(LPoint3)::
operator = (FLOATTYPE fill_value) {
  FLOATNAME(LVecBase3)::operator = (fill_value);
  return *this;
}

////////////////////////////////////////////////////////////////////
//     Function: LPoint3::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LPoint3)::
FLOATNAME(LPoint3)(FLOATTYPE fill_value) :
  FLOATNAME(LVecBase3)(fill_value)
{
}

////////////////////////////////////////////////////////////////////
//     Function: LPoint3::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LPoint3)::
FLOATNAME(LPoint3)(FLOATTYPE x, FLOATTYPE y, FLOATTYPE z) :
  FLOATNAME(LVecBase3)(x, y, z)
{
}

////////////////////////////////////////////////////////////////////
//     Function: LPoint3::zero Named Constructor
//       Access: Public
//  Description: Returns a zero-length point.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH const FLOATNAME(LPoint3) &FLOATNAME(LPoint3)::
zero() {
  return (const FLOATNAME(LPoint3) &)FLOATNAME(LVecBase3)::zero();
}

////////////////////////////////////////////////////////////////////
//     Function: LPoint3::unit_x Named Constructor
//       Access: Public
//  Description: Returns a unit X point.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH const FLOATNAME(LPoint3) &FLOATNAME(LPoint3)::
unit_x() {
  return (const FLOATNAME(LPoint3) &)FLOATNAME(LVecBase3)::unit_x();
}

////////////////////////////////////////////////////////////////////
//     Function: LPoint3::unit_y Named Constructor
//       Access: Public
//  Description: Returns a unit Y point.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH const FLOATNAME(LPoint3) &FLOATNAME(LPoint3)::
unit_y() {
  return (const FLOATNAME(LPoint3) &)FLOATNAME(LVecBase3)::unit_y();
}

////////////////////////////////////////////////////////////////////
//     Function: LPoint3::unit_z Named Constructor
//       Access: Public
//  Description: Returns a unit Z point.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH const FLOATNAME(LPoint3) &FLOATNAME(LPoint3)::
unit_z() {
  return (const FLOATNAME(LPoint3) &)FLOATNAME(LVecBase3)::unit_z();
}

////////////////////////////////////////////////////////////////////
//     Function: LPoint3::unary -
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LPoint3) FLOATNAME(LPoint3)::
operator - () const {
  return FLOATNAME(LVecBase3)::operator - ();
}

////////////////////////////////////////////////////////////////////
//     Function: LPoint3::point + vecbase
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LVecBase3) FLOATNAME(LPoint3)::
operator + (const FLOATNAME(LVecBase3) &other) const {
  return FLOATNAME(LVecBase3)::operator + (other);
}

////////////////////////////////////////////////////////////////////
//     Function: LPoint3::point + vector
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LPoint3) FLOATNAME(LPoint3)::
operator + (const FLOATNAME(LVector3) &other) const {
  return FLOATNAME(LVecBase3)::operator + (other);
}

////////////////////////////////////////////////////////////////////
//     Function: LPoint3::point - vecbase
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LVecBase3) FLOATNAME(LPoint3)::
operator - (const FLOATNAME(LVecBase3) &other) const {
  return FLOATNAME(LVecBase3)::operator - (other);
}

////////////////////////////////////////////////////////////////////
//     Function: LPoint3::point - point
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LVector3) FLOATNAME(LPoint3)::
operator - (const FLOATNAME(LPoint3) &other) const {
  return FLOATNAME(LVecBase3)::operator - (other);
}

////////////////////////////////////////////////////////////////////
//     Function: LPoint3::point - vector
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LPoint3) FLOATNAME(LPoint3)::
operator - (const FLOATNAME(LVector3) &other) const {
  return FLOATNAME(LVecBase3)::operator - (other);
}

////////////////////////////////////////////////////////////////////
//     Function: LPoint3::cross
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LPoint3) FLOATNAME(LPoint3)::
cross(const FLOATNAME(LVecBase3) &other) const {
  return FLOATNAME(LVecBase3)::cross(other);
}

////////////////////////////////////////////////////////////////////
//     Function: LPoint3::operator * scalar
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LPoint3) FLOATNAME(LPoint3)::
operator * (FLOATTYPE scalar) const {
  return FLOATNAME(LPoint3)(FLOATNAME(LVecBase3)::operator * (scalar));
}

////////////////////////////////////////////////////////////////////
//     Function: LPoint3::operator / scalar
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LPoint3) FLOATNAME(LPoint3)::
operator / (FLOATTYPE scalar) const {
  return FLOATNAME(LPoint3)(FLOATNAME(LVecBase3)::operator / (scalar));
}

////////////////////////////////////////////////////////////////////
//     Function: LPoint3::origin
//       Access: Public, Static
//  Description: Returns the origin of the indicated coordinate
//               system.  This is always 0, 0, 0 with all of our
//               existing coordinate systems; it's hard to imagine it
//               ever being different.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LPoint3) FLOATNAME(LPoint3)::
origin(CoordinateSystem) {
  return FLOATNAME(LPoint3)(0.0f, 0.0f, 0.0f);
}

////////////////////////////////////////////////////////////////////
//     Function: LPoint3::rfu
//       Access: Public, Static
//  Description: Returns a point described by right, forward, up
//               displacements from the origin, wherever that maps to
//               in the given coordinate system.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LPoint3) FLOATNAME(LPoint3)::
rfu(FLOATTYPE right_v, FLOATTYPE fwd_v, FLOATTYPE up_v,
    CoordinateSystem cs) {
  return origin(cs) +
    FLOATNAME(LVector3)::rfu(right_v, fwd_v, up_v, cs);
}

#ifdef HAVE_PYTHON
////////////////////////////////////////////////////////////////////
//     Function: LPoint3::python_repr
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH void FLOATNAME(LPoint3)::
python_repr(ostream &out, const string &class_name) const {
  FLOATNAME(LVecBase3)::python_repr(out, class_name);
}
#endif  // HAVE_PYTHON
