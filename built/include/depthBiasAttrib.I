/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file depthBiasAttrib.I
 * @author rdb
 * @date 2021-08-24
 */

/**
 * Use DepthBiasAttrib::make() to construct a new DepthBiasAttrib object.
 */
INLINE DepthBiasAttrib::
DepthBiasAttrib(PN_stdfloat slope_factor, PN_stdfloat constant_factor,
                PN_stdfloat clamp) :
  _slope_factor(slope_factor),
  _constant_factor(constant_factor),
  _clamp(clamp)
{
}

/**
 * Returns the slope factor.
 */
INLINE PN_stdfloat DepthBiasAttrib::
get_slope_factor() const {
  return _slope_factor;
}

/**
 * Returns the constant factor.
 */
INLINE PN_stdfloat DepthBiasAttrib::
get_constant_factor() const {
  return _constant_factor;
}

/**
 * Returns the maximum (or minimum, if negative) value of the bias.  If zero,
 * no clamping is performed.
 */
INLINE PN_stdfloat DepthBiasAttrib::
get_clamp() const {
  return _clamp;
}
