/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file flangeDSP.I
 * @author lachbr
 * @date 2020-10-04
 */

/**
 *
 */
INLINE FlangeDSP::
FlangeDSP(float mix, float depth, float rate) :
  DSP(DSP::DT_flange),
  _mix(mix),
  _depth(depth),
  _rate(rate) {
}

/**
 * Sets the percentage of wet signal in mix.
 */
INLINE void FlangeDSP::
set_mix(float mix) {
  _mix = mix;
  on_values_changed();
}

/**
 * Returns the percentage of wet signal in mix.
 */
INLINE float FlangeDSP::
get_mix() const {
  return _mix;
}

/**
 * Sets the flange depth.
 */
INLINE void FlangeDSP::
set_depth(float depth) {
  _depth = depth;
  on_values_changed();
}

/**
 * Returns the flange depth.
 */
INLINE float FlangeDSP::
get_depth() const {
  return _depth;
}

/**
 * Sets the flange speed.
 */
INLINE void FlangeDSP::
set_rate(float rate) {
  _rate = rate;
  on_values_changed();
}

/**
 * Returns the flange speed.
 */
INLINE float FlangeDSP::
get_rate() const {
  return _rate;
}
