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
  return _prod.get_start_t();
}

////////////////////////////////////////////////////////////////////
//     Function: NurbsCurveResult::get_end_t
//       Access: Public
//  Description: Returns the last legal value of t on the curve.
////////////////////////////////////////////////////////////////////
INLINE float NurbsCurveResult::
get_end_t() const {
  return _prod.get_end_t();
}

////////////////////////////////////////////////////////////////////
//     Function: NurbsCurveResult::eval_point
//       Access: Published
//  Description: Computes the point on the curve corresponding to the
//               indicated value in parametric time.  Returns true if
//               the t value is value, false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool NurbsCurveResult::
eval_point(float t, LVecBase3f &point) {
  int segment = find_segment(t);
  if (segment == -1) {
    return false;
  }

  eval_segment_point(segment, _prod.scale_t(segment, t), point);
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
  return _prod.get_num_segments();
}
