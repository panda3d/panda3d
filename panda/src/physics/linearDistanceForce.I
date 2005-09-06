// Filename: linearDistanceForce.I
// Created by:  charles (21Jun00)
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
//    Function : set_falloff_type
//      Access : Public
// Description : falloff_type encapsulating wrap
////////////////////////////////////////////////////////////////////
INLINE void LinearDistanceForce::
set_falloff_type(FalloffType ft) {
  _falloff = ft;
}

////////////////////////////////////////////////////////////////////
//    Function : set_radius
//      Access : Public
// Description : set the radius
////////////////////////////////////////////////////////////////////
INLINE void LinearDistanceForce::
set_radius(float r) {
  _radius = r;
}

////////////////////////////////////////////////////////////////////
//    Function : set_force_center
//      Access : Public
// Description : set the force center
////////////////////////////////////////////////////////////////////
INLINE void LinearDistanceForce::
set_force_center(const LPoint3f& p) {
  _force_center = p;
}

////////////////////////////////////////////////////////////////////
//    Function : get_falloff_type
//      Access : public
// Description : falloff_type query
////////////////////////////////////////////////////////////////////
INLINE LinearDistanceForce::FalloffType LinearDistanceForce::
get_falloff_type() const {
  return _falloff;
}

////////////////////////////////////////////////////////////////////
//    Function : get_radius
//      Access : public
// Description : radius query
////////////////////////////////////////////////////////////////////
INLINE float LinearDistanceForce::
get_radius() const {
  return _radius;
}

////////////////////////////////////////////////////////////////////
//    Function : get_force_center
//      Access : public
// Description : force_center query
////////////////////////////////////////////////////////////////////
INLINE LPoint3f LinearDistanceForce::
get_force_center() const {
  return _force_center;
}

////////////////////////////////////////////////////////////////////
//    Function : get_scalar_term
//      Access : private
// Description : calculate the term based on falloff
////////////////////////////////////////////////////////////////////
INLINE float LinearDistanceForce::
get_scalar_term() const {
  float r = _radius;
  if (_falloff == FT_ONE_OVER_R_SQUARED)
    r = r * r;
  else if (_falloff == FT_ONE_OVER_R_CUBED)
    r = r * r * r;

  return (1.0f / r);
}
