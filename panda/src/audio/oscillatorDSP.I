/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file oscillatorDSP.I
 * @author lachbr
 * @date 2020-10-04
 */

/**
 *
 */
INLINE OscillatorDSP::
OscillatorDSP(OscillatorDSP::OscillatorType type, float rate) :
  DSP(DSP::DT_oscillator),
  _osc_type(type),
  _rate(rate) {
}

/**
 * Sets the oscillator waveform type.
 */
INLINE void OscillatorDSP::
set_oscillator_type(OscillatorDSP::OscillatorType type) {
  _osc_type = type;
  on_values_changed();
}

/**
 * Returns the oscillator waveform type.
 */
INLINE OscillatorDSP::OscillatorType OscillatorDSP::
get_oscillator_type() const {
  return _osc_type;
}

/**
 * Sets the frequency of the sinewave in Hertz.
 */
INLINE void OscillatorDSP::
set_rate(float rate) {
  _rate = rate;
  on_values_changed();
}

/**
 * Returns the frequency of the sinewave in Hertz.
 */
INLINE float OscillatorDSP::
get_rate() const {
  return _rate;
}
