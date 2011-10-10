// Filename: nurbsBasisVector.I
// Created by:  drose (04Dec02)
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
//     Function: NurbsBasisVector::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE NurbsBasisVector::
NurbsBasisVector() {
  _order = 0;
}

////////////////////////////////////////////////////////////////////
//     Function: NurbsBasisVector::Destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE NurbsBasisVector::
~NurbsBasisVector() {
}

////////////////////////////////////////////////////////////////////
//     Function: NurbsBasisVector::get_order
//       Access: Public
//  Description: Returns the order of the segments in the curve.
////////////////////////////////////////////////////////////////////
INLINE int NurbsBasisVector::
get_order() const {
  return _order;
}

////////////////////////////////////////////////////////////////////
//     Function: NurbsBasisVector::get_num_segments
//       Access: Public
//  Description: Returns the number of piecewise continuous segments
//               in the curve.
////////////////////////////////////////////////////////////////////
INLINE int NurbsBasisVector::
get_num_segments() const {
  return _segments.size();
}

////////////////////////////////////////////////////////////////////
//     Function: NurbsBasisVector::get_start_t
//       Access: Public
//  Description: Returns the first legal value of t on the curve.
//               Usually this is 0.0.
////////////////////////////////////////////////////////////////////
INLINE PN_stdfloat NurbsBasisVector::
get_start_t() const {
  nassertr(!_segments.empty(), 0.0f);
  return _segments.front()._from;
}

////////////////////////////////////////////////////////////////////
//     Function: NurbsBasisVector::get_end_t
//       Access: Public
//  Description: Returns the last legal value of t on the curve.
////////////////////////////////////////////////////////////////////
INLINE PN_stdfloat NurbsBasisVector::
get_end_t() const {
  nassertr(!_segments.empty(), 0.0f);
  return _segments.back()._to;
}

////////////////////////////////////////////////////////////////////
//     Function: NurbsBasisVector::get_vertex_index
//       Access: Public
//  Description: Returns the vertex index of the nth segment.  This is
//               the index number of the first associated control
//               vertex within the source NurbsCurveEvaluator object.
////////////////////////////////////////////////////////////////////
INLINE int NurbsBasisVector::
get_vertex_index(int segment) const {
  nassertr(segment >= 0 && segment < (int)_segments.size(), 0);
  return _segments[segment]._vertex_index;
}

////////////////////////////////////////////////////////////////////
//     Function: NurbsBasisVector::get_from
//       Access: Public
//  Description: Returns the t value of the beginning of this segment.
////////////////////////////////////////////////////////////////////
INLINE PN_stdfloat NurbsBasisVector::
get_from(int segment) const {
  nassertr(segment >= 0 && segment < (int)_segments.size(), 0.0f);
  return _segments[segment]._from;
}

////////////////////////////////////////////////////////////////////
//     Function: NurbsBasisVector::get_to
//       Access: Public
//  Description: Returns the t value of the end of this segment.
////////////////////////////////////////////////////////////////////
INLINE PN_stdfloat NurbsBasisVector::
get_to(int segment) const {
  nassertr(segment >= 0 && segment < (int)_segments.size(), 0.0f);
  return _segments[segment]._to;
}

////////////////////////////////////////////////////////////////////
//     Function: NurbsBasisVector::get_basis
//       Access: Public
//  Description: Returns the basis matrix associated with the nth
//               segment.  This is the pure matrix based on the knot
//               vector over the segment; it does not depend on the
//               control vertices.
////////////////////////////////////////////////////////////////////
INLINE const LMatrix4 &NurbsBasisVector::
get_basis(int segment) const {
  nassertr(segment >= 0 && segment < (int)_segments.size(), LMatrix4::ident_mat());
  return _segments[segment]._basis;
}

////////////////////////////////////////////////////////////////////
//     Function: NurbsBasisVector::scale_t
//       Access: Public
//  Description: Scales the value of t into the range [0, 1]
//               corresponding to [from, to].  Returns the scaled
//               value.
////////////////////////////////////////////////////////////////////
INLINE PN_stdfloat NurbsBasisVector::
scale_t(int segment, PN_stdfloat t) const {
  nassertr(segment >= 0 && segment < (int)_segments.size(), 0.0f);
  PN_stdfloat from = _segments[segment]._from;
  PN_stdfloat to = _segments[segment]._to;
  t = (t - from) / (to - from);
  return min(max(t, (PN_stdfloat)0.0), (PN_stdfloat)1.0);
}
