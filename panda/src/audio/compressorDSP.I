/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file compressorDSP.I
 * @author lachbr
 * @date 2020-10-04
 */

/**
 *
 */
INLINE CompressorDSP::
CompressorDSP(float threshold, float ratio, float attack, float release,
              float gainmakeup, bool linked) :
  DSP(DSP::DT_compressor),
  _threshold(threshold),
  _ratio(ratio),
  _attack(attack),
  _release(release),
  _gainmakeup(gainmakeup),
  _linked(linked)
{
}

/**
 * Sets the threshold level in Decibels.
 */
INLINE void CompressorDSP::
set_threshold(float threshold) {
  _threshold = threshold;
  on_values_changed();
}

/**
 * Returns the threshold level in Decibels.
 */
INLINE float CompressorDSP::
get_threshold() const {
  return _threshold;
}

/**
 * Sets the compression ratio.
 */
INLINE void CompressorDSP::
set_ratio(float ratio) {
  _ratio = ratio;
  on_values_changed();
}

/**
 * Returns the compression ratio.
 */
INLINE float CompressorDSP::
get_ratio() const {
  return _ratio;
}

/**
 * Sets the attack time.
 */
INLINE void CompressorDSP::
set_attack(float attack) {
  _attack = attack;
  on_values_changed();
}

/**
 * Returns the attack time.
 */
INLINE float CompressorDSP::
get_attack() const {
  return _attack;
}

/**
 * Sets the release time.
 */
INLINE void CompressorDSP::
set_release(float release) {
  _release = release;
  on_values_changed();
}

/**
 * Returns the release time.
 */
INLINE float CompressorDSP::
get_release() const {
  return _release;
}

/**
 *
 */
INLINE void CompressorDSP::
set_gainmakeup(float gainmakeup) {
  _gainmakeup = gainmakeup;
  on_values_changed();
}

/**
 *
 */
INLINE float CompressorDSP::
get_gainmakeup() const {
  return _gainmakeup;
}

/**
 * Sets whether or not this is an independent (false) or linked (true)
 * compressor.
 */
INLINE void CompressorDSP::
set_linked(bool linked) {
  _linked = linked;
  on_values_changed();
}

/**
 * Returns whether or not this is an independent (false) or linked (true)
 * compressor.
 */
INLINE bool CompressorDSP::
get_linked() const {
  return _linked;
}
