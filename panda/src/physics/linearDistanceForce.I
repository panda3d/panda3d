// Filename: linearDistanceForce.I
// Created by:  charles (21Jun00)
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
set_radius(PN_stdfloat r) {
  _radius = r;
}

////////////////////////////////////////////////////////////////////
//    Function : set_force_center
//      Access : Public
// Description : set the force center
////////////////////////////////////////////////////////////////////
INLINE void LinearDistanceForce::
set_force_center(const LPoint3& p) {
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
INLINE PN_stdfloat LinearDistanceForce::
get_radius() const {
  return _radius;
}

////////////////////////////////////////////////////////////////////
//    Function : get_force_center
//      Access : public
// Description : force_center query
////////////////////////////////////////////////////////////////////
INLINE LPoint3 LinearDistanceForce::
get_force_center() const {
  return _force_center;
}

////////////////////////////////////////////////////////////////////
//    Function : get_scalar_term
//      Access : private
// Description : calculate the term based on falloff
////////////////////////////////////////////////////////////////////
INLINE PN_stdfloat LinearDistanceForce::
get_scalar_term() const {
  PN_stdfloat r = _radius;
  if (_falloff == FT_ONE_OVER_R_SQUARED)
    r = r * r;
  else if (_falloff == FT_ONE_OVER_R_CUBED)
    r = r * r * r;

  return (1.0f / r);
}
