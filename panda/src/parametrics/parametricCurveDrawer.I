// Filename: parametricCurveDrawer.I
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
//     Function: ParametricCurveDrawer::get_max_t
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE float ParametricCurveDrawer::
get_max_t() const {
  if (_curves == (ParametricCurveCollection *)NULL) {
    return 0.0f;
  } else {
    return _curves->get_max_t();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: ParametricCurveDrawer::redraw
//       Access: Public
//  Description: Redraws the curve if it is currently visible.
////////////////////////////////////////////////////////////////////
INLINE void ParametricCurveDrawer::
redraw() {
  if (_drawn) {
    draw();
  }
}
