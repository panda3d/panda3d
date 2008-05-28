// Filename: nurbsCurve.I
// Created by:  drose (02Mar01)
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
//     Function: NurbsCurve::get_curveseg
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE CubicCurveseg *NurbsCurve::
get_curveseg(int ti) {
  return (CubicCurveseg *)PiecewiseCurve::get_curveseg(ti);
}
