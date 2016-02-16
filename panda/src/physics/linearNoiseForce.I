/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file linearNoiseForce.I
 * @author charles
 * @date 2000-06-19
 */

/**
 * Returns a valid entry in the prn table
 */
INLINE unsigned char LinearNoiseForce::
prn_lookup(int index) const {
  return _prn_table[index & 255];
}

/**
 * Hashes a point, returns a prn
 */
INLINE unsigned char LinearNoiseForce::
get_prn_entry(const LPoint3& point) const {
  return prn_lookup((int)(point[0] + prn_lookup((int)(point[1] + prn_lookup((int)point[2])))));
}

/**
 * Hashes a point, returns a prn (piecewise)
 */
INLINE unsigned char LinearNoiseForce::
get_prn_entry(const PN_stdfloat x, const PN_stdfloat y, const PN_stdfloat z) const {
  return prn_lookup((int)(x + prn_lookup((int)(y + prn_lookup((int)z)))));
}

/**
 * Hashes a point, returns a gradient vector
 */
INLINE LVector3& LinearNoiseForce::
get_lattice_entry(const LPoint3& point) {
  return _gradient_table[get_prn_entry(point)];
}

/**
 * Hashes a point, returns a gradient vector (piecewise)
 */
INLINE LVector3& LinearNoiseForce::
get_lattice_entry(const PN_stdfloat x, const PN_stdfloat y, const PN_stdfloat z) {
  return _gradient_table[get_prn_entry(x, y, z)];
}

/**
 * Smooths a parameterized interpolation using 2x^3 - 3x^2
 */
INLINE PN_stdfloat LinearNoiseForce::
cubic_step(const PN_stdfloat x) const {
  return x * x * ((2 * x) - 3);
}

/**
 * Vector linear interpolation
 */
INLINE LVector3 LinearNoiseForce::
vlerp(const PN_stdfloat t, const LVector3& v0, const LVector3& v1) const {
  return v0 + ((v1 - v0) * t);
}
