/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file linearCylinderVortexForce.I
 * @author charles
 * @date 2000-07-24
 */

/**

 */
INLINE void LinearCylinderVortexForce::
set_radius(PN_stdfloat radius) {
  _radius = radius;
}

/**

 */
INLINE void LinearCylinderVortexForce::
set_length(PN_stdfloat length) {
  _length = length;
}

/**

 */
INLINE void LinearCylinderVortexForce::
set_coef(PN_stdfloat coef) {
  _coef = coef;
}

/**

 */
INLINE PN_stdfloat LinearCylinderVortexForce::
get_radius() const {
  return _radius;
}

/**

 */
INLINE PN_stdfloat LinearCylinderVortexForce::
get_length() const {
  return _length;
}

/**

 */
INLINE PN_stdfloat LinearCylinderVortexForce::
get_coef() const {
  return _coef;
}
