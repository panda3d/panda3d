// Filename: linearNoiseForce.I
// Created by:  charles (19Jun00)
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
//    Function : prn_lookup
//      Access : Private
// Description : Returns a valid entry in the prn table
////////////////////////////////////////////////////////////////////
INLINE unsigned char LinearNoiseForce::
prn_lookup(int index) const {
  return _prn_table[index & 255];
}

////////////////////////////////////////////////////////////////////
//    Function : get_prn_entry
//      Access : Private
// Description : Hashes a point, returns a prn
////////////////////////////////////////////////////////////////////
INLINE unsigned char LinearNoiseForce::
get_prn_entry(const LPoint3f& point) const {
  return prn_lookup((int)(point[0] + prn_lookup((int)(point[1] + prn_lookup((int)point[2])))));
}

////////////////////////////////////////////////////////////////////
//    Function : get_prn_entry
//      Access : Private
// Description : Hashes a point, returns a prn (piecewise)
////////////////////////////////////////////////////////////////////
INLINE unsigned char LinearNoiseForce::
get_prn_entry(const float x, const float y, const float z) const {
  return prn_lookup((int)(x + prn_lookup((int)(y + prn_lookup((int)z)))));
}

////////////////////////////////////////////////////////////////////
//    Function : get_lattice_entry
//      Access : Private
// Description : Hashes a point, returns a gradient vector
////////////////////////////////////////////////////////////////////
INLINE LVector3f& LinearNoiseForce::
get_lattice_entry(const LPoint3f& point) {
  return _gradient_table[get_prn_entry(point)];
}

////////////////////////////////////////////////////////////////////
//    Function : get_lattice_entry
//      Access : Private
// Description : Hashes a point, returns a gradient vector (piecewise)
////////////////////////////////////////////////////////////////////
INLINE LVector3f& LinearNoiseForce::
get_lattice_entry(const float x, const float y, const float z) {
  return _gradient_table[get_prn_entry(x, y, z)];
}

////////////////////////////////////////////////////////////////////
//    Function : cubic_step
//      Access : Private
// Description : Smooths a parameterized interpolation using
//               2x^3 - 3x^2
////////////////////////////////////////////////////////////////////
INLINE float LinearNoiseForce::
cubic_step(const float x) const {
  return x * x * ((2 * x) - 3);
}

////////////////////////////////////////////////////////////////////
//    Function : vlerp
//      Access : Private
// Description : Vector linear interpolation
////////////////////////////////////////////////////////////////////
INLINE LVector3f LinearNoiseForce::
vlerp(const float t, const LVector3f& v0, const LVector3f& v1) const {
  return v0 + ((v1 - v0) * t);
}
