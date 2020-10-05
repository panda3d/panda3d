/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file limiterDSP.I
 * @author lachbr
 * @date 2020-10-04
 */

/**
 *
 */
INLINE LimiterDSP::
LimiterDSP(float release_time, float ceiling, float maximizer_gain,
           bool linked) :
  DSP(DSP::DT_limiter),
  _release_time(release_time),
  _ceiling(ceiling),
  _maximizer_gain(maximizer_gain),
  _linked(linked) {
}

/**
 * Sets the time to ramp the silence to full in milliseconds.
 */
INLINE void LimiterDSP::
set_release_time(float time) {
  _release_time = time;
}

/**
 * Returns the time to ramp the silence to full in milliseconds.
 */
INLINE float LimiterDSP::
get_release_time() const {
  return _release_time;
}

/**
 * Sets the maximum level of the output signal.
 */
INLINE void LimiterDSP::
set_ceiling(float ceiling) {
  _ceiling = ceiling;
}

/**
 * Returns the maximum level of the output signal.
 */
INLINE float LimiterDSP::
get_ceiling() const {
  return _ceiling;
}

/**
 * Sets the maximum amplification allowed in decibels.
 */
INLINE void LimiterDSP::
set_maximizer_gain(float gain) {
  _maximizer_gain = gain;
}

/**
 * Returns the maximum amplification allowed in decibels.
 */
INLINE float LimiterDSP::
get_maximizer_gain() const {
  return _maximizer_gain;
}

/**
 * Sets whether or not the processing mode is limited (false, limiter per channel)
 * or linked (true).
 */
INLINE void LimiterDSP::
set_linked(bool linked) {
  _linked = linked;
}

/**
 * Returns whether or not the processing mode is limited (false, limiter per channel)
 * or linked (true).
 */
INLINE bool LimiterDSP::
get_linked() const {
  return _linked;
}
