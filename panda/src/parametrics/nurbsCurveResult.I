// Filename: nurbsCurveResult.I
// Created by:  drose (04Dec02)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////
//     Function: NurbsCurveResult::Destructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE NurbsCurveResult::
~NurbsCurveResult() {
}

////////////////////////////////////////////////////////////////////
//     Function: NurbsCurveResult::get_start_t
//       Access: Public
//  Description: Returns the first legal value of t on the curve.
//               Usually this is 0.0.
////////////////////////////////////////////////////////////////////
INLINE float NurbsCurveResult::
get_start_t() const {
  return _basis.get_start_t();
}

////////////////////////////////////////////////////////////////////
//     Function: NurbsCurveResult::get_end_t
//       Access: Public
//  Description: Returns the last legal value of t on the curve.
////////////////////////////////////////////////////////////////////
INLINE float NurbsCurveResult::
get_end_t() const {
  return _basis.get_end_t();
}

////////////////////////////////////////////////////////////////////
//     Function: NurbsCurveResult::eval_point
//       Access: Published
//  Description: Computes the point on the curve corresponding to the
//               indicated value in parametric time.  Returns true if
//               the t value is valid, false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool NurbsCurveResult::
eval_point(float t, LVecBase3f &point) {
  int segment = find_segment(t);
  if (segment == -1) {
    return false;
  }

  eval_segment_point(segment, _basis.scale_t(segment, t), point);
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: NurbsCurveResult::eval_tangent
//       Access: Published
//  Description: Computes the tangent to the curve at the indicated
//               point in parametric time.  This tangent vector will
//               not necessarily be normalized, and could be zero.
//               See also eval_point().
////////////////////////////////////////////////////////////////////
INLINE bool NurbsCurveResult::
eval_tangent(float t, LVecBase3f &tangent) {
  int segment = find_segment(t);
  if (segment == -1) {
    return false;
  }

  eval_segment_tangent(segment, _basis.scale_t(segment, t), tangent);
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: NurbsCurveResult::eval_extended_point
//       Access: Published
//  Description: Evaluates the curve in n-dimensional space according
//               to the extended vertices associated with the curve in
//               the indicated dimension.
////////////////////////////////////////////////////////////////////
INLINE float NurbsCurveResult::
eval_extended_point(float t, int d) {
  int segment = find_segment(t);
  if (segment == -1) {
    return 0.0f;
  }

  return eval_segment_extended_point(segment, _basis.scale_t(segment, t), d);
}

////////////////////////////////////////////////////////////////////
//     Function: NurbsCurveResult::eval_extended_points
//       Access: Published
//  Description: Simultaneously performs eval_extended_point on a
//               contiguous sequence of dimensions.  The dimensions
//               evaluated are d through (d + num_values - 1); the
//               results are filled into the num_values elements in
//               the indicated result array.
////////////////////////////////////////////////////////////////////
INLINE bool NurbsCurveResult::
eval_extended_points(float t, int d, float result[], int num_values) {
  int segment = find_segment(t);
  if (segment == -1) {
    return false;
  }

  eval_segment_extended_points(segment, _basis.scale_t(segment, t), d,
                               result, num_values);
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: NurbsCurveResult::get_num_segments
//       Access: Public
//  Description: Returns the number of piecewise continuous segments
//               within the curve.  This number is usually not
//               important unless you plan to call
//               eval_segment_point().
////////////////////////////////////////////////////////////////////
INLINE int NurbsCurveResult::
get_num_segments() const {
  return _basis.get_num_segments();
}

////////////////////////////////////////////////////////////////////
//     Function: NurbsCurveResult::get_segment_t
//       Access: Public
//  Description: Accepts a t value in the range [0, 1], and assumed to
//               be relative to the indicated segment (as in
//               eval_segment_point()), and returns the corresponding
//               t value in the entire curve (as in eval_point()).
////////////////////////////////////////////////////////////////////
INLINE float NurbsCurveResult::
get_segment_t(int segment, float t) const {
  return t * (_basis.get_to(segment) - _basis.get_from(segment)) + _basis.get_from(segment);
}
