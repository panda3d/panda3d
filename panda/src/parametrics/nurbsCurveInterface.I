// Filename: nurbsCurveInterface.I
// Created by:  drose (02Mar01)
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
//     Function: NurbsCurveInterface::append_cv
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE int NurbsCurveInterface::
append_cv(float x, float y, float z) {
  return append_cv(LVecBase3f(x, y, z));
}

////////////////////////////////////////////////////////////////////
//     Function: NurbsCurveInterface::append_cv
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE int NurbsCurveInterface::
append_cv(const LVecBase3f &v) {
  return append_cv(LVecBase4f(v[0], v[1], v[2], 1.0f));
}

////////////////////////////////////////////////////////////////////
//     Function: NurbsCurveInterface::append_cv
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE int NurbsCurveInterface::
append_cv(const LVecBase4f &v) {
  return append_cv_impl(v);
}

////////////////////////////////////////////////////////////////////
//     Function: NurbsCurveInterface::set_cv_point
//       Access: Public, Scheme
//  Description: Repositions the indicated CV.  Returns true if
//               successful, false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool NurbsCurveInterface::
set_cv_point(int n, float x, float y, float z) {
  return set_cv_point(n, LVecBase3f(x, y, z));
}

////////////////////////////////////////////////////////////////////
//     Function: NurbsCurveInterface::set_cv_point
//       Access: Public, Scheme
//  Description: Repositions the indicated CV.  Returns true if
//               successful, false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool NurbsCurveInterface::
set_cv_point(int n, const LVecBase3f &v) {
  nassertr(n >= 0 && n < get_num_cvs(), false);
  return set_cv(n, LVecBase4f(v[0], v[1], v[2], 1.0f) * get_cv_weight(n));
}

////////////////////////////////////////////////////////////////////
//     Function: NurbsCurveInterface::get_cv_point
//       Access: Public, Scheme
//  Description: Returns the position of the indicated CV.
////////////////////////////////////////////////////////////////////
INLINE LVecBase3f NurbsCurveInterface::
get_cv_point(int n) const {
  nassertr(n >= 0 && n < get_num_cvs(), LVecBase3f::zero());
  LVecBase4f p = get_cv(n);
  nassertr(p[3] != 0.0f, LVecBase3f::zero());
  return LVecBase3f(p[0], p[1], p[2]) / p[3];
}

////////////////////////////////////////////////////////////////////
//     Function: NurbsCurveInterface::get_cv_weight
//       Access: Published
//  Description: Returns the weight of the indicated CV.
////////////////////////////////////////////////////////////////////
INLINE float NurbsCurveInterface::
get_cv_weight(int n) const {
  return get_cv(n)[3];
}
