// Filename: nurbsCurveInterface.I
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
//     Function: NurbsCurveInterface::append_cv
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE int NurbsCurveInterface::
append_cv(PN_stdfloat x, PN_stdfloat y, PN_stdfloat z) {
  return append_cv(LVecBase3(x, y, z));
}

////////////////////////////////////////////////////////////////////
//     Function: NurbsCurveInterface::append_cv
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE int NurbsCurveInterface::
append_cv(const LVecBase3 &v) {
  return append_cv(LVecBase4(v[0], v[1], v[2], 1.0f));
}

////////////////////////////////////////////////////////////////////
//     Function: NurbsCurveInterface::append_cv
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE int NurbsCurveInterface::
append_cv(const LVecBase4 &v) {
  return append_cv_impl(v);
}

////////////////////////////////////////////////////////////////////
//     Function: NurbsCurveInterface::set_cv_point
//       Access: Public, Scheme
//  Description: Repositions the indicated CV.  Returns true if
//               successful, false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool NurbsCurveInterface::
set_cv_point(int n, PN_stdfloat x, PN_stdfloat y, PN_stdfloat z) {
  return set_cv_point(n, LVecBase3(x, y, z));
}

////////////////////////////////////////////////////////////////////
//     Function: NurbsCurveInterface::set_cv_point
//       Access: Public, Scheme
//  Description: Repositions the indicated CV.  Returns true if
//               successful, false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool NurbsCurveInterface::
set_cv_point(int n, const LVecBase3 &v) {
  nassertr(n >= 0 && n < get_num_cvs(), false);
  return set_cv(n, LVecBase4(v[0], v[1], v[2], 1.0f) * get_cv_weight(n));
}

////////////////////////////////////////////////////////////////////
//     Function: NurbsCurveInterface::get_cv_point
//       Access: Public, Scheme
//  Description: Returns the position of the indicated CV.
////////////////////////////////////////////////////////////////////
INLINE LVecBase3 NurbsCurveInterface::
get_cv_point(int n) const {
  nassertr(n >= 0 && n < get_num_cvs(), LVecBase3::zero());
  LVecBase4 p = get_cv(n);
  nassertr(p[3] != 0.0f, LVecBase3::zero());
  return LVecBase3(p[0], p[1], p[2]) / p[3];
}

////////////////////////////////////////////////////////////////////
//     Function: NurbsCurveInterface::get_cv_weight
//       Access: Published
//  Description: Returns the weight of the indicated CV.
////////////////////////////////////////////////////////////////////
INLINE PN_stdfloat NurbsCurveInterface::
get_cv_weight(int n) const {
  return get_cv(n)[3];
}
