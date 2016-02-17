/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file linearForce.I
 * @author charles
 * @date 2000-06-13
 */

/**

 */
INLINE void LinearForce::
set_amplitude(PN_stdfloat a) {
  _amplitude = a;
}

/**

 */
INLINE PN_stdfloat LinearForce::
get_amplitude() const {
  return _amplitude;
}

/**

 */
INLINE bool LinearForce::
get_mass_dependent() const {
  return _mass_dependent;
}

/**

 */
INLINE void LinearForce::
set_mass_dependent(bool m) {
  _mass_dependent = m;
}

/**

 */
INLINE void LinearForce::
set_vector_masks(bool x, bool y, bool z) {
  _x_mask = x;
  _y_mask = y;
  _z_mask = z;
}

/**

 */
INLINE LVector3 LinearForce::
get_vector_masks() {
  return LVector3(_x_mask, _y_mask, _z_mask);
}
