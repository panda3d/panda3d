// Filename: nurbsCurveResult.I
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
//     Function: NurbsCurveResult::Destructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE NurbsCurveResult::
~NurbsCurveResult() {
}

////////////////////////////////////////////////////////////////////
//     Function: NurbsCurveResult::get_start_t
//       Access: Published
//  Description: Returns the first legal value of t on the curve.
//               Usually this is 0.0.
////////////////////////////////////////////////////////////////////
INLINE PN_stdfloat NurbsCurveResult::
get_start_t() const {
  return _basis.get_start_t();
}

////////////////////////////////////////////////////////////////////
//     Function: NurbsCurveResult::get_end_t
//       Access: Published
//  Description: Returns the last legal value of t on the curve.
////////////////////////////////////////////////////////////////////
INLINE PN_stdfloat NurbsCurveResult::
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
eval_point(PN_stdfloat t, LVecBase3 &point) {
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
eval_tangent(PN_stdfloat t, LVecBase3 &tangent) {
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
INLINE PN_stdfloat NurbsCurveResult::
eval_extended_point(PN_stdfloat t, int d) {
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
eval_extended_points(PN_stdfloat t, int d, PN_stdfloat result[], int num_values) {
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
//       Access: Published
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
//       Access: Published
//  Description: Accepts a t value in the range [0, 1], and assumed to
//               be relative to the indicated segment (as in
//               eval_segment_point()), and returns the corresponding
//               t value in the entire curve (as in eval_point()).
////////////////////////////////////////////////////////////////////
INLINE PN_stdfloat NurbsCurveResult::
get_segment_t(int segment, PN_stdfloat t) const {
  return t * (_basis.get_to(segment) - _basis.get_from(segment)) + _basis.get_from(segment);
}

////////////////////////////////////////////////////////////////////
//     Function: NurbsCurveResult::get_num_samples
//       Access: Published
//  Description: Returns the number of sample points generated by the
//               previous call to adaptive_sample().
////////////////////////////////////////////////////////////////////
INLINE int NurbsCurveResult::
get_num_samples() const {
  return (int)_adaptive_result.size();
}

////////////////////////////////////////////////////////////////////
//     Function: NurbsCurveResult::get_sample_t
//       Access: Published
//  Description: Returns the t value of the nth sample point generated
//               by the previous call to adaptive_sample().
////////////////////////////////////////////////////////////////////
INLINE PN_stdfloat NurbsCurveResult::
get_sample_t(int n) const {
  nassertr(n >= 0 && n < (int)_adaptive_result.size(), 0.0f);
  return _adaptive_result[n]._t;
}

////////////////////////////////////////////////////////////////////
//     Function: NurbsCurveResult::get_sample_point
//       Access: Published
//  Description: Returns the point on the curve of the nth sample
//               point generated by the previous call to
//               adaptive_sample().
//
//               For tangents, or extended points, you should use
//               get_sample_t() and pass it into eval_tangent() or
//               eval_extended_point().
////////////////////////////////////////////////////////////////////
INLINE const LPoint3 &NurbsCurveResult::
get_sample_point(int n) const {
  nassertr(n >= 0 && n < (int)_adaptive_result.size(), LPoint3::zero());
  return _adaptive_result[n]._point;
}

////////////////////////////////////////////////////////////////////
//     Function: NurbsCurveResult::get_segment_t
//       Access: Public
//  Description: Accepts a t value in the range [0, 1], and assumed to
//               be relative to the indicated segment (as in
//               eval_segment_point()), and returns the corresponding
//               t value in the entire curve (as in eval_point()).
////////////////////////////////////////////////////////////////////
INLINE NurbsCurveResult::AdaptiveSample::
AdaptiveSample(PN_stdfloat t, const LPoint3 &point) :
  _t(t),
  _point(point)
{
}

