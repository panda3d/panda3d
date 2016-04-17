/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file angularVectorForce.I
 * @author charles
 * @date 2000-08-09
 */

/**

 */
INLINE void AngularVectorForce::
set_quat(const LRotation &v) {
  _fvec = v;
}

/**

 */
INLINE void AngularVectorForce::
set_hpr(PN_stdfloat h, PN_stdfloat p, PN_stdfloat r) {
  _fvec.set_hpr(LVecBase3(h, p, r));
}

/**

 */
INLINE LRotation AngularVectorForce::
get_local_quat() const {
  return _fvec;
}
