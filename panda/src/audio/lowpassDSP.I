/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file lowpassDSP.I
 * @author lachbr
 * @date 2020-10-04
 */

/**
 *
 */
INLINE LowpassDSP::
LowpassDSP(float cutoff, float resonance) :
  DSP(DSP::DT_lowpass),
  _cutoff(cutoff),
  _resonance(resonance) {
}

/**
 * Sets the lowpass cutoff frequency in Hertz.
 */
INLINE void LowpassDSP::
set_cutoff(float cutoff) {
  _cutoff = cutoff;
  on_values_changed();
}

/**
 * Returns the lowpass cutoff frequency in Hertz.
 */
INLINE float LowpassDSP::
get_cutoff() const {
  return _cutoff;
}

/**
 * Sets the lowpass resonance Q value.
 */
INLINE void LowpassDSP::
set_resonance(float resonance) {
  _resonance = resonance;
  on_values_changed();
}

/**
 * Returns the lowpass resonance Q value.
 */
INLINE float LowpassDSP::
get_resonance() const {
  return _resonance;
}
