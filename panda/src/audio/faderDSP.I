/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file faderDSP.I
 * @author lachbr
 * @date 2020-10-04
 */

/**
 *
 */
INLINE FaderDSP::
FaderDSP(float gain) :
  DSP(DSP::DT_fader),
  _gain(gain) {
}

/**
 * Sets the signal gain in Decibels.
 */
INLINE void FaderDSP::
set_gain(float gain) {
  _gain = gain;
}

/**
 * Returns the signal gain in Decibels.
 */
INLINE float FaderDSP::
get_gain() const {
  return _gain;
}
