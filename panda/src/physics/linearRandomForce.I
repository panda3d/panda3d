/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file linearRandomForce.I
 * @author charles
 * @date 2000-06-19
 */

/**
 * generates a random unit vector
 */
INLINE LVector3 LinearRandomForce::
random_unit_vector() {
  PN_stdfloat z, r, theta;

  z = 1.0 - (2.0 * bounded_rand());
  r = csqrt(1.0 - (z * z));
  theta = 2.0 * MathNumbers::pi * bounded_rand();

  return LVector3(r * ccos(theta), r * csin(theta), z);
}
