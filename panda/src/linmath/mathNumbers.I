/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file mathNumbers.I
 * @author drose
 * @date 2005-01-22
 */

/**
 * Returns pi as a single-precision or double-precision number, according to
 * the type of the parameter.
 */
INLINE float MathNumbers::
cpi(float) {
  return pi_f;
}

/**
 * Returns ln(2) as a single-precision or double-precision number, according
 * to the type of the parameter.
 */
INLINE float MathNumbers::
cln2(float) {
  return ln2_f;
}

/**
 * Returns pi as a single-precision or double-precision number, according to
 * the type of the parameter.
 */
INLINE double MathNumbers::
cpi(double) {
  return pi;
}

/**
 * Returns ln(2) as a single-precision or double-precision number, according
 * to the type of the parameter.
 */
INLINE double MathNumbers::
cln2(double) {
  return ln2;
}
