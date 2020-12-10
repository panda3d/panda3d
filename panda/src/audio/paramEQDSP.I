/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file paramEQDSP.I
 * @author lachbr
 * @date 2020-10-08
 */

/**
 *
 */
INLINE ParamEQDSP::
ParamEQDSP(float center, float bandwith, float gain):
  DSP(DSP::DT_parameq),
  _center(center),
  _bandwith(bandwith),
  _gain(gain)
{
}

/**
 * Sets the frequency center.
 * Unit: Hertz
 */
INLINE void ParamEQDSP::
set_center(float center) {
  _center = center;
  on_values_changed();
}

/**
 * Returns the frequency center.
 * Unit: Hertz
 */
INLINE float ParamEQDSP::
get_center() const {
  return _center;
}

/**
 * Sets the octave range around the center frequency.
 */
INLINE void ParamEQDSP::
set_bandwith(float bandwith) {
  _bandwith = bandwith;
  on_values_changed();
}

/**
 * Returns the octave range around the center frequency.
 */
INLINE float ParamEQDSP::
get_bandwith() const {
  return _bandwith;
}

/**
 * Sets the frequency gain.
 * Unit: Decibels
 */
INLINE void ParamEQDSP::
set_gain(float gain) {
  _gain = gain;
  on_values_changed();
}

/**
 * Returns the frequency gain.
 * Unit: Decibels
 */
INLINE float ParamEQDSP::
get_gain() const {
  return _gain;
}
