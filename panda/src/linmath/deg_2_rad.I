/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file deg_2_rad.I
 * @author drose
 * @date 2011-12-14
 */

INLINE_LINMATH double deg_2_rad(double f) {
  return f * MathNumbers::deg_2_rad_d;
}

INLINE_LINMATH double rad_2_deg(double f) {
  return f * MathNumbers::rad_2_deg_d;
}

INLINE_LINMATH float deg_2_rad(float f) {
  return f * MathNumbers::deg_2_rad_f;
}

INLINE_LINMATH float rad_2_deg(float f) {
  return f * MathNumbers::rad_2_deg_f;
}
