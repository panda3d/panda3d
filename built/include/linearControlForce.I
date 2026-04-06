/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file linearControlForce.I
 * @author Dave Schuyler
 * @date 2006
 */

/**
 * encapsulating wrapper
 */
INLINE void LinearControlForce::
clear_physics_object() {
  _physics_object = nullptr;
}

/**
 * encapsulating wrapper
 */
INLINE void LinearControlForce::
set_physics_object(const PhysicsObject *po) {
  _physics_object = po;
}

/**
 * piecewise encapsulating wrapper
 */
INLINE CPT(PhysicsObject) LinearControlForce::
get_physics_object() const {
  return _physics_object;
}

/**
 * encapsulating wrapper
 */
INLINE void LinearControlForce::
set_vector(const LVector3& v) {
  _fvec = v;
}

/**
 * piecewise encapsulating wrapper
 */
INLINE void LinearControlForce::
set_vector(PN_stdfloat x, PN_stdfloat y, PN_stdfloat z) {
  _fvec.set(x, y, z);
}

/**
 *
 */
INLINE LVector3 LinearControlForce::
get_local_vector() const {
  return _fvec;
}
