// Filename: parametricCurveCollection.I
// Created by:  drose (04Mar01)
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
//     Function: ParametricCurveCollection::Destructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE ParametricCurveCollection::
~ParametricCurveCollection() {
}

////////////////////////////////////////////////////////////////////
//     Function: ParametricCurveCollection::get_num_curves
//       Access: Published
//  Description: Returns the number of ParametricCurves in the collection.
////////////////////////////////////////////////////////////////////
INLINE int ParametricCurveCollection::
get_num_curves() const {
  return _curves.size();
}

////////////////////////////////////////////////////////////////////
//     Function: ParametricCurveCollection::get_curve
//       Access: Published
//  Description: Returns the nth ParametricCurve in the collection.
////////////////////////////////////////////////////////////////////
INLINE ParametricCurve *ParametricCurveCollection::
get_curve(int index) const {
  nassertr(index >= 0 && index < (int)_curves.size(), (ParametricCurve *)NULL);

  return _curves[index];
}

////////////////////////////////////////////////////////////////////
//     Function: ParametricCurveCollection::get_max_t
//       Access: Published
//  Description: Returns the maximum T value associated with the
//               *last* curve in the collection.  Normally, this will
//               be either the XYZ or HPR curve, or a timewarp curve.
////////////////////////////////////////////////////////////////////
INLINE float ParametricCurveCollection::
get_max_t() const {
  if (_curves.empty()) {
    return 0.0f;
  }
  return _curves.back()->get_max_t();
}

////////////////////////////////////////////////////////////////////
//     Function: ParametricCurveCollection::evaluate_xyz
//       Access: Published
//  Description: Computes only the XYZ part of the curves.  See
//               evaluate().
////////////////////////////////////////////////////////////////////
INLINE bool ParametricCurveCollection::
evaluate_xyz(float t, LVecBase3f &xyz) const {
  LVecBase3f hpr;
  return evaluate(t, xyz, hpr);
}

////////////////////////////////////////////////////////////////////
//     Function: ParametricCurveCollection::evaluate_hpr
//       Access: Published
//  Description: Computes only the HPR part of the curves.  See
//               evaluate().
////////////////////////////////////////////////////////////////////
INLINE bool ParametricCurveCollection::
evaluate_hpr(float t, LVecBase3f &hpr) const {
  LVecBase3f xyz;
  return evaluate(t, xyz, hpr);
}

////////////////////////////////////////////////////////////////////
//     Function: ParametricCurveCollection::adjust_xyz
//       Access: Published
//  Description: Adjust the XYZ curve at the indicated time to the new
//               value.  The curve shape will change correspondingly.
//               Returns true if successful, false if unable to make
//               the adjustment for some reason.
////////////////////////////////////////////////////////////////////
INLINE bool ParametricCurveCollection::
adjust_xyz(float t, float x, float y, float z) {
  return adjust_xyz(t, LVecBase3f(x, y, z));
}

////////////////////////////////////////////////////////////////////
//     Function: ParametricCurveCollection::adjust_hpr
//       Access: Published
//  Description: Adjust the HPR curve at the indicated time to the new
//               value.  The curve shape will change correspondingly.
//               Returns true if successful, false if unable to make
//               the adjustment for some reason.
////////////////////////////////////////////////////////////////////
INLINE bool ParametricCurveCollection::
adjust_hpr(float t, float h, float p, float r) {
  return adjust_hpr(t, LVecBase3f(h, p, r));
}
