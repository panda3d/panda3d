// Filename: arcEmitter.I
// Created by:  charles (26Jun00)
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
//    Function : set_start_angle
//      Access : public
// Description : start angle set
////////////////////////////////////////////////////////////////////

INLINE void ArcEmitter::
set_start_angle(float angle) {
  _start_theta = deg_2_rad(angle);
}

////////////////////////////////////////////////////////////////////
//    Function : set_end_angle
//      Access : public
// Description : end angle set
////////////////////////////////////////////////////////////////////

INLINE void ArcEmitter::
set_end_angle(float angle) {
  _end_theta = deg_2_rad(angle);
}

////////////////////////////////////////////////////////////////////
//    Function : set_arc
//      Access : public
// Description : arc sweep set
////////////////////////////////////////////////////////////////////

INLINE void ArcEmitter::
set_arc(float startAngle, float endAngle) {
  _start_theta = deg_2_rad(startAngle);
  _end_theta = deg_2_rad(endAngle);
}

////////////////////////////////////////////////////////////////////
//    Function : get_start_angle
//      Access : public
// Description : get start angle
////////////////////////////////////////////////////////////////////

INLINE float ArcEmitter::
get_start_angle() {
  return rad_2_deg(_start_theta);
}

////////////////////////////////////////////////////////////////////
//    Function : get_end_angle
//      Access : public
// Description : get end angle
////////////////////////////////////////////////////////////////////

INLINE float ArcEmitter::
get_end_angle() {
  return rad_2_deg(_end_theta);
}

