/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file chorusDSP.I
 * @author lachbr
 * @date 2020-10-04
 */

/**
 *
 */
INLINE ChorusDSP::
ChorusDSP(float mix, float rate, float depth) :
  DSP(DSP::DT_chorus),
  _mix(mix),
  _rate(rate),
  _depth(depth) {
}

/**
 * Sets the volume of original signal to pass to output as a percentage from
 * 0-100.
 */
INLINE void ChorusDSP::
set_mix(float mix) {
  _mix = mix;
  on_values_changed();
}

/**
 * Returns the volume of original signal to pass to output as a percentage from
 * 0-100.
 */
INLINE float ChorusDSP::
get_mix() const {
  return _mix;
}

/**
 * Sets the chorus modulation rate in Hertz.
 */
INLINE void ChorusDSP::
set_rate(float rate) {
  _rate = rate;
  on_values_changed();
}

/**
 * Returns the chorus modulation rate in Hertz.
 */
INLINE float ChorusDSP::
get_rate() const {
  return _rate;
}

/**
 * Sets the chorus modulation depth as a percentage from 0-100.
 */
INLINE void ChorusDSP::
set_depth(float depth) {
  _depth = depth;
  on_values_changed();
}

/**
 * Returns the chorus modulation depth as a percentage from 0-100.
 */
INLINE float ChorusDSP::
get_depth() const {
  return _depth;
}
