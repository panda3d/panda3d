/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file linearVectorForce.I
 * @author charles
 * @date 2000-06-21
 */

/**
 * encapsulating wrapper
 */
INLINE void LinearVectorForce::
set_vector(const LVector3& v) {
  _fvec = v;
}

/**
 * piecewise encapsulating wrapper
 */
INLINE void LinearVectorForce::
set_vector(PN_stdfloat x, PN_stdfloat y, PN_stdfloat z) {
  _fvec.set(x, y, z);
}

/**
 *
 */
INLINE LVector3 LinearVectorForce::
get_local_vector() const {
  return _fvec;
}

/**
 * Performs a memberwise addition between two LinearVectorForces.
 */
LinearVectorForce& LinearVectorForce::
operator += (const LinearVectorForce &other) {
  _fvec+=other._fvec;
  // Should something happen with _amplitude?
  return *this;
}
