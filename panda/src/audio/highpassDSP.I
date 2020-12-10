/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file highpassDSP.I
 * @author lachbr
 * @date 2020-10-04
 */

/**
 *
 */
INLINE HighpassDSP::
HighpassDSP(float cutoff, float resonance) :
  DSP(DSP::DT_highpass),
  _cutoff(cutoff),
  _resonance(resonance) {
}

/**
 * Sets the highpass cutoff frequency in Hertz.
 */
INLINE void HighpassDSP::
set_cutoff(float cutoff) {
  _cutoff = cutoff;
  on_values_changed();
}

/**
 * Returns the highpass cutoff frequency in Hertz.
 */
INLINE float HighpassDSP::
get_cutoff() const {
  return _cutoff;
}

/**
 * Sets the highpass resonance Q value.
 */
INLINE void HighpassDSP::
set_resonance(float resonance) {
  _resonance = resonance;
  on_values_changed();
}

/**
 * Returns the highpass resonance Q value.
 */
INLINE float HighpassDSP::
get_resonance() const {
  return _resonance;
}
