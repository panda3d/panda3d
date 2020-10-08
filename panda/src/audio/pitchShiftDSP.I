/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pitchShiftDSP.I
 * @author lachbr
 * @date 2020-10-08
 */

/**
 *
 */
INLINE PitchShiftDSP::
PitchShiftDSP(float pitch, int fft_size) :
  DSP(DSP::DT_pitchshift),
  _pitch(pitch),
  _fft_size(fft_size) {
}

/**
 * Sets the pitch value. 0.5 = one octave down, 2.0 = one octave up.
 * 1.0 does not change the pitch.
 */
INLINE void PitchShiftDSP::
set_pitch(float pitch) {
  _pitch = pitch;
  on_values_changed();
}

/**
 * Returns the pitch value.
 */
INLINE float PitchShiftDSP::
get_pitch() const {
  return _pitch;
}

/**
 * Sets the FFT window size (must be pow 2).
 */
INLINE void PitchShiftDSP::
set_fft_size(int size) {
  _fft_size = size;
  on_values_changed();
}

/**
 * Returns the FFT window size.
 */
INLINE int PitchShiftDSP::
get_fft_size() const {
  return _fft_size;
}
