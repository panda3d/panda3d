// Filename: stitchFlatScreen.cxx
// Created by:  drose (16Jul01)
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

#include "stitchFlatScreen.h"

////////////////////////////////////////////////////////////////////
//     Function: StitchFlatScreen::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
StitchFlatScreen::
StitchFlatScreen() :
  _plane(LVector3d::back(), LPoint3d::origin() + LVector3d::forward())
{
}

////////////////////////////////////////////////////////////////////
//     Function: StitchFlatScreen::set_plane
//       Access: Public
//  Description: Specifies the plane that defines the screen.  This
//               plane may, of course, be transformed by whatever
//               transform matrix is specified via set_transform(),
//               etc.
//
//               The default plane is one unit along the forward axis,
//               facing back toward the origin.
////////////////////////////////////////////////////////////////////
void StitchFlatScreen::
set_plane(const Planed &plane) {
  _plane = plane;
}

////////////////////////////////////////////////////////////////////
//     Function: StitchFlatScreen::compute_intersect
//       Access: Protected, Virtual
//  Description: Computes the intersection point for the ray beginning
//               at the indicated origin point and continuing in the
//               indicated direction, to infinity.  The return value
//               is a number >= 0.0 that indicates the parametric
//               intersection point along the ray with the screen
//               geometry, or a number < 0.0 if there is no
//               intersection with the screen.
////////////////////////////////////////////////////////////////////
double StitchFlatScreen::
compute_intersect(const LPoint3d &origin, const LVector3d &direction) const {
  LPoint3d p = origin * _inv_transform;
  LVector3d d = direction * _inv_transform;

  double t;
  if (!_plane.intersects_line(t, p, d)) {
    t = -1.0;
  }
  return t;
}
