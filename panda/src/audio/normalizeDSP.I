/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file normalizeDSP.I
 * @author lachbr
 * @date 2020-10-08
 */

/**
 *
 */
INLINE NormalizeDSP::
NormalizeDSP(float fade_time, float threshold, float max_amp) :
  DSP(DSP::DT_normalize),
  _fade_time(fade_time),
  _threshold(threshold),
  _max_amp(max_amp)
{
}

/**
 * Sets the time to ramp the silence to full.
 * Unit: milliseconds
 */
INLINE void NormalizeDSP::
set_fade_time(float time) {
  _fade_time = time;
  on_values_changed();
}

/**
 * Returns the time to ramp the silence to full.
 * Unit: milliseconds
 */
INLINE float NormalizeDSP::
get_fade_time() const {
  return _fade_time;
}

/**
 * Sets the lower volume range threshold to ignore.
 * Unit: milliseconds
 */
INLINE void NormalizeDSP::
set_threshold(float threshold) {
  _threshold = threshold;
  on_values_changed();
}

/**
 * Returns the lower volume range threshold to ignore.
 * Unit: milliseconds
 */
INLINE float NormalizeDSP::
get_threshold() const {
  return _threshold;
}

/**
 * Sets the maximum amplification allowed.
 */
INLINE void NormalizeDSP::
set_max_amp(float max_amp) {
  _max_amp = max_amp;
  on_values_changed();
}

/**
 * Returns the maximum amplification allowed.
 */
INLINE float NormalizeDSP::
get_max_amp() const {
  return _max_amp;
}
