// Filename: fltCurve.I
// Created by:  drose (28Feb01)
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
//     Function: FltCurve::get_num_control_points
//       Access: Public
//  Description: Returns the number of control points assigned to the
//               curve.
////////////////////////////////////////////////////////////////////
INLINE int FltCurve::
get_num_control_points() const {
  return _control_points.size();
}

////////////////////////////////////////////////////////////////////
//     Function: FltCurve::get_control_point
//       Access: Public
//  Description: Returns the nth control point assigned to the curve.
////////////////////////////////////////////////////////////////////
INLINE const LPoint3d &FltCurve::
get_control_point(int n) const {
#ifndef NDEBUG
  static LPoint3d bogus(0.0, 0.0, 0.0);
  nassertr(n >= 0 && n < (int)_control_points.size(), bogus);
#endif
  return _control_points[n];
}
