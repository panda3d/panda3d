// Filename: stitchScreen.cxx
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

#include "stitchScreen.h"

#include "compose_matrix.h"

TypeHandle StitchScreen::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: StitchScreen::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
StitchScreen::
StitchScreen() :
  _transform(LMatrix4d::ident_mat()),
  _inv_transform(LMatrix4d::ident_mat())
{
  _hpr_set = false;
  _pos_set = false;
  _hpr.set(0.0, 0.0, 0.0);
  _pos.set(0.0, 0.0, 0.0);
}

////////////////////////////////////////////////////////////////////
//     Function: StitchScreen::Destructor
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
StitchScreen::
~StitchScreen() {
}

////////////////////////////////////////////////////////////////////
//     Function: StitchScreen::clear_transform
//       Access: Public
//  Description: Resets the transform on this screen to identity.
////////////////////////////////////////////////////////////////////
void StitchScreen::
clear_transform() {
  _transform = LMatrix3d::ident_mat();
  _inv_transform = LMatrix3d::ident_mat();
  _hpr_set = false;
  _pos_set = false;
  _hpr.set(0.0, 0.0, 0.0);
  _pos.set(0.0, 0.0, 0.0);
}

////////////////////////////////////////////////////////////////////
//     Function: StitchScreen::set_transform
//       Access: Public
//  Description: Sets an arbitrary transform matrix on this screen.
//               This adjusts the screen's position and/or size in
//               space accordingly.
////////////////////////////////////////////////////////////////////
void StitchScreen::
set_transform(const LMatrix4d &trans) {
  _transform = trans;
  _inv_transform = invert(trans);
  LVecBase3d scale;
  decompose_matrix(_transform, scale, _hpr, _pos);
  _hpr_set = false;
  _pos_set = false;
}

////////////////////////////////////////////////////////////////////
//     Function: StitchScreen::set_hpr
//       Access: Public
//  Description: Sets the orientation of the screen, independent of
//               its position.
////////////////////////////////////////////////////////////////////
void StitchScreen::
set_hpr(const LVecBase3d &hpr) {
  compose_matrix(_transform, LVecBase3d(1.0, 1.0, 1.0), hpr, _pos);
  _inv_transform.invert_from(_transform);
  _hpr_set = true;
  _hpr = hpr;
}

////////////////////////////////////////////////////////////////////
//     Function: StitchScreen::set_pos
//       Access: Public
//  Description: Sets the position of the screen, independent of
//               its orientation.
////////////////////////////////////////////////////////////////////
void StitchScreen::
set_pos(const LPoint3d &pos) {
  compose_matrix(_transform, LVecBase3d(1.0, 1.0, 1.0), _hpr, pos);
  _inv_transform.invert_from(_transform);
  _pos_set = true;
  _pos = pos;
}


////////////////////////////////////////////////////////////////////
//     Function: StitchScreen::intersect
//       Access: Public, Virtual
//  Description: Determines the intersection point of a ray starting
//               at the indicated origin, and continuing infinitely in
//               the indicated direction, with the screen.
//
//               If there is no intersection, leaves result undefined
//               and returns false.  If there is an intersection, sets
//               result to the intersection point and returns true.
////////////////////////////////////////////////////////////////////
bool StitchScreen::
intersect(LPoint3d &result, 
          const LPoint3d &origin,
          const LVector3d &direction) const {
  double t = compute_intersect(origin, direction);
  if (t < 0.0) {
    // No intersection.
    return false;
  }

  result = origin + direction * t;
  return true;
}
